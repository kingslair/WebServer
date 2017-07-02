#include "http_server_class_log.h"
#include "http_server_class_parser.h"
#include "http_server_class_request_handler.h"
#include "http_server_start.h"
/******************************************************************************
 * function 	  : get_in_addr
 *
 * arguments 	  : struct sockaddr *sa
 *
 * description 	: this function returns the address of the given socket addr.
 *
 * return 	    : returns the IPv6 address
 * ****************************************************************************/
void* get_in_addr(struct sockaddr *sa) {
	// check if we are using ipv4 or ipv6
	if (sa->sa_family == AF_INET) {
		// return ipv4 address
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	// return ipv6 address
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

#ifdef GCOV_RUNNING
void __gcov_flush();
#endif

void interrupt_signal_handler(int sig) {
#ifdef GCOV_RUNNING
	__gcov_flush();
#endif
}

/******************************************************************************
 * function    : server_select_start
 *
 * arguments   : int server_sockfd, unordered_map<string, string> &config_map
 *               unordered_map<string, REDIRECT*> &redirect_map
 *
 * description : this function handles the incoming request and assigns
 *               client sockets to the connections.
 *
 * returns     : integer (status of the operation)
 *
 * ****************************************************************************/
int server_select_start (int server_sockfd, unordered_map<string, string> &config_map, 
unordered_map<string, REDIRECT*> &redirect_map) {
	// store highest number of socket which we are checking in select
	int fd_max;
	// new client socket through which client communicate with server
	int client_sockfd;
	// to store information of new connected client
	struct sockaddr_storage client_addr;
	// to store length of client_storage struct
	socklen_t client_len;
	// to store number of bytes which we have read from socket
	int nbytes;
	// to store client ip address
	char remoteIP[INET6_ADDRSTRLEN];
	char *client_ip_address;

	//create a log class object
	log logger;
	// this will contain all the file descriptors we are
	// watching for change
	fd_set master;
	// this is backup of master which we will pass into select as
	// it might change it
	fd_set read_fds;

	// reset master
	FD_ZERO(&master);
	// reset backup
	FD_ZERO(&read_fds);

	// set main socket into master, so that we can watch it
	// for changes
	FD_SET(server_sockfd, &master);

	// store current max
	fd_max = server_sockfd;

	while (1)
	{
   	// take backup of master and pass it to select
		// to watch for read changes
		read_fds = master;
		// select command starts here
		if (-1 == select(fd_max + 1, &read_fds, NULL, NULL, NULL)) {
			// log data
			logger.log_server_data(SEVERE, "Select Command failed to start");
			// write to server console
			fprintf(stderr, ANSI_COLOR_RED "Select command failed to start" ANSI_COLOR_RESET "\n");
			perror("select");
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

		// iterate over all the sockets which we have set in master
		// they can't we greater than fd_max
		int i;
		for (i = 0; i <= fd_max; i++) {
			// check current file descriptor is set or not
			if (FD_ISSET(i, &read_fds)) {
				// if its set and it is our main server socket
				if (i == server_sockfd) {
					// create new client socket connection
					client_len = sizeof(client_addr);
					client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
					//if the client socket couldn't accept
					if (-1 == client_sockfd) {
						//log data
						logger.log_server_data(SEVERE, "Client Socket couldn't accept");
						//write to server console
						fprintf(stderr, ANSI_COLOR_RED "Error in Accepting new connections"\
            ANSI_COLOR_RESET "\n");
						perror("accept");
						continue;
					}

					// add new client socket into master
					// to check for read changes in this socket in future
					FD_SET(client_sockfd, &master);
					if (client_sockfd > fd_max) {
						fd_max = client_sockfd;
					}
					// log ip address of new connected client
					printf(ANSI_COLOR_YELLOW "Server: New Connection from %s on Socket %d"\
          ANSI_COLOR_RESET "\n", inet_ntop(client_addr.ss_family,get_in_addr\
          ((struct sockaddr *)&client_addr), remoteIP, INET6_ADDRSTRLEN), client_sockfd);
					char buffer[100];
					sprintf (buffer, "%s %s", "Client Connected at", remoteIP);
					//log data
					logger.log_server_data(INFO, buffer);
				} else { // if its not a main socket than it may be client socket
					char ch = -1;
					char prev_ch;
					int count = 0;
					unsigned int buf_size = 100;
					char *buf = (char *)malloc(buf_size * sizeof(char));
					memset(buf, 0, buf_size);
					unsigned int socket_index = 0;
					while (1) {
						prev_ch = ch;
						nbytes = read(i, &ch, 1);

						if ('\r' != ch && '\n' != ch) {
							count = 0;
						}

						if (0 == nbytes) {
							break;
						} else if (nbytes < 0) {
							break;
						}

						buf[socket_index] = ch;
						socket_index++;
						if (socket_index >= buf_size) {
							buf_size = buf_size * 1.5;
							buf = (char *)realloc(buf, buf_size);
						}
						if ('\r' == prev_ch && '\n' == ch) {
							count++;
						}

						if (2 == count) {
							buf[socket_index] = '\0';
							break;
						}

					}
					// if read contain no data than client disconnected
					if (0 == nbytes) {
						// log data
						logger.log_server_data(INFO, "Client Disconnected");
						// write to server console
						fprintf(stdout, ANSI_COLOR_YELLOW "Server: Client Disconnected at socket %d"\
            ANSI_COLOR_RESET "\n", i);
						close(i);
						FD_CLR(i, &master);
						free(buf);
						continue;
					} else if (nbytes < 0) {
						// log data
						logger.log_server_data(SEVERE, "Error in Receiving");
						// write to server console
						fprintf(stderr, ANSI_COLOR_RED "Error in Receiving" ANSI_COLOR_RESET "\n");
						perror("read"); 
						close(i);
						FD_CLR(i, &master);
						free(buf);
						continue;
					}
					client_ip_address = strdup(remoteIP);
					// create a parser class object
					http_parser parser;

					// if data is present in socket than parse it
					unordered_map<string, string> tag_value_map = parser.http_parse_request_header
          (buf, i, &master, config_map, client_ip_address);
					//free the allcoated buf variable
					free(buf);
					if (tag_value_map.empty()) {
            free(client_ip_address);
						continue;
					}
					// create a request handler object
					http_request_handler request_handler;					

					// now serve the requested resources
					request_handler.serve_request(i, &master, tag_value_map, client_ip_address, 
          config_map, redirect_map);
					// free the map
					tag_value_map.clear();
					// free the allocated memory for the hash table
					free(client_ip_address);
				}
			}
		}
}
}
