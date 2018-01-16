#ifndef PUT_REQ_HANDLER
#define PUT_REQ_HANDLER

#include "general_c_headers.h"
#include "macros.h"
#include "log.h"
#include "http_status_codes.h"
#include "http_helper_functions.h"

#include <glib.h>

int store_PUT_multipart_request(int sockfd, int size,char *client_ip_address, GHashTable **config_hash, GHashTable** hash);
int store_PUT_simple_request(int sockfd, unsigned int size, char *client_ip_address, GHashTable **config_hash, GHashTable **hash);
int handle_PUT_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash);

#endif
