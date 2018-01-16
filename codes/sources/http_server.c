#include "http_server_helper.h"

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


	/* Initializing server */
	log_server_data(INFO, "Initializing Server");
	fprintf(stdout, ANSI_COLOR_YELLOW "Initializing Server..." ANSI_COLOR_RESET "\n");
	GHashTable *config_hash = ini_config("../../config/config");
	if (NULL == config_hash) {
		return EXIT_FAILURE;
	}
	//log data
	log_server_data(FINE, "[File Opened] Configuration Loaded Successfully");
	//write to server console
	fprintf(stdout, ANSI_COLOR_GREEN "Configuration Successful" ANSI_COLOR_RESET "\n");
	fprintf(stdout, ANSI_COLOR_YELLOW "Loading Redirect file" ANSI_COLOR_RESET "\n");
	//fetch the .graccess file and insert the data into a hash table
	GHashTable *redirect_hash = ini_redirect("../../.graccess");
	//if the hash table returned NULL
	if (NULL == redirect_hash) {
		//free all the allocated hash table values
		g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
		g_hash_table_destroy(config_hash);
		return EXIT_FAILURE;
	}
	//log data
	log_server_data(FINE, "[File Opened] Redirection Loaded Successfully");
	//write to server console
	fprintf(stdout, ANSI_COLOR_GREEN "Redirection file Loaded Successfully" ANSI_COLOR_RESET "\n");

	// store highest number of socket which we are checking in select
	int fd_max;
	// to store return value of getaddrinfo
	int rv;

	// main server socket in which new client request for new socket connection
	int server_sockfd;
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

	// config struct to pass to getaddrinfo, which will return information about
	// the host
	struct addrinfo hints, *ai, *p;

	// reset struct
	memset(&hints, 0, sizeof(hints));
	// we will use ipv4 or ipv6 whatever our host support
	hints.ai_family = AF_UNSPEC;
	// TCP stream
	hints.ai_socktype = SOCK_STREAM;
	// information about our host (in which we are running our program)
	hints.ai_flags = AI_PASSIVE;

	// get host information
	if (0 != (rv = getaddrinfo(NULL, argv[1], &hints, &ai))) {
		//log data
		log_server_data(SEVERE, "Host Information not found");
		//write to server console
		fprintf(stderr, ANSI_COLOR_RED "select server: %s" ANSI_COLOR_RESET "\n", gai_strerror(rv));
		//free the config hash table
		g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
		g_hash_table_destroy(config_hash);
		//free the redirect hash table
		g_hash_table_foreach(redirect_hash, (GHFunc)free_redirect_hash, NULL);
		g_hash_table_destroy(redirect_hash);
		exit(EXIT_FAILURE);
	}
	//log data
	log_server_data (INFO, "Host Information Found");

	// create and bind to first valid socket
	for (p = ai; p != NULL; p = p->ai_next) {
		server_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (-1 == server_sockfd) {
			//write to server console
			fprintf(stderr, ANSI_COLOR_RED "Valid socket not created" ANSI_COLOR_RESET "\n");
			//log data
			log_server_data(SEVERE, "Valid Socket Not Created");
			continue;
		}

		int yes = 1;
		setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		//Set socket to non-blocking with our setnonblocking routine
		setnonblocking(server_sockfd);
		//binding starts
		if (-1 == bind(server_sockfd, p->ai_addr, p->ai_addrlen)) {
			fprintf(stderr, ANSI_COLOR_RED "Binding to Socket failed" ANSI_COLOR_RESET "\n");
			log_server_data(SEVERE, "Binding to Socket failed");
			close(server_sockfd);
			continue;
		}
		//log data
		log_server_data(INFO, "Binding to Socket Successfull");

		break;
	}

	// if no valid socket and bind happen than exit
	if (NULL == p) {
		//log data
		log_server_data(SEVERE, "No Valid Socket and Bind Exit");
		//write to server console
		fprintf(stderr, ANSI_COLOR_RED "No valid port available to bind" ANSI_COLOR_RESET "\n");
		//free the config hash table
		g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
		g_hash_table_destroy(config_hash);
		//free the redirect hash table
		g_hash_table_foreach(redirect_hash, (GHFunc)free_redirect_hash, NULL);
		g_hash_table_destroy(redirect_hash);
		freeaddrinfo(ai);
		exit(EXIT_FAILURE);
	}
	// freeing structure
	freeaddrinfo(ai);
	//write to server console
	fprintf(stdout, ANSI_COLOR_YELLOW "Starting Server" ANSI_COLOR_RESET "\n");
	log_server_data(INFO, "Starting Server");

	// listen on main server socket with queue of 10
	if (-1 == listen(server_sockfd, 10)) {
		log_server_data(SEVERE, "Listening on Server Socket Failed");
		fprintf(stderr, ANSI_COLOR_RED "Listening on Server Socket Failed" ANSI_COLOR_RESET "\n");
		perror("listen");
		//free the config hash table
		g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
		g_hash_table_destroy(config_hash);
		//free the redirect hash table
		g_hash_table_foreach(redirect_hash, (GHFunc)free_redirect_hash, NULL);
		g_hash_table_destroy(redirect_hash);

		exit(EXIT_FAILURE);
	}
	//free the redirect hash table
	fprintf(stdout, ANSI_COLOR_YELLOW "Server Started" ANSI_COLOR_RESET "\n");
	//log data
	log_server_data(INFO, "Server Started");
	log_server_data (INFO, "Listening to Server Socket successfull");
	fprintf(stdout, ANSI_COLOR_YELLOW "Server listening for new connections..." ANSI_COLOR_RESET "\n");
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
		//select command starts here
		if (-1 == select(fd_max + 1, &read_fds, NULL, NULL, NULL)) {
			//log data
			log_server_data(SEVERE, "Select Command failed to start");
			//write to server console
			fprintf(stderr, ANSI_COLOR_RED "Select command failed to start" ANSI_COLOR_RESET "\n");
			perror("select");
			//free the config hash table
			g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
			g_hash_table_destroy(config_hash);
			//free the redirect hash table
			g_hash_table_foreach(redirect_hash, (GHFunc)free_redirect_hash, NULL);
			g_hash_table_destroy(redirect_hash);

			exit(EXIT_FAILURE);
		}

		// iterate over all the sockets which we have set in master
		// they can't we greater than fd_max
		unsigned int i;
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
						log_server_data(SEVERE, "Client Socket couldn't accept");
						//write to server console
						fprintf(stderr, ANSI_COLOR_RED "Error in Accepting new connections" ANSI_COLOR_RESET "\n");
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
					printf(ANSI_COLOR_YELLOW "Server: New Connection from %s on Socket %d" ANSI_COLOR_RESET "\n", inet_ntop(
								client_addr.ss_family,
								get_in_addr((struct sockaddr *)&client_addr), remoteIP, INET6_ADDRSTRLEN), client_sockfd
					      );
					char buffer[100];
					sprintf (buffer, "%s %s", "Client Connected at", remoteIP);
					//log data
					log_server_data(INFO, buffer);
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
						//log data
						log_server_data(INFO, "Client Disconnected");
						//write to server console
						fprintf(stdout, ANSI_COLOR_YELLOW "Server: Client Disconnected at socket %d" ANSI_COLOR_RESET "\n", i);
						close(i);
						FD_CLR(i, &master);
						free(buf);
						continue;
					} else if (nbytes < 0) {
						//log data
						log_server_data(SEVERE, "Error in Receiving");
						//write to server console
						fprintf(stderr, ANSI_COLOR_RED "Error in Receiving" ANSI_COLOR_RESET "\n");
						perror("read");
						close(i);
						FD_CLR(i, &master);
						free(buf);
						continue;
					}
					client_ip_address = g_strdup(remoteIP);
					// if data is present in socket than parse it
					GHashTable *tag_value_hash = http_parse_request_header(buf, i, &master, &config_hash, client_ip_address);
					//free the allcoated buf variable
					free(buf);
					if (NULL == tag_value_hash) {
						continue;
					}
					// now serve the requested resources
					serve_request(i, &master, &tag_value_hash, client_ip_address, &config_hash, &redirect_hash);
					//free the hash table
					g_hash_table_foreach(tag_value_hash, (GHFunc)free_hash, NULL);
					g_hash_table_destroy(tag_value_hash);
					//free the allocated memory for the hash table
					g_free(client_ip_address);
				}
			}
		}
	}
	//free the config hash table
	g_hash_table_foreach(config_hash, (GHFunc)free_hash, NULL);
	g_hash_table_destroy(config_hash);
	//free the redirect hash table
	g_hash_table_foreach(redirect_hash, (GHFunc)free_redirect_hash, NULL);
	g_hash_table_destroy(redirect_hash);
	return EXIT_SUCCESS;
}

