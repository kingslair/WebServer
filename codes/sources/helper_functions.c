#include "helper_functions.h"

/**********************************************************************
 * function 	: print_hash
 *
 * arguments 	: gpointer key, gpointer value, gpointer user_data
 * 
 * description 	: prints the element of Hash Table.
 *
 * return 	: void
 *
 * *******************************************************************/
void print_hash(gpointer key, gpointer value, gpointer user_data) {
	printf("%s: %s\n", (char *)key, (char *)value);
}

/**********************************************************************
 * function 	: print_redirect
 *
 * arguments 	: gpointer key, gpointer value, gpointer user_data
 *
 * description 	: prints the Hash Table element containing REDIRECT
 * 		  elements.
 *
 * return 	: void
 *
 * *******************************************************************/
void print_redirect(gpointer key, gpointer value, gpointer user_data) {
	REDIRECT *redirect_entry = (REDIRECT *)value;
	printf(user_data, redirect_entry->redirect_code, redirect_entry->old_page_URL, redirect_entry->new_page_URL);

}

/**********************************************************************
 * function 	: free_hash
 *
 * arguments 	: gpointer key, gpointer value, gpointer user_data
 *
 * description 	: frees all the allocated memory's of the Hash Table 
 * 		  element.
 *
 * return 	: void
 *
 * *******************************************************************/
void free_hash(gpointer key, gpointer value, gpointer user_data) {
	g_free(key);
	g_free(value);
}

/**********************************************************************
 * function 	: free_redirect_hash
 *
 * arguments 	: gpointer key, gpointer value, gpointer user_data
 *
 * description 	: frees all the allocated structures in the REDIRECT
 * 		  Hash Table.
 *
 * return 	: void
 *
 * *******************************************************************/
void free_redirect_hash(gpointer key, gpointer value, gpointer user_data) {
	REDIRECT *entry = (REDIRECT *)value;
	g_free(entry->old_page_URL);
	g_free(entry->new_page_URL);
	g_free(entry->redirect_code);
	g_free(entry);
}

/**********************************************************************
 * function 	: get_date_time()
 *
 * arguments 	: void
 *
 * description 	: returns the current date and time.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_date_time() {
	// buffer to store date and time
	char *datestring = g_new(char, 256);
	// get current time
	time_t t = time(NULL);
	// convert time into localtime
	struct tm *time = localtime(&t);

	// format time according to RFC
	strftime(datestring, 256, "%a, %d %b %Y %T %z", time);
	return datestring;
}

/**********************************************************************
 * function 	: get_last_modified_date
 *
 * arguments 	: char *filename
 *
 * description 	: returns the last modified date of the resource.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_last_modified_date(char *filename) {
	// to store file stats
	struct stat file_stat;
	// buffer to store date and time
	char *last_mdate = g_new(char, 256);

	// get file stats
	if (-1 == stat(filename, &file_stat)) {
		fprintf(stderr, "Getting last modified date failed\n");
		return NULL;
	}

	// convert milliseconds into localtime and store them into struct tm
	struct tm *time = localtime(&(file_stat.st_mtime));
	// format time according to RFC
	strftime(last_mdate, 256, "%a, %d %b %Y %T %z", time);

	return last_mdate;
}

/**********************************************************************
 * function 	: get_file_size
 *
 * arguments 	: char *filename
 *
 * description 	: returns the file size of a resource.
 *
 * return 	: integer
 *
 * *******************************************************************/
int get_file_size(char *filename) {
	// to store file stats
	struct stat file_stat;

	// get file stats
	if (-1 == stat(filename, &file_stat)) {
		fprintf(stderr, ANSI_COLOR_RED "Error in getting file size" ANSI_COLOR_RESET "\n");
		return -1;
	}

	// return file size
	return file_stat.st_size;
}

/**********************************************************************
 * function 	: str_to_int
 *
 * arguments 	: char *number
 *
 * description 	: converts a string a integer.
 *
 * return 	: integer
 *
 * *******************************************************************/
int str_to_int(char *number) {
	char *endptr;
	int val = strtod(number, &endptr);

	if (endptr == number) {
		fprintf(stderr, ANSI_COLOR_RED "No digits are found" ANSI_COLOR_RESET "\n");
		return -1;
	}

	return val;
}

/**********************************************************************
 * function 	: get_content_type
 *
 * arguments 	: char *filename
 *
 * description 	: returns the content type of the resource.
 *
 * return 	: string
 *
 * *******************************************************************/
char* get_content_type(char *filename) {
	// split filename at "."
	// example: index.html
	// tokenized: [index] [html]

	unsigned int i;
	unsigned int loc = 0;
	for (i = 0; filename[i] != '\0'; i++) {
		if ('.' == filename[i]) {
			loc = i;
		}
	}

	char *file_ext = g_strdup(filename+loc);

	// to store content type of a file
	char *content_type = NULL;

	if (0 == strcmp(".html", file_ext)) {		// check if file type is html
		content_type = g_strdup("text/html");
	} else if (0 == strcmp(".css", file_ext)) {		// check if file type is css
		content_type = g_strdup("text/css");
	} else if (0 == strcmp(".pdf", file_ext)) {		// check if file type is pdf
		content_type = g_strdup("application/pdf");
	} else if (0 == strcmp(".jpeg", file_ext) || (0 == strcmp("jpg", file_ext))) {	// check if file type is jpeg
		content_type = g_strdup("image/jpeg");
	} else if (0 == strcmp(".png", file_ext)) {		// check if file type is png
		content_type = g_strdup("image/png");
	} else if (0 == strcmp(".js", file_ext)) {		// check if file type is javascript
		content_type = g_strdup("application/javascript");
	} else if (0 == strcmp(".gif", file_ext)) {		// check if file type is gif
		content_type = g_strdup("image/gif");
	} else if (0 == strcmp(".json", file_ext)) {		// check if file type is json
		content_type = g_strdup("text/json");
	} else {
		content_type = g_strdup("text/plain");			// if not from above types
		// use text/plain as default
	}

	g_free(file_ext);
	return content_type;
}

/**********************************************************************
 * function 	: is_directory
 *
 * arguments 	: char *file_path
 *
 * description 	: returns whether a resource is a directory or not.
 *
 * return 	: integer
 *
 * *******************************************************************/
int is_directory(char *file_path) {
	struct stat file_stat;

	FILE *fp = fopen(file_path, "r");
	if (NULL == fp) {
		return 0;
	}
	int is_close = fclose(fp);
	if (0 != is_close) {
		fprintf(stderr, ANSI_COLOR_MAGENTA "[FILE NOT CLOSED PROPERLY IN IS_DIRECTORY]" ANSI_COLOR_RESET "\n");
	}

	if (-1 == stat(file_path, &file_stat)) {
		return -1;
	}

	return (S_ISDIR(file_stat.st_mode)) ? 1 : 0;
}
