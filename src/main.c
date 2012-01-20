#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util/gfile.h"
//#include "graphe.h"
#include "ealgo.h"




int main (int argc, char * const argv[]) {


  //================================== IIS inspired algorithm ================ BGN


  srand((unsigned)time(NULL));
  printf("d: Procedure de calcul \n");
  
  if(argc<5){
    printf("Erreur d'utilisation !!\n");
    exit(-1);
    }
  testAlgo(argv[1], argv[2],argv[3],argv[4],argv[5],
	   argv[6], argv[7]);
	   return 0;
  //================================== IIS inspired algorithm ================ END


  

	printf("Procedure de calcul\n");
	
	solutionFileName = malloc(sizeof(char)*250);
	fitnessFileName = malloc(sizeof(char)*250);
	
	
	//srand(time(NULL));
	srand(123456);

	
	
	clock_t startTime=clock();
	clock_t currentStartTime;
	double elapsedTime;
	
	
	//// >>>>>>>>>  affichage de l'heure au debut
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "Lancement du calcul :  %s", asctime (timeinfo) );
	//// <<<<<<<<<  affichage de l'heure de debut

	
	
	for (int i=0; i<1000; i++) {
/*///////// >>>>>> Lancement sur dsjc1000.5
		nbColor=48;
		populationSize=10;
		nbLocalSearch=10000;
		withBoucle=0;
		loadGraphe("data/DSJC500.5.txt");
		fitnessFileName="fitness_dsjc500_5.csv";
		solutionFileName="solution_dsjc500_5.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<3000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
*/
		
///////// >>>>>> Lancement sur dsjc1000.5
		nbColor=48;
		populationSize=15;
		nbLocalSearch=10000;
		withBoucle=1;
		alpha=150.0;
		loadGraphe("data/DSJC500.5.txt");
		fitnessFileName="fitness_dsjc500_5_boucle_150.csv";
		solutionFileName="solution_dsjc500_5_boucle_150.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<3000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d (boucle)  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		cleanMatrix();
	/*	
//////////////
		nbColor=48;
		populationSize=10;
		nbLocalSearch=10000;
		withBoucle=1;
		alpha=50.0;
		fitnessFileName="fitness_dsjc500_5_boucle_50.csv";
		solutionFileName="solution_dsjc500_5_boucle_50.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<3000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d (boucle)  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		
		
//////////////
		nbColor=48;
		populationSize=10;
		nbLocalSearch=10000;
		withBoucle=1;
		alpha=200.0;
		fitnessFileName="fitness_dsjc500_5_boucle_200.csv";
		solutionFileName="solution_dsjc500_5_boucle_200.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<3000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d (boucle)  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		cleanMatrix();

 /*
///////// >>>>>> Lancement sur dsjc1000.5
		nbColor=83;
		populationSize=10;
		nbLocalSearch=16000;
		withBoucle=0;
		loadGraphe("data/DSJC1000.5.txt");
		fitnessFileName="fitness_dsjc1000_5.csv";
		solutionFileName="solution_dsjc1000_5.csv";

		clock_t currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();

		if (nbEdgesConflict==0)
			save();
		
		double elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
///////// >>>>>> Lancement sur dsjc1000.5
		nbColor=83;
		populationSize=10;
		nbLocalSearch=16000;
		withBoucle=1;
		fitnessFileName="fitness_dsjc1000_5_boucle.csv";
		solutionFileName="solution_dsjc1000_5_boucle.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		cleanMatrix();
///////// >>>>>> Lancement sur flat1000_76
		nbColor=83;
		populationSize=10;
		nbLocalSearch=16000;
		withBoucle=0;
		loadGraphe("data/flat1000_76_0.txt");
		fitnessFileName="fitness_flat1000_76_0.csv";
		solutionFileName="solution_flat1000_76_0.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);		
	
		
///////// >>>>>> Lancement sur flat1000_76
		nbColor=83;
		populationSize=10;
		nbLocalSearch=16000;
		withBoucle=1;
		fitnessFileName="fitness_flat1000_76_0_boucle.csv";
		solutionFileName="solution_flat1000_76_0_boucle.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		cleanMatrix();
///////// >>>>>> Lancement sur flat300_28
		nbColor=31;
		populationSize=10;
		nbLocalSearch=2000;
		withBoucle=0;
		loadGraphe("data/flat300_28_0.txt");
		fitnessFileName="fitness_flat300_28_0.csv";
		solutionFileName="solution_flat300_28_0.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		
///////// >>>>>> Lancement sur flat300_28
		nbColor=31;
		populationSize=10;
		nbLocalSearch=2000;
		withBoucle=1;
		fitnessFileName="fitness_flat300_28_0_boucle.csv";
		solutionFileName="solution_flat300_28_0_boucle.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<5000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
	
	
		cleanMatrix();

  ///////// >>>>>> Lancement sur le450_25c
		nbColor=25;
		populationSize=10;
		nbLocalSearch=12000;
		withBoucle=0;
		loadGraphe("data/le450_25c.txt");
		fitnessFileName="fitness_le450_25c.csv";
		solutionFileName="solution_le450_25c.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<8000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		
		
		
///////// >>>>>> Lancement sur le450_25c
		nbColor=25;
		populationSize=10;
		nbLocalSearch=12000;
		withBoucle=1;
		fitnessFileName="fitness_le450_25c_boucle.csv";
		solutionFileName="solution_le450_25c_boucle.csv";
		
		currentStartTime=clock();
		
		initRandomColor();
		initConflict();
		improvInitPop();
		
		
		while (nbEdgesConflict > 0 && nbIterationsCross<8000)
			crossIteration();
		
		if (nbEdgesConflict==0)
			save();
		
		elapsedTime=(clock()-currentStartTime) / (double)CLOCKS_PER_SEC;
		printf("\n\n%d  => Fin apres %d iteration et %d croisements (%f sec)\n",i,nbIterations, nbIterationsCross,elapsedTime);
		printf("\tNombre d'iterations pour le fils final avant solution : %d\n", nbIterations%nbLocalSearch);
		printf("\t\tRecherche meilleur : %fsec\n", time1 / (double)CLOCKS_PER_SEC);
		printf("\t\tMise a jour : %fsec\n", time2 / (double)CLOCKS_PER_SEC);
		

		cleanMatrix();
  */
	}	
	
	
	
    elapsedTime=(clock()-startTime) / (double)CLOCKS_PER_SEC;
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
//	///////////////////////////////////////////////////////////////
//	/////// Ajout pour analyse de la façon dont on se rapproche de la solution finale
//	///////////////////////////////////////////////////////////////
//	
//	int* tOptimum;
//	tOptimum=malloc(sizeof(int) * nbSommets);
//	for (int i=0; i<nbSommets; i++) {
//		tOptimum[i]=tColor[i];
//	};
//	srand(123456);
//	initRandomColor();
//	initConflict();
//	improvInitPop();
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
//		crossIteration();
//		
//		printf("Proximite avec l'optimum : ");
//		for (int i=0; i<populationSize; i++) {
//			int proxi=getProximite(tOptimum, tPopulationColor[i]);
//			printf("\t %d", proxi);
//		}
//		printf("\n");
//	}
//	
//	
//
//	printf("Proximite avec l'optimum : ");
//	int proxi=getProximite(tOptimum, tColor);
//	printf("\t %d", proxi);
//	printf("\n");
	
    return 0;
}
