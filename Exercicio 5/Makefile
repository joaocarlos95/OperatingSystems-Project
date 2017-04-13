# Makefile 

all: par-shell fibonacci par-shell-terminal

par-shell: commandlinereader.o par-shell.o list.o 
	gcc -o par-shell commandlinereader.o par-shell.o list.o -lpthread

par-shell-terminal: commandlinereader.o par-shell-terminal.o list.o 
	gcc -o par-shell-terminal commandlinereader.o par-shell-terminal.o list.o -lpthread

fibonacci: fibonacci.c
	gcc -o fibonacci fibonacci.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc -Wall -g -c commandlinereader.c

list.o: list.c list.h
	gcc -Wall -g -c list.c

par-shell.o: par-shell.c commandlinereader.h list.h
	gcc -Wall -g -c par-shell.c

par-shell-terminal.o: par-shell-terminal.c
	gcc -Wall -g -c par-shell-terminal.c

clear:
	rm -f *.o *.txt par-shell fibonacci par-shell-terminal