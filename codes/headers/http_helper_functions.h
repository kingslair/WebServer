#ifndef HTTP_HELPER_FUNCS
#define HTTP_HELPER_FUNCS

#include "general_c_headers.h"
#include "http_status_codes.h"
#include "macros.h"
#include "log.h"

#include <glib.h>
#include <sys/socket.h>

int send_http_response(int sockfd, char *headers, char *filename);
int http_page_access_rights(char *page_url);
char* get_valid_file_path(char *url_with_data, int sockfd, fd_set *master, char *client_ip_address, GHashTable **config_hash, GHashTable **redirect_hash, GHashTable **hash);

#endif

