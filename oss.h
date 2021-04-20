#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXCHILDREN 18
#define MAXRESOURCES 20
#define SEMCLOCK 1
#define SEMRESOURCE 0

typedef struct {
	int maxClaim[MAXCHILDREN];
	int request[MAXCHILDREN];
	int allocated[MAXCHILDREN];
	int release[MAXCHILDREN];

	int available;
	int instances;			//number of instance so the resource
	bool shared;			//shared resources
	int remaining;
}resources;

typedef struct {

	unsigned int seconds;			//shared memory clock
	unsigned int milliseconds;		//shared memory clock not needed
	unsigned int nanoseconds;		//shared memory clock

	int arrPid[MAXCHILDREN];				//status of forked children//finished
	bool waiting[MAXCHILDREN];				//status of child if waiting or not
	int wantedResources[MAXCHILDREN];		//the resources the child wants
	long childWaitTime[MAXCHILDREN];			//wait time accumulator
	long cpuTime[MAXCHILDREN];				//cpu time accumulator
	int childThroughput[MAXCHILDREN];		//counter for throughput
	int sleep[MAXCHILDREN];					//if the process is sleeping or not
	int childRunning[MAXCHILDREN];			//currently running pids
	resources arrResources[MAXRESOURCES];	//array of resource descriptor

}timeStruct;