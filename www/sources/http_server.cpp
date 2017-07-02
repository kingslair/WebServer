#include "http_server_general.h"
#include "http_server_specific.h"
#include "http_server_class_log.h"
#include "http_server_class_init.h"
#include "http_server_class_parser.h"
#include "http_server_class_request_handler.h"
#include "http_server_class_socket.h"
#include "http_server_start.h"

/************ Program starts here *****************************/
int main(int argc, char **argv) {
  
	// struct to handle signal action
	struct sigaction new_action, old_action;
	// define signal handler
	new_action.sa_handler = interrupt_signal_handler;
	// empty the new action struct
	sigemptyset(&new_action.sa_mask);
	// reset flags
	new_action.sa_flags = 0;

	sigaction(SIGUSR1, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) {
		sigaction(SIGUSR1, &new_action, NULL);
	}
 
	// create a log class object
	log logger;
	// create a init class object
	init initialization;	

	// Initializing server
	logger.log_server_data(INFO, "Initializing Server");
	fprintf(stdout, ANSI_COLOR_YELLOW "Initializing Server..." ANSI_COLOR_RESET "\n");

	// write the data to a unordered map
	unordered_map<string, string> config_map = initialization.ini_config("../../config/config");
	if (config_map.empty()) {
		return EXIT_FAILURE;
	}

	// log data
	logger.log_server_data(FINE, "[File Opened] Configuration Loaded Successfully");
	// write to server console
	fprintf(stdout, ANSI_COLOR_GREEN "Configuration Successful" ANSI_COLOR_RESET "\n");
	fprintf(stdout, ANSI_COLOR_YELLOW "Loading Redirect file" ANSI_COLOR_RESET "\n");

	// fetch the .graccess file and insert the data into a unordered map
	unordered_map<string, REDIRECT*> redirect_map = initialization.ini_redirect("../../.graccess");
	// if the map returned NULL
	if (redirect_map.empty()) {
		// free all the allocated maps
		config_map.clear();
		return EXIT_FAILURE;
	}


	// log data
	logger.log_server_data(FINE, "[File Opened] Redirection Loaded Successfully");
	// write to server console
	fprintf(stdout, ANSI_COLOR_GREEN "Redirection file Loaded Successfully" ANSI_COLOR_RESET "\n");

	// create a socket class object
	http_socket socket(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);

	// get host information
	int valid_addr_info = socket.get_addr_info(argv[1]);
	if (valid_addr_info) {
		// free the config map
		config_map.clear();
		// free the redirect map
		for (auto data = redirect_map.begin(); data != redirect_map.end(); data++) {
			delete[] ((REDIRECT *)data->second)->new_page_URL;
			delete[] ((REDIRECT *)data->second)->old_page_URL;
			delete[] ((REDIRECT *)data->second)->redirect_code; 
			delete ((REDIRECT *)data->second); 			
		}
		redirect_map.clear();
		exit(EXIT_FAILURE);	
	}

	// log data
	logger.log_server_data(INFO, "Host Information Found");

	int valid_bind = socket.bind_socket();
	if (valid_bind) {
		// free the config map
		config_map.clear();
		// free the redirect map
		for (auto data = redirect_map.begin(); data != redirect_map.end(); data++) {
			delete[] ((REDIRECT *)data->second)->new_page_URL;
			delete[] ((REDIRECT *)data->second)->old_page_URL;
			delete[] ((REDIRECT *)data->second)->redirect_code; 			
			delete ((REDIRECT *)data->second);
		}
		redirect_map.clear();
		exit(EXIT_FAILURE);
	}
	//write to server console
	fprintf(stdout, ANSI_COLOR_YELLOW "Starting Server" ANSI_COLOR_RESET "\n");
	logger.log_server_data(INFO, "Starting Server");

	int valid_listen = socket.listen_socket();
	if (valid_listen) {
		// free the config map
		config_map.clear();
		// free the redirect map
		for (auto data = redirect_map.begin(); data != redirect_map.end(); data++) {
			delete[] ((REDIRECT *)data->second)->new_page_URL;
			delete[] ((REDIRECT *)data->second)->old_page_URL;
			delete[] ((REDIRECT *)data->second)->redirect_code; 
			delete ((REDIRECT *)data->second);
		}
		redirect_map.clear();
		exit(EXIT_FAILURE);
	}	

	// free the redirect map
	fprintf(stdout, ANSI_COLOR_YELLOW "Server Started" ANSI_COLOR_RESET "\n");
	// log data
	logger.log_server_data(INFO, "Server Started");
	logger.log_server_data (INFO, "Listening to Server Socket successfull");
	fprintf(stdout, ANSI_COLOR_YELLOW "Server listening for new connections..." ANSI_COLOR_RESET "\n");
  // fetch the server socket from socket class
	int server_sockfd = socket.get_server_socket();
  // start the server
	server_select_start (server_sockfd, config_map, redirect_map);

	// free the config map
	config_map.clear();
	// free the redirect map
	for (auto data = redirect_map.begin(); data != redirect_map.end(); data++) {
		delete[] ((REDIRECT *)data->second)->new_page_URL;
		delete[] ((REDIRECT *)data->second)->old_page_URL;
		delete[] ((REDIRECT *)data->second)->redirect_code; 
		delete ((REDIRECT *)data->second);
	}
	redirect_map.clear();
	return EXIT_SUCCESS;
}

