# Makefile 

par-shell: commandlinereader.o par-shell.o list.o 
	gcc -o par-shell commandlinereader.o par-shell.o list.o

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc -Wall -g -c commandlinereader.c

list.o: list.c list.h
	gcc -Wall -g -c list.c

par-shell.o: par-shell.c commandlinereader.h list.h
	gcc -Wall -g -c par-shell.c

fibonacci: fibonacci.c
	gcc -o fibonacci fibonacci.c
clear:
	rm -f *.o par-shell fibonacci
