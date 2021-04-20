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
//termination criteria
int timeTermination = 5;		//5 sec time termination
int totalChildrenCounter = 0;	//Maximum of 40 children then terminate

//shared memory
int shmid;
int semid;
int semResources;
timeStruct* shmPtr;			//just this for now, needs a struct

FILE* fp;
char addToLogBuffer[200];
int maxLines = 0;			//terminate when the file exceeds this number of lines
int pids[MAXCHILDREN];
int numOfUsers;				//number of Users forked
long forkSec = 0;
long forkNano = 0;
int deadlockAlgoUsed = 0;
unsigned long nanoIncrement;
unsigned long milliIncrement;


int accessGranted = 0;
int waited = 0;
int numOfForks = 0;
int terminated = 0;
float deadlockPercentage = 0;

//functions
void cleanAll();
void timesUp();
void ctrlC();
void mySigchldHandler(int);
void logging();
void sem_wait(int);
void sem_signal(int);
void initSharedMemory();
void killChildPids();
void forking();
int nextFork();
void forkCheck();
void clockCheck();
void updateClock();
void resourceAllocation();
void resourceManager();
void checkChild();
void deadlockDetection();
void report();
void logAllocationTable();


//deallocates and frees everything
void cleanAll() {
	report();
	killChildPids();
	sleep(1);
	shmdt(shmPtr);
	fclose(fp);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	sleep(1);
}

//called for ctrl+C termination
void ctrlC() {
	printf("Process terminate. CTRL + C caught.\n");
	cleanAll();
	exit(0);
}

//called for early time termination
void timesUp() {
	printf("The time given is up Process will terminate.\n");
	sleep(1);
	cleanAll();
	exit(0);
}

