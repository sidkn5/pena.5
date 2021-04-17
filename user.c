#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#include "oss.h"

int shmid;
int semid;
int userPid;

FILE* fp;
timeStruct* shmPtr;

void cleanAll();
void logging();
void sem_wait(int);
void sem_signal(int);
int freeIndex();

int main() {

	userPid = getpid();

	key_t shmKey;

	//allocate shared memory for the clock
	shmKey = ftok("./README.md", 'a');
	shmid = shmget(shmKey, sizeof(timeStruct), IPC_EXCL);
	if (shmid < 0) {
		perror("user: Error: shmget error, creation failure.\n");
		cleanAll();
		exit(1);
	}

	shmPtr = (timeStruct*)shmat(shmid, NULL, 0);
	if (shmPtr == (timeStruct*)-1) {
		perror("user: Error: shmat error, attachment failure.\n");
		cleanAll();
		exit(1);
	}

	key_t semKey;
	//attach semaphores
	semid = semget(semKey, 1, IPC_EXCL);
	if (semid < -1) {
		perror("user: Error: semget error, creation failure.\n");
	}


}

void cleanAll() {
	shmdt(shmPtr);
}

//sem_wait function for the semaphore
void sem_wait(int n) {
	struct sembuf semaphore;
	semaphore.sem_op = -1;
	semaphore.sem_num = 0;
	semaphore.sem_flg = 0;
	semop(semid, &semaphore, 1);
}

//semaphore signal function, increment sem
void sem_signal(int n) {
	struct sembuf sem;
	sem.sem_op = 1;
	sem.sem_num = 0;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
}

void logging(char* buffer) {
	fputs(buffer, fp);
}

int freeIndex(int p) {
	int i;
	for (i = 0; i < 19; i++) {
		if (p == shmPtr->arrPid[i] == 0) {
			return i;
		}
	}
	//return -1;
}