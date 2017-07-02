#include "http_server_class_log.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_helper.h"
#include "http_server_class_status_codes.h"

/****************************************************************************
 * function 	  : store_PUT_simple_request
 *
 * arguments 	  : int sockfd, unsigned int size, char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, string> map
 *
 * description 	: stores form/x-www-urlencoded data into a data file.
 *
 * return 	    : integer
 * *************************************************************************/
int http_request_handler::store_PUT_simple_request(int sockfd, unsigned int size, 
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

	string file_path = base_dir->second + "/PUT/data_" + string(time_string) + ".data";
	int len_file_path = file_path.size();
	char data_file_path[MAX_SIZE];
	memset(data_file_path, 0, MAX_SIZE);
	for (i = 0; i < len_file_path; i++) {
		data_file_path[i] = file_path[i];		
	}
  // get the method type from map
	auto get_method_type = map.find("method");
	if (get_method_type == map.end()) {
		return 1;
	}
	const char *method_type = get_method_type->second.c_str();
	ofstream data_file (data_file_path);
	if (!data_file.is_open()) {
		// incase of error log it to log file
		log_data = "[client " + string(client_ip_address) + "] [" + string(method_type) + 
    "] [ERROR IN CREATING FILE] " + data_file_path;
		logger.log_server_data(SEVERE, log_data);

		// log it to stderr
		fprintf(stderr, ANSI_COLOR_RED "[client %s] [%s] ERROR IN CREATING FILE %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, data_file_path);

		// free all the resource which you have used
		free(buf);
		data_file.close();
		return 1;
	}
	// log file creation to log file
	log_data = "[client " + string(client_ip_address) + "] [" + string(method_type) + 
  "] [FILE CREATED] " + data_file_path;
	logger.log_server_data(INFO, log_data);

	// log file creation to stdout
	fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] FILE CREATED %s"\
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

/***************************************************************************
 * function 	    : store_PUT_request
 *
 * arguments 	    : int sockfd, int size, char *client_ip_address, 
 * 		              unordered_map<string, string> config_map, 
 * 		              unordered_map<string, string> map
 *
 * description 	  : this function parses the incoming data from client like
 * 		              images or files and stores them into the given directory
 * 		              as requested by the client. If the resource is already
 * 		              existing then it updates the current resource with the
 * 		              new incoming request or else it stores the new resource.
 * 		              It can also create directories if there are a set of
 * 		              directories inside which the resource is to be stored.
 *
 * return 	      : integer
 * **************************************************************************/
int http_request_handler::store_PUT_multipart_request(int sockfd, int size, 
char *client_ip_address, unordered_map<string, string> config_map, 
unordered_map<string, string> map)
{
	// to point to log data string
	string log_data;
	// create a log class object
	log logger;
	// create a http_status_codes class object
	http_status_codes http_status;
	// create a http_helper class object
	http_helper helper;

	// variables to parse the user data
	int j;
	int i = 0;
	int count = 0;
	char ch = -1;
	char curr, prev;
	int boundary_start = 1;
	int data_start = 0;
	int header_start = 0;
	int buf_size = 100;
	// initialize the buffer to store the client data
	char *buf = (char *)malloc(buf_size);
	if (NULL == buf) {
		logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

		return 1;
	}
	// clear the buffer to 0
	memset(buf, 0, buf_size);
	// create a FILE pointer
	FILE *fp = NULL;
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

	// start parsing the incoming data
	for (j = 0; j < size; j++) {
		if (boundary_start) {
			prev = ch;
			// read from the client
			read(sockfd, &ch, 1);
			curr = ch;
			// if the current and previous character is CRLF
			if ('\r' == prev && '\n' == curr) {
				//assign value to header_start as 1
				header_start = 1;
				boundary_start = 0;
				count = 0;
				i = 0;
				continue;
			}
			// if the current character is '\r'
			if ('\r' == curr) {
				continue;
			}
		} else if (header_start) {
			prev = ch;
			read(sockfd, &ch, 1);
			curr = ch;
			// if the characters is CRLF
			if ('\r' == prev && '\n' == curr) {
				count++;
			}
			// if the characters is not CRLF
			if ('\n' != curr && '\r' != curr) {
				count = 0;
			}
			// assign the current character to buffer
			buf[i] = curr;
			i++;
			// if the buffer is already full then reallocated
			if (i >= buf_size) {
				buf_size = buf_size * 1.5;
				// reallocate the buf_size again
				buf = (char *)realloc(buf, buf_size);
				if (NULL == buf) {
					logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull"\
          ANSI_COLOR_RESET "\n");
					free(buf);
					return 1;
				}
			}
			// if the count is equal to 2
			if (2 == count) {
				buf[i] = '\0';
				// split the complete request at '\r\n'
				char **lines = helper.split_string(buf, "\r\n", -1);
				//calculate the length of the data received
				unsigned int len = strlen(lines[0]);
				unsigned int k;
				int filename_start = 0;
				// allocated memort to hold the name
				char *name = (char *)malloc(200);
				if (NULL == name) {
					free(buf);
					helper.freev(lines);
					logger.log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull"\
          ANSI_COLOR_RESET "\n");
					return 1;
				}
				// set the variable to zero
				memset(name, 0, 200);
				int l = 0;

				for (k = 0; k < len; k++) {
					// check for character '"'
					if (!filename_start && lines[0][k] == '"') {
						filename_start = 1;
						continue;
					} else if (filename_start && lines[0][k] == '"') {
						filename_start = 0;
						name[l] = '\0';
						l = 0;
					}
					// if the filename_start is valid
					if (filename_start) {
						name[l] = lines[0][k];
						l++;
					}
				}

				// stat buffer to check for access rights
				struct stat access;
				//check the filename
				char *is_name_present = strstr(lines[0], "filename=");
			
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
        // get the url from the map
				auto get_url = map.find("url");
				if (get_url == map.end()) {
					free (buf);
					return 1;
				}
				char complete_string[MAX_SIZE];
				memset(complete_string, 0, MAX_SIZE);
				int len_url = get_url->second.size();
        				for (i = 0; i < len_url; i++) {
					complete_string[i] = get_url->second[i];
				}
        if (string::npos != get_url->second.find(".")) {
            auto loc = get_url->second.rfind("/");
            get_url->second.erase(loc);
        }
        char make_dir_url[MAX_SIZE];
        memset(make_dir_url, 0, MAX_SIZE);
        int url_len = get_url->second.size();
        for (i = 0; i < url_len; i++) {
          make_dir_url[i] = get_url->second[i];
        }

        if (make_dir_url[url_len] != '/') {
            make_dir_url[url_len+1] = '/';
        }

				// generate the complete file path for the requested URL
				char *file_name = NULL;
        try { file_name = new char[MAX_SIZE]; }
        catch(bad_alloc&bd) {
            free(buf);
            return 1;
        }
				memset(file_name, 0, MAX_SIZE);
				strcat(file_name, base_dir);
				strcat(file_name, complete_string);
				
				// variables to get the directory structure
				char *reverse = helper.string_reverse(file_name);
				char **splitted = helper.split_string(reverse, "/", 2);
				char *create_dir = helper.string_reverse(splitted[1]);
				// No dots are present then come here
				if (NULL == strstr(splitted[0], "."))
				{
					// create a new directory structure using system command
					int slash_flag = 0;
					char *check_slash = helper.string_reverse(file_name);
					int len = strlen (check_slash);
					if (check_slash[len-1] == '/') {
						slash_flag = 1;
					}

					int len_file_name = strlen(file_name);
					if (file_name[len_file_name-1] != '/') {
						memset(file_name, 0, MAX_SIZE);
						strcat(file_name, base_dir);
						strcat(file_name, complete_string);
						strcat(file_name, "/");
					}
				
					char *directory_with_file = NULL;
          try { directory_with_file = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) {
            helper.freev(lines);
						free(buf);
						free(name);
						helper.freev(splitted);
						delete[] file_name;
						return 1;
          }
					memset(directory_with_file, 0, MAX_SIZE);
					strcat(directory_with_file, file_name);
					strcat(directory_with_file, name);

					FILE *check_exists = fopen(directory_with_file, "r");
					if (NULL != check_exists) {
						// log to log file
						log_data = "[client " + string(client_ip_address) + "] [" + string(method_type) + 
								"] [UPDATING EXISTING RESOURCE] " + string(directory_with_file);
						logger.log_server_data(SEVERE, log_data);
						
						fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [UPDATING EXISTING RESOURCE] %s"\
            ANSI_COLOR_RESET "\n", 
						client_ip_address, method_type, directory_with_file);

						fclose(check_exists);
					}
					delete[] directory_with_file;

					char *make_dir = NULL;
          try { make_dir = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) {
            helper.freev(lines);
						free(buf);
						free(name);
						helper.freev(splitted);
						delete[] file_name;
						return 1;
          }
					memset(make_dir, 0, MAX_SIZE);
					strcat(make_dir, "mkdir -p ");
          strcat(make_dir, base_dir);
					//strcat (make_dir, helper.string_reverse(helper.string_reverse(file_name)));
					strcat(make_dir, make_dir_url);

					// create directory
					int a = system(make_dir);
					// if the command was not executed
					if (-1 == a)
					{
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						logger.log_server_data (SEVERE, "Problem Occured in creating the Base Directory");
						fprintf (stderr, ANSI_COLOR_RED "Problem Occured in creating the Base Directory"\
            ANSI_COLOR_RESET "\n");
						return 1;
					}

					// create the file inside the directory using touch
					char *move = NULL;
          try { move = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) {
            helper.freev(lines);
            free(buf);
            free(name);
            delete[] make_dir;
            helper.freev(splitted);
            delete[] file_name;
            logger.log_server_data (SEVERE, "Problem Occured in creating the file");
            fprintf (stderr, ANSI_COLOR_RED "Problem Occured in creating the file"\
            ANSI_COLOR_RESET "\n");
            return 1;
            }

					memset(move, 0, MAX_SIZE);
					strcat(move, "touch ");
          strcat(move, base_dir);
					strcat(move, make_dir_url);
          strcat(move, "/");
					strcat(move, name);
	
					// create file inside directory
					int c = system(move);
					// if the command was not executed
					if (-1 == c)
					{	
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] move;
						delete[] file_name;
						logger.log_server_data (SEVERE, "Problem Occured in creating the file");
						fprintf (stderr, ANSI_COLOR_RED "Problem Occured in creating the file"\
            ANSI_COLOR_RESET "\n");
						return 1;
					}
					// updating the file path 
					memset(file_name, 0, MAX_SIZE);
					strcat(file_name, base_dir);
					//strcat (file_name, complete_string);
					strcat(file_name, make_dir_url);
					strcat(file_name, "/");
					strcat(file_name, name);

					// open filename
					fp = fopen(file_name, "w");
					if (NULL == fp) {
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						delete[] move;
						return 1;
					}
					stat(file_name,&access);
					char page_permission = (access.st_mode & S_IROTH) ? 'r':'-';
					if (page_permission != 'r')
					{
						logger.log_server_data(SEVERE, "Access Forbidden to the Resource");
						fprintf(stdout, ANSI_COLOR_YELLOW "Access Forbidden to the Resource"\
            ANSI_COLOR_RESET "\n");

						// return 403 Forbidden error 
						char *file_path = NULL;
            try { file_path = new char[MAX_SIZE]; }
            catch(bad_alloc&bd) { 
              helper.freev(lines);
              free(buf);
              free(name);
              delete[] make_dir;
              helper.freev(splitted);
              delete[] file_name;
              delete[] move;
              fclose(fp);
              return 1;
            }
						memset(file_path, 0, MAX_SIZE);
						strcat(file_path, base_dir);
						strcat(file_path, "/error/403.html");
						char *response = http_status.get_http_response_403(file_path);
            if (NULL == response) {
              // clear all the allocated variable	
              helper.freev(lines);
              free(buf);
              free(name);
              delete[] make_dir;
              helper.freev(splitted);
              delete[] file_name;
              delete[] file_path;
              delete[] move;
              fclose(fp);
              return 1;
            }
						// write to client the error response
						helper.send_http_response(sockfd, response, file_path);
						// clear all the allocated variable	
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						delete[] file_path;
						delete[] move;
						fclose(fp);
						return 1;

					} 
   	
					char *new_path = NULL;
          try { new_path = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) { 
            helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						delete[] move;
						fclose(fp);
						return 1;
          }
					memset(new_path, 0, MAX_SIZE);
					strcat(new_path, complete_string+1);
					strcat(new_path, "/");
					strcat(new_path, name);
					//send a 201 created response to client
					auto get_host = map.find("Host");
					if (get_host == map.end()) {
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						delete[] new_path;
						delete[] move;
						fclose(fp);
						return 1;
					}
					char host[MAX_SIZE];
					int j = 0;
					memset(host, 0, MAX_SIZE);
					int len_host = get_host->second.size();
					for (i = 1, j = 0; i < len_host; i++, j++) {
						host[j] = get_host->second[i];
					}

			 
					char *header_201 = http_status.get_http_response_201(new_path, host);
          if (NULL == header_201) {
            // free all the allocated memory
            delete[] header_201;
            delete[] new_path;
            delete[] move;
            delete[] make_dir;
            return 1;
          }
					// send status to client
					int send_status = helper.send_http_response(sockfd, header_201, NULL);
					// if the response was not sent
					if (0 != send_status)
					{
						// log server data
						logger.log_server_data (SEVERE, "Error in sending HTTP 201 data to Client");
						// write to server console
						fprintf (stderr, ANSI_COLOR_RED "Error in sending HTTP 201 data to Client"\
            ANSI_COLOR_RESET "\n");
					}
					// free all the allocated memory
					delete[] header_201;
					delete[] new_path;
					delete[] move;
					delete[] make_dir;
				}
				else
				{
					FILE *check_exists = fopen(helper.string_reverse(file_name), "r");
					if (NULL != check_exists) {
						// log to log file
						log_data = "[client " + string(client_ip_address) + "] [" + string(method_type) + 
								"] [UPDATING EXISTING RESOURCE] " + string(file_name);
						logger.log_server_data(SEVERE, log_data);
						
						fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [UPDATING EXISTING RESOURCE] %s"\
            ANSI_COLOR_RESET "\n", 
						client_ip_address, method_type, file_name);

						fclose(check_exists);
					}

         	// create a new directory structure using system command
					char *make_dir = NULL;
          try { make_dir = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) { 
            helper.freev(lines);
						free(buf);
						free(name);
						helper.freev(splitted);
						delete[] file_name;
						fclose(check_exists);
						return 1;
          }
					memset(make_dir, 0, MAX_SIZE);
					strcat(make_dir, "mkdir -p ");
					strcat(make_dir, base_dir);
					strcat(make_dir, "/");
					//strcat (make_dir, create_dir);
					strcat(make_dir, make_dir_url);
				
					// create directory
					int a = system(make_dir);
					// if the command was not executed
					if (-1 == a)
					{
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						logger.log_server_data(SEVERE, "Problem Occured in creating the Base Directory");
						fprintf(stderr, ANSI_COLOR_RED "Problem Occured in creating the Base Directory"\
            ANSI_COLOR_RESET "\n");
						fclose(check_exists);
						return 1;
					}

					log_data = "Base Directory Created " + string(create_dir); 
					logger.log_server_data (INFO, log_data);
					fprintf (stdout, ANSI_COLOR_YELLOW "Base Directory Created %s"\
          ANSI_COLOR_RESET "\n", create_dir);

					// retreive the file name to be created
					char *create_file = file_name;
					// create the file inside the directory using touch
					char *move = NULL;
          try { move = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) { 
            helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] file_name;
						fclose(check_exists);
						return 1;
          }
					memset(move, 0, MAX_SIZE);
					strcat(move, "touch ");
					strcat(move, create_file);
					// create file inside directory
					int c = system(move);
					// if the command was not executed
					if (-1 == c)
					{
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] move;
						delete[] file_name;
						logger.log_server_data(SEVERE, "Problem Occured in creating the file");
						fprintf(stderr, ANSI_COLOR_RED "Problem Occured in creating the file"\
            ANSI_COLOR_RESET "\n");
						fclose(check_exists);
						return 1;
					}
					// log data
					log_data = "Resource Created " + string(create_file); 
					logger.log_server_data (INFO, log_data);
					fprintf (stdout, ANSI_COLOR_YELLOW "Resource Created %s"\
          ANSI_COLOR_RESET "\n", create_file);

					// open filename
					fp = fopen(file_name, "w");
					if (NULL == fp) {
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] move;
						delete[] file_name;
						fclose(check_exists);
						fclose (fp);
						return 1;
					}
					stat(file_name,&access);
					char page_permission = (access.st_mode & S_IROTH) ? 'r':'-';
					if (page_permission != 'r')
					{
						logger.log_server_data(SEVERE, "Access Forbidden to the Resource");
						fprintf(stdout, ANSI_COLOR_YELLOW "Access Forbidden to the Resource"\
            ANSI_COLOR_RESET "\n");

						// return 403 Forbidden error 
						char *file_path_1 = NULL;
            try { file_path_1 = new char[MAX_SIZE]; }
            catch(bad_alloc&bd) { 
              helper.freev(lines);
              free(buf);
              free(name);
              delete[] make_dir;
              helper.freev(splitted);
              delete[] move;
              delete[] file_name;
              fclose(check_exists);
              fclose(fp);
              return 1; 
            }
						memset(file_path_1, 0, MAX_SIZE);
						strcat(file_path_1, base_dir);
						strcat(file_path_1, "/error/403.html");
						char *response_1 = http_status.get_http_response_403(file_path_1);
            if (NULL == response_1) {
              //clear all the allocated variable	
              helper.freev(lines);
              free(buf);
              free(name);
              delete[] make_dir;
              helper.freev(splitted);
              delete[] move;
              delete[] file_name;
              delete[] file_path_1;
              fclose(check_exists);
              fclose(fp);
              return 1;
            }
						//write to client the error response
						helper.send_http_response(sockfd, response_1, file_path_1);
						//clear all the allocated variable	
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] make_dir;
						helper.freev(splitted);
						delete[] move;
						delete[] file_name;
						delete[] file_path_1;
						delete[] response_1;
						fclose(check_exists);
						fclose(fp);
						return 1;

					} 
					// send a 201 created response to client
					auto get_host = map.find("Host");
					if (get_host == map.end()) {
						return 1;
					}
					int j = 0;
					char host[MAX_SIZE];
					memset(host, 0, MAX_SIZE);
					int len_host = get_host->second.size();
					for (i = 1, j = 0; i < len_host; i++, j++) {
						host[j] = get_host->second[i];
					}
					char *header_201 = http_status.get_http_response_201(complete_string+1, host);
          if (NULL == header_201) {
            delete[] move;
            delete[] make_dir;
            return 1;
          }
					// send status to client
					int send_status = helper.send_http_response(sockfd, header_201, NULL);
					// if the response was not sent
					if (0 != send_status)
					{
						// log server data
						logger.log_server_data (SEVERE, "Error in sending HTTP 201 data to Client");
						// write to server console
						fprintf (stderr, ANSI_COLOR_RED "Error in sending HTTP 201 data to Client"\
            ANSI_COLOR_RESET "\n");
					}
					// free all the allocated variable
					delete[] header_201;
					delete[] move;
					delete[] make_dir;
				}
				// free all the allocated memory
				helper.freev(splitted);
				// continue if 'is_name_present' is not NULL
				if (NULL == is_name_present) {
          char *put_data_path = NULL;
          try { put_data_path = new char[MAX_SIZE]; }
          catch(bad_alloc&bd) { 
            helper.freev(lines);
						free(buf);
						free(name);
						delete[] file_name;
						fclose (fp);
            return 1;
          } 
					memset(put_data_path, 0, MAX_SIZE);
					strcat(put_data_path, base_dir);
					strcat(put_data_path, "put_data.txt");
					// open the file
					if (NULL != fp) {
						fclose(fp);
					}
					fp = fopen(put_data_path, "a");
					if (NULL == fp) {
						helper.freev(lines);
						free(buf);
						free(name);
						delete[] put_data_path;
						delete[] file_name;
						fclose (fp);
						return 1;
					}
					// write to the file
					fprintf(fp, "\n%s: ", name);
					// free the allocated variable
					delete[] put_data_path;
				}
				// clear the buf_size
				memset(buf, 0, buf_size);
				// reinitialize the variables
				header_start = 0;
				data_start = 1;
				// free the allocated variable
				helper.freev(lines);
				free(name);
				delete[] file_name;
			} 
			// else start parsing the data
		} else if (data_start) {

			prev = ch;
			// read data from the socket
			read(sockfd, &ch, 1);
			curr = ch;

			if ('\r' == prev && '\n' == curr) {
				boundary_start = 1;
				data_start = 0;
				// close file
				fclose(fp);
				continue;
			}
			// if the current character read is '\r'
			if ('\r' == curr) {
				continue;
			} 
			// if the previous character is '\r' and the next character is not '\n'
			if ('\r' == prev && '\n' != curr) {
				fprintf(fp, "%c%c", prev, curr);
				// clear the file variable
				fflush(fp);
			} else {
				// write to file
				fprintf(fp, "%c", curr);
				fflush(fp);
			} 
		} 
	}

	// free the allocated variable
	free (buf);
	return 0;
}

/******************************************************************************
 * function 	  : handle_PUT_request
 *
 * arguments 	  : int sockfd, fd_set *master, unordered_map<string, string> map, 
 * 		            char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map 
 *
 * description 	: this function handles the incoming PUT request from
 * 		            the client. Data handled can be both simple data and
 *		            mutilpart data.
 *
 * return 	    : integer
 * ****************************************************************************/
int http_request_handler::handle_PUT_request(int sockfd, fd_set *master, 
unordered_map<string, string> map, char *client_ip_address, 
unordered_map<string, string> config_map, unordered_map<string, REDIRECT*> redirect_map) {
	//create a http_helper class object
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
  // get the url from the map
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
		int store_status = store_PUT_simple_request(sockfd, content_length_int, 
    client_ip_address, config_map, map);
		if (0 != store_status) {
			return 1;
		}
	} else {
		// if content type is multipart
		int store_status = store_PUT_multipart_request(sockfd, content_length_int, 
    client_ip_address, config_map, map);
		if (0 != store_status) {
			return 1;
		}
	} 
return 0;
}