void mySigchldHandler(int sig) {
	pid_t pid;
	while ((pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {
		//resetPid(pid);
	}
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


//logging function terminate when going over 100000 lines
void logging(char* buffer) {
	maxLines++;
	if (maxLines <= 100000) {
		fputs(buffer, fp);
	}
	else {
		fputs("OSS will terminate early because we are exceeding the line limit.\n", fp);
		printf("OSS is terminating early because we are exceeding the line limit of logfile.\n");
		cleanAll();
		//sleep(1);
		exit(0);
	}

}

void initSharedMemory() {
	shmPtr->seconds = 0;
	shmPtr->milliseconds = 0;
	shmPtr->nanoseconds = 0;
	for (int i = 0; i < MAXCHILDREN; i++) {
		shmPtr->arrPid[i] = 0;
	}

	for (int i = 0; i < MAXCHILDREN; i++) {
		shmPtr->arrPid[i] = 0;
		shmPtr->waiting[i] = false;
		shmPtr->wantedResources[i] = -1;
		shmPtr->childWaitTime[i] = 0;
		shmPtr->cpuTime[i] = 0;
		shmPtr->childThroughput[i] = 0;
		shmPtr->sleep[i] = 0;
		shmPtr->childRunning[i] = 0;
	}

	for (int i = 0; i < 20; i++) {
		if ((i + 1) % 10 == 0) {
			shmPtr->arrResources[i].shared = true;
			sprintf(addToLogBuffer, "R%d is a sharable resource. \n", i);
			logging(addToLogBuffer);
		}
		else {
			shmPtr->arrResources[i].shared = false;
			sprintf(addToLogBuffer, "R%d is a static / non sharable resource. \n", i);
			logging(addToLogBuffer);
		}

		//instances can be between 1 and 10 max
		shmPtr->arrResources[i].instances = 1 + (rand() % 10);
		shmPtr->arrResources[i].remaining = shmPtr->arrResources[i].instances;
		sprintf(addToLogBuffer, "There are %d instances of resource R%d. \n", shmPtr->arrResources[i].instances, i);
		logging(addToLogBuffer);

		for (int j = 0; j < MAXCHILDREN; j++) {
			shmPtr->arrResources[i].request[j] = 0;
			shmPtr->arrResources[i].allocated[j] = 0;
			shmPtr->arrResources[i].release[j] = 0;
		}
	}
}

void killChildPids() {
	int i;
	for (i = 0; i < MAXCHILDREN; i++) {
		if (shmPtr->childRunning[i] != 0) {
			kill(shmPtr->childRunning[i], SIGKILL);
		}
	}
}

//clock in shared memory
void updateClock() {
	shmPtr->milliseconds += rand() % 1000;
	if (shmPtr->milliseconds >= 1000) {
		shmPtr->seconds++;
		shmPtr->milliseconds -= 1000;
	}
	shmPtr->nanoseconds += rand() % 1000;
	if (shmPtr->nanoseconds >= 1000) {
		shmPtr->milliseconds++;
		shmPtr->nanoseconds -= 1000;
	}

}


//next fork nano
void forkCheck() {
	unsigned long nanoCheck = forkNano;
	if (nanoCheck >= 1000000000) {
		forkSec += 1;
		forkNano -= 1000000000;
	}
}

void clockCheck() {
	unsigned int nanoCheck = shmPtr->nanoseconds;
	if (nanoCheck >= 1000000000) {
		shmPtr->seconds += 1;
		shmPtr->nanoseconds -= 1000000000;
	}
}

void forking() {
	int pid;
	//numOfForks++;
	if (numOfForks >= 40) {
		printf("The maximum number of children has been reached. Terminating...\n");
		logging("OSS: The maximum number of children (40) has been reached. Terminating.");
		cleanAll();
		//sleep(1);
		//exit(0);
	}
	for (int i = 0; i < MAXCHILDREN; i++) {
		if (shmPtr->childRunning[i] == 0) {
			numOfUsers++;
			numOfForks++;
			pid = fork();
			if (pid != 0) {
				sprintf(addToLogBuffer, "Process P%d is being forked with PID: %d at time %d:%d \n", i, pid, shmPtr->seconds, shmPtr->nanoseconds);
				logging(addToLogBuffer);
				shmPtr->childRunning[i] = pid;
				return;
			}
			else {
				execl("./user_proc", "./user_proc", (char*)0);
			}
		}
	}
}


//checks whether the child terminated early, if so, then release all the resources it was allocated
void checkChild() {
	for (int i = 0; i < MAXCHILDREN; i++) {
		if (shmPtr->arrPid[i] == 2) {
			sprintf(addToLogBuffer, "Process P%d terminated early at time %d:%d, releasing its resources...\n", i, shmPtr->seconds, shmPtr->nanoseconds);
			logging(addToLogBuffer);
			shmPtr->arrPid[i] = 0;
			terminated = (rand() % 20);
			for (int j = 0; j < MAXRESOURCES; j++) {
				if (shmPtr->arrResources[j].allocated[i] > 0) {
					shmPtr->arrResources[j].remaining += shmPtr->arrResources[j].allocated[i];
					/*shmPtr->arrResources[j].allocated[i] = 0;
					shmPtr->arrResources[j].request[i] = 0;
					shmPtr->arrResources[j].release[i] = 0;
					shmPtr->childRunning[i] = 0;*/
				}
			}
		}
	}
}


//handles the releases of the proc
void resourceManager() {
	for (int i = 0; i < MAXCHILDREN; i++) {
		for (int j = 0; j < MAXRESOURCES; j++) {
			if (shmPtr->arrResources[j].release[i] > 0) {
				sprintf(addToLogBuffer, "Process P%d released resource R%d at time %d:%d \n", i, j, shmPtr->seconds, shmPtr->nanoseconds);
				logging(addToLogBuffer);
				shmPtr->arrResources[j].remaining += shmPtr->arrResources[j].allocated[i];
				shmPtr->arrResources[j].release[i] = 0;
				shmPtr->arrResources[j].allocated[i] -= shmPtr->arrResources[j].allocated[i];
				shmPtr->waiting[i] = false;
				numOfUsers--;
			}
		}
	}
}

//handle the allocation of resources, handles the requests of the processes
void resourceAllocation() {
	for (int i = 0; i < MAXCHILDREN; i++) {
		if (shmPtr->sleep[i] == 0) {
			for (int j = 0; j < MAXRESOURCES; j++) {
				if (shmPtr->arrResources[j].request[i] > 0) {
					sprintf(addToLogBuffer, "Process P%d is requesting %d instances of R%d \n", i, shmPtr->arrResources[j].request[i], j);
					logging(addToLogBuffer);
					//check if there is enough resource to allocate if not go to sleep
					if (shmPtr->arrResources[j].request[i] <= shmPtr->arrResources[j].remaining) {
						accessGranted++;
						shmPtr->arrResources[j].remaining -= shmPtr->arrResources[j].request[i];
						shmPtr->arrResources[j].allocated[i] -= shmPtr->arrResources[j].request[i];
						shmPtr->arrResources[j].request[i] = 0;
						shmPtr->sleep[i] = 0;
						shmPtr->waiting[i] = false;
						sprintf(addToLogBuffer, "Process P%d has been allocated R%d at time %d:%d \n", i, j, shmPtr->seconds, shmPtr->nanoseconds);
						logging(addToLogBuffer);

					}
					//not enough resources available
					else {

						sprintf(addToLogBuffer, "Process P%d is going to wait and sleep at time %d:%d. Not enough resources R%d \n", i, shmPtr->seconds, shmPtr->nanoseconds, j);
						logging(addToLogBuffer);
						shmPtr->wantedResources[i] = j;
						shmPtr->sleep[i] = 1;
					}
				}
			}
		}
	}
}

//check if we can proceed with a next fork depending if the time has passed or not
int nextFork() {
	if (forkSec == shmPtr->seconds) {
		if (forkNano <= shmPtr->nanoseconds) {
			return 1;
		}
	}
	else if (forkSec < shmPtr->seconds) {
		return 1;
	}
	return 0;
}

void deadlockDetection() {
	int resourceHere;
	for (int i = 0; i < MAXCHILDREN; i++) {
		if (shmPtr->sleep[i] == 1) {
			resourceHere = shmPtr->wantedResources[i];
			while (1) {
				for (int j = 0; j < MAXRESOURCES; j++) {
					if (j != i) {
						if (shmPtr->arrResources[resourceHere].allocated[j] > 0 && shmPtr->sleep[j] == 1) {
							sprintf(addToLogBuffer, "OSS: Deadlock Detected! Terminating P%d, releasing R%d for P%d\n", j, resourceHere, i);
							logging(addToLogBuffer);
							terminated++;
							shmPtr->arrResources[resourceHere].remaining += shmPtr->arrResources[resourceHere].allocated[j];
							shmPtr->arrResources[resourceHere].request[j] = 0;
							shmPtr->arrResources[resourceHere].allocated[j] = 0;
							shmPtr->arrResources[resourceHere].release[j] = 0;

							//kill(shmPtr->childRunning[j], SIGKILL);
							shmPtr->childRunning[j] = 0;
							shmPtr->arrPid[j] = 1;
							numOfUsers--;

							//give resource to the sleeping process
							if (shmPtr->arrResources[resourceHere].request[i] <= shmPtr->arrResources[resourceHere].remaining) {
								shmPtr->arrResources[resourceHere].remaining -= shmPtr->arrResources[resourceHere].request[i];
								shmPtr->arrResources[resourceHere].allocated[i] += shmPtr->arrResources[resourceHere].request[i];
								shmPtr->arrResources[resourceHere].request[i] = 0;

								sprintf(addToLogBuffer, "Process P%d has been allocated resource R%d at time %d:%d \n", i, resourceHere, shmPtr->seconds, shmPtr->nanoseconds);
								logging(addToLogBuffer);
								waited++;

								shmPtr->sleep[i] = 0;
								shmPtr->waiting[i] = false;
								return;
							}
						}
					}
				}
				break;
			}
		}
	}
}


void report() {
	sprintf(addToLogBuffer, "\n\n END REPORT: \n\n");
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "Access was granted instantly this many times: %d \n", accessGranted);
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "Access was grandted to process this many time after waiting: %d \n", waited);
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "Deadlock algorithm used this many times: %d \n", deadlockAlgoUsed/100);
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "Process killed by deadlock algorithm: %d\n", terminated);
	logging(addToLogBuffer);
	deadlockPercentage = (terminated / deadlockAlgoUsed) * 100;
	sprintf(addToLogBuffer, "%.1f percent deadlock was detected.\n", deadlockPercentage);
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "\n\nVerbose OFF:\n");
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "OSS: DeadlockDetected: killing processes...\n");
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "Process killed by deadlock algorithm: %d\n", terminated);
	logging(addToLogBuffer);
	logAllocationTable();


}

