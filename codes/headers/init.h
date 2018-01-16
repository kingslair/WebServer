#ifndef INIT_HEADERS
#define INIT_HEADERS

#include "macros.h"
#include "structs.h"
#include "general_c_headers.h"
#include "log.h"

#include <limits.h>

GHashTable* ini_config(const char *config_file_path);
GHashTable* ini_redirect(const char *redirect_file_path);

#endif
