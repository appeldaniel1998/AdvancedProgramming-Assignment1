# Compilation of all files must be enabled using the command 'Make all'
# Make clean must delete all craft

CC = gcc
AR = ar
FLAGS = -Wall -g

run: main
	./myshell

all: main

main: myshell.o
	$(CC) -pthread -o myshell myshell.c $(FLAGS)

.PHONY: clean all

#clean all files
clean:
	rm -f *.o shell
