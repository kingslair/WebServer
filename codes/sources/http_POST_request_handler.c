#include "http_POST_request_handler.h"

/************************************************************************************************************
 * function 	: store_POST_simple_request
 *
 * arguments 	: int sockfd, unsigned int size, char *client_ip_address, GHashTable **config_hash, GHashTable **hash
 *
 * description 	: store POST request in a json file, if content type is not multipart
 *
 * return 	: integer (status of the operation)
 * **********************************************************************************************************/
int store_POST_simple_request(int sockfd, unsigned int size, char *client_ip_address, GHashTable **config_hash, GHashTable **hash) {
	// pointer to store log string
	char *log_data = NULL;

	// initial buffer size
	unsigned int buf_size = 100;
	// allocate memory to store request
	char *buf = (char *)malloc(sizeof(char) * buf_size);
	if (NULL == buf){
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

	char *json_file_path = g_strconcat(base_dir, "/POST/data_", time_string, ".json", NULL);
	char *method_type = g_ascii_strup(g_hash_table_lookup(*hash, "method"), -1);

	FILE *fp = fopen(json_file_path, "w");
	if (NULL == fp) {
		// incase of error log it to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [ERROR IN CREATING FILE] ", json_file_path, NULL);
		log_server_data(SEVERE, log_data);
		g_free(log_data);

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

/******************************************************************************************************************
 * function 	: store_POST_multipart_request
 *
 * arguments 	: int sockfd, unsigned int size,char *client_ip_address, GHashTable **config_hash, GHashTable **hash
 *
 * description 	: store the POST request data if content type is multipart
 *
 * return 	: integer (status of the operation)
 * ****************************************************************************************************************/
int store_POST_multipart_request(int sockfd, unsigned int size,char *client_ip_address, GHashTable **config_hash, GHashTable **hash) {
	// to point to log data string
	char *log_data = NULL;

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
		log_server_data(SEVERE, "Memory Allocation unsuccessful");
		fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

		return 1;
	}
	// reset the buffer
	memset(buf, 0, buf_size);

	// read method type from hash table
	// it might be possible to hard code POST here but, for fexibility
	// we are reading from it from parsed data hash table
	char *method_type = g_ascii_strup(g_hash_table_lookup(*hash, "method"), -1);

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
					log_server_data(SEVERE, "Memory Allocation unsuccessful");
					fprintf(stderr, ANSI_COLOR_RED "Memory Allocation unsuccessfull" ANSI_COLOR_RESET "\n");

					return 1;
				}
			}

			// if we encountered two crlf's than
			// process data in buffer
			if (2 == count) {
				// NULL terminate the buffer
				buf[i] = '\0';
				// divide buffer at crlf
				char **lines = g_strsplit(buf, "\r\n", -1);
				// find the number of lines
				unsigned int len = strlen(lines[0]);
				// sentinel to show start of filename
				int filename_start = 0;

				// allocate memory to store filename or key of key value pair
				char *name = (char *)malloc(200);
				if (NULL == name) {
					free(buf);
					g_strfreev(lines);
					log_server_data(SEVERE, "Memory Allocation unsuccessful");
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
				char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
				// if base dir is not present than take default value of 'www'
				if (NULL == base_dir) {
					base_dir = g_strdup("../../www");
				}
				char *url = g_hash_table_lookup(*hash, "url");
				// create file path to store data coming from socket if its file not key
				gchar *file_name = g_strconcat(base_dir, url, NULL);

				// check if filename is present or not
				gchar *is_name_present = g_strrstr(lines[0], "filename=");
				// if its file
				if (NULL != is_name_present) {
					// log data to log file
					log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [NEW RESOURCE CREATED] ", file_name, NULL);
					log_server_data(INFO, log_data);
					g_free(log_data);
					log_data = NULL;
					// log data to stdout
					fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [NEW RESOURCE CREATED] %s" ANSI_COLOR_RESET "\n",
							client_ip_address, method_type, file_name);

					fp = fopen(file_name, "w");
					if (NULL == fp) {
						// if error in creating file
						// log it to log file
						log_data = g_strconcat ("[client ", client_ip_address, "] [", method_type, "] [ERROR IN CREATING FILE] ", file_name, NULL);
						log_server_data (INFO, log_data);
						g_free(log_data);
						// log to stderr
						fprintf(stderr, ANSI_COLOR_RED "[client %s] [%s] [ERROR IN CREATING FILE] %s" ANSI_COLOR_RESET "\n",
								client_ip_address, method_type, file_name);

						// free all resource used
						free(buf);
						g_free(file_name);
						g_free(method_type);
						g_free(base_dir);
						free(name);
						g_strfreev(lines);

						return 1;
					}

					// send http response of 201 to user that resource created successfully
					char *header_201 = get_http_response_201(url+1, g_hash_table_lookup(*hash, "Host"));
					int send_status = send_http_response(sockfd, header_201, NULL);
					if (0 != send_status) {
						log_server_data(SEVERE, "Error in sending HTTP 201 response to Client");
						fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 201 response to Client" ANSI_COLOR_RESET "\n");
					}
					g_free(header_201);

				} else {
					// if data present is key value pair than create file path to store that data
					char *post_data_path = g_strconcat(base_dir, "/POST/multipart_post_data.txt", NULL);
					fp = fopen(post_data_path, "a");
					if (NULL == fp) {
						// in case of error log it to log file
						log_data = g_strconcat ("[client ", client_ip_address, "] [", method_type, "] [ERROR IN CREATING FILE] ", post_data_path, NULL);
						log_server_data (INFO, log_data);
						g_free(log_data);
						log_data = NULL;
						// log it to stderr
						fprintf(stderr, ANSI_COLOR_RED "[client %s] [%s] [ERROR IN CREATING FILE] %s" ANSI_COLOR_RESET "\n",
								client_ip_address, method_type, post_data_path);

						// free all the resources used
						free(buf);
						g_free(file_name);
						g_free(method_type);
						g_free(base_dir);
						g_free(post_data_path);
						free(name);
						g_strfreev(lines);

						return 1;
					}
					// if file create successfully than log it
					log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [FILE CREATED] ", post_data_path, NULL);
					log_server_data(INFO, log_data);
					g_free(log_data);
					log_data = NULL;
					// log it to stdout
					fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [FILE CREATED] %s" ANSI_COLOR_RESET "\n",
							client_ip_address, method_type, post_data_path);

					// free path you have created to store data
					g_free(post_data_path);
					fprintf(fp, "\n%s: ", name);
				}
				// reset the buffer
				// for next iteration
				memset(buf, 0, buf_size);
				// break out from header section and move to data section
				header_start = 0;
				data_start = 1;

				// free resources
				g_strfreev(lines);
				free(name);
				g_free(file_name);
				g_free(base_dir);
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
					log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [ERROR IN CLOSING FILE]", NULL);
					log_server_data(WARNING, log_data);
					g_free(log_data);
					log_data = NULL;
					// log it to stderr
					fprintf(stderr, ANSI_COLOR_MAGENTA "[client %s] [%s] [ERROR IN CLOSING FILE]" ANSI_COLOR_RESET "\n",
							client_ip_address, method_type);
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
	g_free(method_type);
	free(buf);

	return 0;
}

