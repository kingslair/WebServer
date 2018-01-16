#include "http_parser.h"

/**********************************************************************
 * function 	: http_parse_status_line
 *
 * arguments 	: char *status_line, GHashTable **hash, int sockfd, 
 * 		  fd_set *master, GHashTable **config_hash, 
 * 		  char *client_ip_address
 *
 * description 	: extract status line from request.
 *
 * return 	: integer
 * *******************************************************************/
int http_parse_status_line(char *status_line, GHashTable **hash, int sockfd, fd_set *master, GHashTable **config_hash, char *client_ip_address) {
	// split status line at " "
	// example: GET /url HTTP/1.1
	// tokenized into: [GET] [/url] [HTTP/1.1]
	if (NULL == status_line) {
		return -1;
	}
	char **tokens = g_strsplit_set(status_line, " ", -1);
	if (NULL == tokens[2]) {
		g_strfreev(tokens);
		return -1;
	}
	// split http version at "/"
	// example: HTTP/1.1
	// tokenized into: [HTTP] [1.1]
	char **http_version_line = g_strsplit(tokens[2], "/", 2);

	if (0 != strcmp("1.1", http_version_line[1])) {
		char *log_data = g_strconcat("[client ", client_ip_address, "] [505 HTTP VERSION NOT SUPPORTED]", NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);

		fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [505 HTTP VERSION NOT SUPPORTED]" ANSI_COLOR_RESET "\n", client_ip_address);

		char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
		if (NULL == base_dir) {
			base_dir = g_strdup("www");
		}
		char *file_path_505 = g_strconcat(base_dir, "/error/505.html", NULL);
		char *header_505 = get_http_response_505(file_path_505);
		int send_status = send_http_response(sockfd, header_505, file_path_505);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP 505 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 505 response to Client" ANSI_COLOR_RESET "\n");
		}
		g_free(header_505);
		g_free(base_dir);
		close(sockfd);
		FD_CLR(sockfd, &(*master));

		// free tokens
		g_strfreev(tokens);
		// free http_version_line
		g_strfreev(http_version_line);
		return -1;
	}
	// store request method in hash table (GET, POST and etc)
	g_hash_table_insert(*hash, g_strdup("method"), g_strdup(tokens[0]));
	// store url into hash table
	g_hash_table_insert(*hash, g_strdup("url"), g_strdup(tokens[1]));

	// free tokens
	g_strfreev(tokens);
	// free http_version_line
	g_strfreev(http_version_line);
	return 0;
}

/**********************************************************************
 * function 	: http_parse_header_fields
 *
 * arguments 	: char **fields, unsigned int length, GHashTable **hash, 
 * 		  int sockfd, fd_set *master, GHashTable **config_hash, 
 * 		  char *client_ip_address
 *
 * description 	: extract the header fields from the request.
 *
 * return 	: integer
 * *******************************************************************/
int http_parse_header_fields(char **fields, unsigned int length, GHashTable **hash, int sockfd, fd_set *master, GHashTable **config_hash, char *client_ip_address) {
	unsigned int i;
	for (i = 1; i < length-1; i++) {
		if (0 == strcmp("", fields[i]) || NULL == fields[i]) {
			continue;
		}
		// split header field at ":"
		// example: Content-Type: text/html
		// tokenized into: [Content-Type] [text/html]
		gchar **header_fields = g_strsplit(fields[i], ":", 2);
		gchar *key = header_fields[0];
		// strip any whitespace if present in entry
		gchar *value = g_strstrip(header_fields[1]);

		// check if header field hash space before ":"
		// if yes than its a bad request
		if (TRUE == g_str_has_suffix(key, " ")) {
			char *log_data = g_strconcat("[client ", client_ip_address, "] [400 BAD REQUEST]", NULL);
			log_server_data(INFO, log_data);
			g_free(log_data);

			fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [400 BAD REQUEST]" ANSI_COLOR_RESET "\n", client_ip_address);
			
			// get 400 headers
			char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
			if (NULL == base_dir) {
				base_dir = g_strdup("www");
			}
			char *file_path_400 = g_strconcat(base_dir, "/error/400.html", NULL);
			char *header_400 = get_http_response_400(file_path_400);

			// send headers and file content to client
			int send_status = send_http_response(sockfd, header_400, file_path_400);
			if (0 != send_status) {
				log_server_data(SEVERE, "Error in sending HTTP 400 response to Client");
				fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 400 response to Client" ANSI_COLOR_RESET "\n");
			}

			g_free(base_dir);
			g_free(file_path_400);
			g_free(header_400);

			close(sockfd);
			FD_CLR(sockfd, &(*master));
			g_strfreev(header_fields);
			return -1;
		}
		// store key, value into hash table
		g_hash_table_insert(*hash, g_strdup(key), g_strdup(value));
		g_strfreev(header_fields);
	}
	return 0;
}

/**********************************************************************
 * function 	: http_parse_request_header
 *
 * arguments 	: char *request, int sockfd, fd_set *master, 
 * 		  GHashTable** config_hash, char *client_ip_address
 *
 * description 	: extracts the request header.
 *
 * return 	: integer
 * *******************************************************************/
GHashTable* http_parse_request_header(char *request, int sockfd, fd_set *master, GHashTable** config_hash, char *client_ip_address) {
	// split lines at \r\n after this we have all request divided into lines
	if (NULL == request) {
		return NULL;
	}
	char **lines = g_strsplit(request, "\r\n", -1);
	// find number of lines in request
	gsize length = g_strv_length(lines);

	// create hash table to store request fields
	GHashTable *request_hash = g_hash_table_new(g_str_hash, g_str_equal);
	// parse status line
	int status_line_parsed = http_parse_status_line(lines[0], &request_hash, sockfd, &(*master), &(*config_hash), client_ip_address);
	if (0 != status_line_parsed) {
		g_strfreev(lines);
		g_hash_table_destroy(request_hash);
		return NULL;
	}
	// parse header fields
	int header_fields_parsed = http_parse_header_fields(lines, length, &request_hash, sockfd, &(*master), &(*config_hash), client_ip_address);
	if (0 != header_fields_parsed) {
		g_strfreev(lines);
		g_hash_table_foreach(request_hash, (GHFunc)free_hash, NULL);
		g_hash_table_destroy(request_hash);
		return NULL;
	}

	g_strfreev(lines);
	return request_hash;
}

