/*
 *  graphe.c
 *  tabucol
 *
 *  Created by Laurent Moalic on 14/02/11.
 *  Copyright 2011 UTBM. All rights reserved.
 *
 */

#include <stdlib.h>
#include "graphe.h"
#include "util/gfile.h"



clock_t  time1=0;
clock_t  time2=0;
clock_t  time3=0;


float lambda=0.6;
int L=10;


void loadGraphe(char* filename){	
	openfile(filename);

	
	char* buf;
	char* tok;
	
	int nbArretesAjoutee=0;
	
	while (buf=readUncommentedLine()) {
		tok=getNextToken();
		
		if(*tok=='p'){ // lecture de la taille du graphe
			tok=getNextToken(); //pas interessant
			tok=getNextToken();
			nbSommets=atoi(tok);
			tok=getNextToken();
			nbArretes=atoi(tok);
			
			// creation du graphe
			tConnect=malloc(sizeof(char*) * nbSommets);
			for(int i=0; i<nbSommets; i++){
				tConnect[i]=malloc(sizeof(char) * nbSommets);
				for (int j=0; j<nbSommets; j++)
					tConnect[i][j]=0;
			}
			
			// creation de la couleur associée à chaque sommet pour tous les individus de la population
			tPopulationColor=malloc(sizeof(int*) * populationSize);
			for (int i=0; i<populationSize; i++) {
				tPopulationColor[i] = malloc(sizeof(int) * nbSommets);
			}
			tColor = tPopulationColor[0];
			
			// creation de la meilleure solution trouvee
			tBestColor = malloc(sizeof(int) * nbSommets);
	
			
			// creation du nombre de conflits de chaque sommet
			tConflicts = malloc(sizeof(int) * nbSommets);
			
			// creation de l'impact pour chaque sommet d'une transition vers chaque couleur
			tNewConflitsWithColor = malloc(sizeof(int*) * nbSommets );
			for(int i=0; i<nbSommets; i++){
				tNewConflitsWithColor[i]=malloc(sizeof(int) * (nbColor+2));
			}			
			
			// creation des periodes taboues des sommets
			tTabou = malloc(sizeof(int*) * nbSommets);
			for(int i=0; i<nbSommets; i++){
				tTabou[i]=malloc(sizeof(int) * (nbColor));
			}
			
			// creation de la table contenant le nombre de modificaiton de couleur pour chaque noeud
			tVisite = malloc(sizeof(int) * nbSommets);
			/// init des visites de chaque noeud (nb de changement de couleur)
			for (int i=0; i<nbSommets; i++) {
				tVisite[i]=0;
			}
			
			// creation de la table contenant le nombre de modification de chaque couleur
			tVisitesColor = malloc(sizeof(int) * nbColor);
			/// init des visites de chaque noeud (nb de changement de couleur)
			for (int i=0; i<nbColor; i++) {
				tVisitesColor[i]=0;
			}
			
			// HCA - creation de la proximite entre chaque couple d'individus
			tProximite = malloc(sizeof(int*) * populationSize);
			for (int i=0; i<populationSize; i++) {
				tProximite[i]=malloc(sizeof(int) * populationSize);
			}
			
			// HCA - creation de la fitness de chaque individu
			tFitness = malloc(sizeof(int) * populationSize);
			
			
			// HCA - creation du nb de visite pour chaque noeud de chaque individu
			tVisitesPopulation=malloc(sizeof(int*) * populationSize);
			for (int i=0; i<populationSize; i++) {
				tVisitesPopulation[i]=malloc(sizeof(int) * nbSommets);
			}
			
			// HCA - creation du nombre de modificaiton pour chaque couleur et pour chaque indivdu (nb de fois ou on quitte une couleur)
			tVisitesColorPopulation = malloc(sizeof(int*) * populationSize);
			for (int i=0; i<populationSize; i++) {
				tVisitesColorPopulation[i]=malloc(sizeof(int) * nbColor);
			}
			
			// HCA - creation de la couleur associe a chaque noeud du fils
			tChild = malloc(sizeof(int) * nbSommets);
			
			// HCA - tableau des indices des parents pour creation du fils
			tParentsIndices = malloc(sizeof(int) * MAX_NB_PARENTS);
			
			
			
			printf("Sommets ajoutes: %d\n", nbSommets);
		}
		
		if(*tok=='e'){ // lecture d'une arrete
			tok=getNextToken();
			int v1=atoi(tok);
			tok=getNextToken();
			int v2=atoi(tok);
			tConnect[v1-1][v2-1]=1;
			tConnect[v2-1][v1-1]=1;
			nbArretesAjoutee++;
		}
	}
	
	printf("Arretes ajoutees: %d / %d\n",nbArretesAjoutee ,nbArretes);
	closefile();


	buildVoisins();
}


