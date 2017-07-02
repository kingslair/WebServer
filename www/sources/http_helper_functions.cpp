#include "http_server_class_helper.h"
#include "http_server_class_log.h"
#include "http_server_class_status_codes.h"
/******************************************************************************
 * function 	  : get_date_time()
 *
 * arguments 	  : void
 *
 * description 	: returns the current date and time.
 *
 * return 	    : string
 *
 * ****************************************************************************/
string http_helper_base::get_date_time() {
	// buffer to store date and time
	char datestring[SIZE];
	// get current time
	time_t t = time(NULL);
	// convert time into localtime
	struct tm *time = localtime(&t);

	// format time according to RFC
	strftime(datestring, SIZE, "%a, %d %b %Y %T %z", time);
	return string(datestring);
}

/******************************************************************************
 * function 	  : get_file_size
 *
 * arguments 	  : char *filename
 *
 * description 	: returns the file size of a resource.
 *
 * return 	    : integer
 * ****************************************************************************/
int get_file_size(char *filename) {
	// to store file stats
	struct stat file_stat;

	// get file stats
	if (-1 == stat(filename, &file_stat)) {
		fprintf(stderr, ANSI_COLOR_RED "Error in getting file size" ANSI_COLOR_RESET "\n");
		return -1;
	}

	// return file size
	return file_stat.st_size;
}

/******************************************************************************
 * function 	  : get_last_modified_date
 *
 * arguments 	  : char *filename
 *
 * description 	: returns the last modified date of the resource.
 *
 * return 	    : string
 * ****************************************************************************/
string http_helper_base::get_last_modified_date(char *filename) {
	// to store file stats
	struct stat file_stat;
	// buffer to store date and time
	char last_mdate[SIZE];

	// get file stats
	if (-1 == stat(filename, &file_stat)) {
		fprintf(stderr, "Getting last modified date failed\n");
		return NULL;
	}

	// convert milliseconds into localtime and store them into struct tm
	struct tm *time = localtime(&(file_stat.st_mtime));
	// format time according to RFC
	strftime(last_mdate, SIZE, "%a, %d %b %Y %T %z", time);

	return string(last_mdate);
}

/******************************************************************************
 * function 	  : get_file_size
 *
 * arguments  	: char *filename
 *
 * description 	: returns the file size of a resource.
 *
 * return     	: integer
 * ****************************************************************************/
int http_helper_base::get_file_size(char *filename) {
	// to store file stats
	struct stat file_stat;

	// get file stats
	if (-1 == stat(filename, &file_stat)) {
		fprintf(stderr, ANSI_COLOR_RED "Error in getting file size" ANSI_COLOR_RESET "\n");
		return -1;
	}

	// return file size
	return file_stat.st_size;
}

/******************************************************************************
 * function 	  : str_to_int
 *
 * arguments  	: char *number
 *
 * description 	: converts a string a integer.
 *
 * return     	: integer
 * ****************************************************************************/
int http_helper_base::str_to_int(char *number) {
	char *endptr;
	int val = strtod(number, &endptr);

	if (endptr == number) {
		fprintf(stderr, ANSI_COLOR_RED "No digits are found" ANSI_COLOR_RESET "\n");
		return -1;
	}

	return val;
}

/******************************************************************************
 * function   	: get_content_type
 *
 * arguments  	: char *filename
 *
 * description 	: returns the content type of the resource.
 *
 * return     	: string
 * ****************************************************************************/
