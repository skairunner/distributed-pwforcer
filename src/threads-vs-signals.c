#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

pthread_t children[N];

pthread_mutex_t threadlock;
int threadcount = 1;

pthread_mutex_t initlock;
pthread_cond_t initcond; // will be fired once last thread made
pthread_mutex_t endlock;
pthread_cond_t endcond; // will be fired once main thread acknowledges SIGINT

void* makechild(void* arg) {
    printf("Hello World\n");
    // try to make another thread
    pthread_mutex_lock(&threadlock);
    if (threadcount < N) {
        pthread_create(&(children[threadcount]), NULL, makechild, NULL);
        threadcount++;
    } else {
        // if threadcount is larger or equal than N, unblock!
        pthread_mutex_lock(&initlock);
        pthread_cond_signal(&initcond);
        pthread_mutex_unlock(&initlock);
    }
    pthread_mutex_unlock(&threadlock);
    // wait for go-ahead to end
    pthread_mutex_lock(&endlock);
    pthread_cond_wait(&endcond, &endlock);
    pthread_mutex_unlock(&endlock);
    return NULL; // end
}

int main (int argc, char ** argv) {
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL); // block everything

    // initialize conds and mutexes
    if (pthread_cond_init(&initcond, NULL) != 0) {
        printf("cond initcond failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&initlock, NULL) != 0) {
        printf("mutex initlock failed\n");
        exit(1);
    }
    if (pthread_mutex_init(&threadlock, NULL) != 0) {
        printf("mutex numthreadlock failed\n");
        exit(1);
    }

    int e = pthread_create(&(children[0]), NULL, makechild, NULL);
    // unblock SIGINT
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    //pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    // wait for init to end
    pthread_mutex_lock(&initlock);
    pthread_cond_wait(&initcond, &initlock);
    pthread_mutex_unlock(&initlock);
    printf("All my descendants are created.\n");

    int sig;
    while (1) {
        sigwait(&mask, &sig);
        if (sig == SIGINT) {
            pthread_mutex_lock(&endlock);
            pthread_cond_broadcast(&endcond);
            pthread_mutex_unlock(&endlock);
            break;
        }
    }

    // join all threads
    for(int i = 0; i < N; i++) {
        pthread_join(children[i], NULL);
    }
    printf("All my descendants have ended.");

    return EXIT_SUCCESS;
}
