#ifndef HTTP_SERVER
#define HTTP_SERVER

#include "general_c_headers.h"
#include "macros.h"
#include "log.h"
#include "http_parser.h"
#include "http_request_handler.h"
#include "init.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include <glib.h>

void* get_in_addr(struct sockaddr *sa);
void setnonblocking(int sock);

#ifdef GCOV_RUNNING
void __gcov_flush();
#endif
void interrupt_signal_handler(int sig);

#endif
