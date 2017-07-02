#include "http_server_class_log.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_helper.h"
#include "http_server_class_status_codes.h"

/*******************************************************************************
 * function 	  : parse_GET_data
 *
 * arguments 	  : int sockfd, char *data, 
 * 		            unordered_map<string, string> config_hash
 *
 * description 	: store data present in url in GET request into data file
 * 		            and send response to client
 *
 * return 	    : integer (status of the operation)
 * ****************************************************************************/
int http_request_handler::parse_GET_data(int sockfd, char *data, 
unordered_map<string, string> config_map) {

	// create a log class object
	log logger;

	// create a http_helper class object
	http_helper helper;
	
	// create a http_status_codes class object
	http_status_codes http_status;

	char time_string[20];
	sprintf(time_string, "%lu", time(NULL));

	// looking up base directory in config map
	auto base_dir = config_map.find ("BASE_DIR");

	// creating file path to store GET request data in json file
	string data_filename;
	data_filename = "/GET/data_" + string(time_string) + ".data";

	string data_filename_path = base_dir->second + string(data_filename);

	ofstream write_data (data_filename_path);
	if (!write_data.is_open()) {
		// incase of error in creating file log this to log file
		logger.log_server_data(WARNING, "[ERROR IN CREATING DATA FILE] www/GET/");
		// log this error to stderr also
		fprintf(stderr, ANSI_COLOR_MAGENTA "Error in creating DATA file to store GET\
    request data" ANSI_COLOR_RESET "\n");
		return 1;
	}

	// data divided at '&'
	int len = strlen (data);
	int i = 0;
	for (i = 0; i < len; i++) {
		if (i == 0) {
			char *first_token = strtok (data, "&");
			write_data << first_token << endl;
			write_data.flush();
		}
		else {
			char *second_token = strtok (NULL, "&");
			write_data << second_token << endl;	
			write_data.flush();
		}
	}
	write_data.close();
	char file_path[MAX_SIZE];
	memset(file_path, 0, MAX_SIZE);
	int length = data_filename_path.size();
	for (i = 0; i < length; i++) {
		file_path[i] = data_filename_path[i];	
	}
	// send created json file to client, so that it can check data created
	char *data_response_200 = http_status.get_http_response_200(file_path);
  if (NULL == data_response_200) {
    return 0;
  }
	int send_status = helper.send_http_response(sockfd, data_response_200, file_path);
	if (0 != send_status) {
		logger.log_server_data(SEVERE, "Error in sending DATA file to Client");
		fprintf(stderr, ANSI_COLOR_RED "Error in sending DATA file to Client"\
    ANSI_COLOR_RESET "\n");
	}
	delete[] data_response_200;
    return 0;
}
/**********************************************************************
 * function 	  : handle_GET_request
 *
 * arguments 	  : int sockfd, fd_set *master, 
 * 		            unordered_map<string, string> map, int is_head, 
 * 		            char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map 
 *
 * description 	: handle HTTP GET request
 *
 * return 	    : integer (status of the operation)
 * *******************************************************************/
int http_request_handler::handle_GET_request(int sockfd, fd_set *master, 
unordered_map<string, string> map, int is_head, char *client_ip_address, 
unordered_map<string, string> config_map, unordered_map<string, REDIRECT*> redirect_map) {

	// lookup url in the map, it may contain data also
	auto url_with_data = map.find("url");
	int len = url_with_data->second.size();
	char token[1024];
	memset(token, 0, 1024);
	int i = 0;
	for (i = 0; i < len; i++) {
		token[i] = url_with_data->second[i];
	}
	char *w_filename = NULL;
  try { w_filename = new char[MAX_SIZE]; }
  catch(bad_alloc& bd) { 
     FD_CLR(sockfd, &(*master));	
     close(sockfd);
     return 1; 
  }
	memset(w_filename, 0, MAX_SIZE);

	for (i = 0; i < len; i++) {
		w_filename[i] = url_with_data->second[i];
	}
	
	// url will contain data after "?" so we are splitting at "?"
	// example: /hello.html?name=john+doe
	// tokenized: [/hello.html] [name=jhon+doe]
	if (strstr(token, "?") != NULL) {
		char *first_token = strtok(token, "?");
	  char *second_token = NULL;
    try { second_token = new char[MAX_SIZE]; }
    catch(bad_alloc& bd) { 
      FD_CLR(sockfd, &(*master));
		  close(sockfd);
      delete[] w_filename;
      return 1; 
    }
		second_token = strtok(NULL, "?");
		first_token = NULL;
		// parse data present in url
		if (0 != parse_GET_data(sockfd, second_token, config_map)) {
			fprintf(stderr, "Error in Parsing data present in GET request\n");
		}

	}
	// create a http_helper class object
	http_helper helper;

	// create a log class object
	log logger;

	// create a http_status_codes class object
	http_status_codes http_status;

	// remove "/" from url
	// example: "/hello.html" ---->  "hello.html"
	char *filename = helper.get_valid_file_path(w_filename, sockfd, master, 
  client_ip_address, config_map, redirect_map, map);
	//g_free(unsanitized_filename);
	if (NULL == filename) {
		FD_CLR(sockfd, &(*master));
		close(sockfd);
		delete[] w_filename;
		return 1;
	}

	// create a 200 OK http response	
	char *header_200 = http_status.get_http_response_200(filename);
  if (NULL == header_200) {
    delete[] w_filename;
    free(filename);
    return 0;
  }
	if (1 != is_head) {
		int send_status = helper.send_http_response(sockfd, header_200, filename);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 data to Client"\
      ANSI_COLOR_RESET "\n");
		}
	} else {
		int send_status = helper.send_http_response(sockfd, header_200, NULL);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
	}
	delete[] header_200;
	delete[] w_filename;
	free(filename);
	return 0;
}
