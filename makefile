CC = gcc
FLAGS = -Wall -g
LDFLAGS = 


all: myshell 

myshell: myshell.o
	$(CC) $(FLAGS) myshell.o -o myshell -lreadline

myshell.o: myshell.c
	$(CC) $(FLAGS)  -c myshell.c



clean: 
	rm -f *.o myshell *.txt

