#include "http_OPTIONS_request_handler.h"

/*************************************************************************
 * function  	: handle_OPTIONS_request
 *
 * arguments 	: int sockfd, fd_set *master, GHashTable **hash,
 * 	          char *client_ip_address, GHashTable** config_hash,
 * 	          GHashTable** redirect_hash
 *
 * description 	: this function handles the OPTION request header for a
 * 		  given resource and return to the client which methods
 * 		  are supported for the given resource i.e the method by
 * 		  which we can handle the given resource.
 *
 * return 	: integer
 *
 * ************************************************************************/
int handle_OPTIONS_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash) {
	// logging file
	char *log_data = NULL;

	// lookup url in hash table, it may contain data also
	char *url_with_data  = g_hash_table_lookup(*hash, "url");

	// url will contain data after "?" so we are splitting at "?"
	// example: /hello.html?name=john+doe
	// tokenized: [/hello.html] [name=jhon+doe]
	char **tokenized_url = g_strsplit_set(url_with_data, "?", 2);

	// store a copy
	char *unsanitized_filename = g_strdup(tokenized_url[0]);
	//free the tokenized variable
	g_strfreev(tokenized_url);

	// remove "/" from url
	// example: "/hello.html" ---->  "hello.html"
	char *filename = get_valid_file_path(unsanitized_filename, sockfd, &(*master), client_ip_address, &(*config_hash), &(*redirect_hash), &(*hash));
	g_free(unsanitized_filename);
	if (NULL == filename) {
		// remove client from select watching list
		FD_CLR(sockfd, &(*master));
		return 1;
	}

	// try to open file requested
	FILE *fp = fopen(filename, "r");
	// if failed in opening file send 404 error
	g_free(filename);
	if (NULL == fp) {
		// log data
		log_data = g_strconcat ("[client ",client_ip_address ,"]", " GET", "[Page Served] 404 Error", NULL);
		log_server_data (FINE, log_data);
		g_free(log_data);
		//fetch the base directory path from hash table
		char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
		//error checking if the base directory is NULL
		if (NULL == base_dir) {
			//default location
			base_dir = g_strdup("../../www");
		}
		//fetch the 404 Not Found Page
		char *file_path_404 = g_strconcat(base_dir, "/error/404.html", NULL);
		//fetch the header for the 404 Not Found File
		char *header_404 = get_http_response_404(file_path_404);
		// send the data to the client
		int send_status = send_http_response(sockfd, header_404, file_path_404);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP 404 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 404 response to Client" ANSI_COLOR_RESET "\n");
		}
		g_free(header_404);
		g_free(base_dir);
		g_free(file_path_404);
		//clear the socket from the master set
		FD_CLR(sockfd, &(*master));

		return 1;
	}
	// close file pointer
	int is_close = fclose(fp);
	if (0 != is_close) {
		// if file not closed properly log this to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [FILE NOT CLOSED PROPERLY] ", filename, NULL);
		log_server_data(WARNING, log_data);
		g_free(log_data);
		// if file not closed properly log this to stderr
		fprintf(stderr, ANSI_COLOR_MAGENTA "[client %s] [FILE NOT CLOSED PROPERLY] %s" ANSI_COLOR_RESET "\n", client_ip_address, filename);
	}

	// construct the header
	char *OPTION_200 = get_http_OPTIONS_response_200();

	// send the data to the client
	int send_status = send_http_response(sockfd, OPTION_200, NULL);
	if (0 != send_status) {
		log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
		fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client" ANSI_COLOR_RESET "\n");
	}
	g_free(OPTION_200);

	return 0;
}