string http_helper_base::get_content_type(char *filename) {
	// split filename at "."
	// example: index.html
	// tokenized: [index] [html]

	unsigned int i;
	unsigned int loc = 0;
	for (i = 0; filename[i] != '\0'; i++) {
		if ('.' == filename[i]) {
			loc = i;
		}
	}

	char *file_ext = strdup(filename+loc);

	// to store content type of a file
	//char *content_type = NULL;
	char content_type[SIZE];

	if (0 == strcmp(".html", file_ext)) {		// check if file type is html
		strcpy(content_type, "text/html");
	} else if (0 == strcmp(".css", file_ext)) {		// check if file type is css
		strcpy(content_type, "text/css");
	} else if (0 == strcmp(".pdf", file_ext)) {		// check if file type is pdf
		strcpy(content_type, "application/pdf");
	} else if (0 == strcmp(".jpeg", file_ext) || (0 == strcmp(".jpg", file_ext))) {	// check if file type is jpeg
		strcpy(content_type, "image/jpeg");
	} else if (0 == strcmp(".png", file_ext)) {		// check if file type is png
		strcpy(content_type, "image/png");
	} else if (0 == strcmp(".js", file_ext)) {		// check if file type is javascript
		strcpy(content_type, "application/javascript");
	} else if (0 == strcmp(".gif", file_ext)) {		// check if file type is gif
		strcpy(content_type, "image/gif");
	} else if (0 == strcmp(".json", file_ext)) {		// check if file type is json
		strcpy(content_type, "text/json");
	} else {
		strcpy(content_type, "text/plain");			// if not from above types
		// use text/plain as default
	}

	free(file_ext);
	return string(content_type);
}

/******************************************************************************
 * function 	  : send_http_response
 *
 * arguments 	  : int sockfd, char *headers, char *filename
 *
 * description 	: sends the requested resource to the client.
 *
 * return 	    : integer
 * ****************************************************************************/
