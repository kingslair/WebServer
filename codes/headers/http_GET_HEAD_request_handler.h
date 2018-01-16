#ifndef GET_REQ_HANDLER
#define GET_REQ_HANDLER

#include "general_c_headers.h"
#include "macros.h"
#include "log.h"
#include "http_status_codes.h"
#include "http_helper_functions.h"

#include <glib.h>


int parse_GET_data_to_json(int sockfd, char *data, GHashTable **config_hash);
int handle_GET_request(int sockfd, fd_set *master, GHashTable **hash, int is_head, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash);

#endif
