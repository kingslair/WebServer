#include "http_server_class_log.h"
/******************************************************************************
 * function 	    : log_server_data
 *
 * arguments 	    : int severity_level, string message
 *
 * description 	    : logs messages to the log file accoeding to severity level.
 *
 * return 	    : integer
 * ****************************************************************************/
int log::log_server_data(int severity_level, string message) {
	
	// create a http_helper class object
	http_helper helper;
	
	// open file in which we are going to add log
	ofstream write_log (LOG_FILE_PATH, ofstream::app);
	if (!write_log.is_open()) {
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

	// creating a formated log string
	string log_data;
	log_data = helper.get_date_time() + " " + string(severity_level_string) + 
  " " + string(message) + "\n";

	// write to the log file
	write_log << log_data ;
	// close file
	write_log.close();
	// free all the allocated memory
	free(severity_level_string);	
	return 0;
}
