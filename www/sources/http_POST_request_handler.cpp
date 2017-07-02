#include "http_server_class_log.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_helper.h"
#include "http_server_class_status_codes.h"

/***************************************************************************
 * function 	  : store_POST_simple_request
 *
 * arguments 	  : int sockfd, unsigned int size, char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, string> map
 *
 * description 	: store POST request in a json file, if content type is 
 * 		            not multipart.
 *
 * return 	    : integer (status of the operation)
 * *************************************************************************/
int http_request_handler::store_POST_simple_request(int sockfd, unsigned int size, 
char *client_ip_address, unordered_map<string, string> config_map, 
unordered_map<string, string> map) {
	// pointer to store log string
	string log_data;

	// create a log class object
	log logger;

	// create a http_helper class object
	http_helper helper;
	
	// create a http_status_codes class object
	http_status_codes http_status;

	// initial buffer size
	int buf_size = 100;
	// allocate memory to store request
	char *buf = (char *)malloc(sizeof(char) * buf_size);
	if (NULL == buf){
		logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");
		free(buf);
		return 1;
	}
	// reset the buffer
	memset(buf, 0, buf_size);

	// read request data from socket and store it in buffer
	int i = 0;
	while (i < static_cast<int>(size)) {
		char ch;
		read(sockfd, &ch, 1);

		buf[i] = ch;
		i++;
		if (i >= buf_size) {
			buf_size = buf_size * 1.5;
			buf  = (char *)realloc(buf, buf_size);
			if (NULL == buf) {
				logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
				fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");
				free(buf);
				return 1;
			}
		}
	}
	// NULL terminate the buffer
	buf[i] = '\0';

	// create file path to store post request data in json format
	auto base_dir = config_map.find("BASE_DIR");
	if (base_dir == config_map.end()) {
		base_dir->second = "../../www";
	}
	char time_string[20];
	sprintf(time_string, "%lu", time(NULL));
  // create a string variable to store the file_path
	string file_path = base_dir->second + "/POST/data_" + string(time_string) + ".data";
	int len_file_path = file_path.size();
	char data_file_path[MAX_SIZE];
	memset (data_file_path, 0, MAX_SIZE);
	for (i = 0; i < len_file_path; i++) {
		data_file_path[i] = file_path[i];		
	}
	auto get_method_type = map.find("method");
	if (get_method_type == map.end()) {
		free(buf);	
		return 1;
	}
	const char *method_type = get_method_type->second.c_str();
	ofstream data_file (data_file_path);
	if (!data_file.is_open()) {
		// incase of error log it to log file
		log_data = "[Client " + string(client_ip_address) + "] [" + 
    string(method_type) + "] [ERROR IN CREATING FILE] " + data_file_path;
		logger.log_server_data(SEVERE, log_data);

		// log it to stderr
		fprintf(stderr, ANSI_COLOR_RED "[Client %s] [%s] ERROR IN CREATING FILE %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, data_file_path);

		data_file.close();
		// free all the resource which you have used
		free(buf);

		return 1;
	}
	// log file creation to log file
	log_data = "[Client " + string(client_ip_address) + "] [" + 
  string(method_type) + "] [FILE CREATED] " + data_file_path;
	logger.log_server_data(INFO, log_data);

	// log file creation to stdout
	fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] FILE CREATED %s"\
  ANSI_COLOR_RESET "\n", client_ip_address, method_type, data_file_path);

	// find number of entries
	//unsigned int no_entries = g_strv_length(data_entries);
	int buf_len = strlen (buf);
	for (i = 0; i < buf_len; i++) {
		if (i == 0) {
			char *first_token = strtok (buf, "&");
			data_file << first_token << endl;
			data_file.flush();
		}
		else {
			char *second_token = strtok (NULL, "&");
			data_file << second_token << endl;	
			data_file.flush();
		}
	
	}
	// send created json file to user
	char *header_200 = http_status.get_http_response_200(data_file_path);
  if (NULL == header_200) {
    // close file
    data_file.close();	
    // free all the resources
    free(buf);
    return 0;
  }
	int send_status = helper.send_http_response(sockfd, header_200, data_file_path);
	if (0 != send_status) {
		logger.log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
		fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client"\
    ANSI_COLOR_RESET "\n");
	}
	delete[] header_200;

	// close file
	data_file.close();	
	// free all the resources
	free(buf);
	return 0;
}

/******************************************************************************
 * function 	  : store_POST_multipart_request
 *
 * arguments 	  : int sockfd, unsigned int size, char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, string> map
 *
 * description 	: store the POST request data if content type is multipart
 *
 * return 	    : integer (status of the operation)
 * ****************************************************************************/
