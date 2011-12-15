#ifndef MY_GRAPHE_H
#define MY_GRAPHE_H
/*
 *  graphe.h
 *  tabucol
 *
 *  Created by Laurent Moalic on 14/02/11.
 *  Copyright 2011 UTBM. All rights reserved.
 *
 */

//#include "node.h"

#include<time.h>
#include <stdbool.h>

float lambda;
int L;

int nbColor; // number of colors given
int populationSize; // number of individuals
int nbLocalSearch;

int nbIterations;
int nbIterationsCross;
int nbEdgesConflict;
int nbNodesConflict;
int nbBestEdgesConflict;

/// caraterisation du graphe
char filename[255];
int nbSommets;
int nbArretes;
char** tConnect; // tableau carre des connections noeud à noeud
int** tVoisins; // tableau qui pour chaque noeud contient la liste des voisins
int* tNbVoisins; // tableau qui pour chaque noeud contient le nombre de voisins

/// caracterisation d'une instance de solution
// tPopulationColor is a matrix where each line is an individual, each column is a color of node
int** tPopulationColor; // tableau d'individus, chaque individu etant un tColor
int* tColor; // tableau contenant pour chaque noeud sa couleur. tColor pointe sur l'instance a intensifier (pas de malloc !)
int* tConflicts; // tableau contenant le nombre de conflits de chaque noeud
int** tNewConflitsWithColor; // contient pour chaque noeud ne nb de conflis en plus ou moins si changement vers chaque couleur, suivi de la meilleure couleur et meilleur gain associé 
int** tTabou; // contient pour chaque noeud et chaque couleur la fin de la période taboue


void loadGraphe();
void buildVoisins();
void initRandomColor();
void improvInitPop();
void initConflict();
//int countNbConflicts();
//int countNbNodesConflicts();
void determineBestImprove();
void updateTables(int node, int color);
void buildNextColorCost(int node);
void testUpdateNexConflicts(int node);

///////////////////////////////
/////////// EA ////////////////
///////////////////////////////
int* tSizeOfColors1; /// nombre de sommets dans chaque classe pour le parent 1
int* tSizeOfColors2; /// nombre de sommets dans chaque classe pour le parent 2
int* tParent1Tmp; /// vecteur pour operation de croisement
int* tParent2Tmp; /// vecteur pour operation de croisement
int* tChild; /// contient le resultat du croisement de 2 parents

void updatePopulation();
void buildChild(int* p1, int* p2);


///////////////////////////////
/////////// EA with distance 
///////////////////////////////
bool tabuCol(int* a, char** graph); /// tabuCol with gamma matrix
//int distance(int* a, int* b); /// distance between two individuals
//int** chooseParents(int** population); /// choose the parents from population
//int crossover(int** parents, int* offspring); /// crossover the parents to create offspring
bool ea(char** graph); /// ea + distance 

///////////////////////////////
/////////// FILE ////////////////
///////////////////////////////
void save(char* filename);


clock_t  time1;
clock_t  time2;
clock_t  time3;
int nbMinutes1;
int nbMinutes2;
int nbMinutes3;

#endif
