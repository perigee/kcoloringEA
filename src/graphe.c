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
#include "util/datas.h"

float lambda=0.6;
int L=10;

int nbColor=49;
int populationSize=20;
int nbLocalSearch=1000;
int Nb_Generation=10000;

clock_t  time1=0;
clock_t  time2=0;
clock_t  time3=0;
int nbMinutes1=0;
int nbMinutes2=0;
int nbMinutes3=0;

char DISPLAY = 0; // output the results

/*!
 * read data file and create the individuals
 */
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


/*!
 * represent the graph as adjacent list
 */
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


/*!
 *  randomly assign the color on nodes
 */
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
				if(j!=color && (tTabou[i][j]<nbIterations || 
						(currentImprove+nbEdgesConflict) 
						< nbBestEdgesConflict)){ // si la couleur n'est pas taboue

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

/*!
 * update the gamma table 
 */
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

	// TabuCol implementation
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




//////////////////////////////////////////////////////////////////////////////////////////////
/////////// EA with distance 
//////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * TabuCol c implementation
 * @return true: if the solution is consistent
 * false: if the solution is inconsistent
 * @parameter graph: adjacent list of graph
 */



typedef struct assignment{
  int sommet;
  int color;
} Move;


/// only for testing
void printTableNK(int** table){
  for (int i=0; i<nbSommets; ++i){
    for (int j=0; j< nbColor; ++j){
      printf("%d ", table[i][j]); 
    }
    printf("\n"); 
  }
}


/*!
 * initialize the Gamma table
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param tGamma gamma table for total incremental objective function computing
 * @return the number of violated edges
 */
int initGammaTable(int* a, char** graph, int** tGamma){
  	/// determine les conflits entre les noeuds
	int nbConflict=0;
	
	// check the link
	for (int i=0; i<nbSommets; ++i){
	  for (int j=i; j<nbSommets; ++j){
	    if( graph[i][j] ){
	      if (a[i] == a[j])
		++nbConflict;

	      ++tGamma[i][a[j]]; 
	      ++tGamma[j][a[i]];
	    }
	  }
	}


	

	return nbConflict;
}


/*!
 * find the best move in tabu search
 * @param move bring out the best move
 * @param tGamma gamma table for total incremental objective function computing
 * @param tTabu tabu value duration table
 * @param individual individual color table
 * @return the change of objective function
 */
int bestMove(Move* move, int** tGamma,  int** tTabu, int* individual){
  int delta = -1;// best delta found, be careful the value 
  bool isSet = false;

  for (int i=0; i<nbSommets; ++i){
    if (tGamma[i][individual[i]]>0){
      // traverse all the non-tabu colors of sommet i
      int minGamma = -1;
      int tmpSommet = -1;
      int tmpColor = -1;

      for (int j=0; j<nbColor; ++j){
       
	// decrease the tabu duration
	if (tTabu[i][j]>0)
	  --tTabu[i][j];

	// skip tabu color and assigned color
	if (tTabu[i][j] > 0 || j == individual[i])
	  continue;
	
	if (minGamma < 0 || minGamma > tGamma[i][j]){
	  minGamma = tGamma[i][j];
	  tmpSommet = i; 
	  tmpColor = j;
	}
      }

      // update delta if necessary
      if (minGamma < 0) continue;

      if (!isSet || delta > minGamma - tGamma[i][individual[i]]){
	if (!isSet) isSet = true;

	delta = minGamma - tGamma[i][individual[i]];
	move->sommet = tmpSommet;
	move->color =  tmpColor;
	//printf("bestMove: %d,%d", move->sommet, move->color);
      } 
    }
  }

  if (isSet)  return delta;
  
  // in case all tabu, very rare
  
  return delta;
}

/*!
 * update the Gamma table according to best move found
 * @param sommet the best move node
 * @param colorOrigin current assignment of the best move node
 * @param colorCandidate the best move color
 * @param tGamma gamma table for total incremental objective function computing
 * @param graph adjacent matrix of graph
 */
void updateMove(int sommet, int colorOrigin, int colorCandidate, 
		int** tGamma, char** graph){
  
  for (int i=0; i< nbSommets; ++i){
    if (graph[sommet][i]){
      if (tGamma[i][colorOrigin]>0)
	--tGamma[i][colorOrigin];
      
      ++tGamma[i][colorCandidate];
    }
  }
  
}

/*!
 * tabuCol implementation
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return true if consistent solution found, otherwise false
 */
bool tabuCol(int* a, char** graph){

 
  int** tGamma = malloc(sizeof(int*)*nbSommets);
  int** tTabu = malloc(sizeof(int*)*nbSommets);


  int* tTmpColor = malloc(sizeof(int)*nbSommets);// store the temp color assignment
  int maxIteration = nbLocalSearch;
  int maxNoImpIteration = nbLocalSearch;
 

  // init Tabu and Gamma Tables
  for (int i=0; i<nbSommets; ++i) {
   //copy color assignment     
    tTmpColor[i] = a[i];

    tGamma[i] = malloc(sizeof(int)*nbColor);
    tTabu[i] = malloc(sizeof(int)*nbColor);
    for (int j=0; j<nbColor; ++j) {
	tTabu[i][j]=-1;
	tGamma[i][j] = 0;
    }
  }
    
  
  int obj = initGammaTable(a,graph,tGamma); // init gamma table

  //printf("========= T ========");
  //printTableNK(tGamma);

  int bestObj = obj;
  
  Move* move = malloc(sizeof(Move));
  // a always records best so far solution
  for (int i=0;  i< maxNoImpIteration ; ++i){
    
    move->sommet = -1;
    move->color = -1;
    if(bestObj < 1) break; // find consistent solution

    // find best move based on gamma table
    int delta = bestMove(move, tGamma, tTabu, tTmpColor);

    //printf("%d,%d",move->sommet,move->color);
    
    // all tabu case
    if (move->sommet < 0 || move->color < 0) continue;

    //    printf("\t%d\t%d\n", obj,bestObj);
    
    // in case find best delta 
    if( delta < 0 && obj+delta < bestObj){
      bestObj = obj+delta;
      for (int j=0; j<nbSommets; ++j){
	a[j] = tTmpColor[j];
      }
      
      i = 0; // reset i if found best so far
      a[move->sommet] = move->color;
    }

    // update move
    updateMove(move->sommet, tTmpColor[move->sommet], move->color, tGamma, graph);
 
    
    // calculate the nbVariable in conflict
    int nbConflict = 0;
    for (int j=0; j<nbSommets; ++j){
      if (tGamma[j][tTmpColor[j]]>0)
	nbConflict += tGamma[j][tTmpColor[j]];
    }
    
    int rdx=(rand()/(float)RAND_MAX) * L;
    tTabu[move->sommet][tTmpColor[move->sommet]] = rdx + lambda*nbConflict/2; // tabu duration

    tTmpColor[move->sommet] = move->color;
    obj += delta;
    

    // testing print
    if (DISPLAY){
      printf("%d\t%d\t%d\t%d\n", i, bestObj, obj, rdx);
    }
  }
  

 
  // free all dynamic memory before return 
  for (int i=0; i<nbSommets; ++i){
    free(tGamma[i]);
    free(tTabu[i]);
  }

  free(tGamma);
  free(tTabu);
  free(tTmpColor);
  free(move);

  if(bestObj < 1) return true;
  return false;
}


/*!
 * calculate the number of violated edges
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return the number of violated edges
 */
int cost(int* a, char** graph){
  register int nbConflict = 0;

  for (int i=0; i<nbSommets; ++i){
    for (int j=i; j<nbSommets; ++j){
      if( graph[i][j] && a[i] == a[j]){
	++nbConflict;
      }
    }
  }
  return nbConflict;
}



/*!
 * extract the subset of nodes composed by nogoods
 * dynamic create the 
 * @param a an individual
 * @param graph adjacent matrix of graph 
 * @param ngd subset nodes of individual
 * @return the number of subset nodes forming nogoods
 */
int nogood(int* a, char** graph, char* ngd){
  
  int nbVars = 0;

  // initialize the values
  for (int i= 0; i<nbSommets; ++i)
    ngd[i]=0;


  // not optimized code
  for (int i= 0; i<nbSommets; ++i){
    for (int j= i; j<nbSommets; ++j){
      if ( graph[i][j] && a[i] == a[j]){
        if (!ngd[i]){
	  ++nbVars;
	  ngd[i] = 1;
	}
	// add its neigbors
	/*for (int k = 0; k< nbSommets; ++k){
	  if (!ngd[k] && graph[i][k]){
	    ++nbVars;
	    ngd[k] = 1;
	  }
	  }*/
      }
    }
  }

  return nbVars;

}

/*!
 * calculate the similarity of two subsets of nodes
 * converted nogoods+deadends subsets nodes
 * @param a subset nodes of individual
 * @param b subset nodes of individual 
 * @return the number of nodes in common
 */
int similarityNogood(char* a, char* b){
  int s = 0;

  for (int i=0; i<nbSommets; ++i){
    if(a[i] && b[i]) ++s;
  }

  return s;
}

/*!
 * calculate the similarity of two subsets of nodes
 * @param a subset nodes of individual
 * @param b subset nodes of individual 
 * @return the number of nodes in common
 */
int similarityNodeClass(int* assignTable, int* nodeClass){
  int s = 0;
  
 
  return s;
}

/*!
 * calculate the distance between two individual
 * How to measure:
 * @param a a comparing individual 
 * @param b another comparing individual
 * @return the value of similarity
 */
int distance(int* a, int* b){
  int dist = -1;
  

  return dist;
}

/*!
 * choose the parent based on the sample's nogoods 
 * @return true the qualified parent, false otherwise
 */
bool chooseParentNogood(int* parent, char* ngd, char** graph){
  char* pNgd = malloc(sizeof(char)*nbSommets);
  nogood(parent, graph, pNgd);
  
  // compute the distance
  int dst = distance(pNgd, ngd);
  
  for (int i=0; i<nbSommets; ++i){
    
  }
}



/*!
 * Convert the color assignment to color partition 
 * @param a individual color assigment 
 * @param node the color partition counterpart of a
 * @return 
 */
int assign2partition(int* a, Node* node){
  
}




bool hasUnsigned(int* a){
  for (int i=0; i<nbSommets;++i)
    if (a[i]<0) return true;

  return false;
}

bool hasAssignedNeighbor(int node,int* a, char** graph){
  for (int i=0; i<nbSommets;++i)
    if (i!=node && graph[node][i] && a[i]>-1) return true;

  return false;
}


/*!
 * inConflict
 * @param node index of specific node
 * @param a the individual
 * @param graph adjacent matrix of the graph
 * @return the number of conflict neighbors
 */
int inConflict(int node, int* a, char** graph){
  int nb = 0;
  for (int i=0; i<nbSommets;++i){
    if (graph[node][i] && a[node] <0 && i!=node && a[i] == a[node])
      ++nb;
  }

  return nb;
}


/*!
 * hasConflict
 * @param node index of specific node
 * @param a the individual
 * @param graph adjacent matrix of the graph
 * @return true if there is conflict, otherwise false
 */
bool hasConflict(int node, int* a, char** graph){
  int nb = 0;
  for (int i=0; i<nbSommets;++i){
    if (graph[node][i] && a[node] > -1 && a[i] == a[node])
      return true;
  }

  return false;
}



void neh(int node, char** graph, int* offspring, int* a){
  
  if(a[node]<0){
    // in case the node in conflict
    if(hasConflict(node, offspring,graph)){
      int minConflictIdx = -1;
      int nbConflict = -1;
      for (int i=0; i<nbColor;++i){
	int nb = inConflict(i, a, graph);
	if (nb < 1){ 
	  minConflictIdx = i;
	  break;
	}
      
	if (nbConflict <0 || nbConflict > nb){
	  minConflictIdx = i;
	  nbConflict = nb;
	}
      
      }

      // assign current node
      a[node] = minConflictIdx;

      // color its neighbors
      /*for (int n=0; n<nbSommets;++n){
	if (a[n]<0 &&  graph[node][n]){

	  minConflictIdx = -1;
	  nbConflict = -1;
	  for (int i=0; i<nbColor;++i){
	    int nb = inConflict(i, a, graph);
	    if (nb < 1){ 
	      minConflictIdx = i;
	      break;
	    }
      
	    if (nbConflict <0 || nbConflict > nb){
	      minConflictIdx = i;
	      nbConflict = nb;
	    }
      
	  }
	  a[n] = minConflictIdx;
	}
      }*/
    
    }

    // ============================================================= BGN
    // in case the node has assigned neighbor
    /*if (hasAssignedNeighbor(node,a,graph)){
      int minConflictIdx = -1;
      int nbConflict = -1;
      for (int i=0; i<nbColor;++i){
	int nb = inConflict(i, a, graph);
	if (nb < 1){ 
	  minConflictIdx = i;
	  break;
	}
      
	if (nbConflict <0 || nbConflict > nb){
	  minConflictIdx = i;
	  nbConflict = nb;
	}
      
      }

      // assign current node
      a[node] = minConflictIdx;
    }*/

    // ============================================================= END
  }

}

/*!
 * NEH algorithm for graph coloring
 * @param node the index of node
 * @param color the candidate color for the node
 * @param indu the partial assigned solution
 * @param graph adjacent matrix 
 * @return the number of conflict edges
 */
void counterpart(int* offspring, char** graph, int* counterpart){

  // init counterpart
  for (int i=0; i<nbSommets;++i){
    counterpart[i] = -1;
  }
  
 
  for (int i=0; i<nbSommets;++i){
    neh(i,graph,offspring,counterpart);
  }

  for (int i=0; i<nbSommets;++i){
    if (counterpart[i]<0 && hasAssignedNeighbor(i,counterpart,graph)){
      int minConflictIdx = -1;
      int nbConflict = -1;
      for (int i=0; i<nbColor;++i){
	int nb = inConflict(i, counterpart, graph);
	if (nb < 1){ 
	  minConflictIdx = i;
	  break;
	}
      
	if (nbConflict <0 || nbConflict > nb){
	  minConflictIdx = i;
	  nbConflict = nb;
	}
      
      }

      // assign current node
      counterpart[i] = minConflictIdx;
    }
  }

  while(hasUnsigned(counterpart)){
    int nb =0;
    for (int i=0; i<nbSommets;++i){
      if (counterpart[i] <0){
	counterpart[i] = (rand()/(float)RAND_MAX) * nbColor;
	++nb;
      }
    }    

    printf("counter randomly: %d\n",nb);
  }
}


void initialArray(int* a,int size,int value){
  for (int i=0; i<size;++i){
    a[i] = value;
  }
}

/*!
 * TOTAL RANDOM CROSSOVER
 * crossover operator - improve the solution and decrease 
 * the computational time of local search
 * randomly choose the number of parents 
 * @param population the whole population
 * @param offspring carry out the created offspring
 * @return the number of conflicted edges
 */
void crossover_maximal(int nbParents, int** parents, int* offspring, char** graph){

  // initialize the offspring  
  initialArray(offspring,nbSommets,-1);
  
  // waiting-assigned node list
  int* tmpParent = malloc(sizeof(int)*nbSommets);
  initialArray(tmpParent,nbSommets,0);

  int* classSize = malloc(sizeof(int)*nbColor);



  // initial the 
  for (int i=0; i<nbColor;++i){
    // get the random id of parent
    int jth = (rand()/(float)RAND_MAX) * nbParents ;
    int maxColorIdx = -1;
    int maxClassSize = 0;
    initialArray(classSize,nbColor,0);


    
    for (int j=0; j<nbSommets;++j){
      if (tmpParent[j]> -1 ){
	++classSize[parents[jth][j]];
	if (maxClassSize < classSize[parents[jth][j]]){
	  maxClassSize = classSize[parents[jth][j]];
	  maxColorIdx = parents[jth][j];
	}
      }
    }


    for (int j=0; j<nbSommets;++j){
      if (tmpParent[j] > -1 && parents[jth][j] == maxColorIdx){
	bool inConflict = false;
	for (int k=0; k<nbSommets;++k){
	  // only assigned non-conflict node
	  if (graph[j][k] && parents[jth][j] == parents[jth][k]){
	    inConflict = true;
	    break;
	  }
	}

	if (!inConflict){
	  offspring[j] = i;
	  tmpParent[j] = -1; // remove the node from waiting-assigned node list
	}
	
      }
    }

    //    printf("find maxColorIdx: %d\n",maxColorIdx);
    
  }
  
  // randomly assign non-asigned nodes
  //int nbno = 0;
  for (int k=0; k < nbSommets; ++k){
    if (offspring[k] < 0){
      offspring[k] = (rand()/(float)RAND_MAX) * nbColor;
      //    ++nbno;
    }
  }

  //  printf("assign randomly: %d\n",nbno);
  free(tmpParent);
  free(classSize);
  
}


/*!
 * create the offspring based on parents
 * @param population the whole population
 * @param offspring carry out the created offspring
 */
void selection(int** population, char** nogoods, char** graph, int* offspring, char* ngd){
    int min = -1;
    int index = -1;
    nogood(offspring,graph,ngd);
    printf("distp:");
    for (int i=0; i<populationSize; ++i){
      int distance = similarityNogood(ngd, nogoods[i]);
      printf("\t%d",distance);
      if (min<0 || min < distance){
	min = distance;
	index = i;
      }      
    }

    printf("\n");

    // update the population
    free(population[index]);
    free(nogoods[index]);
    population[index] = offspring;
    nogoods[index] = ngd;   
}


/*!
 * ea + distance
 * @param graph adjacent matrix of given graph
 * @param population the table of individuals
 * @return true if the solution found is consistent, otherwise false 
 */
bool ea(char** graph){
  
  int** population = malloc(sizeof(int*)*populationSize);
  
  for (int i =0; i<populationSize;++i){
    population[i] = malloc(sizeof(int)*nbSommets);
    randomSolution(population[i]);
  }
  


  // initialize the population

  bool setBest = false;
  int* bestSolution = malloc(sizeof(int)*nbSommets);
  int bCost = -1;
  int* tmpSolution;
  int* tmpCounterpart;
  int tCost = -1;
  
  // initialize all nogoods 
  char** nogoods = malloc(sizeof(int*)*populationSize);
  char* tmpNogood;
  char* tmpNogoodCounterpart;
  for (int i=0; i< populationSize; ++i){
    nogoods[i] = malloc(sizeof(char)*nbSommets);
    nogood(population[i], graph, nogoods[i]);
  }

  // iterate the generation
  int cent = 0;
  for (int g = 0; g < Nb_Generation; ++g){
    ++cent;
    tmpSolution = malloc(sizeof(int)*nbSommets); 

    tmpNogood = malloc(sizeof(char)*nbSommets);
    //// crossover operator
    crossover_maximal(populationSize, population, tmpSolution, graph);
    
    int crossCost = cost(tmpSolution, graph);


    //// mutation (tabuCol) operator  
    if (tabuCol(tmpSolution,graph)){
      bestSolution = tmpSolution;
      bCost = 0;
      break;
    }else{

      tCost = cost(tmpSolution,graph);

      if (!setBest || bCost > tCost){
	if (!setBest) setBest = true;
	for (int i = 0; i<nbSommets; ++i){
	  bestSolution[i] = tmpSolution[i];
	  bCost = tCost;
	}
      } 
    }
    
    
    //// selection operator: update population
    //printf("childNogood:%d\n",nogood(tmpSolution, graph, tmpNogood)); //convert the nogood
    // traverse all the nogoods
    selection(population, nogoods, graph, tmpSolution, tmpNogood);

    int nb = 0;
    for (int n=0; n<nbSommets;++n){
      bool isViolated = false;
      for (int m=0; m<populationSize; ++m){
	if (hasConflict(n,population[m],graph)){
	      isViolated = true;
	      ++nb;
	      break;
	}
	
      }
    }

    printf("parents in conflict:%d\n",nb);

    // create counterpart    
    if (cent > 70){
      cent = 0;
      tmpCounterpart = malloc(sizeof(int)*nbSommets); // create a counterpart of tmpSolution
      tmpNogoodCounterpart = malloc(sizeof(char)*nbSommets);
      counterpart(bestSolution, graph, tmpCounterpart);

      int ccost = cost(tmpCounterpart,graph);

      printf("counterpart: %d\t",ccost);


      if (tabuCol(tmpCounterpart,graph)){
	bestSolution = tmpCounterpart;
	bCost = 0;
	break;
      }else{

	tCost = cost(tmpCounterpart,graph);
	printf("%d\n",tCost);

	if (!setBest || bCost > tCost){
	  if (!setBest) setBest = true;
	  for (int i = 0; i<nbSommets; ++i){
	    bestSolution[i] = tmpCounterpart[i];
	    bCost = tCost;
	  }
	} 
      }
      
      
      selection(population, nogoods, graph, tmpCounterpart, tmpNogoodCounterpart);
    }

    printf("costg:\t%d\t%d\t%d\t%d\n",g,crossCost,tCost,bCost);
    printf("costp:");
    for (int i=0; i<populationSize;++i){
      printf("\t%d",cost(population[i],graph));
    }
    printf("\n");
  }


  // free the dynamic memory
  for (int i=0; i<populationSize; ++i){
    free(nogoods[i]);
    nogoods[i] = NULL;
  }


  free(nogoods);
  //free(tColor);
  free(bestSolution);

  if (bCost != 0)
    return false;

  return true;
  
}


/*!
 * initialize random solution
 * @param a individual color table
 */
void randomSolution(int* a){
  for (int i=0; i<nbSommets; ++i){
    int col = (rand()/(float)RAND_MAX) * nbColor ;
    a[i] = col;
  }
}

// ============ UNIT TESTING ============
void testAlgo(char* filename){
  loadGraphe(filename);
  // tConnect
  // fill the tConnect table (adjacent matrix representation of graph)
  
  //int* individual = malloc(sizeof(int)*nbSommets);
  
  //randomSolution(individual);
  
  //  printf("color number: %d\n",nbColor);

  //bool feasible = tabuCol(individual, tConnect);
  bool feasible = ea(tConnect);
  
  if (feasible)
    printf("feasible\n");
  else
    printf("found infeasible\n");

  for (int i=0; i<populationSize;++i){
    free(tPopulationColor[i]);
    tPopulationColor[i] = NULL;
  }

  free(tPopulationColor);
  
  for (int i=0; i<nbSommets;++i){
    free(tConnect[i]);
    tConnect[i] = NULL;
  }
  
  free(tConnect);
  
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


