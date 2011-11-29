#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util/gfile.h"
#include "graphe.h"



int main (int argc, char * const argv[]) {
	printf("Procedure de calcul\n");
	
	if(argc<2){
		printf("Erreur d'utilisation !!\n");
		exit(-1);
	}
	
	srand(time(NULL));
	//srand(123456);
	loadGraphe(argv[1]);

	
	
	
	clock_t startTime=clock();
	
	//// >>  affichage de l'heure au debut
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "Lancement du calcul :  %s", asctime (timeinfo) );
	//// <<  affichage de l'heure de debut
	
	
	
	initRandomColor();
	initConflict();
	//improvInitPop();

	// genetic algorithm: stops while no edge conflict found
	while (nbEdgesConflict > 0 /*&& nbIterations<200000*/) {
		//printf("%d : %d \t %d\n", nbIterations, nbEdgesConflict, nbNodesConflict);
		//determineBestImprove();
		updatePopulation();
		printf("%d\t", nbNodesConflict);
	}
	
	printf("\n\nFin apres %d iteration et %d croisements\n",nbIterations, nbIterationsCross);
	printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
	
	
	
	save("resultat.txt");
	
	
	
	
    double elapsedTime=(clock()-startTime) / (double)CLOCKS_PER_SEC;
	printf("<< Procedure de calcul apres: %f secondes\n",elapsedTime);
	printf("\ttemps d'initialisation des conflits : %f sec\n", time1 / (double)CLOCKS_PER_SEC);
	printf("\ttemps de croisement : %f sec\n", time2 / (double)CLOCKS_PER_SEC);
	printf("\ttemps d'intensification tabucol : %f sec\n", time3 / (double)CLOCKS_PER_SEC);
	
	
	
	initConflict();
	printf("JUST FOR CHEKING : %d   %d\n\n", nbEdgesConflict, nbNodesConflict);
	
	
	//// >>  affichage de l'heure a la fin
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "Fin du calcul :  %s", asctime (timeinfo) );
	//// <<  affichage de l'heure a la fin
	
	
	///////////////////////////////////////////////////////////////
	/////// Ajout pour analyse de la façon dont on se rapproche de la solution finale
	///////////////////////////////////////////////////////////////
//	buildTfinal();
//	srand(123456);
//	initRandomColor();
//	initConflict();
//	//nbEdgesConflict=countNbConflicts();
//	
//	
//	startTime=clock();
//	
//	printf("\n\n\n2EME PASSE POUR ANALYSE\n");
//	printf("nb edges conflits au demarrage: %d\n", nbEdgesConflict);
//	printf("nb nodes conflits au demarrage: %d\n", nbNodesConflict);
//	
//	
//	
//	while (nbEdgesConflict > 0 /*&& nbIterations<600000*/) {
//				if( nbIterations%100 == 0)
//					printf("%d : %d \t %d \t %d\n", nbIterations, nbEdgesConflict, nbNodesConflict, getNbNoeudsCommuns());
//		determineBestImprove();
//	}
//	
//	
//	printf("Fin apres %d iteration\n",nbIterations);
//	//	cout << "Nombre d'arretes en conflits : "<<g.countNbConflicts()<<endl;
//	//	cout << "Nombre de noeud en conflits : "<<g.nbNodesConflict <<endl;
//	
//	
//	elapsedTime=(clock()-startTime) / (double)CLOCKS_PER_SEC;
//	printf("<< Procedure de calcul apres: %f secondes\n",elapsedTime);
//	printf("\ttemps de calcul du meilleur : %f sec\n", time1 / (double)CLOCKS_PER_SEC);
//	printf("\ttemps de maj : %f sec\n", time2 / (double)CLOCKS_PER_SEC);
//	printf("\tdont temps de couleurs suivantes : %f sec\n", time3 / (double)CLOCKS_PER_SEC);
//	printf("Compteur=%d\n", compteur);
	
	
    return 0;
}
