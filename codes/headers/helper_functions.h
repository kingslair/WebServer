#ifndef HELPER_FUNC_HEADER
#define HELPER_FUNC_HEADER

#include "general_c_headers.h"
#include "structs.h"
#include "macros.h"

#include <glib.h>
#include <sys/time.h>
#include <sys/stat.h>

void print_hash(gpointer key, gpointer value, gpointer user_data);
void print_redirect(gpointer key, gpointer value, gpointer user_data);
void free_hash(gpointer key, gpointer value, gpointer user_data);
void free_redirect_hash(gpointer key, gpointer value, gpointer user_data);
char* get_date_time();
char* get_last_modified_date(char *filename);
char* get_content_type(char *filename);
int get_file_size(char *filename);
int str_to_int(char *number);
int is_directory(char *file_path);

#endif
