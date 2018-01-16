#include "http_PUT_request_handler.h"

/**********************************************************************
 * function 	: store_PUT_simple_request
 *
 * arguments 	: int sockfd, unsigned int size, char *client_ip_address
 * 		  GHashTable **config_hash, GHashTable **hash
 *
 * description 	: stores form/x-www-urlencoded data into a JSON file.
 *
 * return 	: integer
 * *******************************************************************/
int store_PUT_simple_request(int sockfd, unsigned int size, char *client_ip_address, GHashTable **config_hash, GHashTable **hash) {
	// pointer to store log string
	char *log_data = NULL;

	// initial buffer size
	unsigned int buf_size = 100;
	// allocate memory to store request
	char *buf = (char *)malloc(sizeof(char) * buf_size);
	if (NULL == buf) {
		log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

		return 1;
	}
	// reset the buffer
	memset(buf, 0, buf_size);

	// read request data from socket and store it in buffer
	unsigned int i = 0;
	while (i < size) {
		char ch;
		read(sockfd, &ch, 1);

		buf[i] = ch;
		i++;
		if (i >= buf_size) {
			buf_size = buf_size * 1.5;
			buf  = (char *)realloc(buf, buf_size);
			if (NULL == buf) {
				log_server_data(SEVERE, "Memory Allocation unsuccessful");
				fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

				return 1;
			}
		}
	}
	// NULL terminate the buffer
	buf[i] = '\0';

	// create file path to store post request data in json format
	char *base_dir = g_hash_table_lookup(*config_hash, "BASE_DIR");
	char time_string[20];
	sprintf(time_string, "%lu", time(NULL));

	char *json_file_path = g_strconcat(base_dir, "/PUT/data_", time_string, ".json", NULL);
	char *method_type = g_ascii_strup(g_hash_table_lookup(*hash, "method"), -1);

	FILE *fp = fopen(json_file_path, "w");
	if (NULL == fp) {
		// incase of error log it to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [ERROR IN CREATING FILE] ", json_file_path, NULL);
		log_server_data(SEVERE, log_data);
		g_free(log_data);
		log_data = NULL;

		// log it to stderr
		fprintf(stderr, ANSI_COLOR_RED "[client %s] [%s] ERROR IN CREATING FILE %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, json_file_path);

		// free all the resource which you have used
		free(buf);
		g_free(json_file_path);
		g_free(method_type);

		return 1;
	}
	// log file creation to log file
	log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [FILE CREATED] ", json_file_path, NULL);
	log_server_data(INFO, log_data);
	g_free(log_data);
	log_data = NULL;
	// log file creation to stdout
	fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] FILE CREATED %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, json_file_path);

	// split data into inidividual entries
	// individual entries are separated by '&'
	char **data_entries = g_strsplit_set(buf, "&", -1);
	// find number of entries
	unsigned int no_entries = g_strv_length(data_entries);

	// write data into file into json format
	fprintf(fp, "{\n");
	for (i = 0; i < no_entries; i++) {
		// split individual entry at '=' into key and value
		char **entry = g_strsplit_set(data_entries[i], "=", 2);
		// unescape the value part it may contain escaped data
		char *unescaped_value = g_uri_unescape_string(entry[1], "");
		fprintf(fp, "\t\"%s\": \"%s\"", entry[0], unescaped_value);

		if (i == no_entries-1) {
			fprintf(fp, "\n");
		} else {
			fprintf(fp, ",\n");
		}

		g_strfreev(entry);
		g_free(unescaped_value);
	}
	fprintf(fp, "}\n");
	fflush(fp);

	// send created json file to user
	char *header_200 = get_http_response_200(json_file_path);
	int send_status = send_http_response(sockfd, header_200, json_file_path);
	if (0 != send_status) {
		log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
		fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client" ANSI_COLOR_RESET "\n");
	}
	g_free(header_200);

	// close file
	int is_close = fclose(fp);
	if (0 != is_close) {
		// if file not closed properly log this to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [FILE NOT CLOSED PROPERLY] ", json_file_path, NULL);
		log_server_data(WARNING, log_data);
		g_free(log_data);
		log_data = NULL;
		// if file not closed properly log this to stderr
		fprintf(stderr, ANSI_COLOR_MAGENTA "[client %s] [%s] [FILE NOT CLOSED PROPERLY] %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, json_file_path);
	}

	// free all the resources
	g_free(json_file_path);
	g_strfreev(data_entries);
	free(buf);
	g_free(method_type);

	return 0;
}

