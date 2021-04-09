#include <stdio.h>
#include <unistd.h>


#include "oss.h"
//termination criteria
int timeTermination = 5;		//5 sec time termination
int totalChildrenCounter = 0;	//Maximum of 40 children then terminate

//shared memory
int shmid;
timeStruct* shmPtr;			//just this for now, needs a struct

int main(int argc, char *argv[]){

	
	key_t shmKey;

	//allocate shared memory for the clock
	shmKey = ftok("./README.md", 'a');
	shmid = shmget(shmKey, sizeof(timeStruct), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("oss: Error: shmget error, creation failure.\n");
		cleanAll();
		exit(1);
	}

	shmPtr = (int*)shmat(shmid, NULL, 0);
	if (shmPtr == (holder*)-1) {
		perror("oss: Error: shmat error, attachment failure.\n");
		cleanAll();
		exit(1);
	}
	
	return 0;

}
