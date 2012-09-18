// ====================================================
// Generic TabuCol Algorithm
// ====================================================


#include <stdlib.h>
#include <assert.h>
#include "util/gfile.h"
#include "matching.h"



float lambdaValue = 0.6;
int LValue = 10;
int MAX_Gamma = 100;



typedef struct assignment{
  int sommet;
  int color;
  int nbVars;
} Move;


// initialize the matrix memory for tabucol
void mallocTabuColMemory(int nbNodes, int nbColors,
			 int** tTabu, int** tGamma){
  
  tTabu = malloc(sizeof(int*)*nbNodes);
  tGamma = malloc(sizeof(int*)*nbNodes);


  
  for (int i=0; i<nbNodes;++i){
    tTabu[i] = malloc(sizeof(int)*nbColors);
    tGamma[i]= malloc(sizeof(int)*nbColors);
  }
}

void freeTabuColMemory(int nbNodes, int** tTabu, int** tGamma){

  for (int i=0; i<nbNodes;++i){
    free(tTabu[i]);
    tTabu[i] = NULL;
    free(tGamma[i]);
    tGamma[i] = NULL;
  }

  free(tTabu);
  tTabu = NULL;
  free(tGamma);
  tGamma = NULL;
}




/*!
 * initialize the Gamma table
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param tGamma gamma table for total incremental objective function computing n*k
 * @return the number of violated edges
 */
