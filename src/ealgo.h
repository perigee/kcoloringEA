#ifndef MY_EALGO_H
#define MY_EALGO_H

///////////////////////////////
/////////// EA with IIS
/////////// author: Jun HU 
///////////////////////////////

#include <stdbool.h>
#include "graphe.h"



//========================= Algorithm parameters ============= BGN
int MAX_LocalSearch_Iteration;
int Nb_Generation;
int MAX_RemoveColors;
//========================= Algorithm parameters ============= END



bool tabuCol(int* a, char** graph, int colorNB, 
	     int maxIteration, int *weightVars); /// tabuCol with gamma matrix
//int distance(int* a, int* b); /// distance between two individuals
//int** chooseParents(int** population); /// choose the parents from population
//int crossover(int** parents, int* offspring); /// crossover the parents to create offspring
bool ea(char** graph, char *filename, char *inputFile); /// ea + distance 

void testAlgo(char *filename, char *inNbColor, char *inPopuSize, 
	      char *inLSIter, char *inMaxLSIter, char *inGenItr,
	      char *inMaxRemoveColor, char* savefilename);

#endif
