#ifndef DELETE_REQ_HANLDER
#define DELETE_REQ_HANLDER

#include "general_c_headers.h"
#include "macros.h"
#include "log.h"
#include "http_status_codes.h"
#include "http_helper_functions.h"

#include <glib.h>

int handle_DELETE_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash);

#endif
