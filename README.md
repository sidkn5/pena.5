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
	even enough time to see valuable data? With all my test I don't think I've ran to a deadlock just yet.
	The time that we are allowed to run the program is too short in my opinion. I really like looking at
	the resource allocation at work though, since we can actually see that despite the short time we are
	given. 

	In terms of logging and reporting, I don't think I missed anything but I did not copy the logging example
	given in the prompt word per word so it might look different.

	During the report at the end of the logfile.txt, the deadlock detection and percentage will most likely
	if not always zero. As I mentioned above, I have never encountered a deadlock mainly due to the fact
	that we are only running it for 5 seconds. This is just a guess, however.

	I have added sleeps in the program as well, just to give the program time to clean up when terminated.

	That being said, I didn't really find any bugs from my testing.


Functions in oss.c:


