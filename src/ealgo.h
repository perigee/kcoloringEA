#ifndef MY_EALGO_H
#define MY_EALGO_H

///////////////////////////////
/////////// EA with IIS
/////////// author: Jun HU 
///////////////////////////////

#include <stdbool.h>
#include "graphe.h"


//====================================================
// Algorithm parameters
int MAX_LocalSearch_Iteration;
int Nb_Generation;
int MAX_RemoveColors;
//====================================================


//==================================================== BGN
// define the crossover and mutation function types
// ease the testing among different crossover and mutation 
// combinations

/*!
 * crossover operator
 * @param 0 number of parents participated in crossover
 * @param 1 the number of whole population
 * @param 2 the whole population
 * @param 3 carry out the created offspring
 * @param 4 adjacent matrix
 * @param 5 counter the participation number of each parent
 * @param 6 maximal number of removed colors
 * @param 7 weights on variables/nodes
 * @return  true if find consistent solution
 */
typedef bool (*CrossoverFuncPtr)(int, int, int**, 
			      int*, char**, int*,
			      int, int*);

/*!
 * mutation operator
 * @param 0 carry out the mutant (input the origin)
 * @param 1 adjacent matrix
 * @return true if the individual is complete and consistent
 */
typedef bool (*MutationFuncPtr)(int*, char**, int*);
//==================================================== END




bool tabuCol(int* a, char** graph, int colorNB, 
	     int maxIteration); /// tabuCol with gamma matrix


/*!
 * ea + distance
 * @param funcCrossPtr crossover operator
 * @param funcMutationPtr mutation operator
 * @param graph adjacent matrix of given graph
 * @param savefile output file
 * @param inputFile input file
 * @return true if the solution found is consistent, otherwise false 
 */
bool ea(CrossoverFuncPtr funcCrossPtr, MutationFuncPtr funcMutationPtr,
	char** graph, char *filename, char *inputFile); /// ea + distance 


void testAlgo(char *filename, char *inNbColor, char *inPopuSize, 
	      char *inLSIter, char *inMaxLSIter, char *inGenItr,
	      char *inMaxRemoveColor, char* savefilename);

#endif
