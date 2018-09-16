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
#include <packets.h>
#include <fcntl.h>
#include <passwordhelper.h>
#include <semaphore.h>
#include <pw_checker.h>
#define BATCH_SIZE 10000

// have a buffer to give out work.
struct workunit {
	int workindex;
	unsigned long long start;
	unsigned long long range;
	char* hash;
	short pwdlen;
};

int currentworkindex; // read-only for threads
// critical resources
pthread_mutex_t bufferlock;
struct workunit* buffer[100];
int putcursor = 0;
int takecursor = 0;
sem_t OPENSPOTS, FILLEDSPOTS; // 100 and 0, respectively

sem_t CANPUTANSWER, CANTAKEANSWER; // 1 and 0, respectively
struct packet_answer* globalanswerpacket = 0;

void put_answer(struct packet_answer* answer) {
	sem_wait(&CANPUTANSWER);
	globalanswerpacket = answer;
	sem_post(&CANTAKEANSWER);
}

void* take_answer_thread(void* arg) {
	int servsock = *(int*)arg;
	while (1) {
		sem_wait(&CANTAKEANSWER);
		printf("Sending an answer.\n");
		send_packet(servsock, PACKET_ANSWER, globalanswerpacket);
		free(globalanswerpacket->pwd);
		free(globalanswerpacket);
		globalanswerpacket = 0;
		sem_post(&CANPUTANSWER);
	}
	free(arg);
	return 0;
}

void put(struct workunit* val) {
    sem_wait(&OPENSPOTS);
    pthread_mutex_lock(&bufferlock);
    buffer[putcursor++] = val;
    putcursor = putcursor % 100;
    pthread_mutex_unlock(&bufferlock);
    sem_post(&FILLEDSPOTS);
}

void take(struct workunit** retval) {
    sem_wait(&FILLEDSPOTS);
    pthread_mutex_lock(&bufferlock);
    *retval = buffer[takecursor++];
    takecursor = takecursor % 100;
    pthread_mutex_unlock(&bufferlock);
    sem_post(&OPENSPOTS);
}

void* dowork(void* arg) {
	struct workunit* work;
	unsigned long long index, maxval;
	char* conjecture = malloc(sizeof(char) * 8);
	while (1) {
		take(&work);
		conjecture = realloc(conjecture, sizeof(char) * (work->pwdlen+1));
		conjecture[work->pwdlen] = 0;
		index  = work->start;
		maxval = work->start + work->range;
		// only do current work, skip non-current work.
		while((work->workindex >= currentworkindex) && (index < maxval)) {
			nth_pwd(index, work->pwdlen, conjecture);
			if (check_pw_with_salt(conjecture, work->pwdlen, "salt", 4, work->hash) == 1) {
				// gotta send answer
				printf("Password is %s\n", conjecture);
				struct packet_answer* ap = malloc(sizeof(struct packet_answer));
				ap->workindex = work->workindex;
				ap->pwdlen    = work->pwdlen;
				ap->pwd       = malloc(sizeof(char) * (work->pwdlen+1));
				strcpy(ap->pwd, conjecture);
				put_answer(ap);
			}
			index++;
		}
		free(work);
	}
	free(conjecture);
	return NULL;
}

// fill buffer
void* fillbuffer(void* arg) {
	struct packet_work* apacket = (struct packet_work*)arg;
	printpacket_work(apacket);
	// break work into batches of N
	unsigned long long start, range, current;
	calc_pwrange(apacket->nworkers, apacket->rangenum, apacket->pwdlen, &start, &range);
	current = start;
	while (current < start + range) {
		struct workunit* w = (struct workunit*) malloc(sizeof(struct workunit));
		w->workindex = apacket->workindex;
		w->start = current;
		if (current + BATCH_SIZE > start + range)
			w->range = start + range - current;
		else
			w->range = BATCH_SIZE;
		w->hash = apacket->hash;
		w->pwdlen = apacket->pwdlen;
		current += BATCH_SIZE;
		put(w);
	}
	return 0;
}

int main(int argc, char** argv) {
	// init various pthread things
    if (sem_init(&OPENSPOTS, 0, 100) != 0) {
        perror("semaphore OPENSPOTS init");
    }
    if (sem_init(&FILLEDSPOTS, 0, 0) != 0) {
        perror("semaphore FILLEDSPOTS init");
    }
    if (sem_init(&CANPUTANSWER, 0, 1) != 0) {
        perror("semaphore CANPUTANSWER init");
    }
    if (sem_init(&CANTAKEANSWER, 0, 0) != 0) {
        perror("semaphore CANTAKEANSWER init");
    }
    if (pthread_mutex_init(&bufferlock, NULL) != 0) {
        perror("mutex bufferlock init");
        exit(1);
    }

	int numworkers = (int)atol(argv[2]);
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * numworkers);
	for (int i = 0; i < numworkers; i++) {
		pthread_create(&threads[i], 0, dowork, 0);
	}

	int servsock = establish_connection(argv[1], "7777");
	if (servsock == -1) {
		printf("Connection failed.");
		exit(1);
	}
	printf("Connected.\n");
	// Set up answer-sending thread.
	pthread_t answerthread;
	int* servsockcopy = malloc(sizeof(int));
	*servsockcopy = servsock;
	pthread_create(&answerthread, 0, take_answer_thread, servsockcopy);
	// try to recieve a work packet
	void* packet;
	int packettype = 0;
	struct packet_work* prevpacket = 0;
	struct packet_work* apacket = 0;
	currentworkindex = -1;
	pthread_t bufferfiller = -1;
	while (1) {
		// must poll servsock, then check answer
		recv_packet(servsock, &packettype, &packet);
		apacket = (struct packet_work*)packet;
		if (currentworkindex < apacket->workindex) {
			// try killing the previous buffer filler
			currentworkindex = apacket->workindex;
			if (bufferfiller != -1) {
				if (prevpacket != 0) {
					free(prevpacket);
					prevpacket = 0;
				}
				pthread_cancel(bufferfiller);
				pthread_join(bufferfiller, NULL);
			}
			if (pthread_create(&bufferfiller, 0, fillbuffer, apacket) != 0) {
				perror("pthread_create bufferfiller");
				exit(1);
			}
			// do work
			printf("do work\n");
			prevpacket = apacket;
		} else {
			free(apacket);
		}
	}
	scanf(" ");
	return 0;
}