#include "log.h"

/**********************************************************************
 * function 	: log_server_data
 *
 * arguments 	: int severity_level, char *message
 *
 * description 	: logs messages to the log file accoeding to severity 
 * 		  level.
 *
 * return 	: integer
 * *******************************************************************/
int log_server_data(int severity_level, char *message) {
	// open file in which we are going to add log
	FILE *write_log = fopen(LOG_FILE_PATH,"a");
	if (NULL == write_log) {
		// incase of error log this to stderr
		fprintf(stderr, ANSI_COLOR_RED "Error in Opening Log File" ANSI_COLOR_RESET "\n");
		return -1;
	}

	// string to point to severity level
	char *severity_level_string = NULL;
	switch (severity_level) {
		case INFO:
			severity_level_string = strdup("INFO");
			break;
		case FINE:
			severity_level_string = strdup("FINE");
			break;
		case WARNING:
			severity_level_string = strdup("WARNING");
			break;
		case SEVERE:
			severity_level_string = strdup("SEVERE");
			break;
	}

	// datestring to add to log string
	char *datestring = get_date_time();
	// creating a formated log string
	char *log_data = g_strconcat(datestring, " ", severity_level_string, " ", message, "\n", NULL);
	free(datestring);
	free(severity_level_string);

	fprintf(write_log, "%s", log_data);
	fflush(write_log);
	g_free(log_data);

	// close file with proper error handling
	int is_closed = fclose(write_log);
	if (0 != is_closed) {
		fprintf(stderr, ANSI_COLOR_MAGENTA "Error in Closing Log File" ANSI_COLOR_RESET "\n");
		return -1;
	}
	return 0;
}
