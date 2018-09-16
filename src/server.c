#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <simplesockets.h>
#include <fcntl.h>
#include <packets.h>

int jobs = 0;
int jobcounter = 0;
short pwlens[10];
char* hashes[10]; // hashes are always 64 long
int listener;

int numsockets = 0;
int maxsockets = 10;
int* sockets;

pthread_mutex_t startlock;
pthread_cond_t startcond;
int startflag = 0;

FILE* outputfile;

// Liason thread handles everything.
void* liaison(void* arg) {

	fd_set masterset, read_fds;
	int retval, fdmax;
	sockets = malloc(sizeof(int) * 10);

	// First, create a socket.
	listener = bind_socket("7777");
	int flags = fcntl(listener, F_GETFL);
	fcntl(listener, F_SETFL, flags | O_NONBLOCK);
	if(listen(listener, 10) == -1) {
		perror("listen listener");
		exit(1);
	}

	pthread_mutex_lock(&startlock);
	startflag = 1;
	pthread_cond_signal(&startcond);
	pthread_mutex_unlock(&startlock);

	FD_ZERO(&masterset);
	FD_ZERO(&read_fds);
	FD_SET(listener, &masterset);
	fdmax = listener;

	// Now select() for incoming requests and add to sockets.
	printf("Listening.\n");
	int spinflag = 1;
	while(spinflag) {
		read_fds = masterset;

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		// loop through FDs to find the right ones
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					retval = accept(listener, NULL, NULL);
					if(retval == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							printf("Duplicate?\n");
						} else {
							perror("accept");
						}
					} else {
						// add to list of sockets
						if (numsockets == maxsockets) {
							sockets = (int*)realloc(sockets, sizeof(int) * 2 * maxsockets);
							maxsockets *= 2;
						}
						sockets[numsockets++] = retval;
						printf("New client. Total %d\n", numsockets);
						if (retval > fdmax) fdmax = retval;
						FD_SET(retval, &masterset);
					}
				} else { // handle data
					void* packet;
					int packettype = 0;
					recv_packet(i, &packettype, &packet);
					if (packettype == PACKET_START)
						spinflag = 0; // time to start work
					// remove it from numsockets
					int foundit = 0;
					for (int j = 0; j < numsockets; j++) {
						if (foundit) {
							// start overwriting
							if (numsockets == maxsockets)
								sockets[j] = -1;
							else
								sockets[j] = sockets[j+1];
						} else {
							if (sockets[j] == i) {
								foundit = 1;
								j--;
							}
						}
					}
					numsockets--;
					free(packet);
				}
			}
		}
	} // select while loop

	// Time to get the party started
	// Will continuously accept handshakes, but only get job after
	// current round ends.

	// First, if there's zero workers, complain and quit
	if (numsockets == 0) {
		printf("There are no workers!\n");
		exit(1);
	}

	struct packet_work workpacket;
	int sentwork = -1;
	while(jobcounter < jobs) {
		// divide password into ranges, send out jobs.
		if (sentwork < jobcounter) {
			for (int i = 0; i < numsockets; i++) {
				workpacket.workindex = jobcounter;
				workpacket.nworkers  = numsockets;
				workpacket.rangenum  = i;
				workpacket.hash      = hashes[jobcounter];
				workpacket.pwdlen    = pwlens[jobcounter];
				send_packet(sockets[i], PACKET_WORK, (void*)&workpacket);
				printf("Sending out work %d: %d/%d\n", jobcounter, i+1, numsockets);
			}
			sentwork++;
		}
		// wait for an answer
		read_fds = masterset;

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		// loop through FDs to find the right ones
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					printf("Accepting new client\n");
					retval = accept(listener, NULL, NULL);
					if(retval == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							printf("Duplicate?\n");
						} else {
							perror("accept");
						}
					} else {
						// add to list of sockets
						if (numsockets == maxsockets) {
							sockets = (int*)realloc(sockets, sizeof(int) * 2 * maxsockets);
							maxsockets *= 2;
						}
						sockets[numsockets++] = retval;
						if (retval > fdmax) fdmax = retval;
						FD_SET(retval, &masterset);
						printf("New client. Total %d\n", numsockets);
					}
				} else { // handle data
					void* packet = 0;
					int packettype = 0;
					recv_packet(i, &packettype, &packet);
					if (packettype == PACKET_ANSWER) {
						// handle answer
						// YOU CAN'T HANDLE THE ANSWER
						struct packet_answer* panswer = (struct packet_answer*)packet;
						printf("Answer for job %d: %s\n", panswer->workindex, panswer->pwd);
						fprintf(outputfile, "%s\n", panswer->pwd);
						fflush(outputfile);
						// do next job.
						jobcounter++;
					}
					free(packet);
				}
			}
		}
	} // end of gigantic while loop

	return 0;
}

// Main thread creates liason, waits for start signal, then waits for join.
int main(int argc, char** argv) {
	char* buffer = NULL;
	unsigned int bufflen = 0;
	pthread_t thread;
	int mysocket = -1;

	// Zeroth, init my structures
	if (pthread_mutex_init(&startlock, NULL) != 0) {
        printf("mutex startlock failed\n");
        exit(1);
    }
	
	if (pthread_cond_init(&startcond, NULL) != 0) {
        printf("cond startcond failed\n");
        exit(1);
    }

	// First set up liason
	pthread_create(&thread, 0, liaison, NULL);
	// Load and blank output file
	if ((outputfile = fopen(argv[2], "w")) == NULL) {
		perror("output fopen");
		exit(1);
	}

	// Next, load work file.
	FILE* f;
	if((f = fopen(argv[1], "r")) == NULL) {
		perror("input fopen");
		exit(1);
	}
	// only a string of len 1 would be the EOF
	ssize_t len;

	while ((len = getline(&buffer, &bufflen, f)) > 1) {
		pwlens[jobs] = (short)strtol(&buffer[65], NULL, 10);
		// deallocate extra length and assign
		hashes[jobs] = realloc(buffer, 65); // 64 + 1 for termination char
		hashes[jobs][64] = '\0';
		buffer = NULL;
		jobs++;
	}
	if (len < 1) {
		perror("line");
	}
	fclose(f);

	printf("Read %d jobs\n", jobs);

	// wait for socket to form
	pthread_mutex_lock(&startlock);
	while (startflag == 0) {
		pthread_cond_wait(&startcond, &startlock);
	}
	pthread_mutex_unlock(&startlock);
	
	// wait for line enter.
	printf("Waiting for clients, press any key and enter to start.\n");
	scanf(" ");
	printf("Starting.\n");
	// send start packet
	mysocket = establish_connection("127.0.0.1", "7777");
	struct packet_start pstart;
	send_packet(mysocket, PACKET_START, (void*)&pstart);

	pthread_join(thread, 0);

	// cleanup
	free(buffer);
	for (int i = 0; i < jobs; i++) {
		free(hashes[i]);
	}
	return 0;
}