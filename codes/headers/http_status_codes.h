#ifndef HTTP_STATUS_CODES
#define HTTP_STATUS_CODES

#include "general_c_headers.h"
#include "helper_functions.h"
#include <glib.h>

char* get_http_response_400(char *filename);
char* get_http_response_403(char *filename);
char* get_http_response_404(char *filename);
char* get_http_response_200(char *filename);
char* get_http_OPTIONS_response_200();
char* get_http_response_201(char *page_name, char *host);
char* get_http_response_501(char *filename);
char* get_http_response_505(char *filename);
char* http_error_response_307(char *page_name, char *host);
char* http_error_response_308(char *page_name, char *host);

#endif
