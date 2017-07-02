#include "http_server_class_log.h"
#include "http_server_class_init.h"

/******************************************************************************
 * function 	  : ini_config
 *
 * arguments 	  : const char* config_file_path
 *
 * description 	: loads the configuration file into a Unordered Map.
 *
 * return 	    : Unordered Map
 * ****************************************************************************/
unordered_map<string, string> init::ini_config(const char* config_file_path) {
	
	//create a log class object
	log logger;
	// character pointer to point to log data
	string log_data;
	int i = 0;
	//declaring a unordered map to store the config file
	unordered_map<string, string> config_map;

	// open the file 	
	fstream config_file (config_file_path, ios::in | ios::out);
	if (config_file.is_open()) {
		// log to log file
		log_data = "[Config File Opened Successfully] " + string(config_file_path);
		logger.log_server_data(FINE, log_data);

		// read line by line and store it in hash table
		string line;
		// variables to store the broken tokens
		char *first_token, *second_token;
		while (config_file.is_open()) {
			// read line by line
			getline (config_file, line);
			// if the line is NULL break
			if (line.empty()) {
				break;
			}
			// buffer to hold the data in the string variable
			char word[MAX_SIZE];
			// change the string to a char array
			int length = line.size();
			for (i = 0; i <= length; i++) {
				word[i] = line[i];
			}	
			// break the line into two different parts
			first_token = strtok (word, " ");
			second_token = strtok (NULL, " ");	
			// insert the data to the configuration map
			config_map.insert({string(first_token), string(second_token)});	
      	
		}
		// close the file with proper error handling
		config_file.close();
		// retrun config_map to callee
		return config_map;
	}
	else {
		// incase of error in opening file log it to log file
		log_data = "[Error in Opening Config File] " + string(config_file_path);
		logger.log_server_data(SEVERE, log_data);

		// incase of error in opening file log it to stderr
		fprintf(stderr, ANSI_COLOR_RED "Error in Opening Config File" ANSI_COLOR_RESET "\n");

		return config_map;
	}
}

/******************************************************************************
 * function 	  : ini_redirect
 *
 * arguments 	  : const char *redirect_file_path
 *
 * description 	: loads the Redirect file into unordered map.
 *
 * return 	    : unordered map
 * 
 * ****************************************************************************/
unordered_map<string, REDIRECT*> init::ini_redirect(const char* redirect_file_path) {
	
	// create a log class object	
	log logger;
	// log string to log data
	string log_data;
	// unordered map to store the data in the file
	unordered_map<string, REDIRECT*> redirect_map;

	// fstream object to read data from the file
	fstream redirect_file (redirect_file_path, ios::in | ios::out);
	if (redirect_file.is_open()) {
		// log this to log file
		log_data = "[Redirect File Opened Successfully] " + string(redirect_file_path);
		logger.log_server_data(FINE, log_data);

		// variable to store the incoming files line by line
		string line;	
		int i = 0;
		while (redirect_file.is_open()) {
			// read line by line
			getline (redirect_file, line);
			// if the line is NULL break
			if (line.empty()) {
				break;
			}
			char word[MAX_SIZE];
			// change the string to a char array
			int length = line.size();
			for (i = 0; i <= length; i++) {
				word[i] = line[i];
			}	
			// allocate memory for the structure REDIRECT
			REDIRECT *redirect_entry = NULL;
			try {
        redirect_entry = new REDIRECT;
        redirect_entry->redirect_code = new char[1024];
        redirect_entry->old_page_URL = new char[1024];
        redirect_entry->new_page_URL = new char[1024];
      }
      catch(bad_alloc& bd) {
        return redirect_map;
      }
			// write to the splitted words into the redirect map
			char *task = strtok (word, " ");	
			strcpy(redirect_entry->redirect_code, strtok (NULL, " "));
			strcpy(redirect_entry->old_page_URL, strtok (NULL, " "));
			strcpy(redirect_entry->new_page_URL, strtok (NULL, " "));
			task = NULL;

			// insert the data to the configuration map
			redirect_map.insert({string(redirect_entry->old_page_URL), redirect_entry});
		} 
		// close file with proper error handling
		redirect_file.close();
		// return the redirect_map to callee function 
		return redirect_map;
		
	}
	else {
		// incase of error in opening file log it to log file
		log_data = "[Error in Opening Redirect File] " + string(redirect_file_path);
		logger.log_server_data(WARNING, log_data);

		// incase of error in opening file log it to stderr
		fprintf(stderr, ANSI_COLOR_MAGENTA "Redirect file is missing " ANSI_COLOR_RESET "\n");

		// return the redirect_map to callee function
		return redirect_map;
	}

}

