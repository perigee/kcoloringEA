#ifndef MY_FILE_H
#define MY_FILE_H

#include <stdio.h>

#define MAX_LINE_LENGTH 32768


FILE* file ;
char line[MAX_LINE_LENGTH] ;
char token[MAX_LINE_LENGTH] ;
int indiceToken ;


void openfile(char *) ;
void closefile();
char* readLine();
char* readUncommentedLine();
char* getNextToken();

#endif
