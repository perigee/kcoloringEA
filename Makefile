TARGET = eacolor #executive file name
CC = gcc
CFLAGS = -Wall -std=c99 #need the iso99 standard 
VPATH = src/util:src #space seperate the different dirs
BIN_DIR = bin
OBJECTS = $(wildcard *.o)

eacolor : main.o gfile.o graphe.o analyseGraphe.o
	$(CC) -o $(TARGET) $^
main.o: main.c gfile.h graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
gfile.o : gfile.c gfile.h
	$(CC) -c $(CFLAGS) $< -o $@
graphe.o: graphe.c graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
analyseGraphe.o: analyseGraphe.c analyseGraphe.h
	$(CC) -c $(CFLAGS) $< -o $@


clean: 
	rm  $(OBJECTS)