int http_request_handler::store_POST_multipart_request(int sockfd, unsigned int size, 
char *client_ip_address, unordered_map<string, string> config_map, 
unordered_map<string, string> map) {

	// to point to log data string
	string log_data;

	// create a log class object
	log logger;
	// create a http_status_codes class object
	http_status_codes http_status;
	// create a http_helper class object
	http_helper helper;

	// buffer index
	int i = 0;
	// number of crlf's
	int count = 0;
	// character to read from socket
	char ch = -1;
	// current and previous character read from socket
	char curr, prev;
	// sentinel to show we are in boundary section
	int boundary_start = 1;
	// sentinel to show we are in data section
	int data_start = 0;
	// sentinel to show we are in header section
	int header_start = 0;
	// initial buffer size to store data read from socket
	int buf_size = 100;

	// allocate buffer to store data read from socket
	char *buf = (char *)malloc(buf_size * sizeof(char));
	if (NULL == buf) {
		logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

		return 1;
	}
	// reset the buffer
	memset(buf, 0, buf_size);

	// read method type from hash table
	// it might be possible to hard code POST here but, for fexibility
	// we are reading from it from parsed data hash table
	auto get_method = map.find("method");
	if (get_method == map.end()) {
		free (buf);
		return 1;
	}
	char method_type[MAX_SIZE];
	memset(method_type, 0, MAX_SIZE);
	int len_method = get_method->second.size();
	for (i = 0; i < len_method; i++) {
		method_type[i] = get_method->second[i];
	}

	FILE *fp = NULL;

	// loop to read particular size of characters from socket
	unsigned int j;
	for (j = 0; j < size; j++) {
		// we are in boundary section
		if (boundary_start) {
			// read character from socket
			prev = ch;
			read(sockfd, &ch, 1);
			curr = ch;

			// check if previous character is '\r' and current character is '\n'
			// in this case we break out boundary section and enter in header section
			if ('\r' == prev && '\n' == curr) {
				header_start = 1;
				boundary_start = 0;
				count = 0;
				i = 0;
				continue;
			}

			// check if current character is '\r' if yes than
			// decision of breaking out of this section is depends on next character
			if ('\r' == curr) {
				continue;
			}
		} else if (header_start) {
			// we are in header section
			// read one character from socket
			prev = ch;
			read(sockfd, &ch, 1);
			curr = ch;

			// check if previous character is '\r' and current character '\n'
			// in this case we increase the count to check for two crlf's
			if ('\r' == prev && '\n' == curr) {
				count++;
			}

			// check if current character is not '\n' and '\r'
			// reset the count
			if ('\r' != curr && '\n' != curr) {
				count = 0;
			}

			// store the character in buffer
			buf[i] = curr;
			i++;
			// if buffer is smaller than required increase its size
			if (i >= buf_size) {
				buf_size = buf_size * 1.5;
				buf = (char *)realloc(buf, buf_size);
				if (NULL == buf) {
					logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");
					free(buf);
					return 1;
				}
			}

			// if we encountered two crlf's than
			// process data in buffer
			if (2 == count) {
				// NULL terminate the buffer
				buf[i] = '\0';
				// divide buffer at crlf
				char **lines = helper.split_string(buf, "\r\n", -1);
				// find the number of lines
				unsigned int len = strlen(lines[0]);
				// sentinel to show start of filename
				int filename_start = 0;

				// allocate memory to store filename or key of key value pair
				char *name = (char *)malloc(200);
				if (NULL == name) {
					free(buf);
					
					logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

					return 4;
				}
				// reset the buffer
				memset(name, 0, 200);
				// index for name
				int l = 0;

				// loop to read filename or key name
				unsigned int k;
				for (k = 0; k < len; k++) {
					if (!filename_start && lines[0][k] == '"') {
						filename_start = 1;
						continue;
					} else if (filename_start && lines[0][k] == '"') {
						filename_start = 0;
						name[l] = '\0';
						l = 0;
					}

					if (filename_start) {
						name[l] = lines[0][k];
						l++;
					}
				}
				// read base dir from config hash
				auto base_directory = config_map.find("BASE_DIR");
				// if base dir is not present than take default value of 'www'
				if (base_directory == config_map.end()) {
					base_directory->second = "../../www";
				}
				char base_dir[MAX_SIZE];
				memset(base_dir, 0, MAX_SIZE);
				int len_base_dir = base_directory->second.size();
				for (i = 0; i < len_base_dir; i++) {
					base_dir[i] = base_directory->second[i];
				}
				
				auto get_url = map.find("url");
				if (get_url == map.end()) {
					free (buf);
					return 1;
				}
				char url[MAX_SIZE];
				memset(url, 0, MAX_SIZE);
				int len_url = get_url->second.size();
				for (i = 0; i < len_url; i++) {
					url[i] = get_url->second[i];
				}
				// create file path to store data coming from socket if its file not key
				char *file_name = NULL;
        try { file_name = new char[MAX_SIZE]; }
        catch(bad_alloc& bd) {
          free(buf);
					free(name);
					helper.freev(lines);
					return 1;
        }
				memset(file_name, 0, MAX_SIZE);
				strcat(file_name, base_dir);
				strcat(file_name, "/");
				strcat(file_name, name);

				// check if filename is present or not
				char *is_name_present = strstr(lines[0], "filename=");
				// if its file
				if (NULL != is_name_present) {
					// log data to log file
					log_data = "[Client " + string(client_ip_address) + "] [" + 
          string(method_type) + "] [NEW RESOURCE CREATED] " + string(file_name);
					logger.log_server_data(INFO, log_data);
					
					
					// log data to stdout
					fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [NEW RESOURCE CREATED] %s"\
          ANSI_COLOR_RESET "\n", client_ip_address, method_type, file_name);

					fp = fopen(file_name, "w");
					if (NULL == fp) {
						// if error in creating file
						// log it to log file
						log_data = "[Client " + string(client_ip_address) + "] [" + 
            string(method_type) + "] [ERROR IN CREATING FILE] " + string(file_name);
						logger.log_server_data (INFO, log_data);
						
						// log to stderr
						fprintf(stderr, ANSI_COLOR_RED "[Client %s] [%s] [ERROR IN CREATING FILE] %s"\
            ANSI_COLOR_RESET "\n", client_ip_address, method_type, file_name);

						// free all resource used
						free(buf);
						delete[] file_name;
						free(name);
						helper.freev(lines);
						return 1;
					}
					
					auto get_host = map.find("Host");
					if (get_host == map.end()) {
						get_host->second = "tsus007.gur.aricent.com:5000";
					}
					char host[MAX_SIZE];
					memset(host, 0, MAX_SIZE);
					int len_host = get_host->second.size();
          int j = 0;
					for (i = 1, j = 0; i < len_host; i++, j++) {
						host[j] = get_host->second[i];
					}
					// send http response of 201 to user that resource created successfully
					char *header_201 = http_status.get_http_response_201(name, host);
          if (NULL == header_201) {
            free(buf);
						delete[] file_name;
						free(name);
						helper.freev(lines);
						return 1;
          }

					int send_status = helper.send_http_response(sockfd, header_201, NULL);
					if (0 != send_status) {
						logger.log_server_data(SEVERE, "Error in sending HTTP 201 response to Client");
						fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 201 response to Client"\
            ANSI_COLOR_RESET "\n");
					}
					delete[] header_201;

				} else {
					// if data present is key value pair than create file path to store that data
					char *post_data_path = NULL;
          try { post_data_path = new char[MAX_SIZE]; }
          catch(bad_alloc& bd) {
            free(buf);
						delete[] file_name;
						free(name);
						helper.freev(lines);
						return 1; 
          }      
          memset(post_data_path, 0, MAX_SIZE);
					strcat(post_data_path, base_dir);
					strcat(post_data_path, "/POST/multipart_post_data.txt");

					fp = fopen(post_data_path, "a");
					if (NULL == fp) {
						// in case of error log it to log file
						log_data = "[Client " + string(client_ip_address) + "] [" + 
            string(method_type) + "] [ERROR IN CREATING FILE] " + string(post_data_path);
						logger.log_server_data (INFO, log_data);
						
						// log it to stderr
						fprintf(stderr, ANSI_COLOR_RED "[Client %s] [%s] [ERROR IN CREATING FILE] %s"\
            ANSI_COLOR_RESET "\n", client_ip_address, method_type, post_data_path);

						// free all the resources used
						free(buf);
						delete[] file_name;
						delete[] post_data_path;
						free(name);
						helper.freev(lines);
						return 1;
					}
					// if file create successfully than log it
					log_data = "[Client " + string(client_ip_address) + "] [" + 
          string(method_type) + "] [FILE CREATED] " + string(post_data_path);
					logger.log_server_data(INFO, log_data);
					
					// log it to stdout
					fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [FILE CREATED] %s"\
          ANSI_COLOR_RESET "\n", client_ip_address, method_type, post_data_path);

					// free path you have created to store data
					fprintf(fp, "\n%s: ", name);
				}
				// reset the buffer
				// for next iteration
				memset(buf, 0, buf_size);
				// break out from header section and move to data section
				header_start = 0;
				data_start = 1;

				// free resources
				helper.freev(lines);
				free(name);
				delete[] file_name;
			}
		} else if (data_start) {
			// we are in data section
			// read character from socket
			prev = ch;
			read(sockfd, &ch, 1);
			curr = ch;

			// check if current character is '\n' and previous character is '\r'
			// than start boundary section and end data section
			if ('\r' == prev && '\n' == curr) {
				boundary_start = 1;
				data_start = 0;
				int is_closed = fclose(fp);
				if (0 != is_closed) {
					// if file not closed properly
					// log it to log file
					log_data = "[Client " + string(client_ip_address) + "] [" + 
          string(method_type) + "] [ERROR IN CLOSING FILE]";
					logger.log_server_data(WARNING, log_data);
					// log it to stderr
					fprintf(stderr, ANSI_COLOR_MAGENTA "[Client %s] [%s] [ERROR IN CLOSING FILE]"\
          ANSI_COLOR_RESET "\n", client_ip_address, method_type);
				}
				continue;
			}
			// if current character is '\r'
			// than dont process it wait for next character
			if ('\r' == curr) {
				continue;
			}

			// if previous character is '\r' and current character is not '\n'
			// than write both characters in file
			if ('\r' == prev && '\n' != curr) {
				fprintf(fp, "%c%c", prev, curr);
				fflush(fp);
			} else {
				fprintf(fp, "%c", curr);
				fflush(fp);
			}
		}
	} 
	// free all the resources
	free(buf);
	return 0;
}

