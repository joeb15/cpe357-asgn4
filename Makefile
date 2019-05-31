CC = gcc
CFLAGS = -Wall -ansi -pedantic --std=c99
OBJECTS = utils.o create.o extract.o table.o


all : mytar

mytar : $(OBJECTS) mytar.c
	$(CC) $(CFLAGS) -o mytar $(OBJECTS) mytar.c
	rm *.o

create.o : create.c create.h
	$(CC) $(CFLAGS) -o create.o -c create.c

extract.o : extract.c extract.h
	$(CC) $(CFLAGS) -o extract.o -c extract.c

table.o : table.c table.h
	$(CC) $(CFLAGS) -o table.o -c table.c

utils.o : utils.c utils.h
	$(CC) $(CFLAGS) -o utils.o -c utils.c
