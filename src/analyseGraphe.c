/*
 *  analyseGraphe.c
 *  tabucol
 *
 *  Created by Laurent Moalic on 14/02/11.
 *  Copyright 2011 UTBM. All rights reserved.
 *
 */
#include <stdlib.h>
#include "analyseGraphe.h"

void buildTfinal(){
	tColorFinal = malloc(sizeof(int)*nbSommets);
	
	for (int i=0; i<nbSommets; i++) {
		tColorFinal[i]=tColor[i];
	}
}


int getNbNoeudsCommuns(){
	int res=0;
	for (int i=0; i<nbSommets; i++) {
		if (tColor[i]==tColorFinal[i]) {
			res++;
		}
	}
	
	return res;
}