/***************************************************************************
 * function 	: store_PUT_request
 *
 * arguments 	: int sockfd, int size,char *client_ip_address,
 * 		  GHashTable **config_hash, GHashTable** hash
 *
 * description 	: this function parses the incoming data from client like
 * 		  images or files and stores them into the given directory
 * 		  as requested by the client. If the resource is already
 * 		  existing then it updates the current resource with the
 * 		  new incoming request or else it stores the new resource.
 * 		  It can also create directories if there are a set of
 * 		  directories inside which the resource is to be stored.
 *
 * return 	: integer
 *
 * **************************************************************************/
int store_PUT_multipart_request(int sockfd, int size,char *client_ip_address, GHashTable **config_hash, GHashTable** hash)
{
	//variables to parse the user data
	char *log_data = NULL;
	unsigned int j;
	int i = 0;
	int count = 0;
	char ch = -1;
	char curr, prev;
	int boundary_start = 1;
	int data_start = 0;
	int header_start = 0;
	int buf_size = 100;
	//initialize the buffer to store the client data
	char *buf = (char *)malloc(buf_size);
	if (NULL == buf) {
		log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

		return 1;
	}
	//clear the buffer to 0
	memset(buf, 0, buf_size);
	//create a FILE pointer
	FILE *fp = NULL;

	char *method_type = g_ascii_strup(g_hash_table_lookup(*hash, "method"), -1);

	//start parsing the incoming data
	for (j = 0; j < size; j++) {
		if (boundary_start) {
			prev = ch;
			//read from the client
			read(sockfd, &ch, 1);
			curr = ch;
			//if the current and previous character is CRLF
			if ('\r' == prev && '\n' == curr) {
				//assign value to header_start as 1
				header_start = 1;
				boundary_start = 0;
				count = 0;
				i = 0;
				continue;
			}
			//if the current character is '\r'
			if ('\r' == curr) {
				continue;
			}
		} else if (header_start) {
			prev = ch;
			read(sockfd, &ch, 1);
			curr = ch;
			//if the characters is CRLF
			if ('\r' == prev && '\n' == curr) {
				count++;
			}
			//if the characters is not CRLF
			if ('\n' != curr && '\r' != curr) {
				count = 0;
			}
			//assign the current character to buffer
			buf[i] = curr;
			i++;
			//if the buffer is already full then reallocated
			if (i >= buf_size) {
				buf_size = buf_size * 1.5;
				//reallocate the buf_size again
				buf = (char *)realloc(buf, buf_size);
				if (NULL == buf) {
					log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");
					g_free(method_type);

					return 1;
				}
			}
			//if the count is equal to 2
			if (2 == count) {
				buf[i] = '\0';
				//split the complete request at '\r\n'
				char **lines = g_strsplit(buf, "\r\n", -1);
				//calculate the length of the data received
				unsigned int len = strlen(lines[0]);
				unsigned int k;
				int filename_start = 0;
				//allocated memort to hold the name
				char *name = (char *)malloc(200);
				if (NULL == name) {
					free(buf);
					g_strfreev(lines);
					log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");
					g_free(method_type);

					return 1;
				}
				//set the variable to zero
				memset(name, 0, 200);
				int l = 0;

				for (k = 0; k < len; k++) {
					//check for character '"'
					if (!filename_start && lines[0][k] == '"') {
						filename_start = 1;
						continue;
					} else if (filename_start && lines[0][k] == '"') {
						filename_start = 0;
						name[l] = '\0';
						l = 0;
					}
					//if the filename_start is valid
					if (filename_start) {
						name[l] = lines[0][k];
						l++;
					}
				}

				//stat buffer to check for access rights
				struct stat access;
				//check the filename
				gchar *is_name_present = g_strrstr(lines[0], "filename=");
				//fetch the base directory path from the configuration hash table
				char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
				//error checking if the base directory is returned NULL
				if (NULL == base_dir) {
					//default base directory
					base_dir = g_strdup("../../www");
				}

				//fetch the requested URL from hash table
				char *complete_string = g_hash_table_lookup (*hash, "url");
				//error checking if the string is returned NULL
				if (NULL == complete_string){
					g_strfreev(lines);
					free(buf);
					free(name);
					g_free(base_dir);
					g_free(method_type);

					return 1;
				}

				//generate the complete file path for the requested URL
				gchar *file_name = g_strconcat(base_dir, complete_string, NULL);

				//variables to get the directory structure
				gchar *reverse = g_strreverse (file_name);
				gchar **splitted = g_strsplit_set (reverse, "/", 2);
				gchar *create_dir = g_strreverse (splitted[1]);
				//No dots are present then come here
				if (NULL == g_strrstr(splitted[0], "."))
				{
					//create a new directory structure using system command
					int slash_flag = 0;
					char *check_slash = g_strreverse(file_name);
					gboolean slash_present = g_str_has_suffix(check_slash, "/");
					if (FALSE == slash_present) {
						slash_flag = 1;
					}

					char *directory_with_file = g_strconcat(file_name, name, NULL);
					FILE *check_exists = fopen(directory_with_file, "r");
					if (NULL != check_exists) {
						// log to log file
						log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, 
								"] [UPDATING EXISTING RESOURCE] ", directory_with_file, NULL);
						log_server_data(SEVERE, log_data);
						g_free(log_data);
						log_data = NULL;
						
						fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [UPDATING EXISTING RESOURCE] %s" ANSI_COLOR_RESET "\n", 
						client_ip_address, method_type, directory_with_file);

						fclose(check_exists);
					}
					g_free(directory_with_file);

					char *make_dir = g_strconcat ("mkdir -p ", g_strreverse(g_strreverse(file_name)), NULL);
					//create directory
					int a = system (make_dir);
					//if the command was not executed
					if (-1 == a)
					{
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(file_name);
						log_server_data (SEVERE, "Problem Occured in creating the Base Directory");
						fprintf (stderr, ANSI_COLOR_RED "Problem Occured in creating the Base Directory" ANSI_COLOR_RESET "\n");
						g_free(method_type);
						return 1;
					}

					//create the file inside the directory using touch
					char *move = g_strconcat ("touch ", g_strreverse(g_strreverse(file_name)),name, NULL);
					//create file inside directory
					int c = system (move);
					//if the command was not executed
					if (-1 == c)
					{
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(move);
						g_free(file_name);
						log_server_data (SEVERE, "Problem Occured in creating the file");
						fprintf (stderr, ANSI_COLOR_RED "Problem Occured in creating the file" ANSI_COLOR_RESET "\n");
						g_free(method_type);
						return 1;
					}
					g_free (file_name);
					//updating the file path 
					file_name = g_strconcat (base_dir, complete_string, name, NULL);
					if (1 == slash_flag) {
						g_free (file_name);
						file_name = g_strconcat (base_dir, complete_string, "/", name, NULL);
					}
					//open filename
					fp = fopen(file_name, "w");
					if (NULL == fp) {
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(file_name);
						g_free(move);
						g_free(method_type);
						return 1;
					}
					stat(file_name,&access);
					char page_permission = (access.st_mode & S_IROTH) ? 'r':'-';
					if (page_permission != 'r')
					{
						log_server_data(SEVERE, "Access Forbidden to the Resource");
						fprintf(stdout, ANSI_COLOR_YELLOW "Access Forbidden to the Resource" ANSI_COLOR_RESET "\n");

						// return 403 Forbidden error 
						char *file_path = g_strconcat(base_dir, "/error/403.html", NULL);
						char *response = get_http_response_403(file_path);

						//write to client the error response
						send_http_response(sockfd, response, file_path);
						//clear all the allocated variable	
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(file_name);
						g_free(file_path);
						g_free(move);
						g_free(response);
						g_free(method_type);
						fclose(fp);
						return 1;

					} 

					gchar *new_path = g_strconcat (complete_string+1, name, NULL);
					if (1 == slash_flag) {
						g_free (new_path);
						new_path = g_strconcat (complete_string+1, "/", name, NULL);
					}
					//send a 201 created response to client
					char *header_201 = get_http_response_201(new_path, g_hash_table_lookup(*hash, "Host"));
					//send status to client
					int send_status = send_http_response(sockfd, header_201, NULL);
					//if the response was not sent
					if (0 != send_status)
					{
						//log server data
						log_server_data (SEVERE, "Error in sending HTTP 201 data to Client");
						//write to server console
						fprintf (stderr, ANSI_COLOR_RED "Error in sending HTTP 201 data to Client" ANSI_COLOR_RESET "\n");
					}
					//free all the allocated memory
					g_free (header_201);
					g_free (new_path);
					g_free (move);
					g_free (make_dir);
				}
				else
				{
					FILE *check_exists = fopen(g_strreverse(file_name), "r");
					if (NULL != check_exists) {
						// log to log file
						log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, 
								"] [UPDATING EXISTING RESOURCE] ", file_name, NULL);
						log_server_data(SEVERE, log_data);
						g_free(log_data);
						log_data = NULL;
						
						fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [UPDATING EXISTING RESOURCE] %s" ANSI_COLOR_RESET "\n", 
						client_ip_address, method_type, file_name);

						fclose(check_exists);
					}
					//create a new directory structure using system command
					char *make_dir = g_strconcat ("mkdir -p ", create_dir, NULL);
					//create directory
					int a = system (make_dir);
					//if the command was not executed
					if (-1 == a)
					{
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(file_name);
						log_server_data(SEVERE, "Problem Occured in creating the Base Directory");
						fprintf(stderr, ANSI_COLOR_RED "Problem Occured in creating the Base Directory" ANSI_COLOR_RESET "\n");
						g_free(method_type);
						return 1;
					}

					log_data = g_strconcat("Base Directory Created ", create_dir, NULL); 
					log_server_data (INFO, log_data);
					fprintf (stdout, ANSI_COLOR_YELLOW "Base Directory Created %s" ANSI_COLOR_RESET "\n", create_dir);
					g_free(log_data);
					log_data = NULL;

					//retreive the file name to be created
					gchar *create_file = file_name;
					//create the file inside the directory using touch
					char *move = g_strconcat ("touch ", create_file, NULL);
					//create file inside directory
					int c = system (move);
					//if the command was not executed
					if (-1 == c)
					{
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(move);
						g_free(file_name);
						log_server_data(SEVERE, "Problem Occured in creating the file");
						fprintf(stderr, ANSI_COLOR_RED "Problem Occured in creating the file" ANSI_COLOR_RESET "\n");
						g_free(method_type);
						return 1;
					}
					//log data
					log_data = g_strconcat("Resource Created ", create_file, NULL); 
					log_server_data (INFO, log_data);
					fprintf (stdout, ANSI_COLOR_YELLOW "Resource Created %s" ANSI_COLOR_RESET "\n", create_file);
					g_free(log_data);
					log_data = NULL;

					//open filename
					fp = fopen(file_name, "w");
					if (NULL == fp) {
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(move);
						g_free(file_name);
						g_free(method_type);
						return 1;
					}
					stat(file_name,&access);
					char page_permission = (access.st_mode & S_IROTH) ? 'r':'-';
					if (page_permission != 'r')
					{
						log_server_data(SEVERE, "Access Forbidden to the Resource");
						fprintf(stdout, ANSI_COLOR_YELLOW "Access Forbidden to the Resource" ANSI_COLOR_RESET "\n");

						// return 403 Forbidden error 
						char *file_path_1 = g_strconcat(base_dir, "/error/403.html", NULL);
						char *response_1 = get_http_response_403(file_path_1);

						//write to client the error response
						send_http_response(sockfd, response_1, file_path_1);
						//clear all the allocated variable	
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free(base_dir);
						g_free(make_dir);
						g_strfreev(splitted);
						g_free(move);
						g_free(file_name);
						g_free(file_path_1);
						g_free(response_1);
						g_free(method_type);
						fclose(fp);
						return 1;

					} 


					//send a 201 created response to client
					char *header_201 = get_http_response_201(complete_string+1, g_hash_table_lookup(*hash, "Host"));
					//send status to client
					int send_status = send_http_response(sockfd, header_201, NULL);
					//if the response was not sent
					if (0 != send_status)
					{
						//log server data
						log_server_data (SEVERE, "Error in sending HTTP 201 data to Client");
						//write to server console
						fprintf (stderr, ANSI_COLOR_RED "Error in sending HTTP 201 data to Client" ANSI_COLOR_RESET "\n");
					}
					//free all the allocated variables
					g_free (header_201);
					g_free (move);
					g_free (make_dir);
				}
				//free all the allocated memory
				g_strfreev (splitted);
				g_free (base_dir);
				//continue if 'is_name_present' is not NULL
				if (NULL == is_name_present) {
					char *put_data_path = g_strconcat(base_dir, "put_data.txt", NULL);
					//open the file
					if (NULL != fp) {
						fclose(fp);
					}
					fp = fopen(put_data_path, "a");
					if (NULL == fp) {
						g_strfreev(lines);
						free(buf);
						free(name);
						g_free (put_data_path);
						g_free(file_name);
						return 1;
					}
					//write to the file
					fprintf(fp, "\n%s: ", name);
					//free the allocated variable
					g_free (put_data_path);
				}
				//clear the buf_size
				memset(buf, 0, buf_size);
				//reinitialize the variables
				header_start = 0;
				data_start = 1;
				//free the allocated variable
				g_strfreev(lines);
				free(name);
				g_free(file_name);
			}
			//else start parsing the data
		} else if (data_start) {

			prev = ch;
			//read data from the socket
			read(sockfd, &ch, 1);
			curr = ch;

			if ('\r' == prev && '\n' == curr) {
				boundary_start = 1;
				data_start = 0;
				//close file
				fclose(fp);
				continue;
			}
			//if the current character read is '\r'
			if ('\r' == curr) {
				continue;
			}
			//if the previous character is '\r' and the next character is not '\n'
			if ('\r' == prev && '\n' != curr) {
				fprintf(fp, "%c%c", prev, curr);
				//clear the file variable
				fflush(fp);
			} else {
				//write to file
				fprintf(fp, "%c", curr);
				fflush(fp);
			}
		}
	}

	//free the allocated variable
	g_free (buf);
	g_free(method_type);
	return 0;
}