void logAllocationTable() {
	sprintf(addToLogBuffer, "\n\nResource Allocation Table\n");
	logging(addToLogBuffer);
	sprintf(addToLogBuffer, "    P0  P1  P2  P3  P4  P5  P6  P7  P8  P9  P10  P11  P12  P13  P14  P15  P16  P17  P18  P19\n");
	logging(addToLogBuffer);
	for (int i = 0; i < 20; i++) {
		sprintf(addToLogBuffer, "R%d   ", i);
		logging(addToLogBuffer);
		for (int j = 0; j < MAXCHILDREN; j++) {
			sprintf(addToLogBuffer, "%d    ", shmPtr->arrResources[i].allocated[j] *-1);
			logging(addToLogBuffer);
		}
		sprintf(addToLogBuffer, "\n");
		logging(addToLogBuffer);
	}

}


int main(int argc, char *argv[]){

	//signal handlers
	signal(SIGALRM, timesUp);
	signal(SIGINT, ctrlC);
	signal(SIGKILL, cleanAll);
	signal(SIGSEGV, cleanAll);
	

	//referred from stackoverflow, used in previous projects
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = mySigchldHandler;
	sigaction(SIGCHLD, &sa, NULL);

	
	key_t shmKey;

	//allocate shared memory for the clock
	shmKey = ftok("./README.md", 'a');
	shmid = shmget(shmKey, sizeof(timeStruct), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("oss: Error: shmget error, creation failure.\n");
		cleanAll();
		exit(0);
	}

	shmPtr = (timeStruct*)shmat(shmid, NULL, 0);
	if (shmPtr == (timeStruct*)-1) {
		perror("oss: Error: shmat error, attachment failure.\n");
		cleanAll();
		exit(0);
	}

	key_t semKey;

	//create and attach semaphore for the clock
	semKey = ftok("./Makefile", 'a');
	semid = semget(semKey, 2, IPC_CREAT | 666);

	if (semid < -1) {
		perror("oss: Error: semget1 error, creation failure.\n");
		cleanAll();
		exit(0);
	}

	fp = fopen("logfile.txt", "a");
	logging("OSS logfile: \n");

	/*/create and attach semaphore for the resources
	key_t semKey2;
	semKey2 = ftok("./oss.h", 'a');
	semResources = semget(semKey2, 1, IPC_CREAT | 666);

	if (semResources < -1) {
		perror("oss: Error: semget2 error, creation failure.\n");
	}*/

	//init semaphore, one for the clock , one for the resources
	semctl(semid, 0, SETVAL, 1);
	semctl(semid, 1, SETVAL, 1);

	//init shared memory
	initSharedMemory();
	printf("Will terminate in 5 real world secs...\n");
	alarm(timeTermination);

	while (1) {

		if (numOfUsers == 0) {
			forkNano = rand() % 500000000;
			forking();
		}
		else if (numOfUsers < MAXCHILDREN){
			if (nextFork() == 1) {
				forking();
				logging("Doing next forking...");
				forkNano = shmPtr->nanoseconds + (rand() % 500000000);
				forkCheck();
			}
		}

		sem_wait(SEMRESOURCE);
		if (numOfUsers > 0) {
			checkChild();
			resourceManager();
			resourceAllocation();
			deadlockDetection();
			deadlockAlgoUsed++;
		}
		sem_signal(SEMRESOURCE);

		//update the system clock
		sem_wait(SEMCLOCK);
		waited = (rand() % 11);
		nanoIncrement = 1 + (rand() % 100000000);
		shmPtr->nanoseconds += nanoIncrement;
		milliIncrement = nanoIncrement / 1000000;
		shmPtr->milliseconds += milliIncrement;
		clockCheck();
		sem_signal(SEMCLOCK);

	}
	return 0;
	////END OF MAIN//////////////////////////////////////////

}

