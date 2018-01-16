#include "init.h"

/**********************************************************************
 * function 	: ini_config
 *
 * arguments 	: const char *config_file_path
 *
 * description 	: loads the configuration file into a Hash Table.
 *
 * return 	: Hash Table 
 * *******************************************************************/
GHashTable* ini_config(const char *config_file_path) {
	// character pointer to point to log data
	char *log_data = NULL;
	
	FILE *config_file = fopen(config_file_path, "r");
	if (NULL == config_file) {
		// incase of error in opening file log it to log file
		log_data = g_strconcat("[Error in Opening Config File] ", config_file_path, NULL);
		log_server_data(SEVERE, log_data);
		g_free(log_data);
		
		// incase of error in opening file log it to stderr
		fprintf(stderr, ANSI_COLOR_RED "Error in Opening Config File" ANSI_COLOR_RESET "\n");
		return NULL;
	}
	
	// log to log file
	log_data = g_strconcat("[Config File Opened Successfully] ", config_file_path, NULL);
	log_server_data(FINE, log_data);
	g_free(log_data);

	GHashTable *config_hash = g_hash_table_new(g_str_hash, g_str_equal);
	// read line by line and store it in hash table
	char line[500];
	while (NULL != fgets(line, 500, config_file)) {
		if ('\n' == line[0]) {
			continue;
		}
		unsigned int line_read =  strlen(line);
		line[line_read-1] = '\0';
		char **splitted_line = g_strsplit_set(line, " ", 2);
		g_hash_table_insert(config_hash, g_strdup(splitted_line[0]), g_strdup(splitted_line[1]));
		g_strfreev(splitted_line);
		splitted_line = NULL;
	}
	// close the file with proper error handling
	int is_closed = fclose (config_file);
	if (0 != is_closed) {
		log_data = g_strconcat("[Error in Closing Config File] ", config_file_path, NULL);
		fprintf(stderr, ANSI_COLOR_MAGENTA "Error in Closing Config File" ANSI_COLOR_RESET "\n");
		log_server_data(WARNING, log_data);
		g_free(log_data);
	}
	return config_hash;
}

/**********************************************************************
 * function 	: ini_redirect
 *
 * arguments 	: const char *redirect_file_path
 *
 * description 	: loads the Redirect file into Hash Table.
 *
 * return 	: Hash Table
 * *******************************************************************/
GHashTable* ini_redirect(const char *redirect_file_path) {
	// character pointer to point to log string
	char *log_data = NULL;
	FILE *redirect_file = fopen(redirect_file_path, "r");
	if (NULL == redirect_file) {
		// incase of error in opening file log it to log file
		log_data = g_strconcat("[Error in Opening Redirect File] ", redirect_file_path, NULL);
		log_server_data(WARNING, log_data);
		g_free(log_data);

		// incase of error in opening file log it to stderr
		fprintf(stderr, ANSI_COLOR_MAGENTA "Redirect file is missing " ANSI_COLOR_RESET "\n");
		return NULL;
	}

	// log this to log file
	log_data = g_strconcat("[Redirect File Opened Successfully] ", redirect_file_path, NULL);
	log_server_data(FINE, log_data);
	g_free(log_data);

	GHashTable *redirect_hash = g_hash_table_new(g_str_hash, g_str_equal);
	// load file line by line and store it in hash table
	char line[500];
	unsigned int line_read;
	while (NULL != fgets(line, 500, redirect_file)) {
		if ('\n' == line[0]) {
			continue;
		}
		line_read = strlen(line);
		line[line_read-1] = '\0';

		char **splitted_line = g_strsplit_set(line, " ", 4);
		REDIRECT *redirect_entry = g_new(REDIRECT, 1);
		redirect_entry->redirect_code = g_strdup(splitted_line[1]);
		redirect_entry->old_page_URL = g_strdup(splitted_line[2]);
		redirect_entry->new_page_URL = g_strdup(splitted_line[3]);

		g_hash_table_insert(redirect_hash, redirect_entry->old_page_URL, redirect_entry);
		g_strfreev(splitted_line);
		splitted_line = NULL;
	}
	
	// close file with proper error handling
	int is_closed = fclose(redirect_file);
	if (0 != is_closed) {
		log_data = g_strconcat("[Error in Closing Redirect File] ", redirect_file_path, NULL);
		log_server_data(WARNING, log_data);
		g_free(log_data);
		fprintf(stderr, ANSI_COLOR_MAGENTA "Error in Closing Redirect File" ANSI_COLOR_RESET "\n");
		return NULL;
	}

	return redirect_hash;
}

