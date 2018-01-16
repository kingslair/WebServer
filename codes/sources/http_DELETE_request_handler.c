#include "http_DELETE_request_handler.h"

/****************************************************************************
 * function 	: handle_DELETE_request
 *
 * arguments	: int sockfd, fd_set *master, GHashTable **hash,
 * 		  char *client_ip_address, GHashTable** config_hash,
 * 		  GHashTable** redirect_hash
 *
 * description 	: this function handles the DELETE request on a partic-
 *   		  uclar resource and removes the resource from the current
 *   		  base directory and moves it to the trash outside of the
 *		  base directory
 *
 * return 	: integer
 * 
 * ***************************************************************************/
int handle_DELETE_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash) {
	// character pointer to point to log string
	char *log_data = NULL;

	// lookup url in hash table, it may contain data also
	char *url_with_data  = g_hash_table_lookup(*hash, "url");


	// url will contain data after "?" so we are splitting at "?"
	// example: /hello.html?name=john+doe
	// tokenized: [/hello.html] [name=jhon+doe]
	char **tokenized_url = g_strsplit_set(url_with_data, "?", 2);

	// store a copy
	char *unsanitized_filename = g_strdup(tokenized_url[0]);
	//free the tokenized url
	g_strfreev(tokenized_url);

	// remove "/" from url
	// example: "/hello.html" ---->  "hello.html"
	char *filename = get_valid_file_path(unsanitized_filename, sockfd, &(*master), client_ip_address, &(*config_hash), &(*redirect_hash), &(*hash));
	g_free(unsanitized_filename);
	if (NULL == filename) {
		FD_CLR(sockfd, &(*master));
		close(sockfd);
		return 1;
	}

	// try to open file requested
	FILE *fp = fopen(filename, "r");
	// if failed in opening file send 404 error
	if (NULL == fp) {
		//log info to log file
		char *log_data = g_strconcat("[client ", client_ip_address, "] [GET]", " [Page Served] 404 Error", NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);

		//clear the filename variable
		g_free(filename);
		//fetch the path of the base directory path from the hash table
		char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
		//error checking if the base directory is NULL
		if (NULL == base_dir) {
			//default base directory incase hash table returned NULL
			base_dir = g_strdup("../../www");
		}
		//fetch the 404 Not Found Page
		char *file_path_404 = g_strconcat(base_dir, "/error/404.html", NULL);
		char *header_404 = get_http_response_404(file_path_404);
		//send the response back to client
		int send_status = send_http_response(sockfd, header_404, file_path_404);
		if (0 != send_status) {
			log_server_data(SEVERE, "Error in sending HTTP 404 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 404 response to Client" ANSI_COLOR_RESET "\n");
		}
		//clear the socket from the master set
		FD_CLR(sockfd, &(*master));
		//close the socket
		close(sockfd);
		//clear all the allocated memory
		g_free(header_404);
		g_free(file_path_404);
		g_free(base_dir);
		g_free(log_data);

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

	char *header_200 = get_http_response_200(filename);
	//send the response back to client
	int send_status = send_http_response(sockfd, header_200, filename);
	if (0 != send_status) {
		log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
		fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client" ANSI_COLOR_RESET "\n");
	}

	// delete the file name
	char *trash_dir = g_hash_table_lookup(*hash, "trash");
	if (NULL == trash_dir) {
		trash_dir = g_strdup("../../trash/");
	}
	char *arr = g_strconcat("mv ", filename, " ", trash_dir, NULL);
	// system call to move the file to trash
	int a = system(arr);
	// error checking
	if (-1 != a) {
		printf ("File Moved to Trash. Deleted from www successfully\n");
		// log info to log file
		char *log_data = g_strconcat ("[client ", client_ip_address, "] [DELETE] [File Deleted] ", filename, NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);
	} else {
		printf ("File cannot be moved to trash. Cannot be deleted successfully\n");
		// log info to log file
		char *log_data = g_strconcat("[client ", client_ip_address, "] [DELETE] [File Cannot Be Deleted] ", filename, NULL);
		log_server_data(INFO, log_data);
		g_free(log_data);
	}
	g_free(filename);
	g_free(arr);
	g_free(trash_dir);
	g_free(header_200);

	// remove client from select watching list
	FD_CLR(sockfd, &(*master));

	return 0;
}

