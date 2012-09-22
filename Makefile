TARGET = eagraph #executive file name
CC = clang -g #-g
CFLAGS = -O2 -Wall -std=c99 -mtune=native -march=native #need the iso99 standard 
VPATH = src/util:src #space seperate the different dirs
#BIN_DIR = bin
OBJECTS = $(wildcard *.o)

#all : main.o gfile.o graphe.o ealgo.o matching.o analyseGraphe.o
all : main.o gfile.o graphe.o matching.o analyseGraphe.o
	#$(CC) -o $(TARGET) $^ -pg -lm -O2 -mtune=native -march=native
	$(CC) -o $(TARGET) $^  -O2 -mtune=native -march=native

#create obj files
main.o: main.c gfile.h graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
gfile.o : gfile.c gfile.h
	$(CC) -c $(CFLAGS) $< -o $@
graphe.o: graphe.c graphe.h
	$(CC) -c $(CFLAGS) $< -o $@
matching.o : matching.c matching.h
	$(CC) -c $(CFLAGS) $< -o $@ 
analyseGraphe.o: analyseGraphe.c analyseGraphe.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean: 
	rm  $(TARGET) $(OBJECTS)