int http_helper::send_http_response(int sockfd, char *headers, char *filename) {
	
	// create a log class object
	log logger;	

	size_t len = strlen(headers);
	// send headers first
	send(sockfd, headers, len, 0);
	string log_data;
	if (NULL != filename) {
		// open a file
		FILE *fp = fopen(filename, "r");
		if (NULL == fp) {
			// if error happen in reading file log it to stderr and log file
			log_data = "Error in reading file " + string(filename);
			logger.log_server_data(SEVERE, log_data);

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
		fclose (fp);
	}
	// end http response by sending "\r\n"
	write(sockfd, "\r\n", 2);
	return 0;
}

/******************************************************************************
 * function   	: http_page_access_rights
 *
 * arguments  	: char *page_url
 *
 * description 	: checks the access rights of a particular resource.
 *
 * return 	    : integer
 * ****************************************************************************/
int http_helper_base::http_page_access_rights(char *page_url) {
	
	// variable to store the page/file URL
	int response = 0;
	struct stat buf;

	stat(page_url,&buf);

	// Holds the file permission temporary before
	// writing to memory  
	char page_permission = (buf.st_mode & S_IROTH) ? 'r':'-';

	// check for file
	if ('r' == page_permission) {
		response = 1;
	}

	// return response to caller function 
	return response;
}

/******************************************************************************
 * function 	  : is_directory
 *
 * arguments 	  : char *file_path
 *
 * description 	: returns whether a resource is a directory or not.
 *
 * return 	    : integer
 * ****************************************************************************/
int http_helper_base::is_directory(char *file_path) {
	struct stat file_stat;

	FILE *fp = fopen(file_path, "r");
	if (NULL == fp) {
		return 0;
	}
	int is_close = fclose(fp);
	if (0 != is_close) {
		fprintf(stderr, ANSI_COLOR_MAGENTA "[FILE NOT CLOSED PROPERLY IN IS_DIRECTORY]"\
    ANSI_COLOR_RESET "\n");
	}

	if (-1 == stat(file_path, &file_stat)) {
		return -1;
	}

	return (S_ISDIR(file_stat.st_mode)) ? 1 : 0;
}
/******************************************************************************
 * function 	  : split_string
 *
 * arguments 	  : const char *line, const char *delm, int frags
 *
 * description 	: this function splits a string at a given delimiter
 *
 * return 	    : splitted string
 *
 * ****************************************************************************/
char** http_helper_base::split_string(const char *line, const char *delm, int frags) {
    // copy because we can't use const char* in strtok it
    // expects char*
    char *cp_line = strdup(line);
    char *cp_delm = strdup(delm);

    unsigned int buf_size = 10;
    unsigned int buf_index = 0;
    int frag_count = 0;

    // pointer to char array of char*
    char **splitted_strings = (char **)malloc(sizeof(char*) * buf_size);

    char *token = strtok(cp_line, cp_delm);

    // split and store in buffer
    while (token && (-1 == frags || frag_count < frags)) {
        splitted_strings[buf_index] = strdup(token);

        ++buf_index;
        ++frag_count;

        if (buf_index >= buf_size) {
            buf_size *= 1.5;
            splitted_strings = (char **)realloc(splitted_strings, buf_size);
        }

        token = strtok(NULL, cp_delm);
    }

    splitted_strings[buf_index] = 0;

    free(cp_delm);
    free(cp_line);

    return splitted_strings;
}
/******************************************************************************
 * function 	  : freev
 *
 * arguments 	  : char **arr
 *
 * description 	: this function iterates over the array to clear all
 * 		            the allocated memory
 *
 * return 	    : void
 * ****************************************************************************/
int http_helper_base::freev(char **arr) {
    char **it = arr;
    unsigned int i = 0;

    while (it[i]) {
        free(it[i]);
        ++i;
    }

    free(it);
    return 0;
}
/******************************************************************************
 * function 	  : string_reverse
 *
 * arguments 	  : char *str
 *
 * description 	: this function reverses a given character string
 *
 * return 	    : character array
 *
 * ****************************************************************************/
char* http_helper_base::string_reverse (char *str) {
	char temp;
	int i, j = 0;
	i = 0;
	j = strlen(str) - 1;

	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
	return str;
}

/******************************************************************************
 * function 	  : get_valid_file_path
 *
 * arguments 	  : char *url_with_data, int sockfd, fd_set *master, 
 * 		            char *client_ip_address, 
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map, 
 * 		            unordered_map<string, string> map)

 * description	: checks the validity of requested resource for file exists
 * 		            or not, client has permission or not and if redirect needs
 * 		            to perform to serve the resource.
 *
 * return 	    : string
 * ****************************************************************************/
char* http_helper::get_valid_file_path(char *url_with_data, int sockfd, fd_set *master, 
char *client_ip_address, unordered_map<string, string> config_map, 
unordered_map<string, REDIRECT*> redirect_map, unordered_map<string, string> map) {

	char original_url[MAX_SIZE];
	memset(original_url, 0, MAX_SIZE);
	int j = 0;
	int len_original_url = strlen (url_with_data);
	for (j = 0; j < len_original_url; j++) {
		original_url[j] = url_with_data[j];
	}	
	
	// create a log class object
	log logger;

	string log_data;
	// check if "?" present in url
	// incase of GET request data is present in url after "?"
	char *question_location = strstr(url_with_data, "?");
	//char *url = (char *)malloc(sizeof(char) * 1024);
	char url[MAX_SIZE];
	memset(url, 0, MAX_SIZE);
	auto request_method = map.find("method");
	char method_type[20];
	memset(method_type, 0, 20);
	int len = request_method->second.size();
	int i = 0;
	for (i = 0; i < len; i++) {
		method_type[i] = request_method->second[i];
	}
	auto root_dir = config_map.find("BASE_DIR");
	char base_dir[20];
	memset(base_dir, 0, 20);
	int len_dir = root_dir->second.size();
	for (i = 0; i < len_dir; i++) {
		base_dir[i] = root_dir->second[i];
	}
	
	// if "?" is found than split at it
	if (NULL != question_location) {
		// split at "?"
		// example: /hello.html?name=jhon
		// tokenized: [/hello.html] [name=jhon]
		char *first_token = strtok(url_with_data, "?");
		// create duplicate of url
		strcat(url, base_dir);
		strcat(url, first_token);
	} else {
		// if url is without data than use as it
		strcat(url, base_dir);
		strcat(url, url_with_data);
	}

	// checking "." to check if requested url is file or folder
	char url_with_index[MAX_SIZE];
	memset (url_with_index, 0, MAX_SIZE);
	// if its a folder
	if (strstr(original_url, ".") == NULL) {
		// add "/index2.html" after it
		// example: "jhon"  ---->  "jhon/index2.html"
		if ('/' == original_url[len_original_url-1]) {
			// log data 
			log_data = "[client " + string(client_ip_address) + "] [" + 
      string(method_type) + "] [Page Requested] " + string(url);
			logger.log_server_data(INFO, log_data);

			strcat(url_with_index, url);
			strcat(url_with_index, "index2.html");

			fprintf(stdout, ANSI_COLOR_YELLOW "[client %s] [%s] [Page Requested] %s"\
      ANSI_COLOR_RESET "\n", client_ip_address, method_type, url_with_index);
		} else {
			// log data
			log_data = "[client " + string(client_ip_address) + "] [" + 
      string(method_type) + "] [Page Requested] " + string(url);
			logger.log_server_data(INFO, log_data);
	
			strcat(url_with_index, url);
			strcat(url_with_index, "/");
			strcat(url_with_index, "index2.html");
			
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] %s"\
      ANSI_COLOR_RESET "\n", client_ip_address, method_type, url_with_index);
		}

		// change the url value which it was pointing previously, now its pointing to url
		// with index2.html page
		strcpy(url, url_with_index);
	}

	// checking for 404 error if the page/resource doesn't exist 
	FILE *page_exist = fopen(url, "r");
	if (NULL == page_exist) {
		// if file does not exists log this to log file
		log_data = "[client " + string(client_ip_address) + "] [" + 
    string(method_type) + "] [Page Requested] 404 ERROR " + string(url);
		logger.log_server_data(INFO, log_data);
		// log this to stderr
		fprintf(stderr, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] 404 ERROR %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
			
		// creating file path to 404 page
		char *file_path_404 = NULL;
    try { file_path_404 = new char[MAX_SIZE]; }
    catch(bad_alloc& bd) { 
       return NULL;
    } 
		memset(file_path_404, 0, MAX_SIZE);
    strcat(file_path_404, base_dir);
    strcat(file_path_404, "/error/404.html");

		// create a http_status_codes object
		http_status_codes http_status;

		// send http 404 response to client
		char *header_404 = http_status.get_http_response_404(file_path_404);
    if (NULL == header_404) {
      delete[] file_path_404;
      return NULL; 
    }
		int send_status = send_http_response(sockfd, header_404, file_path_404);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 404 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 404 response to Client"\
      ANSI_COLOR_RESET "\n");
		}

		// free all the resources	
		delete[] file_path_404;
		delete[] header_404;
		return NULL;
	}
	fclose (page_exist);	

	// sentinel value to indicate we need to redirect or not
	int redirect_page = 0;

	// Redirect Code 
	auto redirect_status = redirect_map.find(url);
	if (redirect_status == redirect_map.end()) {
		redirect_page = 0;
	}	
	else {
		char old_page_URL[MAX_SIZE];
		memset (old_page_URL, 0, MAX_SIZE);
		int len_old_page_URL = redirect_status->first.size();
		for (i = 0; i < len_old_page_URL; i++) {
			old_page_URL[i] = redirect_status->first[i];
		}

		// if url requested is one of redirect entry than change sentinel value
		if (NULL !=  ((REDIRECT *)redirect_status->second)->new_page_URL && 
    0 == strcmp(url, old_page_URL)) {
			redirect_page = 1;
		}
	}
	// create a http_status_codes class object
	http_status_codes http_status;

	// check if redirect required or not
	if (1 == redirect_page) {
	
		// pointer to http response string
		char *response = NULL;
		
		auto get_host = map.find("Host");
		if (get_host == map.end()) {
			get_host->second = "tsus007.gur.aricent.com:5000";
		}
		char host[MAX_SIZE], page_URL[MAX_SIZE];
		memset (host, 0, MAX_SIZE);
		memset (page_URL, 0, MAX_SIZE);
		int len_page_URL = strlen(((REDIRECT*)redirect_status->second)->new_page_URL);
		int len_get_host = get_host->second.size();
		int i = 0, j = 0;
		for (i = 1, j = 0; i < len_get_host; i++, j++) {
			host[j] = get_host->second[i];
		}
		for (i = 0; i < len_page_URL; i++) {
			page_URL[i] = ((REDIRECT*)redirect_status->second)->new_page_URL[i];
		}

		// check the redirect type and generate response according to that
		if (0 == strcmp(PERMANENTLY_MOVED, ((REDIRECT*)redirect_status->second)->redirect_code)) {
			// log Redirect into log file
			log_data = "[client " + string(client_ip_address) + "] [" + 
      string(method_type) + "] 308 REDIRECT PERMANENT MOVED" + string(url);
			logger.log_server_data(INFO, log_data);

			// log Redirect to stdout
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] 308 REDIRECT PERMANENT MOVED %s"\
      ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
			// preapre the response to send a 308 request 
			response = http_status.http_error_response_308(page_URL, host);
      if (response == NULL) {
          return NULL;
      }
		} else if (0 == strcmp(TEMPORARILY_MOVED, ((REDIRECT*)redirect_status->second)->redirect_code)) {
			// log Redirect into log file
			log_data = "[client " + string(client_ip_address) + "] [" + 
      string(method_type) + "] 307 REDIRECT TEMPORARILY MOVED" + string(url);
			logger.log_server_data(INFO, log_data);

			// log Redirect to stdout
			fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] 307 REDIRECT TEMPORARILY MOVED %s"\
      ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
			// preapre the response to send a 307 request
			response = http_status.http_error_response_307(page_URL, host);
      if (response == NULL) {
          return NULL;        
      }
		}
		// send response to client
		int send_status = send_http_response (sockfd, response, NULL);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP redirect response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP redirect response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		
		// free all allocated variables
		delete[] response;
		return NULL;
	}

	// check the access rights
	int access_rights = http_page_access_rights(url);

	// if we have access to the file
	if (1 == access_rights) {
		// log data to log file
		log_data = "[client " + string(client_ip_address) + "] [" + 
    string(method_type) + "] [Page Requested] " + string(url);
		logger.log_server_data(INFO, log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Requested] %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);
		// return the URL to client
		return strdup(url);
	} else {
		// log data to log file
		log_data = "[client " + string(client_ip_address) + "] [" + 
    string(method_type) + "] FORBIDDEN TO ACCESS " + string(url);
		logger.log_server_data(INFO, log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page forbidden to access] %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);

		// return 403 Forbidden response
		char *file_path = NULL;
    try { file_path = new char[MAX_SIZE]; }
    catch(bad_alloc& bd) { 
       return NULL;
    } 
		memset(file_path, 0, MAX_SIZE);
		strcat(file_path, base_dir);
		strcat(file_path, "/error/403.html");

		char *response = http_status.get_http_response_403(file_path);
    if (NULL == response) {
      delete[] file_path;
      return NULL;
    }
		// log data to log file
		log_data = "[client " + string(client_ip_address) + "] [" + 
    string(method_type) + "] [Page Served] 403 error " + string(url);
		logger.log_server_data(INFO, log_data);
		// log info to stdout
		fprintf(stdout, ANSI_COLOR_YELLOW "[Client %s] [%s] [Page Served] 403 ERROR %s"\
    ANSI_COLOR_RESET "\n", client_ip_address, method_type, url);

		// write to client the error response
		int send_status = send_http_response(sockfd, response, file_path);
		if (0 != send_status) {
			logger.log_server_data(SEVERE, "Error in sending HTTP 403 response to Client");
			fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 403 response to Client"\
      ANSI_COLOR_RESET "\n");
		}
		
		//free all allocated variables
		delete[] file_path;
		delete[] response;
		return NULL;
	}
	return strdup(url);
}

/**************************************************************************
 * function : clear_maps
 *
 * arguments : unordered_map<string, string> &config_map,
 *             unordered_map<string, REDIRECT*> &redirect_map
 *
 * decription : clears all the allocated memory in the map
 *
 * return : integer (status of the operation)
 * ***********************************************************************/

int http_helper_base::clear_maps(unordered_map<string, string> &config_map, 
unordered_map<string, REDIRECT*> &redirect_map) {
	// free the config map
	config_map.clear();
	// free the redirect map
	for (auto data = redirect_map.begin(); data != redirect_map.end(); data++) {
		delete[] ((REDIRECT *)data->second)->new_page_URL;
		delete[] ((REDIRECT *)data->second)->old_page_URL;
		delete[] ((REDIRECT *)data->second)->redirect_code; 
		delete ((REDIRECT *)data->second);
	}
	redirect_map.clear();
	return 0;
}
