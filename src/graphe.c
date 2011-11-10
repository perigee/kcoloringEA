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

float lambda=0.6;
int L=10;

int nbColor=49;
int populationSize=10;
int nbLocalSearch=2000;

clock_t  time1=0;
clock_t  time2=0;
clock_t  time3=0;
int nbMinutes1=0;
int nbMinutes2=0;
int nbMinutes3=0;



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
			
			// HCA - creation du nombre de noeuds pour chaque couleur
			tSizeOfColors1 = malloc(sizeof(int) * nbColor);
			tSizeOfColors2 = malloc(sizeof(int) * nbColor);
			
			// HCA - creation de la couleur associe a chaque noeud du fils
			tChild = malloc(sizeof(int) * nbSommets);
			
			// HCA - creation de la copie des parents pour faire le croisement dessus
			tParent1Tmp = malloc(sizeof(int) * nbSommets);
			tParent2Tmp = malloc(sizeof(int) * nbSommets);
			
			
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
	nbBestEdgesConflict=999999;
}


void improvInitPop(){
	for (int i=0; i<populationSize; i++) {
		tColor=tPopulationColor[i];
		initConflict();
		
		for (int j=0; j<nbLocalSearch; j++) {
			determineBestImprove();
		}
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
}




void determineBestImprove(){
	/// choisit aléatoirement parmis les meilleurs noeuds
	
	nbIterations++;
	
	int bestVal=99999;
	int bestNode=-1;
	int bestColor=-1;
	int nbBestVal=0;
	
	for(int i=0; i<nbSommets; i++){
		if(tConflicts[i]>0){
			
			int color = tColor[i];
			for (int j=0; j<nbColor; j++) {
				int currentImprove=tNewConflitsWithColor[i][j]; 
				if(j!=color && (tTabou[i][j]<nbIterations || (currentImprove+nbEdgesConflict) < nbBestEdgesConflict)){ // si la couleur n'est pas taboue
					if(currentImprove<bestVal){
						bestVal=currentImprove;
						bestNode=i;
						bestColor=j;
						nbBestVal=1;
					}
					else if(currentImprove==bestVal){ // on tire aleatoirement 1 des 2 
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
	}
	///
	
	
	
	
	/// modifie la couleur du meilleur node avec la premiere meilleure couleur
	if(bestNode>-1){
		//printf("Nb de conflits : %d\tamelioration: %d\n", tConflicts[bestNode], tNewConflitsWithColor[bestNode][bestColor]);
		updateTables(bestNode, bestColor);
	}
	
	
	if (nbEdgesConflict < nbBestEdgesConflict) {
		nbBestEdgesConflict = nbEdgesConflict;
	}
}


void updateTables(int node, int color){
	
	int prevColor=tColor[node];
	tColor[node]=color;
	//tTabou[node][prevColor]=nbIterations + L + lambda*nbEdgesConflict;
	
	int rd=(rand()/(float)RAND_MAX) * L;
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
			}
		}
		
		tNewConflitsWithColor[indiceSommet][prevColor]--;
		tNewConflitsWithColor[indiceSommet][color]++;
		
	}
	
	buildNextColorCost(node);

}




/// pour le noeud node recalcule le cout des transitions....
void buildNextColorCost(int node){
	int nbCurrentConflict=tConflicts[node];
	for (int i=0; i<nbColor; i++) {
		tNewConflitsWithColor[node][i]=-nbCurrentConflict;
	}
	
	int nbVoisins=tNbVoisins[node];
	for (int i=0; i<nbVoisins; i++) {
		tNewConflitsWithColor[node][tColor[tVoisins[node][i]]]++;		
	}
}



///////////////////////////////////////
/////////////// EA PART////////////////
///////////////////////////////////////

/// Choisit 2 parents, cree un enfant, l'intensifie et met a jour la population
void updatePopulation(){
	// Selection des parents
	int parent1=-1;
	int parent2=-1;
	while (parent1==parent2) {
		parent1=(rand()/(float)RAND_MAX) * populationSize ;
		parent2=(rand()/(float)RAND_MAX) * populationSize ;
	}
	
	// creation/intensification de l'enfant
	time2-=clock();
	buildChild(tPopulationColor[parent1], tPopulationColor[parent2]);
	time2+=clock();

	tColor=tChild;
	
	time1-=clock();
	initConflict();
	time1+=clock();
	
	time3-=clock();
	for (int i=0; i<nbLocalSearch && nbEdgesConflict > 0 ; i++) {
		determineBestImprove();
	}
	time3+=clock();
	
	// remplacement d'un parent si la solution n'est pas trouvee
	if (nbEdgesConflict>0) {
		int indice=nbIterationsCross%populationSize;
		int* tmp=tPopulationColor[indice]; // ici on prend le parent le plus vieux
		tPopulationColor[indice]=tChild;
		tChild=tmp;
	}
}

/// Effectue le croisement entre les 2 parents
/// les parents sont les tColor (couleur de chaque sommet)
void buildChild(int* p1, int* p2){
	nbIterationsCross++;
	//printf("\n\n\n");
	for (int i=0; i<nbColor; i++) {
		tSizeOfColors1[i]=0;
		tSizeOfColors2[i]=0;
	}
	
	for (int i=0; i<nbSommets; i++) {
		tSizeOfColors1[p1[i]]++;
		tSizeOfColors2[p2[i]]++;
		
		tParent1Tmp[i]=p1[i];
		tParent2Tmp[i]=p2[i];
		
		tChild[i]=-1;
	}

	int* currentParent;
	int* currentSizeOfColors;
	int valMax;
	int colorMax;
	
	
	for (int i=0; i<nbColor; i++) {
		currentParent = (i%2 == 0) ? tParent1Tmp : tParent2Tmp;
		currentSizeOfColors = (i%2 == 0) ? tSizeOfColors1 : tSizeOfColors2;
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
			if (currentParent[j]==colorMax) {
				tChild[j]=i;
				tSizeOfColors1[tParent1Tmp[j]]--;
				tSizeOfColors2[tParent2Tmp[j]]--;
				tParent1Tmp[j]=-1;
				tParent2Tmp[j]=-1;
			}
		}
	}
	
	

	
	/// complete les noeuds n'ayant pas recu de couleur des parents
	for (int i=0; i<nbSommets; i++) {
		if (tChild[i]<0) {
			tChild[i]=(rand()/(float)RAND_MAX) * nbColor ;
		}
	}
	
}


//////////////////////////////
/////////////  SAVE  //////////
////////////////////////////////

void save(char* filename){
	FILE *f;
	f = fopen(filename, "a");
	
	for (int i=0; i<nbSommets; i++) {
		fprintf(f, "%d\t", tColor[i]);
	}
	fprintf(f, "\n\n\n");
	
	fclose(f);
}


