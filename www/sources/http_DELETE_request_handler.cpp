#include "http_server_class_log.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_helper.h"
#include "http_server_class_status_codes.h"

/******************************************************************************
 * function 	  : handle_DELETE_request
 *
 * arguments	  : int sockfd, fd_set *master, unordered_map<string, string> map, 
 * 		            char *client_ip_address,
 * 		            unordered_map<string, string> config_map, 
 * 		            unordered_map<string, REDIRECT*> redirect_map 
 *
 * description 	: this function handles the DELETE request on a partic-
 *   		          uclar resource and removes the resource from the current
 *   		          base directory and moves it to the trash outside of the
 *		            base directory.
 *
 * return     	: integer
 * ****************************************************************************/
int http_request_handler::handle_DELETE_request(int sockfd, fd_set *master,
unordered_map<string, string> map, char *client_ip_address, 
unordered_map<string, string> config_map, 
unordered_map<string, REDIRECT*> redirect_map) {

  // create a log class object
  log logger;

  // create a http_status_codes log class object
  http_status_codes http_status;

  // create a http_helper helper
  http_helper helper;

  // logging file
  string log_data;

  // lookup url in hash table, it may contain data also
  auto url_with_data = map.find("url");
  if (url_with_data == map.end()) {
    return 1;
  }
  // change the string to a static characeter array	
  int url_length = url_with_data->second.size();
  int i = 0;
  char url[MAX_SIZE];
  memset(url, 0, MAX_SIZE);
  for (i = 0; i < url_length; i++) {
    url[i] = url_with_data->second[i];
  }

  // looking up base directory in config map
  auto base_dir = config_map.find ("BASE_DIR");
  if (base_dir == config_map.end()) {
    base_dir->second = "../../www";
  }
  char base_directory[MAX_SIZE];
  memset (base_directory, 0, MAX_SIZE);
  int len_base_dir = base_dir->second.size();
  for (i = 0; i < len_base_dir; i++) {
    base_directory[i] = base_dir->second[i];
  }	

  // url will contain data after "?" so we are splitting at "?"
  // example: /hello.html?name=john+doe
  // tokenized: [/hello.html] [name=jhon+doe]
  char *tokenized_url = strtok (url, "?");

  // remove "/" from url
  // example: "/hello.html" ---->  "hello.html"
  char *filename = helper.get_valid_file_path(tokenized_url, sockfd, master, 
  client_ip_address, config_map, redirect_map, map);

  if (NULL == filename) {
    FD_CLR(sockfd, &(*master));
    close(sockfd);
    return 1;
  }

  // try to open file requested
  ifstream check_file (filename, ios::in);
  // if failed in opening file send 404 error
  if (!check_file.is_open()) {
    //log info to log file
    log_data = "[client " + string(client_ip_address) + "] [GET]" + 
    " [Page Served] 404 Error";
    logger.log_server_data(INFO, log_data);

    // fetch the 404 Not Found Page
    char *file_path_404 = NULL;
    try { file_path_404 = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { 
      free(filename); 
       return 1;
    } 
    memset(file_path_404, 0, MAX_SIZE);
    strcat(file_path_404, base_directory);
    strcat(file_path_404, "/error/404.html");

    // get the 404 header
    char *header_404 = http_status.get_http_response_404(file_path_404);
    if (NULL == header_404) {
      free(filename);
      // clear the socket from the master set
      FD_CLR(sockfd, &(*master));
      // close the socket
      close(sockfd);
      // clear all the allocated memory
      delete[] file_path_404;
      return 1;
    }
    //send the response back to client
    int send_status = helper.send_http_response(sockfd, header_404, file_path_404);
    if (0 != send_status) {
      logger.log_server_data(SEVERE, "Error in sending HTTP 404 response to Client");
      fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 404 response to Client"\
      ANSI_COLOR_RESET "\n");
    }
    free(filename);
    // clear the socket from the master set
    FD_CLR(sockfd, &(*master));
    // close the socket
    close(sockfd);
    // clear all the allocated memory
    delete[] header_404;
    delete[] file_path_404;
    return 1;
  }
  check_file.close();

  auto trash_dir = config_map.find("TRASH_PATH");
  // in case NULL is returned
  if (trash_dir == map.end()) {
    trash_dir->second = "../../trash";
  }
  // concatenate the data to the string
  string arr = "mv " + string(filename) + " " + trash_dir->second;
  char trash_bin[MAX_SIZE];
  memset(trash_bin, 0, MAX_SIZE);
  int len_trash = arr.size();
  for (i = 0; i < len_trash; i++) {
    trash_bin[i] = arr[i];
  }
  // get the 200 header
  char *header_200 = http_status.get_http_response_200(filename);
  if (NULL == header_200) {
    free(filename);
    // remove client from select watching list
    FD_CLR(sockfd, &(*master));
    return 0;
  }
  // send the response back to client
  int send_status = helper.send_http_response(sockfd, header_200, filename);
  if (0 != send_status) {
    logger.log_server_data(SEVERE, "Error in sending HTTP 200 response to Client");
    fprintf(stderr, ANSI_COLOR_RED "Error in sending HTTP 200 response to Client"\
    ANSI_COLOR_RESET "\n");
  }
  delete[] header_200;

  // system call to move the file to trash
  int a = system(trash_bin);
  // error checking
  if (-1 != a) {
    printf ("File Moved to Trash. Deleted from www successfully\n");
    // log info to log file
    log_data = "[client " + string(client_ip_address) + "] [DELETE] [File Deleted] " 
    + string(filename);
    logger.log_server_data(INFO, log_data);

  } else {
    printf ("File cannot be moved to trash. Cannot be deleted successfully\n");
    // log info to log file
    log_data = "[client " + string(client_ip_address) + 
    "] [DELETE] [File Cannot Be Deleted] " + string(filename);
    logger.log_server_data(INFO, log_data);
  }
  free(filename);
  // remove client from select watching list
  FD_CLR(sockfd, &(*master));
  return 0;
}

