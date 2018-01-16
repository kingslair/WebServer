#ifndef HTTP_PARSER
#define HTTP_PARSER

#include "general_c_headers.h"
#include "http_status_codes.h"
#include "http_helper_functions.h"
#include "macros.h"
#include "log.h"

#include <glib.h>
#include <sys/socket.h>

int http_parse_status_line(char *status_line, GHashTable **hash, int sockfd, fd_set *master, GHashTable **config_hash, char *client_ip_address);
int http_parse_header_fields(char **fields, unsigned int length, GHashTable **hash, int sockfd, fd_set *master, GHashTable **config_hash, char *client_ip_address);
GHashTable* http_parse_request_header(char *request, int sockfd, fd_set *master, GHashTable** config_hash, char *client_ip_address);

#endif 
