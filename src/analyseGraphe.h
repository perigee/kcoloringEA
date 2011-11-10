#ifndef ANALYSE_GRAPHE_H
#define ANALYSE_GRAPHE_H
/*
 *  analyseGraphe.h
 *  tabucol
 *
 *  Created by Laurent Moalic on 14/02/11.
 *  Copyright 2011 UTBM. All rights reserved.
 *
 */


#include "graphe.h"

int* tColorFinal; // tableau contenant pour chaque noeud sa couleur a la fin de l'execution
void buildTfinal(); // cree et recopie le tableau
int getNbNoeudsCommuns(); // retourne à tous moments le nb de noeuds ayant la meme couleur

#endif
