TARGET = eagraph #executive file name
CC = gcc
CFLAGS = -Wall -std=c99 #need the iso99 standard 
VPATH = src/util:src #space seperate the different dirs
#BIN_DIR = bin
OBJECTS = $(wildcard *.o)

all : main.o gfile.o graphe.o analyseGraphe.o
	$(CC) -o $(TARGET) $^

#create obj files
main.o: main.c gfile.h graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
#datas.o : datas.c datas.h
#	$(CC) -c $(CFLAGS) $< -o $@
gfile.o : gfile.c gfile.h
	$(CC) -c $(CFLAGS) $< -o $@
graphe.o: graphe.c graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
analyseGraphe.o: analyseGraphe.c analyseGraphe.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean: 
	rm  $(TARGET) $(OBJECTS)