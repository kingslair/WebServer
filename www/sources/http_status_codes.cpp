#include "http_server_class_status_codes.h"
#include "http_server_class_helper.h"

/**********************************************************************
 * function 	  : get_http_response_400
 *
 * arguments 	  : char *filename
 *
 * description 	: returns a 400 Bad Request Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_400(char *filename) {
    //create a http_helper class object
    http_helper helper;
    
    // get file size
    size_t len = helper.get_file_size(filename);
    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);

    // create response header
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 400 Bad Request\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");
    return buf;
}

/**********************************************************************
 * function 	  : get_http_response_403
 *
 * arguments 	  : char *filename
 *
 * description 	: returns a 403 Forbidden Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_403(char *filename) {
    // create a http_helper class object
    http_helper helper;
    
    // get file size
    size_t len = helper.get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);

    // create response header
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 403 Forbidden\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");

    return buf;
}

/**********************************************************************
 * function 	  : get_http_response_404
 *
 * arguments 	  : char *filename
 *
 * description 	: returns a 404 Not Found Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_404(char *filename) {
    // create a http_helper class object
    http_helper helper;
    
    // get file size
    size_t len = helper.get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);

    // create response header
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 404 Not Found\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");

    return buf;
}

/**********************************************************************
 * function 	  : get_http_response_200
 *
 * arguments 	  : char *filename
 *
 * description 	: returns a 200 OK Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_200(char *filename) {
    // create a http_helper class object
    http_helper helper;
    
    // get file size
    int len = helper.get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%d", len);

    // create response header
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 200 OK\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");

    return buf;
}

/**********************************************************************
 * function 	  : get_http_OPTIONS_response_200
 * 
 * arguments 	  : void
 *
 * description 	: returns a 200 OK Header for OPTIONS Request.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_OPTIONS_response_200() {
    //create a http_helper class object
    http_helper helper;
    
    // create response header
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 200 OK\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Allow: GET,HEAD,POST,PUT,DELETE,OPTIONS\r\n");
    strcat(buf, "Content-Length: 0\r\n");
    strcat(buf, "Content-Type: httpd/unix-directory");
    strcat(buf, "\r\n"); 
    return buf;
}

/**********************************************************************
 * function 	  : get_http_response_501
 *
 * arguments 	  : char *filename
 *
 * description 	: returns a 501 Not Implemented Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_501(char *filename) {
    //create a http_helper class object
    http_helper helper;
    
    size_t len = helper.get_file_size(filename);
    char length_str[10];
    sprintf(length_str, "%zu", len);
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }  
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 501 Not Implemented\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");

    return buf;
}

/**********************************************************************
 * function   	: get_http_response_505
 *
 * arguments  	: char *filename
 *
 * description 	: returns a 505 HTTP Version Not Implemented Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_505(char *filename) {
    // create a http_helper class object
    http_helper helper;
    
    size_t len = helper.get_file_size(filename);
    char length_str[10];
    sprintf(length_str, "%zu", len);
    char *buf = NULL;
    try { buf = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; } 
    memset(buf, 0, MAX_SIZE);
    strcat(buf, "HTTP/1.1 505 HTTP Version Not Implemented\r\n");
    strcat(buf, "Date: ");
    strcat(buf, helper.get_date_time().c_str());
    strcat(buf, "\r\n");  
    strcat(buf, "Last-Modified: ");
    strcat(buf, helper.get_last_modified_date(filename).c_str());
    strcat(buf, "\r\n");
    strcat(buf, "Server: Aricent\r\n");
    strcat(buf, "Connection: Close\r\n");
    strcat(buf, "Accept-Range: bytes\r\n");
    strcat(buf, "Content-Length: ");
    strcat(buf, length_str);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, helper.get_content_type(filename).c_str());
    strcat(buf, "\r\n\r\n");

    return buf;
}


/**********************************************************************
 * function 	  : get_http_response_201
 *
 * arguments  	: char *page_name, char *host
 *
 * description 	: returns a 201 Created Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::get_http_response_201(char *page_name, char *host) {
    /* create a response using headers */
    char *response = NULL;
    try { response = new char[MAX_SIZE]; }
    catch(bad_alloc&bd) { return NULL; }
    memset(response, 0, MAX_SIZE);
    strcat(response, "HTTP/1.1 201 Created\r\n");
    strcat(response, "Location: http://");
    strcat(response, host);
    strcat(response, "/");
    strcat(response, page_name);
    strcat(response, "\r\n");
    strcat(response, "Content-Length: 0");
    strcat(response,  "\r\n\r\n");

    /* return response to the caller function */
    return response;
}

/**********************************************************************
 * function 	  : http_error_response_307
 *
 * arguments  	: char *page_name, char *host
 *
 * description 	: returns a 307 Temporary Redirect Header.
 *
 * return 	    : string
 * *******************************************************************/
char* http_status_codes::http_error_response_307(char *page_name, char *host) {
    /* create a response using headers */
    char *response = NULL;
    try { response = new char[MAX_SIZE]; }
    catch(bad_alloc& bd) { return NULL; }
    memset(response, 0, MAX_SIZE);
    strcat(response, "HTTP/1.1 307 Temporary Redirect\r\n");
    strcat(response, "Location: http://");
    strcat(response, host);
    strcat(response, "/");
    strcat(response,  page_name);
    strcat(response,  "\r\n\r\n");
    /* return response to the caller function */
    return response;
}

/**********************************************************************
 * function 	  : http_error_response_308
 *
 * arguments 	  : char *page_name, char *host
 *
 * description 	: returns a 308 Permanent Redirect Header.
 *
 * return 	    : string		
 * *******************************************************************/
char* http_status_codes::http_error_response_308(char *page_name, char *host)  {
  char *response = NULL;
  /* create a response using headers */
  try { response = new char[MAX_SIZE]; }
  catch(bad_alloc& bd) { return NULL; } 
  memset(response, 0, MAX_SIZE);
  strcat(response, "HTTP/1.1 308 Permanent Redirect\r\n");
  strcat(response, "Location: http://");
  strcat(response, host);
  strcat(response, "/");
  strcat(response,  page_name);
  strcat(response,  "\r\n\r\n");
  /* return response to the caller function */
  return response;
}
