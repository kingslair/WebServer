#ifndef REQ_HANDLER
#define REQ_HANDLER

#include "http_GET_HEAD_request_handler.h"
#include "http_POST_request_handler.h"
#include "http_PUT_request_handler.h"
#include "http_DELETE_request_handler.h"
#include "http_OPTIONS_request_handler.h"

#include "http_status_codes.h"
#include "http_helper_functions.h"
#include "log.h"

#include <glib.h>

int serve_request(int sockfd, fd_set *master, GHashTable **hash, char *client_ip_address, GHashTable** config_hash, GHashTable** redirect_hash);

#endif