void loadGrapheSimple(char* filename){	
	openfile(filename);

	
	char* buf;
	char* tok;
	
	int nbArretesAjoutee=0;
	
	while (buf=readUncommentedLine()) {
		tok=getNextToken();
		
		if(*tok=='p'){ // lecture de la taille du graphe
			tok=getNextToken(); //pas interessant
			tok=getNextToken();
			nbSommets=atoi(tok);
			tok=getNextToken();
			nbArretes=atoi(tok);
			
			// creation du graphe
			tConnect=malloc(sizeof(char*) * nbSommets);
			for(int i=0; i<nbSommets; i++){
				tConnect[i]=malloc(sizeof(char) * nbSommets);
				for (int j=0; j<nbSommets; j++)
					tConnect[i][j]=0;
			}
			
			
			printf("d: Sommets ajoutes: %d\n", nbSommets);
		}
		
		if(*tok=='e'){ // lecture d'une arrete
			tok=getNextToken();
			int v1=atoi(tok);
			tok=getNextToken();
			int v2=atoi(tok);
			tConnect[v1-1][v2-1]=1;
			tConnect[v2-1][v1-1]=1;
			nbArretesAjoutee++;
		}
	}
	
	printf("d: Arretes ajoutees: %d / %d\n",nbArretesAjoutee ,nbArretes);
	closefile();


}



void cleanMatrix(){
	for(int i=0; i<nbSommets; i++){
		free(tConnect[i]);
	}
	free(tConnect);
	
	
	for (int i=0; i<populationSize; i++) {
		free(tPopulationColor[i]);
	}
	free(tPopulationColor);
	
	free(tBestColor);

	free(tConflicts);
	
	for(int i=0; i<nbSommets; i++){
		free(tNewConflitsWithColor[i]);
	}
	free(tNewConflitsWithColor);
				
	
	for(int i=0; i<nbSommets; i++){
		free(tTabou[i]);
	}
	free(tTabou);

	free(tVisite);
	
	free(tVisitesColor);

	for (int i=0; i<populationSize; i++) {
		free(tProximite[i]);
	}
	free(tProximite);

	free(tFitness);
	
	for (int i=0; i<populationSize; i++) {
		free(tVisitesPopulation[i]);
	}
	free(tVisitesPopulation);
	
	for (int i=0; i<populationSize; i++) {
		free(tVisitesColorPopulation[i]);
	}
	free(tVisitesColorPopulation);
	
	
	free(tChild);
	
	free(tParentsIndices);
	
	
	for (int i=0; i<nbSommets; i++) {
		free(tVoisins[i]);
	}
	free(tVoisins);
	free(tNbVoisins);
	
}

void buildVoisins(){
	tNbVoisins=malloc(sizeof(int)*nbSommets);
	tVoisins=malloc(sizeof(int*)*nbSommets);
	for (int i=0; i<nbSommets; i++) {
		int nbVois=0;
		for (int j=0; j<nbSommets; j++) {
			if(tConnect[i][j])
				nbVois++;
		}
		tNbVoisins[i]=nbVois;
		tVoisins[i]=malloc(sizeof(int)*nbVois);
		
		int pos=0;
		for (int j=0; j<nbSommets; j++) {
			if(tConnect[i][j])
				tVoisins[i][pos++]=j;
		}
	}
}


