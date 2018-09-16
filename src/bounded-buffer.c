#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

// buffer access is a bottleneck, i feel like i remember
// an example of non-buffer-bottlenecked consumer/producer
// problem but i cannot recall.

int buffer[100];

// critical resources
pthread_mutex_t bufferlock;
int putcursor = 0;
int takecursor = 0;
sem_t OPENSPOTS, FILLEDSPOTS; // 100 and 0, respectively

// stdout is not threadsafe either
pthread_mutex_t consolelock;

// map return from random to int range
long map(int r, int a, int b) {
    return (double)r / RAND_MAX * (b - a) + a;
}

void put(int val) {
    sem_wait(&OPENSPOTS);
    pthread_mutex_lock(&bufferlock);
    buffer[putcursor++] = val;
    putcursor = putcursor % 100;
    pthread_mutex_unlock(&bufferlock);
    sem_post(&FILLEDSPOTS);
}

int take() {
    int retval;

    sem_wait(&FILLEDSPOTS);
    pthread_mutex_lock(&bufferlock);
    retval = buffer[takecursor++];
    takecursor = takecursor % 100;
    pthread_mutex_unlock(&bufferlock);
    sem_post(&OPENSPOTS);

    return retval;
}

void* produce(void* args) {
    struct timespec sleeptime;
    int result;
    int sleeptimerand;

    pthread_t self = pthread_self();
    unsigned int state = self;

    while(1) {
        // simulate 'work'
        result = rand_r(&state);
        result = map(result, 0, 100);
        sleeptimerand = rand_r(&state);
        // 0 to 1 seconds of sleep
        sleeptime.tv_nsec = (long)map(sleeptimerand, 0, 10) * 100000000;
        nanosleep(&sleeptime, 0);
        // put after sleep over
        put(result);
    }

    return NULL;
}

void* consume(void* args) {
    int r;
    while (1) {
        r = take();
        pthread_mutex_lock(&consolelock);
        printf("%d\n", r);
        pthread_mutex_unlock(&consolelock);
    }

    return NULL;
}

int main (int argc, char ** argv) {
    pthread_t producers[NB_PROD];
    pthread_t consumers[NB_CONS];

    printf("Producers: %d\n", NB_PROD);
    printf("Consumers: %d\n", NB_CONS);
    // init semaphores and mutexes
    if (pthread_mutex_init(&bufferlock, NULL) != 0) {
        printf("mutex bufferlock initialization failed.\n");
        exit(1);
    }
    if (pthread_mutex_init(&consolelock, NULL) != 0) {
        printf("mutex consolelock initialization failed.\n");
        exit(1);
    }
    if (sem_init(&OPENSPOTS, 0, 100) != 0) {
        printf("semaphore OPENSPOTS init failed.\n");
    }
    if (sem_init(&FILLEDSPOTS, 0, 0) != 0) {
        printf("semaphore FILLEDSPOTS init failed.\n");
    }

    // init producers and consumers
    for (int i = 0; i < NB_PROD; i++) {
        int e = pthread_create(&(producers[i]), NULL, produce, NULL);
        if (e != 0) {
            printf("pthread_create fail\n");
            exit(1);
        }
    }
    // init producers and consumers
    for (int i = 0; i < NB_CONS; i++) {
        int e = pthread_create(&(consumers[i]), NULL, consume, NULL);
        if (e != 0) {
            printf("pthread_create fail\n");
            exit(1);
        }
    }

    sleep(10);
    
    return EXIT_SUCCESS;
}
