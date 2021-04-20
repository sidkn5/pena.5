/*
Student: Sean Dela Pena
Professor: Dr. Sanjiv Bhatia
Assignment 5: The goal of this homework is to learn about resource management inside an operating system.
You will work on the specified strategy to manage resources and take care of any possible
starvation/deadlock issues
Date: 4/20/2021

github: github.com/sidkn5
*/

/*
SOURCES:

Please note the the algorithms used in this program are from the stallings book. The semaphores are also
derived from the examples given in the book.

*/

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
int semResources;
int userPid;
int currentIndex;
unsigned int startTimeNs;
unsigned int startTimeSec;
unsigned int waitTimeNs;
unsigned int waitTimeSec;
long startTimeMs = 0;
long lastTimeMs = 0;
int randomTime = 0;
int randomResource;
int randomInstance;


FILE* fp;
timeStruct* shmPtr;

void cleanAll();
void logging();
void sem_wait(int);
void sem_signal(int);
int freeIndex();
void initTime();
int checkTime();
void getIndex();
void initStatus(int);

int main() {

	int resourceUsed;
	userPid = getpid();

	//srand(userPid * 12);
	srand(time(NULL));

	
	signal(SIGALRM, cleanAll);
	signal(SIGINT, cleanAll);
	signal(SIGKILL, cleanAll);
	signal(SIGSEGV, cleanAll);

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
	semKey = ftok("./Makefile", 'a');
	//attach semaphores
	semid = semget(semKey, 2, IPC_EXCL);
	if (semid < -1) {
		perror("user: Error: semget error, creation failure.\n");
		
	}



	currentIndex = freeIndex(userPid);

	//initialize the status of the child
	initStatus(currentIndex);
	//init cpu time and wait time
	initTime(currentIndex);

	/*
	int i;
	int randNum;
	for (i = 0; i < MAXRESOURCES; i++) {
		randNum = rand() % 20;
		if (randNum % 5 == 0) {
			shmPtr->arrResources[i].maxClaim[currentIndex] = 1 + (rand() % shmPtr->arrResources[i].instances);
		}
	}*/

	
	//start of the clock 
	sem_wait(SEMCLOCK);
	startTimeNs = shmPtr->nanoseconds;
	startTimeSec = shmPtr->seconds;
	startTimeMs = shmPtr->milliseconds;
	sem_signal(SEMCLOCK);

	waitTimeNs = startTimeNs;
	waitTimeSec = startTimeSec;


	//random waiting time
	randomTime = rand() % 250000000;
	waitTimeNs += randomTime;

	if (waitTimeNs >= 1000000000) {
		waitTimeNs -= 1000000000;
		waitTimeSec++;
	}

	int tempTimeMs;

	while (1) {

		//wait here until the time is over
		while (1) {
			if (checkTime() == 1) {
				break;
			}
		}

		//increment the time here because we are waiting
		if (shmPtr->sleep[currentIndex] == 1 || shmPtr->waiting[currentIndex] == true) {
			//then wait
		}
		else {
			if (shmPtr->seconds - startTimeSec > 1) {
				if (shmPtr->nanoseconds > startTimeNs) {
					//can terminate early set status to 2, early temination
					if ((rand() % 20) % 2 == 0) {
						//printf("P%d is going to terminate early\n", currentIndex);
						sem_wait(SEMRESOURCE);
						shmPtr->arrPid[currentIndex] = 2;
						sem_signal(SEMRESOURCE);
						cleanAll();
						break;
					}
				}
			}

			randomResource = rand() % 20;

			//the process will either request or release a resource
			sem_wait(SEMRESOURCE);
			if (shmPtr->arrResources[randomResource].allocated[currentIndex] > 0) {
				if (rand() % 11 > 5) {
					printf("P%d is requesting to release R%d... \n", currentIndex, randomResource);
					shmPtr->arrResources[randomResource].release[currentIndex] = shmPtr->arrResources[randomResource].allocated[currentIndex];
					shmPtr->waiting[currentIndex] = true;
				}
			}
			else {
				if (rand() % 11 > 5) {
					if (rand() % 11 > 5) {
						randomInstance = 1 + (rand() % shmPtr->arrResources[randomResource].instances);
						if (randomInstance > 0) {
							printf("P%d is requesting to get R%d (%d instances)\n", currentIndex, randomResource, randomInstance);
							shmPtr->arrResources[randomResource].request[currentIndex] = randomInstance;
							shmPtr->waiting[currentIndex] = true;
						}
					}
				}
			}


			sem_signal(SEMRESOURCE);

			//next wait time
			sem_wait(SEMCLOCK);
			randomTime = rand() % 250000000;
			waitTimeNs = shmPtr->nanoseconds;
			waitTimeNs += randomTime;


			if (waitTimeNs >= 1000000000) {
				waitTimeNs -= 1000000000;
				waitTimeSec++;
			}
			sem_signal(SEMCLOCK);
		}
	}
	return 0;
}

void initStatus(int ind) {
	shmPtr->arrPid[ind] = 0;
	shmPtr->waiting[ind] = 0;
	shmPtr->sleep[ind] = 0;
}


int checkTime() {
	if (shmPtr->seconds > waitTimeSec) {
		return 1;
	}
	else if(shmPtr->nanoseconds > waitTimeNs) {
		return 1;
	}
	return 0;
}

void cleanAll() {
	shmdt(shmPtr);
	exit(0);
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
		if (p == shmPtr->childRunning[i]) {
			return i;
		}
	}
	return -1;
}

//init cpu and wait time
void initTime(int ind) {
	shmPtr->cpuTime[ind] = 0;
	shmPtr->childWaitTime[ind] = 0;
}