void initRandomColor(){
	for (int i=0; i<populationSize; i++) {
		for (int j=0; j<nbSommets; j++) {
			int val=(rand()/(float)RAND_MAX) * nbColor ;
			tPopulationColor[i][j]=val;
		}
	}
	
	nbIterations=0;
	nbIterationsCross=0;
	nbIterationsSinceRestart=0;
	nbBestEdgesConflict=999999;
}


void improvInitPop(){
	for (int i=0; i<populationSize; i++) {
		tColor=tPopulationColor[i];
		initConflict();
		
		for (int j=0; j<nbLocalSearch; j++) {
			determineBestImprove();
		}
		
		tFitness[i]=nbNodesConflict;		
		
		updateProximityTable(i);
	}
}



void initConflict(){	
	/// determine les conflits entre les noeuds
	nbNodesConflict=0;
	nbEdgesConflict=0;
	for (int i=0; i<nbSommets; i++)
		tConflicts[i]=0;
	
	for (int i=0; i<nbSommets; i++) {
		for (int j=i; j<nbSommets; j++) {
			if (tConnect[i][j] && tColor[i]==tColor[j]) {
				tConflicts[i]++;
				tConflicts[j]++;
				nbEdgesConflict++;
				if (tConflicts[i]==1)nbNodesConflict++;
				if (tConflicts[j]==1)nbNodesConflict++;
				
			}
		}
	}
	
	
	/// determine les delta-conflits pour chaque transition de couleur
	for (int i=0; i<nbSommets; i++) {
		int nbCurrentConflict=tConflicts[i];
		for (int j=0; j<nbColor; j++) {
			tNewConflitsWithColor[i][j]=-nbCurrentConflict;
		}
		
		int nbVoisins=tNbVoisins[i];
		for (int j=0; j<nbVoisins; j++) {
			tNewConflitsWithColor[i][tColor[tVoisins[i][j]]]++;		
		}
	}
	
	
	
	/// initialise les durées taboues
	for (int i=0; i<nbSommets; i++) {
		for (int j=0; j<nbColor; j++) {
			tTabou[i][j]=-1;
		}
	}
	
	
	/// 
	
	restartComptageVisites();
}






void determineBestImprove(){
	/// choisit aléatoirement parmis les meilleurs noeuds
	
	nbIterations++;
	nbIterationsSinceRestart++;
	
	int bestVal=99999;
	int bestNode=-1;
	int bestColor=-1;
	int nbBestVal=0;
	
	
	for(int i=0; i<nbSommets; i++){
		if(tConflicts[i]>0){
			
			int color = tColor[i];
			for (int j=0; j<nbColor; j++) {
				int currentImprove=tNewConflitsWithColor[i][j]; 
				if(currentImprove<bestVal && j!=color && (tTabou[i][j]<nbIterations || (currentImprove+nbEdgesConflict) < nbBestEdgesConflict)){
					bestVal=currentImprove;
					bestNode=i;
					bestColor=j;
					nbBestVal=1;
				}
				else if(currentImprove==bestVal && j!=color && (tTabou[i][j]<nbIterations || (currentImprove+nbEdgesConflict) < nbBestEdgesConflict)){ // on tire aleatoirement 1 des 2 
					nbBestVal++;
					int val=(rand()/(float)RAND_MAX) * nbBestVal ;
					if(val==0){
						bestVal=currentImprove;
						bestNode=i;
						bestColor=j;
					}
				}

			}
		}
	}
	///
	
	
	/// modifie la couleur du meilleur node trouve
	if(bestNode>-1){
		tVisite[bestNode]++;
		tVisitesColor[tColor[bestNode]]++;
		
		updateTables(bestNode, bestColor);
	}
	
	
	if (nbEdgesConflict < nbBestEdgesConflict) {
		nbBestEdgesConflict = nbEdgesConflict;
	}
}



