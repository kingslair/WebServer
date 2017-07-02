#include "http_server_class_request_handler_base.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_status_codes.h"
#include "http_server_class_helper.h"
#include "http_server_class_log.h"
/******************************************************************************
 * function 	  : serve_request
 *
 * arguments 	  : int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> &map,char *client_ip_address, 
 * 		            unordered_map<string, string> &config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map
 *
 * description 	: check the requested method and call specified function 
 * 		            to handle that request
 *
 * return 	    : integer
 * ****************************************************************************/

int http_request_handler::serve_request(int sockfd, fd_set *master, 
unordered_map<string, string> &map, char *client_ip_address, 
unordered_map<string, string> &config_map, unordered_map<string, REDIRECT*> redirect_map) {
	
	// create http_request_handler class object
	http_request_handler request_handler;

	// create a http_status_codes class object
	http_status_codes http_status;

	// create a http_helper class object
	http_helper helper;

	// create a log class object
	log logger;
	
	// get request method from the map
	auto request_method = map.find("method");

	char request_type[20];
	int len = request_method->second.size();
	int i = 0;
	for (i = 0; i < len; i++) {
		request_type[i] = request_method->second[i];
	}
	if (0 == strncmp("GET", request_type, 3)) {		// check if method is GET
		int GET_status = request_handler.handle_GET_request(sockfd, master, map, 0, 
    client_ip_address, config_map, redirect_map);
		if (0 != GET_status) {
		 return 1;
		}

	} else if (0 == strncmp("POST", request_type, 4)) {	// check if method is POST
		int POST_status = request_handler.handle_POST_request(sockfd, master, map, 
    client_ip_address, config_map, redirect_map);
		if (0 != POST_status) {
		  return 1;
		}

	} else if (0 == strncmp("PUT", request_type, 3)) {	// check if method is PUT
		int PUT_status = request_handler.handle_PUT_request(sockfd, master, map, 
    client_ip_address, config_map, redirect_map);
		if (0 != PUT_status) {
		   return 1;
		}

	} else if (0 == strncmp("HEAD", request_type, 4)) {	// check if method is HEAD
		int HEAD_status = request_handler.handle_GET_request(sockfd, master, map, 1, 
    client_ip_address, config_map, redirect_map);
		if (0 != HEAD_status) {
		   return 1;
		}

	}
	else if (0 == strncmp("DELETE", request_type, 6)) {	// check if method is DELETE
		int DELETE_status = request_handler.handle_DELETE_request(sockfd, master, 
    map, client_ip_address, config_map, redirect_map);
		if (0 != DELETE_status) {
		    return 1;
		}

	}
	else if (0 == strncmp("OPTIONS", request_type, 7)) {	// check if method is OPTIONS
		int OPTIONS_status = request_handler.handle_OPTIONS_request(sockfd, master, 
    map, client_ip_address, config_map, redirect_map);
		if (0 != OPTIONS_status) {
		   return 1;
		}
	} 
	else {
		// if any other method return response
		// 501 Not Implemented
		// incase method is not one defined above than send 501
		// Not Implemented error
		string log_data;
		auto base_directory = config_map.find("BASE_DIR");
		if (base_directory == config_map.end()) {
			base_directory->second = "../../www";
		}
		char base_dir[MAX_SIZE];
		memset(base_dir, 0, MAX_SIZE);
		int len_base_dir = base_directory->second.size();
		for (i = 0; i < len_base_dir; i++) {
			base_dir[i] = base_directory->second[i];
		}   
		char *file_path_501 = NULL;
    try { file_path_501 = new char[MAX_SIZE]; } 
    catch(bad_alloc&bd) { 
      FD_CLR(sockfd, &(*master)); 
       return 1;
    } 
		memset(file_path_501, 0, MAX_SIZE);
		strcat(file_path_501, base_dir);
		strcat(file_path_501, "/error/501.html");
    log_data = "[Client " + string(client_ip_address) + "] " + "[501 NOT IMPLEMENTED]";
		logger.log_server_data(INFO, log_data);

		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [501 NOT IMPLEMENTED]"\
    ANSI_COLOR_RESET "\n", client_ip_address);


		char *header_501 = http_status.get_http_response_501(file_path_501);
    if (NULL == header_501) {
      delete[] file_path_501;
      FD_CLR(sockfd, &(*master));
      return 0;
    }
		int send_status = helper.send_http_response(sockfd, header_501, file_path_501);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 501 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 501 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		delete[] header_501;
		delete[] file_path_501;
    close(sockfd);
		FD_CLR(sockfd, &(*master));
	}
	return 0;
}
