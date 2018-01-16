#ifndef LOG_HEADER
#define LOG_HEADER

#include "macros.h"
#include "general_c_headers.h"
#include "helper_functions.h"

#include <glib.h>

int log_server_data(int severity_level, char *message);

#endif