void updateTables(int node, int color){
	
	int prevColor=tColor[node];
	tColor[node]=color;	
	
	///>>>>>> attention pour tenir compte du fait qu'un noeud change souvent de couleur (le mettre tabou plus longtemps!!)
	if (nbIterations%(nbSommets) == 0)
		restartComptageVisites();
	///<<<<<< attention pour tenir compte du fait qu'un noeud change souvent de couleur (le mettre tabou plus longtemps!!)
	
	int rd=(rand()/(float)RAND_MAX) * L;
	
	if (withBoucle)
		tTabou[node][prevColor]=nbIterations + rd + (lambda+tVisite[node]/100.0)*nbNodesConflict;
	else
		tTabou[node][prevColor]=nbIterations + rd + lambda*nbNodesConflict;
	
	
	int nbVoisins=tNbVoisins[node];
	for (int i=0; i<nbVoisins; i++) {
		int indiceSommet=tVoisins[node][i];
		
		/// répercution sur les voisins
		if (tColor[indiceSommet] == prevColor) {
			tConflicts[indiceSommet]--;
			tConflicts[node]--;
			nbEdgesConflict--;
			if (tConflicts[indiceSommet]==0) nbNodesConflict--;
			if (tConflicts[node]==0) nbNodesConflict--;
			
			for(int j=0; j<nbColor; j++){
				tNewConflitsWithColor[indiceSommet][j]++;
				tNewConflitsWithColor[node][j]++;
			}
			
		}
		else if (tColor[indiceSommet] == color) {
			tConflicts[indiceSommet]++;
			tConflicts[node]++;
			nbEdgesConflict++;
			if (tConflicts[indiceSommet]==1) nbNodesConflict++;
			if (tConflicts[node]==1) nbNodesConflict++;
			
			for(int j=0; j<nbColor; j++){
				tNewConflitsWithColor[indiceSommet][j]--;
				tNewConflitsWithColor[node][j]--;
			}
			
		}
		
		tNewConflitsWithColor[indiceSommet][prevColor]--;
		tNewConflitsWithColor[indiceSommet][color]++;
	}

}



