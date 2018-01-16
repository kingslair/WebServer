#include "http_server_helper.h"

/**********************************************************************
 * function 	: get_in_addr
 * arguments 	: struct sockaddr *sa
 *
 * description 	: this function returns the address of the given socket
 *		  addr.
 *
 * return 	: returns the IPv6 address
 *
 * *******************************************************************/
void* get_in_addr(struct sockaddr *sa) {
	// check if we are using ipv4 or ipv6
	if (sa->sa_family == AF_INET) {
		// return ipv4 address
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	// return ipv6 address
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/**********************************************************************
 * function 	: setnonblocking
 * arguments 	: int sock
 *
 * description 	: this function makes a blocking socket to a non
 * 		  blocking socket.
 *
 * return 	: void
 * *******************************************************************/
void setnonblocking(int sock) {
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	return;
}


/**********************************************************************
 * function 	: interrupt_signal_handler
 *
 * arguments 	: int sig
 *
 * description 	: used to dump gcov data at runtime
 * 		
 * return 	: void
 *
 * *******************************************************************/
#ifdef GCOV_RUNNING
void __gcov_flush();
#endif
void interrupt_signal_handler(int sig) {
#ifdef GCOV_RUNNING
	__gcov_flush();
#endif
}

