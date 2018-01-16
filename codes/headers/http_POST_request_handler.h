#ifndef POST_REQ_HANDLER
#define POST_REQ_HANDLER

#include "general_c_headers.h"
#include "macros.h"
#include "log.h"
#include "http_status_codes.h"
#include "http_helper_functions.h"

#include <glib.h>

int store_POST_multipart_request(int sockfd, unsigned int size,char *client_ip_address, GHashTable **config_hash, GHashTable **hash);
int store_POST_simple_request(int sockfd, unsigned int size,char *client_ip_address, GHashTable **config_hash, GHashTable **hash);
int handle_POST_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash);

#endif
