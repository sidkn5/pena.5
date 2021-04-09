CC = gcc
CFLAG = -g -lm
 
all: oss user_proc

%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@

oss: oss.o
	$(CC) $(CFLAG) $< -o $@ -lm

user_proc: user_proc.o
	$(CC) $(CFLAG) $< -o $@ -lm

clean:
	rm -f *.o oss user_proc *.txt

