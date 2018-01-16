#include "http_helper_functions.h"

/**********************************************************************
 * function 	: send_http_response
 *
 * arguments 	: int sockfd, char *headers, char *filename
 *
 * description 	: sends the requested resource to the client.
 *
 * return 	: integer
 *
 * *******************************************************************/
int send_http_response(int sockfd, char *headers, char *filename) {
	size_t len = strlen(headers);
	// send headers first
	send(sockfd, headers, len, 0);

	if (NULL != filename) {
		// open a file
		FILE *fp = fopen(filename, "r");
		if (NULL == fp) {
			// if error happen in reading file log it to stderr and log file
			char *log_data = g_strconcat("Error in reading file ", filename, NULL);
			log_server_data(SEVERE, log_data);
			g_free(log_data);

			fprintf(stderr, ANSI_COLOR_RED "Error in reading file %s" ANSI_COLOR_RESET "\n", filename);
			return 1;
		}
		unsigned int size = 100;
		// buffer to store file data
		char buffer[size];
		memset(buffer, 0, size);
		// while upto EOF
		while (!feof(fp)) {
			// read from file
			fread(buffer, 1, size, fp);
			// write to socket
			write(sockfd, buffer, size);
			// reset buffer
			memset(buffer, 0, size);
		}
		int is_closed = fclose(fp);
		if (0 != is_closed) {
			char *log_data = g_strconcat("[FILE NOT CLOSED PROPERLY] ", filename, NULL);
			log_server_data(WARNING, log_data);
			g_free(log_data);

			fprintf(stderr, ANSI_COLOR_MAGENTA "[FILE NOT CLOSED PROPERLY] %s" ANSI_COLOR_RESET "\n", filename);
		}
	}
	// end http response by sending "\r\n"
	write(sockfd, "\r\n", 2);

	return 0;
}

/**********************************************************************
 * function 	: http_page_access_rights
 *
 * arguments 	: char *page_url
 *
 * description 	: checks the access rights of a particular resource.
 *
 * return 	: integer
 * *******************************************************************/
int http_page_access_rights(char *page_url) {
	/* variable to store the page/file URL */
	int response = 0;
	struct stat buf;

	stat(page_url,&buf);

	/* Holds the file permission temporary before
	 * writing to memory  */
	char page_permission = (buf.st_mode & S_IROTH) ? 'r':'-';

	/* check for file*/
	if ('r' == page_permission) {
		response = 1;
	}

	/* return response to caller function */
	return response;
}

/**********************************************************************
 * function 	: get_valid_file_path
 *
 * arguments 	: char *url_with_data, int sockfd, fd_set *master, 
 * 		  char *client_ip_address, GHashTable **config_hash, 
 * 		  GHashTable **redirect_hash, GHashTable **hash
 *
 * description	: checks the validity of requested resource for file exists
 * 		  or not, client has permission or not and if redirect needs
 * 		  to perform to serve the resource.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_valid_file_path(char *url_with_data, int sockfd, fd_set *master, char *client_ip_address, GHashTable **config_hash, GHashTable **redirect_hash, GHashTable **hash) {

	char *log_data = NULL;
	// check if "?" present in url
	// incase of GET request data is present in url after "?"
	char *question_location = g_strrstr(url_with_data, "?");
	char *url = NULL;
	char *method_type = g_ascii_strup(g_hash_table_lookup(*hash, "method"), -1);
	// if "?" is found than split at it
	if (NULL != question_location) {
		// split at "?"
		// example: /hello.html?name=jhon
		// tokenized: [/hello.html] [name=jhon]
		char **tokenize_url = g_strsplit_set(url_with_data, "?", 2);

		// create duplicate of url
		char *base_dir = g_hash_table_lookup(*config_hash, "BASE_DIR");
		url = g_strconcat(base_dir, tokenize_url[0], NULL);
		g_strfreev(tokenize_url);
	} else {
		// if url is without data than use as it
		char *base_dir = g_hash_table_lookup(*config_hash, "BASE_DIR");
		url = g_strconcat(base_dir, url_with_data, NULL);
	}


	// checking "." to check if requested url is file or folder
	int directory = is_directory(url);
	char *url_with_index = NULL;
	// if its a folder
	if (directory) {
		// add "/index2.html" after it
		// example: "jhon"  ---->  "jhon/index2.html"
		if (TRUE == g_str_has_suffix(url, "/")) {
			/* log data */
			log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [Page Requested] ", url, NULL);
			log_server_data(INFO, log_data);
			g_free(log_data);
			url_with_index = g_strconcat(url, "index2.html", NULL);
			fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [Page Requested] %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url_with_index);
		} else {
			/* log data */
			log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [Page Requested] ", url, NULL);
			log_server_data(INFO, log_data);
			g_free(log_data);
			url_with_index = g_strconcat(url, "/", "index2.html", NULL);
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url_with_index);
		}

		// change the url value which it was pointing previously, now its pointing to url
		// with index2.html page
		g_free(url);
		url = url_with_index;
	}

	/* checking for 404 error if the page/resource doesn't exist */
	FILE *page_exist = fopen(url, "r");
	if (NULL == page_exist) {
		// if file does not exists log this to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [Page Requested] 404 ERROR ", url, NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);
		// log this to stderr
		fprintf(stderr, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] 404 ERROR %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);

		char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
		if (NULL == base_dir) {
			base_dir = g_strdup("../../www");
		}

		// creating file path to 404 page
		char *file_path_404 = g_strconcat(base_dir, "/error/404.html", NULL);
		g_free(base_dir);

		// send http 404 response to client
		char *header_404 = get_http_response_404(file_path_404);
		int send_status = send_http_response(sockfd, header_404, file_path_404);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP 404 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 404 response to Client" ANSI_COLOR_RESET "\n");
		}

		// free all the resources
		g_free(file_path_404);
		g_free(header_404);
		g_free(url);
		g_free(method_type);

		return NULL;
	}
	int is_closed = fclose(page_exist);
	if (0 != is_closed) {
		log_data = g_strconcat("[client ", client_ip_address, "] [FILE NOT CLOSED PROPERLY] ", url, NULL);
		log_server_data(WARNING, log_data);
		g_free(log_data);
		fprintf(stderr, ANSI_COLOR_MAGENTA "[client %s] [FILE NOT CLOSED PROPERLY] %s" ANSI_COLOR_RESET "\n", client_ip_address, url);
	}

	// sentinel value to indicate we need to redirect or not
	int redirect_page = 0;

	/* Redirect Code */
	REDIRECT *redirect_status = g_hash_table_lookup(*redirect_hash, url);

	// if url requested is one of redirect entry than change sentinel value
	if (NULL != redirect_status && 0 == strcmp(url, redirect_status->old_page_URL)) {
		redirect_page = 1;
	}

	// check if redirect required or not
	if (1 == redirect_page) {
		// pointer to http response string
		char *response = NULL;

		// check the redirect type and generate response according to that
		if (0 == strcmp(PERMANENTLY_MOVED, redirect_status->redirect_code)) {
			// log Redirect into log file
			log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] 308 REDIRECT PERMANENT MOVED", url, NULL);
			log_server_data(INFO, log_data);
			g_free(log_data);
			// log Redirect to stdout
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] 308 REDIRECT PERMANENT MOVED %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
			/* preapre the response to send a 308 request */
			response = http_error_response_308(redirect_status->new_page_URL, g_hash_table_lookup(*hash, "Host"));
		} else if (strcmp (TEMPORARILY_MOVED,redirect_status->redirect_code) == 0) {
			// log Redirect into log file
			log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] 307 REDIRECT TEMPORARILY MOVED", url, NULL);
			log_server_data(INFO, log_data);
			g_free(log_data);
			// log Redirect to stdout
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] 307 REDIRECT TEMPORARILY MOVED %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
			/* preapre the response to send a 307 request */
			response = http_error_response_307(redirect_status->new_page_URL, g_hash_table_lookup(*hash, "Host"));
		}

		/* send response to client */
		int send_status = send_http_response (sockfd, response, NULL);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP redirect response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP redirect response to Client" ANSI_COLOR_RESET "\n");
		}

		g_free(response);
		g_free(url);
		g_free(method_type);

		return NULL;
	}

	// check the access rights
	int access_rights = http_page_access_rights(url);

	// if we have access to the file
	if (1 == access_rights) {
		// log data to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [Page Requested] ", url , NULL);
		log_server_data(INFO, log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
		g_free(log_data);
		g_free(method_type);

		// return the URL to client
		return url;
	} else {
		// log data to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] FORBIDDEN TO ACCESS ", url, NULL);
		log_server_data(INFO, log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page forbidden to access] %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
		g_free(log_data);

		// return 403 Forbidden response
		char *base_dir = g_hash_table_lookup(*config_hash, "BASE_DIR");
		char *file_path = g_strconcat(base_dir, "/error/403.html", NULL);
		char *response = get_http_response_403(file_path);

		// log data to log file
		log_data = g_strconcat("[client ", client_ip_address, "] [", method_type, "] [Page Served] 403 error ", url, NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Served] 403 ERROR %s" ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);

		// write to client the error response
		int send_status = send_http_response(sockfd, response, file_path);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP 403 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 403 response to Client" ANSI_COLOR_RESET "\n");
		}

		g_free(file_path);
		g_free(response);
		g_free(method_type);
		g_free(url);

		return NULL;
	}
	g_free(method_type);

	return url;
}