/**********************************************************************
 * function 	: handle_PUT_request
 *
 * arguments 	: int sockfd, fd_set *master, GHashTable **hash,
 * 		  char *client_ip_address, GHashTable** config_hash,
 * 		  GHashTable** redirect_hash
 *
 * description 	: this function handles the incoming PUT request from
 * 		  the client. Data handled can be both simple data and
 *		  mutilpart data.
 *
 * return 	: integer
 *
 * *******************************************************************/
int handle_PUT_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash)
{
	//fetch the content length form the hash table
	char *content_length_str = g_hash_table_lookup(*hash, "Content-Length");
	//variable to store content length
	int content_length_int = 0;
	//if the length is NULL
	if (NULL == content_length_str) {
		content_length_int = 0;
		//if the content length is not NULL
	} else {
		content_length_int = str_to_int(content_length_str);
		//if the length if less than -1
		if (-1 == content_length_int) {
			content_length_int = 0;
		}
	}
	//fetch the content type form the hash table
	char *content_type = g_hash_table_lookup(*hash, "Content-Type");
	//check if the data is of multipart/form-data
	if (0 != strncmp(content_type, "multipart/form-data", strlen("multipart/form-data"))) {
		int store_status = store_PUT_simple_request(sockfd, content_length_int, client_ip_address, &(*config_hash), &(*hash));
		// incase of error exists from function with error status
		if (0 != store_status) {
			return 2;
		}
	} else {
		//store the PUT request data
		int store_status = store_PUT_multipart_request(sockfd, content_length_int,client_ip_address, &(*config_hash), &(*hash));
		// incase of error exists from function with error status
		if (0 != store_status) {
			return 3;
		}
	}
	return 0;
}