void restartComptageVisites(){
	nbIterationsSinceRestart=0;
	for (int i=0; i<nbSommets; i++) {
		tVisite[i]=0;
	}
	
	
	for (int i=0; i<nbColor; i++) {
		tVisitesColor[i]=0;
	}
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////
/////////////// EA PART////////////////
///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Choisit n parents, cree un enfant, l'intensifie et met a jour la population
void crossIteration(){
	// Selection des parents
	int nbParents=3;

	
	char* tIsUsed=malloc(sizeof(char) * populationSize);
	for (int i=0; i<populationSize; i++)
		tIsUsed[i]=0;
	
	for (int i=0; i<nbParents; i++) {
		int pos=(int)((rand()/(float)RAND_MAX) * populationSize);
		while ( tIsUsed[pos] ) {
			pos=(pos+1)%populationSize;
		}
		tParentsIndices[i] = pos;
		tIsUsed[pos]=1;
	}
	free(tIsUsed);
	
	

	buildChildN(nbParents);


	tColor=tChild;
	
	initConflict();

	
	bestFitnessValue=99999;
	for (int i=0; i<nbLocalSearch && nbEdgesConflict > 0 ; i++) {
		determineBestImprove();
		if (nbNodesConflict<=bestFitnessValue) {
			bestFitnessValue=nbNodesConflict;
			bestIter=i;
			for (int j=0; j<nbSommets; j++) {
				tBestColor[j]=tColor[j];
			}
		}
	}
	
	printNewChild(nbParents);
	saveFitnessValue();
	
	// remplacement d'un parent si la solution n'est pas trouvee
	if (nbEdgesConflict>0) {
		updatePopulation();		
	}
}




/// Effectue le croisement entre les n parents
/// les parents sont les tColor (couleur de chaque sommet) passés dans **p
void buildChildN(int n){
	nbIterationsCross++;
	float** tSizeOfColors = malloc((sizeof(float*) * n));
	
	for (int i=0; i<n; i++) {
		tSizeOfColors[i] = malloc(sizeof(float) * nbColor);
		
		int* currentParent=tPopulationColor[tParentsIndices[i]];
		
		for (int j=0; j<nbColor; j++)
			tSizeOfColors[i][j]=0;
		
		
		for (int j=0; j<nbSommets; j++) {
			tSizeOfColors[i][currentParent[j]]++; /// compte le nombre de couleurs
			//tSizeOfColors[i][currentParent[j]]-=... faire le test du nb de conflits;
		}
		
		//		for (int j=0; j<nbColor; j++) {
		//			tSizeOfColors[i][j]-=(tVisitesColorPopulation[paramIndice[i]][j]/1000.0); /// retire le nombre de conflit/1000 : pour plusieurs classe de même valeur distingue en fonction du nombre de conflit (0.1 par conflit pour distinguer à l'intrieur d'une classe et pas prendre le dessus)
		//		}
		
		
		
	}
	
	// initialisation du fils qu'on va construire
	for (int i=0; i<nbSommets; i++)
		tChild[i]=-1;
	
	int* currentParent;
	float* currentSizeOfColors;
	int valMax;
	int colorMax;
	
	
	for (int i=0; i<nbColor; i++) {
		currentParent = tPopulationColor[tParentsIndices[i%n]];
		currentSizeOfColors = tSizeOfColors[i%n];
		valMax=-1;
		colorMax=-1;
		
		//determine le max des restants
		for (int j=0; j<nbColor; j++) {
			if (currentSizeOfColors[j]>valMax) {
				valMax=currentSizeOfColors[j];
				colorMax=j;
			}
		}

		
		//affecte la couleur aux noeuds du fils et met a jour les noeuds restant à attribuer au fils
		for (int j=0; j<nbSommets; j++) {
			if (currentParent[j]==colorMax && tChild[j]<0) {
				tChild[j]=i;
				
				for (int k=0; k<n; k++) {
					tSizeOfColors[k][tPopulationColor[tParentsIndices[k]][j]]--;
					//tSizeOfColors[k][tParentTmp[k][j]]+=(tVisitesColorPopulation[i][tParentTmp[k][j]]/1000.0);
				}
			}
		}
	}
	
	
	/// complete les noeuds n'ayant pas recu de couleur des parents
	for (int i=0; i<nbSommets; i++) {
		if (tChild[i]<0) {
			tChild[i]=(rand()/(float)RAND_MAX) * nbColor ;
		}
	}
	
	
	for (int i=0; i<n; i++) {
		free(tSizeOfColors[i]);
	}
	free(tSizeOfColors);	
	
}


// met a jour la population avec l'individu nouvellement cree 
void updatePopulation(){
	
	//// >>>> Choix du parent étant le plus vieux pour remplacement
	int indice=nbIterationsCross%populationSize; // ici on prend le parent le plus vieux
	//// <<<< Choix du parent étant le plus vieux pour remplacement
	
//	int indice=-1;
//	
//	////// >> Poids lié au nb de conflits
//	float* tConflictPoids = malloc(sizeof(float) * populationSize);
//	int totalConflictPoids=0;
//	for (int i=0; i<populationSize; i++) {
//		tConflictPoids[i] = pow(tFitness[i],4);
//		totalConflictPoids += tConflictPoids[i];
//	}
//	for (int i=0; i<populationSize; i++) {
//		tConflictPoids[i] /= totalConflictPoids;
//	}
//	
//	////// >> Poids lié à la proximité
//	float* tProxiPoids = malloc(sizeof(float) * populationSize);
//	int totalProxiPoids=0;
//	for (int i=0; i<populationSize; i++) {
//		tProxiPoids[i] = getMaxProxi(i);
//		totalProxiPoids+=tProxiPoids[i];
//	}
//	for (int i=0; i<populationSize; i++) { /// ramene par rapport a 1
//		tProxiPoids[i] /= totalProxiPoids;
//	}
//	
//	
//	////// >>>>> Choix aleatoire proportionnel au nb de conflits (fitness) et à la proximité => plus il y a de conflits, plus il risque d'être remplacé, plus il est proche, de même
//	double totalPoids=1.0;
//	for (int i=0; i<populationSize && indice<0; i++) {
//		double val=(rand()/(float)RAND_MAX) * totalPoids ;
//		
//		if(val< 0*tConflictPoids[i] + 1.0*tProxiPoids[i] ){
//			indice=i;
//		}
//		else {
//			totalPoids-= ( 0*tConflictPoids[i] + 1.0*tProxiPoids[i] );
//		}
//	}
//	////// <<<<< Choix aleatoire proportionnel au nb de conflits (fitness) => plus il y a de conflits, plus il risque d'être remplacé
//
//
//	
//	
//	////// <<<<< Est-ce que l'individu apporte suffisamment de diversite
//	int maxProxi=0;
//	int maxProxiIndiv;
//	int proxi;
//	for (int i=0; i<populationSize; i++) {
//		if ( (proxi=getProximite(tBestColor, tPopulationColor[i])) > maxProxi ) {
//			maxProxi=proxi;
//			maxProxiIndiv=i;
//		}
//	}
//	
//	if (maxProxi > (0.8*nbSommets )) { // si trop proche d'une solution, remplace que si améliore la fitness
//		if (bestFitnessValue > tFitness[maxProxiIndiv]) { // si trop proche et degrade la solution
//			indice=-1;
//		}
//	}

	
	
	
	
	// test si on peut transferer la solution dans les elites
	//updateElitePool();
	if (indice>-1) {
		// transfert de la solution
		int* tmp=tPopulationColor[indice];
		
		//// on garde le meilleur trouve
		tPopulationColor[indice]=tBestColor;
		tBestColor=tChild;
		tChild=tmp;
		tFitness[indice]=bestFitnessValue;
		updateProximityTable(indice);
		
		
		
//		//// on garde l'individu à la fin
//		tPopulationColor[indice]=tChild;
//		tChild=tmp;
//		tFitness[indice]=nbNodesConflict;
//		updateProximityTable(indice);
		
		
		//		// transfert du nb de visites de chaque noeud
		//		tmp=tVisitesPopulation[indice];
		//		tVisitesPopulation[indice]=tVisite;
		//		tVisite=tmp;
		//		
		//		// transfert du nb de visites de chaque couleur
		//		tmp=tVisitesColorPopulation[indice];
		//		tVisitesColorPopulation[indice]=tVisitesColor;
		//		tVisitesColor=tmp;
		
		//printf("\t\t\tremplacement de %d", indice);
		
	}
	else {
		//printf("\t\t\tREJET DE LA SOLUTION");
	}


}


//prend en parametre la position de l'individu qui a ete modifie pour mettre a jour la table des proximite pour cet individu
void updateProximityTable(int indPos){
	// met a jour la proximite
	for (int i=0; i<populationSize; i++) {
		int res=getProximite(tPopulationColor[indPos], tPopulationColor[i]);
		tProximite[indPos][i]=res;
		tProximite[i][indPos]=res;
	}
}


// determine la proximité de 2 individus (on identifier les meilleurs associations de couleur entre les 2 individus). 
int getProximite(int* ind1, int* ind2){
	int proxi=0;
	int **ttNbSameColor; // pour identifier les meilleurs correspondance de couleurs
	ttNbSameColor=malloc(sizeof(int*) * nbColor);
	for (int i=0; i<nbColor; i++) {
		ttNbSameColor[i]=malloc(sizeof(int) * nbColor);
		for (int j=0; j<nbColor; j++) {
			ttNbSameColor[i][j]=0;
		}
	}
	
	for (int i=0; i<nbSommets; i++) {
		ttNbSameColor[ind1[i]][ind2[i]]++;
	}
	
	
	for (int c=0; c<nbColor; c++) {
		int maxVal=-1,maxI,maxJ; 
		for (int i=0; i<nbColor; i++) {
			for (int j=0; j<nbColor; j++) {
				if( ttNbSameColor[i][j]>maxVal ){
					maxVal=ttNbSameColor[i][j];
					maxI=i;
					maxJ=j;
				}
			}
		}
		
		proxi+=maxVal;
		
		for (int i=0; i<nbColor; i++) {
			ttNbSameColor[maxI][i]=-1;
			ttNbSameColor[i][maxJ]=-1;
		}
	}
	
	
	for (int i=0; i<nbColor; i++)
		free(ttNbSameColor[i]);
	free(ttNbSameColor);

	return proxi;
}


int getMaxProxi(int indiv){
	int max=0;
	for (int i=0; i<populationSize; i++) {
		if ( i!=indiv && tProximite[indiv][i] > max) {
			max=tProximite[indiv][i];
		}
	}
	return max;
}


int getMinProxi(int indiv){
	int min=999999;
	for (int i=0; i<populationSize; i++) {
		if (tProximite[indiv][i] < min) {
			min=tProximite[indiv][i];
		}
	}
	return min;
}

int getMaxProxiOfAll(){
	int max=0;
	int tmp;
	for (int i=0; i<populationSize; i++) {
		if ( (tmp=getMaxProxi(i)) > max) {
			max=tmp;
		}
	}
	return max;
}

int getMinProxiOfAll(){
	int min=999999;
	int tmp;
	for (int i=0; i<populationSize; i++) {
		if ( (tmp=getMinProxi(i)) < min) {
			min=tmp;
		}
	}
	return min;
}


//void updateElitePool(){
//	if(nbNodesConflict<worstEliteValue){
//		int pos=(rand()/(float)RAND_MAX) * populationSize ;
//		
//		while (tPopulationEliteFitness[pos]<nbNodesConflict) {
//			pos=(pos+1)%populationSize;
//		}
//		
//		for (int i=0; i<nbSommets; i++) {
//			tPopulationElite[pos][i]=tColor[i];
//		}
//		
//		tPopulationEliteFitness[pos]=nbNodesConflict;
//		
//		worstEliteValue=0;
//		for (int i=0; i<populationSize; i++) {
//			if (tPopulationEliteFitness[i]>worstEliteValue) {
//				worstEliteValue=tPopulationEliteFitness[i];
//			}
//		}
//	}	
//}
//////////////////////////////
/////////////  SAVE  //////////
////////////////////////////////
void printNewChild(int nbParents){
	printf("\nIteration %4d : ", nbIterationsCross);
	for (int i=0; i<nbParents; i++) {
		printf("%2d;",tParentsIndices[i]);
	}
	
	printf(" => \tproxMin=%d \tproxMax=%d \tf=(%3d , %4d)",getMinProxiOfAll(), getMaxProxiOfAll(), bestFitnessValue, bestIter);
	//printf(" => \tproxMin=%d \tproxMax=%d \tf=(%3d , %4d)",getMinProxiOfAll(), getMaxProxiOfAll(), nbNodesConflict, bestIter);
	printf("\t tFitness =");
	for (int i=0; i<populationSize; i++) {
		printf(" %2d",tFitness[i]);
	}
	printDiametrePop();
	
	//		for (int i=0; i<nbColor; i++) {
	//			printf("%d;", tVisitesColorPopulation[indice][i]);
	//		}
	//		printf("\n\n");
	//printf("\t Elite: ajout de fitness %d worst=%d",nbNodesConflict, worstEliteValue);
	
}

void printIntensification(){
	//printf("Nb de conflits : %d\tamelioration: %d\n", tConflicts[bestNode], tNewConflitsWithColor[bestNode][bestColor]);
	//printf("%f   ",coefVisites);
}


void printDist(){

	
	for (int i=0; i<populationSize; i++) {
		for (int j=0; j<populationSize; j++) {
			printf("%5d \t", tProximite[i][j]);
		}
		
		printf("\n");
	}
}


void printDiametrePop(){
	int diam=999999;
	for (int i=0; i<populationSize; i++) {
		for (int j=0; j<populationSize; j++) {
			if (tProximite[i][j]<diam) {
				diam=tProximite[i][j];
			}
		}
	}
	
	diam=nbSommets-diam;
	printf("\t  D=%d",diam);
}

void save(){
	FILE *f;
	f = fopen(solutionFileName, "a");
	
	for (int i=0; i<nbSommets; i++) {
		fprintf(f, "%d\t", tColor[i]);
	}
	fprintf(f, "\n\n\n");
	
	fclose(f);
}


void saveFitnessValue(){
	FILE *f;
	f = fopen(fitnessFileName, "a");
	
	
	if (nbIterationsCross==1) { /// on a commence une nouvelle recherche (construction du premier enfant)
		fprintf(f, "\n");
	}
	
	
	fprintf(f, "%d;", nbNodesConflict);
	
	fclose(f);
}