/****************************************************************************
 * function 	  : handle_POST_request
 *
 * arguments 	  : int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> map, char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map
 *
 * description 	: handle HTTP POST request
 *
 * return 	    : integer (status of the operation)
 * ***************************************************************************/
int http_request_handler::handle_POST_request(int sockfd, fd_set *master, 
unordered_map<string, string> map, char *client_ip_address, 
unordered_map<string, string> config_map, 
unordered_map<string, REDIRECT*> redirect_map) {
	// create a http_helper class object
	http_helper helper;

	int content_length_int = 0, i = 0;
	// lookup content length from hashtable containg parsed data
	auto content_length_str = map.find("Content-Length");
	if (content_length_str == map.end()) {
		content_length_int = 0;	
	}
	else {
		char content_length[MAX_SIZE];
		memset(content_length, 0, MAX_SIZE);
		int len_str = content_length_str->second.size();
		for (i = 0; i < len_str; i++) {
			content_length[i] = content_length_str->second[i];
		}
		// if content length is a valid string than convert it to
		// integer
		content_length_int = helper.str_to_int(content_length);
		// if error in conversion than take default value of 0
		if (-1 == content_length_int) {
			content_length_int = 0;
		}

	}
	auto get_url = map.find("url");
	if (get_url == map.end()) {
		return 1;
	}
	char url[MAX_SIZE];
	memset(url, 0, MAX_SIZE);
	int len_url = get_url->second.size();
	for (i = 0; i < len_url; i++) {
		url[i] = get_url->second[i];
	}
	// check if requested url is valid or not, if valid than return that valid string
	// otherwise NULL
	char *filename = helper.get_valid_file_path(url, sockfd, master, 
  client_ip_address, config_map, redirect_map, map);
	if (NULL == filename) {
		// incase of invalid file path remove socket from list of our watching sockets
		FD_CLR(sockfd, &(*master));
		return 1;
	}
	free(filename);

	// lookup content type of POST request
	auto get_content_type = map.find("Content-Type");
	if (get_content_type == map.end()) {
		return 1;
	}
	char content_type[MAX_SIZE];
	memset(content_type, 0, MAX_SIZE);
	int len_content_type = get_content_type->second.size();
	for (i = 0; i < len_content_type; i++) {
		content_type[i] = get_content_type->second[i];
	}
	char *get_type = strtok (content_type, ";");
	// if content type is not multipart
	if (0 != strncmp(get_type, " multipart/form-data", strlen(" multipart/form-data"))) {
		int store_status = store_POST_simple_request(sockfd, 
    content_length_int, client_ip_address, config_map, map);
		if (0 != store_status) {
			return 1;
		}
	} else {
		// if content type is multipart
		int store_status = store_POST_multipart_request(sockfd, content_length_int, 
    client_ip_address, config_map, map);
		if (0 != store_status) {
			return 1;
		}
	} 
	return 0;
}