/***************************************************************************************************************************************
 * function 	: handle_POST_request
 *
 * arguments 	: int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash
 *
 * description 	: handle HTTP POST request
 *
 * return 	: integer (status of the operation)
 * *************************************************************************************************************************************/
int handle_POST_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash) {
	// lookup content length from hashtable containg parsed data
	char *content_length_str = g_hash_table_lookup(*hash, "Content-Length");
	int content_length_int = 0;
	// if content length is NULL than take default value of 0
	if (NULL == content_length_str) {
		content_length_int = 0;
	} else {
		// if content length is a valid string than convert it to
		// integer
		content_length_int = str_to_int(content_length_str);
		// if error in conversion than take default value of 0
		if (-1 == content_length_int) {
			content_length_int = 0;
		}
	}

	// check if requested url is valid or not, if valid than return that valid string
	// otherwise NULL
	char *filename = get_valid_file_path(g_hash_table_lookup(*hash, "url"), sockfd, &(*master), client_ip_address, &(*config_hash), &(*redirect_hash), &(*hash));
	if (NULL == filename) {
		// incase of invalid file path remove socket from list of our watching sockets
		FD_CLR(sockfd, &(*master));

		return 1;
	}
	g_free(filename);

	// lookup content type of POST request
	char *content_type = g_hash_table_lookup(*hash, "Content-Type");
	// if content type is not multipart
	if (0 != strncmp(content_type, "multipart/form-data", strlen("multipart/form-data"))) {
		int store_status = store_POST_simple_request(sockfd, content_length_int, client_ip_address, &(*config_hash), &(*hash));
		if (0 != store_status) {
			return 1;
		}
	} else {
		// if content type is multipart
		int store_status = store_POST_multipart_request(sockfd, content_length_int,client_ip_address, &(*config_hash), &(*hash));
		if (0 != store_status) {
			return 1;
		}
	}

	return 0;
}

