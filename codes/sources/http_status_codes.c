#include "http_status_codes.h"

/**********************************************************************
 * function 	: get_http_response_400
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 400 Bad Request Header.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_http_response_400(char *filename) {
    // get file size
    size_t len = get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);
    // get current date and time in a string
    char *date_string = get_date_time();
    // get last modification date and time of file
    char *last_mdate = get_last_modified_date(filename);
    // get content type according to file extension
    char *content_type = get_content_type(filename);

    // create response header
    char *buf = g_strconcat("HTTP/1.1 400 Bad Request\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Server: Aricent\r\n",
            "Connection: Close\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    // free all resource which we have used
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}

/**********************************************************************
 * function 	: get_http_response_403
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 403 Forbidden Header.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_http_response_403(char *filename) {
    // get file size
    size_t len = get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);

    // get current date and time in a string
    char *date_string = get_date_time();
    // get last modification date and time of file
    char *last_mdate = get_last_modified_date(filename);
    // get content type according to file extension
    char *content_type = get_content_type(filename);

    // create response header
    char *buf = g_strconcat("HTTP/1.1 403 Forbidden\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Connection: Close\r\n",
            "Server: Aricent\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    // free all resource which we have used
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}

/**********************************************************************
 * function 	: get_http_response_404
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 404 Not Found Header.
 *
 * return 	: string
 * *******************************************************************/
char* get_http_response_404(char *filename) {
    // get file size
    size_t len = get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%zu", len);

    // get current date and time in a string
    char *date_string = get_date_time();
    // get last modification date and time of file
    char *last_mdate = get_last_modified_date(filename);
    // get content type according to file extension
    char *content_type = get_content_type(filename);

    // create response header
    char *buf = g_strconcat("HTTP/1.1 404 Not Found\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Connection: Close\r\n",
            "Server: Aricent\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    // free all resource which we have used
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}

/**********************************************************************
 * function 	: get_http_response_200
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 200 OK Header.
 *
 * return 	: string
 * *******************************************************************/
char* get_http_response_200(char *filename) {
    // get file size
    int len = get_file_size(filename);

    // convert it into string form
    char length_str[10];
    sprintf(length_str, "%d", len);

    // get current date and time in a string
    char *date_string = get_date_time();
    // get last modification date and time of file
    char *last_mdate = get_last_modified_date(filename);
    // get content type according to file extension
    char *content_type = get_content_type(filename);

    // create response header
    char *buf = g_strconcat("HTTP/1.1 200 OK\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Server: Aricent\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    // free all resource which we have used
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}

/**********************************************************************
 * function 	: get_http_OPTIONS_response_200
 * 
 * arguments 	: void
 *
 * description 	: returns a 200 OK Header for OPTIONS Request.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_http_OPTIONS_response_200() {
    // get current date and time in a string
    char *date_string = get_date_time();

    // create response header
    char *buf = g_strconcat("HTTP/1.1 200 OK\r\n",
            "Date: ", date_string, "\r\n",
            "Server: Aricent\r\n",
            "Allow: GET,HEAD,POST,PUT,DELETE,OPTIONS\r\n"
            "Content-Length: 0\r\n",
            "Content-Type: httpd/unix-directory", "\r\n",
            NULL);

    // free all resource which we have used
    g_free(date_string);
    return buf;
}

/**********************************************************************
 * function 	: get_http_response_501
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 501 Not Implemented Header.
 *
 * return 	: string
 * *******************************************************************/
char* get_http_response_501(char *filename) {
    size_t len = get_file_size(filename);
    char length_str[10];
    sprintf(length_str, "%zu", len);
    char *date_string = get_date_time();
    char *last_mdate = get_last_modified_date(filename);
    char *content_type = get_content_type(filename);

    char *buf = g_strconcat("HTTP/1.1 501 Not Implemented\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Server: Aricent\r\n",
            "Connection: Close\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}

/**********************************************************************
 * function 	: get_http_response_505
 *
 * arguments 	: char *filename
 *
 * description 	: returns a 505 HTTP Version Not Implemented Header.
 *
 * return 	: string
 * *******************************************************************/
char* get_http_response_505(char *filename) {
    size_t len = get_file_size(filename);
    char length_str[10];
    sprintf(length_str, "%zu", len);
    char *date_string = get_date_time();
    char *last_mdate = get_last_modified_date(filename);
    char *content_type = get_content_type(filename);

    char *buf = g_strconcat("HTTP/1.1 505 HTTP Version Not Implemented\r\n",
            "Date: ", date_string, "\r\n",
            "Last-Modified: ", last_mdate, "\r\n",
            "Server: Aricent\r\n",
            "Connection: Close\r\n",
            "Accept-Range: bytes\r\n",
            "Content-Length: ", length_str, "\r\n",
            "Content-Type: ", content_type, "\r\n\r\n",
            NULL);
    g_free(date_string);
    g_free(last_mdate);
    g_free(content_type);
    return buf;
}


/**********************************************************************
 * function 	: get_http_response_201
 *
 * arguments 	: char *page_name, char *host
 *
 * description 	: returns a 201 Created Header.
 *
 * return 	: string
 * *******************************************************************/
char* get_http_response_201(char *page_name, char *host) {
    /* create a response using headers */
    char *response = g_strconcat("HTTP/1.1 201 Created\r\n", 
    "Location: http://", host, "/", page_name, "\r\n", 
    "Content-Length: 0", "\r\n", 
    NULL);

    /* return response to the caller function */
    return response;
}

/**********************************************************************
 * function 	: http_error_response_307
 *
 * arguments 	: char *page_name, char *host
 *
 * description 	: returns a 307 Temporary Redirect Header.
 *
 * return 	: string
 * *******************************************************************/
char* http_error_response_307(char *page_name, char *host) {
    /* create a response using headers */
    char *response = g_strconcat("HTTP/1.1 307 Temporary Redirect\r\n", 
    "Location: http://", host, "/", page_name, "\r\n\r\n", 
    NULL);

    /* return response to the caller function */
    return response;
}

/**********************************************************************
 * function 	: http_error_response_308
 *
 * arguments 	: char *page_name, char *host
 *
 * description 	: returns a 308 Permanent Redirect Header.
 *
 * return 	: string		
 * *******************************************************************/
char* http_error_response_308(char *page_name, char *host)  {
    /* create a response using headers */
    char *response = g_strconcat("HTTP/1.1 308 Permanent Redirect\r\n", 
    "Location: http://", host , "/", page_name, "\r\n\r\n", 
    NULL);

    /* return response to the caller function */
    return response;
}
