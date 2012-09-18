#ifndef MY_EALGO_H
#define MY_EALGO_H

#include <stdbool.h>
#include "graphe.h"

//====================================================
// Algorithm parameters
int MAX_LocalSearch_Iteration;
int Nb_Generation;
int MAX_RemoveColors;
//====================================================

bool partitionMatch(char *filename, char* inNbColor, char *inMaxIter);

#endif
