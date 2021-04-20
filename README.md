Student: Sean Dela Pena

Professor: Dr. Sanjiv Bhatia

Assignment 5: Resource Mangement

Purpose: The goal of this homework is to learn about resource management inside an operating system.
You will work on the specified strategy to manage resources and take care of any possible
starvation/deadlock issues.

USAGE: ./oss
Simply run the code without options. It was mentioned that we could add options ourselves if we wanted
to. So I assumed it was not necessary.


Problems/Notes:

	I'm not really sure why we decided to terminate the program after "5 real world seconds". Is this 
	even enough time to see valuable data? The deadlock percentage will almost always be 0 because of the 
	sheer amount of time the algorithm is ran compared to how many deadlocks it actually detects.

	In terms of logging and reporting, I don't think I missed anything but I did not copy the logging example
	given in the prompt word per word so it might look different.

	During the report at the end of the logfile.txt, I will add a verbose off report, simulating a verbose
	off option since this program does not allow any options from the user. It was not explained very well
	on the project description on what this actually does aside from it should log how if a deadlock is detected.
	So I'm just going to log how many deadlocks were detected, and how process were killed. 

	I have added sleeps in the program as well, just to give the program time to clean up when terminated.

	That being said, I didn't really find any bugs from my testing.


Functions in oss.c:


