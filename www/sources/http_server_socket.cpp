#include "http_server_class_socket.h"
#include "http_server_class_log.h"
/******************************************************************************
 * function    : get_addr_info
 *
 * arguments   : const char *port_number
 *
 * description : gets the address information when connected
 *
 * returns     : integer (status of the operation)
 * ****************************************************************************/
int http_socket::get_addr_info (const char *port_number) {
	// create a log class object
	log logger;
	// get host information
	if (0 != (rv = getaddrinfo(NULL, port_number, &hints, &ai))) {
		// log data
		logger.log_server_data(SEVERE, "Host Information not found");
		// write to server console
		fprintf(stderr, ANSI_COLOR_RED "select server: %s" ANSI_COLOR_RESET "\n", gai_strerror(rv));
		return 1;
	}
	return 0;
}
/******************************************************************************
 * function   : bind_socket
 *
 * arguments  : NONE
 *
 * decription : binds the socket to the incoming request
 *
 * returns    : integer (status of the operation)
 *
 * ****************************************************************************/
int http_socket::bind_socket() {

	// create a log class object
	log logger;
	// create and bind to first valid socket
	for (p = ai; p != NULL; p = p->ai_next) {
		server_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (-1 == server_sockfd) {
			// write to server console
			fprintf(stderr, ANSI_COLOR_RED "Valid socket not created" ANSI_COLOR_RESET "\n");
			// log data
			logger.log_server_data(SEVERE, "Valid Socket Not Created");
			continue;
		}

		int yes = 1;
		setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		// Set socket to non-blocking with our setnonblocking routine
		// binding starts
		if (-1 == bind(server_sockfd, p->ai_addr, p->ai_addrlen)) {
			fprintf(stderr, ANSI_COLOR_RED "Binding to Socket failed" ANSI_COLOR_RESET "\n");
			logger.log_server_data(SEVERE, "Binding to Socket failed");
			close(server_sockfd);
			continue;
		}
		// log data
		logger.log_server_data(INFO, "Binding to Socket Successfull");
		break;
	}

	// if no valid socket and bind happen than exit
	if (NULL == p) {
		// log data
		logger.log_server_data(SEVERE, "No Valid Socket and Bind Exit");
		// write to server console
		fprintf(stderr, ANSI_COLOR_RED "No valid port available to bind" ANSI_COLOR_RESET "\n");
		// freeing structure
		freeaddrinfo(ai);
		return 1;
	}
	// freeing structure
	freeaddrinfo(ai);
	return 0;
}
/******************************************************************************
 * function   : listen_socket
 *
 * arguments  : NONE
 *
 * decription : listening to the socket
 *
 * returns    : integer (status of the operation)
 *
 * ****************************************************************************/
int http_socket::listen_socket() {
	// create a log class object
	log logger;
	if (-1 == listen(server_sockfd, 10)) {
		logger.log_server_data(SEVERE, "Listening on Server Socket Failed");
		fprintf(stderr, ANSI_COLOR_RED "Listening on Server Socket Failed" ANSI_COLOR_RESET "\n");
		perror("listen");
		return 1;
	}
	return 0;
}
/******************************************************************************
 * function   : get_server_socket
 *
 * arguments  : NONE
 *
 * decription : returns the server socket
 *
 * returns    : server_socker
 * ****************************************************************************/
int http_socket::get_server_socket(){
	return server_sockfd;
}	
