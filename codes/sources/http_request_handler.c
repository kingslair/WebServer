#include "http_request_handler.h"

/**********************************************************************
 * function 	: serve_request
 *
 * arguments 	: int sockfd, fd_set *master, GHashTable **hash, 
 * 		  char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash
 *
 * description 	: check the requested method and call specified function 
 * 		  to handle that request
 *
 * return 	: integer
 *
 * *******************************************************************/
int serve_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash) {
    // get request method from hash table
    char *request_method = g_hash_table_lookup(*hash, "method");

    if (0 == strcmp("GET", request_method)) {		// check if method is GET
        int GET_status = handle_GET_request(sockfd, &(*master), &(*hash), 0, client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != GET_status) {
            return 1;
        }

    } else if (0 == strcmp("POST", request_method)) {	// check if method is POST
        int POST_status = handle_POST_request(sockfd, &(*master), &(*hash), client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != POST_status) {
            return 1;
        }

    } else if (0 == strcmp("PUT", request_method)) {	// check if method is PUT
        int PUT_status = handle_PUT_request(sockfd, &(*master), &(*hash), client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != PUT_status) {
            return 1;
        }

    } else if (0 == strcmp("HEAD", request_method)) {	// check if method is HEAD
        // handle HEAD request here
        int HEAD_status = handle_GET_request(sockfd, &(*master), &(*hash), 1, client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != HEAD_status) {
            return 1;
        }

    }
    else if (0 == strcmp("DELETE", request_method)) {	// check if method is HEAD
        int DELETE_status = handle_DELETE_request(sockfd, &(*master), &(*hash), client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != DELETE_status) {
            return 1;
        }

    }
    else if (0 == strcmp("OPTIONS", request_method)) {	// check if method is HEAD
        int OPTIONS_status = handle_OPTIONS_request(sockfd, &(*master), &(*hash), client_ip_address, &(*config_hash), &(*redirect_hash));
        if (0 != OPTIONS_status) {
            return 1;
        }

    }
    else {						// if any other method return response
        // 501 Not Implemented

        // incase method is not one defined above than send 501
        // Not Implemented error
        char *base_dir = g_hash_table_lookup(*config_hash, "BASE_DIR");
        char *file_path_501 = g_strconcat(base_dir, "/error/501.html", NULL);
        char *header_501 = get_http_response_501(file_path_501);
        int send_status = send_http_response(sockfd, header_501, file_path_501);
        if (0 != send_status) {
            log_server_data(SEVERE, "Error in sending HTTP 501 response to Client");
            fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 501 response to Client" ANSI_COLOR_RESET "\n");
        }

        g_free(file_path_501);
        g_free(header_501);
        FD_CLR(sockfd, &(*master));
    }
    return 0;
}


