#include <stdio.h>
#include <stdlib.h>

#define maxChildren 18
#define maxResources 20

typedef struct {
	int maxClaim[maxChildren];		
	int request[maxChildren];
	int allocated[maxChildren];		
	int release[maxChildren];		

	int available;
	int instances;			//number of instance so the resource
	int shared;			//shared resources
}resources;

typedef struct {

	unsigned int seconds;			//shared memory clock
	unsigned int milliseconds;		//shared memory clock
	unsigned int nanoseconds;		//shared memory clock

	resources arrResources[maxResources];	//array of resource descriptor

	


}timeStruct;