int initGammaTable(int* a, char** graph, int** tGamma, 
		   int nbNodes){

  /// determine les conflits entre les noeuds
  int nbConflict=0;    
	
  // check the link
  for (int i=0; i<nbNodes; ++i){
    if (a[i] < 0) continue;

    for (int j=i; j<nbNodes; ++j){
      if (a[j] < 0) continue;
	    
      if( graph[i][j]){ // only verify assigned node
	
	++tGamma[i][a[j]]; 
	++tGamma[j][a[i]];
	  
	if (a[i] == a[j]) ++nbConflict;

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
int bestMove(Move* move, int** tGamma,  int** tTabu, 
	     int* individual, int colorNB, int nbNodes){

  int delta = -1;// best delta found, be careful the value 
  bool isSet = false;
  int bestCnt = 0;
  int bestVarCnt = 0;

  // traverse all the nodes
  for (int i=0; i<nbNodes; ++i){
    if (individual[i] > -1 && tGamma[i][individual[i]]>0){
      // traverse all the non-tabu colors of sommet i
      int minGamma = -1;
      int tmpSommet = -1;
      int tmpColor = -1;

      ++(move->nbVars);

      for (int j=0; j<colorNB; ++j){
       
	// decrease the tabu duration and skip the tabu color
	if (tTabu[i][j]>0){
	  --tTabu[i][j];
	  continue;
	}
	// skip  assigned color
	if (j == individual[i])
	  continue;
	
	if (minGamma < 0 || minGamma > tGamma[i][j]){
	  minGamma = tGamma[i][j];
	  tmpSommet = i; 
	  tmpColor = j;
	  bestCnt = 1;
	}else if (minGamma == tGamma[i][j]){
	  ++bestCnt;
	  float tval=(rand()/(float)RAND_MAX);

	  //if (tval > 100/bestCnt){
	  if (tval < 1/(float)bestCnt){
	    tmpSommet = i; 
	    tmpColor = j;
	  }
	}
      }

      // update delta if necessary, in case of all tabu
      if (minGamma < 0){
	int color = individual[i];
	while (color == individual[i])
	  color = (rand()/(float)RAND_MAX)*colorNB;

	minGamma = tGamma[i][color];
        tmpSommet = i;
	tmpColor = color;
      } 

      if (!isSet || delta > minGamma - tGamma[i][individual[i]]){
	if (!isSet) isSet = true;

	delta = minGamma - tGamma[i][individual[i]];
	move->sommet = tmpSommet;
	move->color =  tmpColor;
	bestVarCnt = 1;
	//printf("bestMove: %d,%d", move->sommet, move->color);
      }else if (delta == minGamma - tGamma[i][individual[i]]){
	// 
	++bestVarCnt;
	float tval=(rand()/(float)RAND_MAX);

	if (tval < 1/(float)bestVarCnt){
	  move->sommet = tmpSommet;
	  move->color =  tmpColor;
	}
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
 * @param ind individual coloring
 */
void updateMove(int sommet, int colorOrigin, int colorCandidate, 
		int** tGamma, char** graph, int* ind, 
		int nbNodes){
  
  for (int i=0; i< nbNodes; ++i){
    if (graph[sommet][i]){
      if (ind[i] > -1){
	
	if( tGamma[i][colorOrigin]>0)
	  --tGamma[i][colorOrigin];
      
	++tGamma[i][colorCandidate];

      }
    }
  }
  
}




/*!
 * tabuCol implementation
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return true if consistent solution found, otherwise false
 */
bool tabuCol(int* a, char** graph, int nbNodes, int colorNB, 
	     int maxIteration, int** tGamma, int** tTabu){

  //assert(tTabu != NULL && tGamma != NULL && tTmpColor != NULL);
  
  // init the memory 
  Move* tabuMove = malloc(sizeof(Move));
  int* tTmpColor = malloc(sizeof(int)*nbNodes);
  
  int maxNoImpIteration = maxIteration;

 
  //printf("in tabucol\n");
 
  // init Tabu and Gamma Tables
  for (int i=0; i<nbSommets; ++i) {
    //copy color assignment     
    tTmpColor[i] = a[i];
  
    // clean the gamma table
    for (int j=0; j<colorNB; ++j) {
      tTabu[i][j]=-1;
      tGamma[i][j] = 0;
    }
  }
    
  
  int obj = initGammaTable(a,graph,tGamma,nbNodes); // init gamma table

  if (obj<1) return true;

  //printf("========= T ========");
  //printTableNK(tGamma);

  int bestObj = obj;

  // a always records best so far solution
  //StableItr = maxNoImpIteration/3*2;
  // stableCnt = 0;
  for (int i=0;  i< maxNoImpIteration ; ++i){
    //    ++stableCnt;
    tabuMove->sommet = -1;
    tabuMove->color = -1;
    tabuMove->nbVars = 0;
    if(bestObj < 1) break; // find consistent solution

    // find best move based on gamma table
    int delta = bestMove(tabuMove, tGamma, tTabu, 
			 tTmpColor, colorNB, nbNodes);

    //printf("%d,%d",move->sommet,move->color);
    
    // all tabu case
    if (tabuMove->sommet < 0 || tabuMove->color < 0) continue;

    //printf("%d:\t%d\t%d\n",i, obj,bestObj);
    
    // in case find best delta 
    if( delta < 0 && obj+delta < bestObj){
      bestObj = obj+delta;
      for (int j=0; j<nbSommets; ++j){
	a[j] = tTmpColor[j];
      }
      
      i = 0; // reset i if found best so far
      a[tabuMove->sommet] = tabuMove->color;
    }else if (bestObj == obj+delta){
      
      int tval = (rand()/(float)RAND_MAX) * 10 ;
      if (tval > 4){
	for (int j=0; j<nbSommets; ++j){
	  a[j] = tTmpColor[j];
	}

	a[tabuMove->sommet] = tabuMove->color;
      }
    }

    

    // update move
    updateMove(tabuMove->sommet, tTmpColor[tabuMove->sommet], tabuMove->color, 
	       tGamma, graph, tTmpColor, nbNodes);
    // ============================= Record circle variable ===== BGN
    //if (stableCnt > StableItr)
    //++weightVars[move->sommet];
    // ============================= Record circle variable ===== END


    
    int rdx=(rand()/(float)RAND_MAX) * LValue;
    //rdx += weightPercent*nbConflict;
   
    rdx +=  lambdaValue*(tabuMove->nbVars);
    tTabu[tabuMove->sommet][tTmpColor[tabuMove->sommet]] = rdx; // tabu duration     
    tTmpColor[tabuMove->sommet] =tabuMove->color;


    obj += delta;
    

  }

  // release memory
  free(tabuMove);
  tabuMove = NULL;
  free(tTmpColor);
  tTmpColor = NULL;

  return bestObj < 1;
}


/*!========================================================================
 * 

  ========================================================================*/


void initMatrix(int lineNb, int colNb, int** table){
    for (int i=0; i<lineNb; ++i)
      for (int j=0; j<colNb; ++j)
	table[i][j]= -1;
}


/*
 * The color table which stores the color partition
 * create the nbColor lines and nbNodes columns
 */
int** createMatrix(int lineNb, int colNb){
  int** table = malloc(sizeof(int*)*lineNb);
  for (int i=0;i<lineNb; ++i)
    table[i] = malloc(sizeof(int)*colNb);
  
  return table;
}



char** createSubgraph(char** graph, int subNb, int* sub){

  char** table = malloc(sizeof(char*)*subNb);
  for (int i=0;i<subNb; ++i)
    table[i] = malloc(sizeof(char)*subNb);


  for (int i=0; i<subNb; ++i){
    for (int j=i; j<subNb; ++j){
      if (graph[sub[i]][sub[j]]){
	table[i][j] = 1;
	table[j][i] = 1;
      }else{
	table[i][j] = 0;
	table[j][i] = 0;
      }
    }
  }

  
  return table;
}

void deleteMatrix(int lineNb, int colNb, int** table){
  for (int i = 0; i<lineNb; ++i){
    free(table[i]);
    table[i] = NULL;
  }

  free(table);

  table = NULL; 
}

// projection between N(x) to subgraph matrix

int generateCft(int nbNodes, int* solution,  char** graph, int* conflictList){

  int nb = 0;
  for (int i=0; i < nbNodes; ++i) conflictList[i] = -1;

  for (int i=0; i < nbNodes; ++i){    
    if (conflictList[i] >0) continue;

    bool consistent = true;
    for (int j=i; j < nbNodes; ++j){
      if (graph[i][j]){ 
	if (solution[i] != solution[j])
	  continue;
	
	consistent = false;
	conflictList[j] = 1;
	++nb;
      }
    }
    if (!consistent){ 
      conflictList[i] = 1;
      ++nb;
    }
  }

  return nb;
}




typedef struct struct_projection{
  int nb;
  int* sub;
} Projection;


void createProjection(int nbNodes, char* conflictList, int* projection){
  int j=-1;
  for (int i=0; i<nbNodes; ++i){
    if (conflictList[i]>0){
      ++j;
      projection[j] = i;
    }
  }
}


// main solver of coloring problem
bool partitionMatch(){


  char** graph = tConnect;
  // generate a tabu assignment
  // initialize two matrix for entire graph
  int** graphGamma = NULL;
  int** graphTabu = NULL;
  int* graphSol = malloc(sizeof(int)*nbSommets);
  int* graphCft = malloc(sizeof(int)*nbSommets);
  
// store the color partition, line with nb of color
  //char** disCp = malloc(sizeof(char*)*nbColor);
  //for (int i= 0; i<nbColor; i++){
  //  disCp[i] = malloc(sizeof(char)*nbSommets);
  //}
   
  
  mallocTabuColMemory(nbSommets, nbColor, graphTabu, graphGamma);

  bool feasible = tabuCol(graphSol, graph, nbSommets, 
	    nbColor, 10000, graphGamma, 
	    graphTabu); 
  
  if(feasible) return true;


  // create the subgraph based on N(x) 
  int nbCft = generateCft(nbSommets, graphSol, graph, graphCft);

  int idxCft = 0;
  int nbNeighbors = 0; 
 
  for (int i=0; i>nbSommets; ++i){
    if (graphCft[i] > 0){
      idxCft = i;

      for (int j=0; j<nbSommets; ++j){
	if (graph[idxCft][j]) ++nbNeighbors;
      }
      
      break;
    }
  }

  // create the color partition of disjoint nodes
  //for (int i=0; i<nbSommets;++i)
  //  disCp[graphSol[i]][i] = 1;


  Projection* neighborSub = malloc(sizeof(Projection));
  neighborSub->nb = nbNeighbors;
  neighborSub->sub = malloc(sizeof(int)*nbNeighbors);
  createProjection(nbSommets, graph[idxCft], neighborSub->sub); 

  char** subgraph = createSubgraph(graph, neighborSub->nb, neighborSub->sub);
  
  int* subSol = malloc(sizeof(int)*nbNeighbors);
  int** subGamma = NULL; 
  int** subTabu = NULL; 
  mallocTabuColMemory(nbNeighbors, nbColor-1, subTabu, subGamma);
  bool subfeasible = tabuCol(subSol, subgraph, neighborSub->nb, 
			     nbColor-1, 10000, subGamma,
			     subTabu);
  // ignore the satisfiability of the subgraph
  freeTabuColMemory(neighborSub->nb, subTabu, subGamma);
  
  // verify the connection among the partition
  // remove the neighbors nodes 
  char* colorTable = malloc(sizeof(char)*nbColor);
  
  //========== loop begin

  for (int i= 0; i< neighborSub->nb; i++){
    for (int ni=0; ni<neighborSub->nb; ni++){
      colorTable[ni] = 0;
    }

    int nbCp = 0;
    for (int j=0; j< nbSommets; ++j){
      if (graph[neighborSub->sub[i]][j] && !graph[idxCft]){
	colorTable[graphSol[j]]=1;
      }
    }

    for (int ni=0; ni<neighborSub->nb; ni++){
      if (colorTable[ni]) ++nbCp;
    }

    if (nbCp == nbColor){
      printf("max connected\n");
      return false;
    }
  }
  
  //========== loop end

  
  
  // free the memory
  free(neighborSub->sub); neighborSub->sub = NULL;
  free(neighborSub); neighborSub = NULL;
  free(colorTable); colorTable = NULL;

  printf("exists disjoint edges\n");
  return true;

}
