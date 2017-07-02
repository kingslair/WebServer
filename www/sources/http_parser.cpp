#include "http_server_class_log.h"
#include "http_server_class_parser.h"
#include "http_server_class_status_codes.h"
#include "http_server_class_helper.h"
#include "http_server_class_helper_base.h"
/******************************************************************************
 * function 	  : check_bad_request 
 *
 * arguments 	  : string first_token, 
 * 		            unordered_map<string, string> &config_map, 
 * 		            char *client_ip_address 
 *
 * description 	: checks whether a 400 Bad Request Error has occured.
 *
 * return 	    : integer
 * ****************************************************************************/
int http_parser::check_bad_request_400 (string first_token, 
unordered_map<string, string> &config_map, char *client_ip_address)
{
	// create a log class object
	log logger;	

	// create a http_status_codes class object
	http_status_codes http_status;

	// create a http_helper class object
	http_helper helper;

	string log_data;
	int i = 0;
	char word[MAX_SIZE];
  memset(word, 0, 1024);
	int len = first_token.size();
	for (i = 0; i <= len; i++) {
		word[i] = first_token[i];
	}

	// if 400 Bad Request
	if (word[len] ==  ' ') {	
		// log string filled with data 
		log_data = "[client " + string(client_ip_address) + "] [400 BAD REQUEST]";
		// write to log file
		logger.log_server_data(INFO, log_data);

		fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [400 BAD REQUEST]"\
    ANSI_COLOR_RESET "\n", client_ip_address);

		auto base_directory = config_map.find("BASE_DIR");	
		// get 400 headers 
		if (base_directory == config_map.end()) {
			base_directory->second = "../../www";
		}
		char base_dir[MAX_SIZE];
		memset(base_dir, 0, MAX_SIZE);
		// find the length of the string
		int len_base_dir = base_directory->second.size();
		for (i = 0; i < len_base_dir; i++) {
			base_dir[i] = base_directory->second[i];
		}
		char* file_path_400 = NULL;
    try { file_path_400 = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { 
       return 1;
    } 
		memset(file_path_400, 0, MAX_SIZE);
		strcat(file_path_400, base_dir);
		strcat(file_path_400, "/error/400.html");
		// get the 400 header
		char* header_400 = http_status.get_http_response_400(file_path_400);
    if (NULL == header_400) {
      delete[] file_path_400;
		  return -1;
    }
		// send headers and file content to client
		int send_status = helper.send_http_response(sockfd, header_400, file_path_400);
		if (0 != send_status) {
			// log the data
			logger.log_server_data(SEVERE, "Error in sending HTTP 400 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 400 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		// free the allocated variable
		delete[] header_400;
    delete[] file_path_400;
		return -1;
	}
	else {
		return 0;
	}

}

/*******************************************************************************
 * function 	  : http_parse_status_line
 *
 * arguments 	  : char *status_line, unordered_map<string, string> map, 
 * 		            int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> config_map, 
 * 		            char *client_ip_address
 *
 * description 	: extract status line from request.
 *
 * return 	    : integer
 *
 * ****************************************************************************/
int http_parser::http_parse_status_line(char *status_line, 
unordered_map<string, string> &map, int sockfd, fd_set *master, 
unordered_map<string, string> config_map, char *client_ip_address) {
	// create a log file object
	log logger;
	// create a http_status_code class object
	http_status_codes http_status;
	// create a http_helper clas object
	http_helper helper;
	int i = 0;

	// split status line at " "
	// example: GET /url HTTP/1.1
	// tokenized into: [GET] [/url] [HTTP/1.1]
	if (NULL == status_line) {
		return -1;
	}
	string log_data;
	char *first_token = NULL;
	char *second_token = NULL;	
	char *third_token = NULL;
	first_token = strtok(status_line, " ");
	second_token = strtok(NULL, " ");
	third_token = strtok(NULL, " ");
	
	if (NULL == second_token) {
		return -1;
	}

  if (NULL == first_token || NULL == second_token || NULL == third_token) {
    // log string filled with data 
		log_data = "[Client " + string(client_ip_address) + "] [400 BAD REQUEST]";
		// write to log file
		logger.log_server_data(INFO, log_data);

		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [400 BAD REQUEST]"\
    ANSI_COLOR_RESET "\n", client_ip_address);

		auto base_directory = config_map.find("BASE_DIR");	
		// get 400 headers 
		if (base_directory == config_map.end()) {
			base_directory->second = "../../www";
		}
		char base_dir[MAX_SIZE];
		memset(base_dir, 0, MAX_SIZE);
		// find the length of the string
		int len_base_dir = base_directory->second.size();
		for (i = 0; i < len_base_dir; i++) {
			base_dir[i] = base_directory->second[i];
		}
		char* file_path_400 = NULL;
    try { file_path_400 = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { 
       return 1;
    } 
		memset(file_path_400, 0, MAX_SIZE);
		strcat(file_path_400, base_dir);
		strcat(file_path_400, "/error/400.html");
		// get the 400 header
		char* header_400 = http_status.get_http_response_400(file_path_400);
    if (NULL == header_400) {
      delete[] file_path_400;
		  return -1;
    }
		// send headers and file content to client
		int send_status = helper.send_http_response(sockfd, header_400, file_path_400);
		if (0 != send_status) {
			// log the data
			logger.log_server_data(SEVERE, "Error in sending HTTP 400 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 400 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		// free the allocated variable
		delete[] header_400;
    delete[] file_path_400;
		return -1;
  }

	// split http version at "/"
	// example: HTTP/1.1
	// tokenized into: [HTTP] [1.1]
	char *first_http_version_line = strtok(third_token, "/");
	char *second_http_version_line = NULL;
	second_http_version_line = strtok(NULL, "/");
	first_http_version_line= NULL;	
	if (0 != strcmp("1.1", second_http_version_line)) {
		log_data = "[Client " + string(client_ip_address) + "] " + "[505 HTTP VERSION NOT SUPPORTED]";
		logger.log_server_data(INFO, log_data);

		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [505 HTTP VERSION NOT SUPPORTED]"\
    ANSI_COLOR_RESET "\n", client_ip_address);

		auto base_directory = config_map.find("BASE_DIR");	
		// get 400 headers 
		if (base_directory == config_map.end()) {
			base_directory->second = "../../www";
		}
		char base_dir[MAX_SIZE];
		memset(base_dir, 0, MAX_SIZE);
		// find the length of the string
		int len_base_dir = base_directory->second.size();
		for (i = 0; i < len_base_dir; i++) {
			base_dir[i] = base_directory->second[i];
		}
		char *file_path_505 = NULL;
    try { file_path_505 = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { 
       return 1;
    } 
		memset(file_path_505, 0, MAX_SIZE);
		strcat(file_path_505, base_dir);
		strcat(file_path_505, "/error/505.html");
    // get the 505 headers
		char *header_505 = http_status.get_http_response_505(file_path_505);
    if (NULL == header_505) {
		  delete[] file_path_505;
		  close(sockfd);
		  FD_CLR(sockfd, &(*master));
		  return -1;
    }
		int send_status = helper.send_http_response(sockfd, header_505, file_path_505);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 505 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 505 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		delete[] file_path_505;
		delete[] header_505;
		close(sockfd);
		FD_CLR(sockfd, &(*master));
		return -1;
	}

	// store request method in hash table (GET, POST and etc)
	map.insert({"method", string(first_token)});
	// store url into hash table
	map.insert({"url", string(second_token)});
	return 0;
}
/******************************************************************************
 * function 	  : http_parse_header_fields
 *
 * arguments 	  : char *fields, unsigned int length, 
 * 		            unordered_map<string, string> &map, 
 * 		            int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> &config_map, 
 * 		            char *client_ip_address
 *
 * description 	: extract the header fields from the request.
 *
 * return 	    : integer
 * ****************************************************************************/

int http_parser::http_parse_header_fields(char *fields, unsigned int length, 
unordered_map<string, string> &map, int sockfd, fd_set *master, 
unordered_map<string, string> &config_map, char *client_ip_address) {

	// split the request leaving the status line
	unsigned int i;
	char *token = strtok(fields, "\r\n");
	// loop over the next set of headers fields 
	for (i = 1; i < length-1; i++) {
		char* get_line = NULL;
		// split ar '\r\n'
		get_line = strtok(NULL, "\r\n");
		if (get_line == NULL || strcmp(get_line,"") == 0) {
			continue;
		}
		// create a duplicate of the original string
		char *new_get_line = strdup(get_line);
		string strFromChar;
		strFromChar.append(new_get_line);
		if (strFromChar.empty()) {
			continue;
		}

		// convert the char* to string
		int pos = strFromChar.find_first_of(':');
		string last_token = strFromChar.substr(pos+1),
		       first_token = strFromChar.substr(0, pos);
		string output_str = first_token + " " + last_token;
		
		// check for 400 Bad Request
		int check_request_400 = check_bad_request_400(first_token, config_map, 
    client_ip_address);
		if (check_request_400 == -1) {
			// clear the allocated memory
			free(new_get_line);
			// close the socket connection
			close(sockfd);
			FD_CLR(sockfd, &(*master));
			return -1;
		}
		else if (check_request_400 == 0) {
			// insert into the map
			map.insert ({first_token, last_token});
		}
		// free the allocated variable
		free(new_get_line);
	}
	token = NULL;
	return 0;
}

/******************************************************************************
 * function 	  : http_parse_request_header
 *
 * arguments  	: char *request, int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> &config_hash, 
 * 	 	            char *client_ip_address
 *
 * description 	: extracts the request header.
 *
 * return     	: unordered map
 *
 * ****************************************************************************/
unordered_map<string, string> http_parser::http_parse_request_header(char *request, 
int sockfd, fd_set *master, unordered_map<string, string> &config_map, 
char *client_ip_address) {
	// create unordered map to store request fields
	unordered_map<string, string> request_map;
	
	if (NULL == request) {
		return request_map;
	}
	// create a duplicate of the complete request string
	char *new_request = strdup (request);
	char *line = strtok (request, "\r\n");
	// find number of lines in request
	int length = strlen(new_request);

	// parse status line
	int status_line_parsed = http_parse_status_line(line, request_map, sockfd, 
  &(*master), config_map, client_ip_address);
	if (0 != status_line_parsed) {
		//clear the request map
		free(new_request);
		request_map.clear();
		return request_map;
	}

	// parse header fields
	int header_fields_parsed = http_parse_header_fields(new_request, length, 
  request_map, sockfd, &(*master), config_map, client_ip_address);
	if (0 != header_fields_parsed) {
		//clear the request map
		free(new_request);
		request_map.clear();
		return request_map;
	}
  free(new_request);
	return request_map;
}

