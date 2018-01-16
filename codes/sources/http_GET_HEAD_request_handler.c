#include "http_GET_HEAD_request_handler.h"

/**********************************************************************
 * function 	: parse_GET_data_to_json
 *
 * arguments 	: int sockfd, char *data, GHashTable **config_hash
 *
 * description 	: store data present in url in GET request into json file
 * 		  and send response to client
 *
 * return 	: integer (status of the operation)
 *
 * *******************************************************************/
int parse_GET_data_to_json(int sockfd, char *data, GHashTable **config_hash) {
    // data divided at '&'
    char **data_divided_amper = g_strsplit_set(data, "&", -1);
    // number of data entries
    unsigned int len = g_strv_length(data_divided_amper);

    char time_string[20];
    sprintf(time_string, "%lu", time(NULL));

    // looking up base directory in config hash
    char *base_dir = g_strdup(g_hash_table_lookup(*config_hash, "BASE_DIR"));
    if (NULL == base_dir) {
        base_dir = g_strdup("www");
    }
    // creating file path to store GET request data in json file
    char *json_filename = g_strconcat("/GET/data_", time_string, ".json", NULL);
    char *json_filename_path = g_strconcat(base_dir, json_filename, NULL);
    g_free(base_dir);

    FILE *fp = fopen(json_filename_path, "w");
    if (NULL == fp) {
        // incase of error in creating file log this to log file
        log_server_data(WARNING, "[ERROR IN CREATING JSON FILE] www/GET/");
        // log this error to stderr also
        fprintf(stderr, ANSI_COLOR_MAGENTA "Error in creating JSON file to store GET request data" ANSI_COLOR_RESET "\n");

        // free all the resources
        g_free(json_filename_path);
        g_free(json_filename);
        g_strfreev(data_divided_amper);

        return 1;
    }

    // storing data in json format
    fprintf(fp, "{\n");
    unsigned int i;
    for (i = 0; i < len; i++) {
        // split individual entry at "=", which divide it in key value pairs
        char **splitted_single_data_entry = g_strsplit_set(data_divided_amper[i], "=", 2);
        fprintf(fp, "\t\"%s\": \"%s\"", splitted_single_data_entry[0], splitted_single_data_entry[1]);

        // applying ',' at end of all key-value pairs except last one
        if (i == len-1) {
            fprintf(fp, "\n");
        } else {
            fprintf(fp, ",\n");
        }
        // free resource which we have created in this iteration
        g_strfreev(splitted_single_data_entry);
    }
    fprintf(fp, "}\n");

    // free data, we dont need this anymore
    g_strfreev(data_divided_amper);

    // closing file with error handling support
    int is_close = fclose(fp);
    if (0 != is_close) {
        // if error in closing file log this to log file
        log_server_data(WARNING, "[FILE CANNOT BE CLOSED] www/GET/");
        fprintf(stderr, ANSI_COLOR_MAGENTA "Error in closing JSON file creating during GET request" ANSI_COLOR_RESET "\n");
    }

    // send created json file to client, so that it can check data created
    char *JSON_response_200 = get_http_response_200(json_filename_path);
    int send_status = send_http_response(sockfd, JSON_response_200, json_filename_path);
    if (0 != send_status) {
	   log_server_data(SEVERE, "Error in sending json data file to Client");
	   fprintf(stderr, ANSI_COLOR_RED "Error in sending json data file to Client" ANSI_COLOR_RESET "\n");
    }

    // free all the resource
    g_free(json_filename_path);
    g_free(json_filename);
    g_free(JSON_response_200);

    return 0;
}

/**********************************************************************
 * function 	: handle_GET_request
 *
 * arguments 	: int sockfd, fd_set *master, GHashTable **hash, int is_head, 
 * 		  char *client_ip_address, GHashTable** config_hash, 
 * 		  GHashTable** redirect_hash
 *
 * description 	: handle HTTP GET request
 *
 * return 	: integer (status of the operation)
 *
 * *******************************************************************/
int handle_GET_request(int sockfd, fd_set *master, GHashTable **hash, int is_head, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash) {

    // lookup url in hash table, it may contain data also
    char *url_with_data  = g_hash_table_lookup(*hash, "url");

    // url will contain data after "?" so we are splitting at "?"
    // example: /hello.html?name=john+doe
    // tokenized: [/hello.html] [name=jhon+doe]
    char **tokenized_url = g_strsplit_set(url_with_data, "?", 2);

    // variable to store unsanitized filename
    char *unsanitized_filename = g_strdup(tokenized_url[0]);

    // if there is data present process it
    if (NULL != tokenized_url[1]) {
        // store a copy
        char *data_in_url = g_strdup(tokenized_url[1]);
        // parse data present in url
        if (0 != parse_GET_data_to_json(sockfd, data_in_url, &(*config_hash))) {
            fprintf(stderr, "Error in Parsing data present in GET request\n");
        }
        g_free(data_in_url);
        data_in_url = NULL;
    }
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

    char *header_200 = get_http_response_200(filename);
    if (1 != is_head) {
       int send_status = send_http_response(sockfd, header_200, filename);
       if (0 != send_status) {
	       log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
	       fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 data to Client" ANSI_COLOR_RESET "\n");
       }
    } else {
       int send_status = send_http_response(sockfd, header_200, NULL);
       if (0 != send_status) {
	       log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
	       fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client" ANSI_COLOR_RESET "\n");
       }
    }
    g_free(header_200);
    g_free(filename);

    return 0;
}

