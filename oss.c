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
timeStruct* shmPtr;			//just this for now, needs a struct

FILE* fp;
char addToLogBuffer[100];
int pids[maxChildren];

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

int main(int argc, char *argv[]){

	//signal handlers
	signal(SIGALRM, timesUp);
	signal(SIGINT, ctrlC);
	signal(SIGKILL, cleanAll);
	signal(SIGSEGV, cleanAll);

	//referred from stackoverflow
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
		exit(1);
	}

	shmPtr = (timeStruct*)shmat(shmid, NULL, 0);
	if (shmPtr == (timeStruct*)-1) {
		perror("oss: Error: shmat error, attachment failure.\n");
		cleanAll();
		exit(1);
	}

	key_t semKey;

	//create and attach semaphore array
	semKey = ftok("./Makefile", 'a');
	semid = semget(semKey, 1, IPC_CREAT | 666);

	if (semid < -1) {
		perror("oss: Error: semget error, creation failure.\n");
	}

	//init semaphore//might change later
	semctl(semid, 0, SETVAL, 0);

	//init shared memory
	initSharedMemory();
	
	return 0;
	////END OF MAIN//////////////////////////////////////////

}


//deallocates and frees everything
void cleanAll() {
	killChildPids();
	shmdt(shmPtr);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID, NULL);

	exit(0);

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

void logging(char *buffer) {
	fputs(buffer, fp);
}

void initSharedMemory() {
	shmPtr->seconds = 0;
	shmPtr->milliseconds = 0;
	shmPtr->nanoseconds = 0;
}

void killChildPids() {
	int i;
	for (i = 0; i < maxChildren; i++) {
		if (pids[i] != 0) {
			kill(pids[i], SIGKILL);
		}
	}
}