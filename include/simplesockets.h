// Heavily adapted from the tserver/tclient shown in class
/**
 * server source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 **/
 /**
 * client source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

// returns the file descriptor for the socket
int establish_connection(char* hostname, char* port);

// bind to a socket
int bind_socket(char* port);