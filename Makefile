TARGET = test #executive file name
CC = clang++ 
CFLAGS = -std=c++11 -stdlib=libc++
VPATH = src/ #space seperate the different dirs
#BIN_DIR = bin
OBJECTS = $(wildcard *.o)

#all : main.o gfile.o graphe.o ealgo.o matching.o analyseGraphe.o
all : main.o io.o #gfile.o graphe.o matching.o analyseGraphe.o
	$(CC) -o $(TARGET) $(CFLAGS) $^ -O2 

#create obj files

main.o: main.cpp io.hpp
	$(CC) -c $(CFLAGS) $< -o $@
io.o: io.hpp
	$(CC) -c $(CFLAGS) $< -o $@
#graphe.o: graphe.c graphe.h
#	$(CC) -c $(CFLAGS) $< -o $@
#matching.o : matching.c matching.h
#	$(CC) -c $(CFLAGS) $< -o $@ 
#analyseGraphe.o: analyseGraphe.c analyseGraphe.h
#	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean: 
	rm  $(TARGET) $(OBJECTS)




