
//////////////////////////////////////////////////////////////////////////////////////////////
/////////// EA with IIS 
/////////// author: Jun HU
//////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>
#include "util/gfile.h"
#include "ealgo.h"
#include "kmeans.c"


float lambdaValue = 0.6;
int LValue = 10;
int MAX_Gamma = 100;
int StableItr;

void initialArray(int* a,int size,int value){
  for (int i=0; i<size;++i){
    a[i] = value;
  }
}


/*!
 * All pairs shortest path
 */
void floyd_warshall(char** graph, int** dist){

  //initialize distance matrix
  for (int i=0;i<nbSommets;++i)
    for (int j=0;j<nbSommets;++j)
      if (graph[i][j]) dist[i][j] = 1;
      else dist[i][j] = 0;
  

  for (int k = 0; k < nbSommets; ++k){
    
    for (int i = 0; i < nbSommets; ++i)
      for (int j = 0; j < nbSommets; ++j)
	if ((dist[i][k] * dist[k][j] != 0) && (i != j))
	  if ((dist[i][k] + dist[k][j] < dist[i][j]) 
	      || (dist[i][j] == 0))
	    dist[i][j] = dist[i][k] + dist[k][j];

  }
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





/*!
 * TabuCol c implementation
 * @return true: if the solution is consistent
 * false: if the solution is inconsistent
 * @parameter graph: adjacent list of graph
 */



typedef struct assignment{
  int sommet;
  int color;
  int nbVars;
} Move;


typedef struct random_parents{
  int nb;
  int* parents;
  int previous;
} ParentsBox;


/// only for testing
void printTableNK(int** table){
  for (int i=0; i<nbSommets; ++i){
    for (int j=0; j< nbColor; ++j){
      printf("%d ", table[i][j]); 
    }
    printf("\n"); 
  }
}



//================= CENTRAL MEMORY FOR TABUCOL ========== BGN
int **tTabu = NULL; // the tabu table used in tabuCol
int **tGamma = NULL;// the Gamma table used in tabuCol
int *tTmpColor = NULL; // for tmp solution in tabuCol
Move *tabuMove = NULL;
//================= CENTRAL MEMORY FOR TABUCOL ========== END



/*!
 * hasConflict
 * @param node index of specific node
 * @param a the individual
 * @param graph adjacent matrix of the graph
 * @return true if there is conflict, otherwise false
 */
bool hasConflict(int node, int* a, char** graph){

  for (int i=0; i<nbSommets;++i){
    if (graph[node][i] && a[node] > -1 && a[i] == a[node])
      return true;
  }

  return false;
}


/*!
 * calculate the number of violated edges
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return the number of violated edges
 */
int cost(int* a, char** graph){
  int nbConflict = 0;
  char *tabuConflictTable = malloc(sizeof(char)*nbSommets);
  
  for (int i=0; i<nbSommets; ++i){
    tabuConflictTable[i] = 0;
  }

  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0 || tabuConflictTable[i]) continue;

    for (int j=0; j<nbSommets; ++j){
      if (graph[i][j] && a[i] == a[j]){
        if (!tabuConflictTable[i]){
	  tabuConflictTable[i] = 1;
	  ++nbConflict;
	}
	
	if (!tabuConflictTable[j]){
	  tabuConflictTable[j] = 1;
	  ++nbConflict;
	}
      }
    }
    
    
    
  }

  free(tabuConflictTable);
  tabuConflictTable = NULL;
  
  return nbConflict;
}

/*!
 * initialize the Gamma table
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param tGamma gamma table for total incremental objective function computing n*k
 * @return the number of violated edges
 */
int initGammaTable(int* a, char** graph, int** tGamma){//, int *tConflict){
  /// determine les conflits entre les noeuds
  int nbConflict=0;    
	
  // check the link
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0) continue;

    for (int j=i; j<nbSommets; ++j){
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
int bestMove(Move* move, int** tGamma,  int** tTabu, int* individual, int colorNB){
  int delta = -1;// best delta found, be careful the value 
  bool isSet = false;
  int bestCnt = 0;
  int bestVarCnt = 0;

  // traverse all the nodes
  for (int i=0; i<nbSommets; ++i){
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
 * find the best move in tabu search, and increase the weights
 * on violated variables
 * @param move bring out the best move
 * @param tGamma gamma table for total incremental objective function computing
 * @param tTabu tabu value duration table
 * @param individual individual color table
 * @param weightsVars bring out the learnt weights
 * @return the change of objective function
 */
int bestMoveWeights(Move* move, int** tGamma,  int** tTabu, 
		    int* individual, int colorNB, int* weightsVars){
  int delta = -1;// best delta found, be careful the value 
  bool isSet = false;
  int bestCnt = 0;
  int bestVarCnt = 0;

  // traverse all the nodes
  for (int i=0; i<nbSommets; ++i){
    if (individual[i] > -1 && tGamma[i][individual[i]]>0){

      // increase the weight on violated variable
      ++weightsVars[i];

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
		int** tGamma, char** graph, int* ind){//, int *tConflict){
  
  for (int i=0; i< nbSommets; ++i){
    if (graph[sommet][i]){
      if (ind[i] > -1){
	
	//	if (tConflict[sommet] || tConflict[i]){
	//	  	if( tGamma[i][colorOrigin]>0)
	//		  tGamma[i][colorOrigin] -=MAX_Gamma;
      
	//		tGamma[i][colorCandidate] +=MAX_Gamma;
	//}else{

	if( tGamma[i][colorOrigin]>0)
	  --tGamma[i][colorOrigin];
      
	++tGamma[i][colorCandidate];
	//}
      }
    }
  }
  
}

bool hasSuperWeight(int i, int *weightVars){
  int firstw = weightVars[i]/100;
  if (firstw < 1) return false;

  return true;
}



void mallocTabuColMemory(){
  
  tTabu = malloc(sizeof(int*)*nbSommets);
  tGamma = malloc(sizeof(int*)*nbSommets);
  tTmpColor = malloc(sizeof(int)*nbSommets);
  tabuMove = malloc(sizeof(Move));
  //tabuConflictTable = malloc(sizeof(char)*nbSommets);
  
  for (int i=0; i<nbSommets;++i){
    tTabu[i] = malloc(sizeof(int)*nbColor);
    tGamma[i]= malloc(sizeof(int)*nbColor);
  }
}

void freeTabuColMemory(){

  for (int i=0; i<nbSommets;++i){
    free(tTabu[i]);
    tTabu[i] = NULL;
    free(tGamma[i]);
    tGamma[i] = NULL;
  }

  free(tTabu);
  tTabu = NULL;
  free(tGamma);
  tGamma = NULL;
  free(tTmpColor);
  tTmpColor = NULL;
  free(tabuMove);
  tabuMove = NULL;
}

/*!
 * tabuCol implementation
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return true if consistent solution found, otherwise false
 */
bool tabuCol(int* a, char** graph, int colorNB, int maxIteration){//, int *weightVars){

  assert(tTabu != NULL && tGamma != NULL && tTmpColor != NULL);
  
  
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
    
  
  int obj = initGammaTable(a,graph,tGamma); // init gamma table

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
    int delta = bestMove(tabuMove, tGamma, tTabu, tTmpColor, colorNB);

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
	       tGamma, graph, tTmpColor);
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
  

  if(bestObj < 1) return true;
  
  return false;
}


/*!
 * tabuCol implementation
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param weightsVars bring out the weights learnt from violation 
 * @return true if consistent solution found, otherwise false
 */
bool tabuColLearntWeights(int* a, char** graph, 
			  int colorNB, int maxIteration,
			  int* weightsVars){

  assert(tTabu != NULL && tGamma != NULL && tTmpColor != NULL);
  
  
  int maxNoImpIteration = maxIteration;
 
  //printf("in tabucol\n");
 
  // init Tabu and Gamma Tables
  for (int i=0; i<nbSommets; ++i) {
    //copy color assignment     
    tTmpColor[i] = a[i];
  
    for (int j=0; j<colorNB; ++j) {
      tTabu[i][j]=-1;
      tGamma[i][j] = 0;
    }
  }
    
  
  int obj = initGammaTable(a,graph,tGamma); // init gamma table

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

    // find best move based on gamma table and increase the weights
    int delta = bestMoveWeights(tabuMove, tGamma, tTabu, 
    				tTmpColor, colorNB, weightsVars);

    //int delta = bestMove(tabuMove, tGamma, tTabu, 
    //				tTmpColor, colorNB);


    //printf("%d,%d",move->sommet,move->color);
    
    // all tabu case
    if (tabuMove->sommet < 0 || tabuMove->color < 0) continue;

    //printf("%d:\t%d\t%d\n",i, obj,bestObj);
    //++weightsVars[tabuMove->sommet];


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
	       tGamma, graph, tTmpColor);
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
  

  if(bestObj < 1) return true;
  
  return false;
}




/*!
 * initialize the Gamma table
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param tGamma gamma table for total incremental objective function computing n*k
 * @return the number of violated edges
 */
int initGammaTable_weighted(int* a, char** graph, int** tGamma,
			    char *conflictList){//, int *tConflict){
  /// determine les conflits entre les noeuds
  int nbConflict=0;    
	
  // check the link
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0) continue;

    for (int j=i; j<nbSommets; ++j){
      if (a[j] < 0) continue;
	    
      if( graph[i][j]){ // only verify assigned node

	if (conflictList[i] || conflictList[j]){
	  tGamma[i][a[j]] += MAX_Gamma;
	  tGamma[i][a[j]] += MAX_Gamma;
	  
	  if (a[i] == a[j]) nbConflict += MAX_Gamma;
	}else{
	
	  ++tGamma[i][a[j]]; 
	  ++tGamma[j][a[i]];
	  
	  if (a[i] == a[j]) ++nbConflict;
        }
      }
    }
  }


  return nbConflict;
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
void updateMove_weighted(int sommet, int colorOrigin, int colorCandidate, 
			 int** tGamma, char** graph, int* ind, char *conflictList){
  
  for (int i=0; i< nbSommets; ++i){
    if (graph[sommet][i]){
      if (ind[i] > -1){
	
        if (conflictList[sommet] || conflictList[i]){
	  tGamma[i][colorOrigin] -= MAX_Gamma;
	  tGamma[i][colorCandidate] += MAX_Gamma;
	}else{
	  if( tGamma[i][colorOrigin]>0)
	    --tGamma[i][colorOrigin];
      
	  ++tGamma[i][colorCandidate];
	}
      }
    }
  }
  
}




// initialize the gamma table based on 



/*!
 * tabuCol implementation
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return true if consistent solution found, otherwise false
 */
bool tabuCol_PreGamma(int* a, char** graph, int colorNB, 
		      int maxIteration, char *weightedList){//, int *weightVars){

  assert(tTabu != NULL && tGamma != NULL && tTmpColor != NULL);
  
  
  int maxNoImpIteration = maxIteration;
 
  //printf("in tabucol\n");
 
  // init Tabu and Gamma Tables
  for (int i=0; i<nbSommets; ++i) {
    //copy color assignment     
    tTmpColor[i] = a[i];
  
    for (int j=0; j<colorNB; ++j) {
      tTabu[i][j]=-1;
      tGamma[i][j] = 0;
    }
  }
    
  
  int obj = initGammaTable_weighted(a,graph,tGamma,weightedList); // init gamma table

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
    int delta = bestMove(tabuMove, tGamma, tTabu, tTmpColor, colorNB);

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
    updateMove_weighted(tabuMove->sommet, tTmpColor[tabuMove->sommet], tabuMove->color, 
			tGamma, graph, tTmpColor, weightedList);
    // ============================= Record circle variable ===== BGN
    //if (stableCnt > StableItr)
    //++weightVars[move->sommet];
    // ============================= Record circle variable ===== END


    
    int rdx=(rand()/(float)RAND_MAX) * LValue;
    //rdx += weightPercent*nbConflict;
   
    rdx += lambdaValue*(tabuMove->nbVars);
     
    
    tTabu[tabuMove->sommet][tTmpColor[tabuMove->sommet]] = rdx; // tabu duration

    tTmpColor[tabuMove->sommet] =tabuMove->color;
    obj += delta;
    

  }
  

  if(bestObj < 1) return true;
  
  return false;
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
    if (graph[node][i] && a[node] > -1  && a[i] == a[node])
      ++nb;
  }

  return nb;
}



/*!
 * inConflict
 * @param node index of specific node
 * @param a the individual
 * @param graph adjacent matrix of the graph
 * @return the number of conflict neighbors
 */
int nodeInConflict(int node, int color, int* a, char** graph){
  int nb = 0;
  for (int i=0; i<nbSommets;++i){
    if (graph[node][i] && a[node] > -1  && color == a[i])
      ++nb;
  }

  return nb;
}

bool isNodeInConflict(int index, int *a, char **graph){
  if (a[index]<0) return false;
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0) continue;
    
    if (graph[index][i] && a[index] == a[i]) return true;
  }
  return false;
}



bool hasConflictSolution(int *a, char **graph){
  
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0) continue;
    if (hasConflict(i, a, graph)) return true;
  }
  
  return false;
}

bool isPartialSolution(int *a){
  for (int i=0; i<nbSommets; ++i){
    if (a[i]<0 || a[i] >= nbColor) return true;
  }

  return false;
}

/*!
 * find out the index of color class of individual a 
 * based on the unsigned nodes addressed by b.
 */
int maxColorClass( int *a, int *b, char **graph){
  
  int *classSize = malloc(sizeof(int)*nbColor);
  
  initialArray(classSize, nbColor, 0);
  
  int maxIdx = -1;
  int maxSize = 0;
  int eqCnt = 0;
  float tval;
  for (int j=0; j<nbSommets;++j){
    if (b[j] > -1 || a[j] <0) continue;
      
    ++classSize[a[j]];
    if (maxIdx < 0 || maxSize < classSize[a[j]]){
      maxIdx = a[j];
      maxSize = classSize[a[j]];
      eqCnt = 1;
    }else if (maxSize == classSize[a[j]]){
      ++eqCnt;
      tval = (rand()/(float)RAND_MAX) ;
      if (tval < 1/(float)eqCnt)
	maxIdx = a[j];
    }
  }


  free(classSize);
  classSize = NULL;
  
  return maxIdx;
}


void maxColorsClasses( int nbParent, int *chosenParent,  int **parents, 
		       int *b, char **graph, Move *move){
  
  int *classSize = malloc(sizeof(int)*nbColor);
  

  
  int maxIdx = -1;
  int parentIdx = -1;
  int maxSize = 0;
  for (int k=0; k<nbParent;++k){
    int i = chosenParent[k];
    
    initialArray(classSize, nbColor, 0);    

    for (int j=0; j<nbSommets;++j){
      if (b[j] > -1 || parents[i][j] <0) continue;
      
      ++classSize[parents[i][j]];
      if (maxIdx < 0 || maxSize < classSize[parents[i][j]]){
	maxIdx = parents[i][j];
	parentIdx = i;
	maxSize = classSize[parents[i][j]]; 
      }
    }
  }

  free(classSize);
  classSize = NULL;
  
  move->sommet = parentIdx;
  move->color = maxIdx;
}


void maxColorClasses( int nbParent, int **parents, int *b, 
		      char **graph, Move *move){
  
  int *classSize = malloc(sizeof(int)*nbColor);
  

  
  int maxIdx = -1;
  int parentIdx = -1;
  int maxSize = 0;
  for (int i=0; i<nbParent;++i){
    
    initialArray(classSize, nbColor, 0);    

    for (int j=0; j<nbSommets;++j){
      if (b[j] > -1 || parents[i][j] <0) continue;
      
      ++classSize[parents[i][j]];
      if (maxIdx < 0 || maxSize < classSize[parents[i][j]]){
	maxIdx = parents[i][j];
	parentIdx = i;
	maxSize = classSize[parents[i][j]]; 
      }
    }
  }

  free(classSize);
  classSize = NULL;
  
  move->sommet = parentIdx;
  move->color = maxIdx;
}


void randomParents(int chooseNb, int *parentsChosen, int nbParents){
  initialArray(parentsChosen, chooseNb, -1);
  
  for (int i=0; i<chooseNb;++i){
    
    bool duplicated = true;
    int iidxx = -1;
    while (duplicated){
      
      iidxx = (rand()/(float)RAND_MAX) * nbParents ;
      
      if (i <1) break;

      duplicated = false;
      for (int j=i-1; j>-1;--j){
	if (iidxx == parentsChosen[j]){
	  duplicated = true;
	  break;
	}
      } 
    }
    
    parentsChosen[i] = iidxx;
    //printf("random choose: %d\n",iidxx);
  }
}

/*!
 * less frequencied parent
 */
void lessFreqParents(int chooseNb, int *parentsChosen, int nbParents, 
		     int **parents, int *freqParents){
  
  int sum = 0;
  for (int i=0; i<nbParents; ++i){
    sum += freqParents[i];
  }
  
  sum = sum/nbParents;


  //printf("need parents:%d\n",chooseNb);
  
  int j=0;
  for (int i=0; i<nbParents; ++i){
    if (freqParents[i] <= sum){
      parentsChosen[j] = i;
      //printf("less  choose: %d\n",i);
      ++j;
    }

    if (j<chooseNb) continue;
    else break;
  }

  if (j<chooseNb){
    return randomParents(chooseNb, parentsChosen, nbParents);
  }

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

  for (int i= 0; i<nbSommets; ++i){
    if (hasConflict(i, a, graph)){
      for (int j= i; j<nbSommets; ++j){
	if (!ngd[j] && graph[i][j]){
	  ngd[j] = 1;
	  ++nbVars;
	}
      }
      break; // only find one nogood
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
 * TOTAL RANDOM CROSSOVER
 * crossover operator - improve the solution and decrease 
 * the computational time of local search
 * randomly choose the number of parents 
 * @param population the whole population
 * @param offspring carry out the created offspring
 * @return the number of conflicted edges
 */
void crossover_maximal(int nbParents, int** parents, int* offspring, 
		       char** graph, int* freqParents){

  // initialize the offspring  
  initialArray(offspring,nbSommets,-1);
  


  int* classSize = malloc(sizeof(int)*nbColor);
  int* classConflict = malloc(sizeof(int)*nbColor);

 
  
  int nbCross = 4;
  int* idxParents = malloc(sizeof(int)*nbCross);
  for (int i=0; i<nbCross;++i){
    idxParents[i] = -1;
  }
  
  for (int i=0; i<nbCross;++i){
    
    bool duplicated = true;
    int iidxx = -1;
    while (duplicated){
      
      iidxx = (rand()/(float)RAND_MAX) * nbParents ;
      
      if (i <1) break;

      duplicated = false;
      for (int j=i-1; j>-1;--j){
	if (iidxx == idxParents[j]){
	  duplicated = true;
	  break;
	}
      } 
    }
    
    idxParents[i] = iidxx;
  }
 
  // initial the 
  int ith = -1;
  bool isFirst = true;
  for (int i=0; i<nbColor;++i){
    // get the random id of parent
    if (ith < nbCross-1) ++ith;
    else ith = 0;
    //int jth = (rand()/(float)RAND_MAX) * nbCross ;
    int jth = idxParents[ith];
    int maxColorIdx = -1;
    int maxClassSize = 0;

    

    initialArray(classSize,nbColor,0);
    initialArray(classConflict,nbColor,0);
    
    if (isFirst){
      isFirst = false;
      freqParents[jth] += nbCross;
    }else
      ++freqParents[jth];
    
    /// less conflict nodes =============================================
    maxClassSize = nbSommets;
    for (int j=0; j<nbSommets;++j){
      if (offspring[j] < 0){
	
	++classSize[parents[jth][j]];
	
	// only counter the non-conflict nodes
	
	for (int k=0; k<nbSommets;++k){
	  // only assigned non-conflict node
	  if (graph[j][k] && parents[jth][j] == parents[jth][k]){
	    ++classConflict[parents[jth][j]];
	    break;
	  }
	}

      }
    }

    int equBest = 0;
    int maxSize = 0;
    int sizeIdx = -1;
    int equSize = 0;
    for (int j=0; j<nbColor;++j){

      if (maxSize < classSize[j]){
	maxSize = classSize[j];
	sizeIdx = j; // index of color class
	equSize = 0;
      }else if (maxSize == classSize[j]){
	++equSize;
	int val = (rand()/(float)RAND_MAX) * 100 ;
	if (val > 100/equSize){
	  sizeIdx = j; // index of color class
	}
      }


      if (classSize[j] < 1)
	continue;

      

      if (maxClassSize > classConflict[j]){
	maxClassSize = classConflict[j];
	maxColorIdx = j; // index of color class
	equBest = 0;
      }else if (maxClassSize == classConflict[j]){
	++equBest;
	int val = (rand()/(float)RAND_MAX) * 100 ;
	if (val > 100/equBest){
	  maxColorIdx = j; // index of color class
	}
      }

	

    }
    
    /*
      printf("crossover conflict nb by classes:\n");
      for (int j=0; j<nbColor;++j){
      printf("%d:%d;",j,classSize[j]);
      }

    

      printf("\nchosen color:%d\n",maxColorIdx);*/

    if (maxColorIdx < 0) maxColorIdx = sizeIdx;

    if (maxColorIdx < 0){

      printf(" problem\n");
      exit(0);
    }

    // assign choose color class 
    for (int j=0; j<nbSommets;++j){
      if (offspring[j] <0 && parents[jth][j] == maxColorIdx){
	offspring[j] = i;
      }
    }

    //    printf("find maxColorIdx: %d\n",maxColorIdx);
    
  }
  
  // randomly assign non-asigned nodes
  //int nbno = 0;
  for (int k=0; k < nbSommets; ++k){
    if (offspring[k] < 0){
      int val = (rand()/(float)RAND_MAX) * nbColor;
      offspring[k] = val;
      //    ++nbno;
    }
  }

  //  printf("assign randomly: %d\n",nbno);

  free(classSize);
  free(idxParents);
  
}





/*!
 * randomly select a conflict node
 */
int randomConflict(int* a, char** graph){

  int idx = -1;
  int cnt = 0;
  for (int i=0; i<nbSommets; ++i){
    if (hasConflict(i, a, graph)){
      ++cnt; 
      float val = (rand()/(float)RAND_MAX) ;
      if (idx < 0 || val < 1/(float)cnt)
	idx = i;
    }
  }

  return idx;
}


int lessWeightedConflict(int* a, char** graph, int *weightVars){

  int idx = -1;
  int maxWeight = -1;
  float tval;
  int eqCnt = 0;

  for (int i=0; i<nbSommets; ++i){
    if (a[i] > -1 && hasConflict(i, a, graph)){
      
      
      if (maxWeight < 0 || maxWeight > weightVars[i]){
	maxWeight =  weightVars[i];
	idx = i;
	eqCnt = 1;
      }else if (maxWeight ==  weightVars[i]){
	++eqCnt;
	tval = (rand()/(float)RAND_MAX) ;
	if (tval< 1/(float)eqCnt){
	  idx = i;
	}
      }
    }
  }

  return idx;
}

int moreWeightedConflict(int* a, char** graph, int *weightVars){

  int idx = -1;
  int maxWeight = -1;
  float tval;
  int eqCnt = 0;

  for (int i=0; i<nbSommets; ++i){
    if (a[i] > -1 && hasConflict(i, a, graph)){
      
      
      if (maxWeight < 0 || maxWeight < weightVars[i]){
	maxWeight =  weightVars[i];
	idx = i;
	eqCnt = 1;
      }else if (maxWeight ==  weightVars[i]){
	++eqCnt;
	tval = (rand()/(float)RAND_MAX) ;
	if (tval< 1/(float)eqCnt){
	  idx = i;
	}
      }
    }
  }

  return idx;
}


int weightedNode(int* a, char** graph, int *weightVars){

  int idx = -1;
  int maxWeight = -1;
  float tval;
  int eqCnt = 0;

  for (int i=0; i<nbSommets; ++i){
    if (a[i] > -1){
      
      
      if (maxWeight < 0 || maxWeight < weightVars[i]){
	maxWeight =  weightVars[i];
	idx = i;
	eqCnt = 1;
      }else if (maxWeight ==  weightVars[i]){
	++eqCnt;
	tval = (rand()/(float)RAND_MAX) ;
	if (tval< 1/(float)eqCnt){
	  idx = i;
	}
      }
    }
  }

  return idx;
}






void generate_sub(int *a, char **graph){
    
  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  
  char *conflict = malloc(sizeof(char)*nbSommets);
  
  for (int i = 0; i<nbSommets; ++i){
    if (hasConflict(i, a, graph))
      conflict[i] = 1;
    else
      conflict[i] = 0;
  }

  for (int i=0; i<nbSommets; ++i){
    if (conflict[i]){
      a[i] = -1;
      continue;
    }
  }

  free(conflict);
  conflict = NULL;
}


void generate_sub_disjoint(int *a, char **graph){
    
  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  
  bool firstItr = true;
  while (hasConflictSolution(a,graph)){
    if (firstItr){
      firstItr = false;
      int index = randomConflict(a, graph);
      a[index] = -1; // remove the chosen node
    }else {
      int index = -1;
      for (int i=0; i<nbSommets; ++i){
	if (a[i] < 0){
	  for (int j=0; j<nbSommets;++j){
	    if (i != j && !graph[i][j] && isNodeInConflict(j,a,graph)){
	      index = j;
	      break;
	    }
	  }
	}

	if (index > -1) break;
      }

      if (index < 0) index = randomConflict(a, graph);
      
      a[index] = -1; // remove the chosen node
    }
  }

}



void generate_sub_weighted_all(int *a, char **graph, int *weightedVars){
    
  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  
  char *conflict = malloc(sizeof(char)*nbSommets);
  
  for (int i = 0; i<nbSommets; ++i){
    if (hasConflict(i, a, graph)){
      conflict[i] = 1;
      ++weightedVars[i];
    }
    else
      conflict[i] = 0;
  }

  for (int i=0; i<nbSommets; ++i){
    if (conflict[i]){
      a[i] = -1;
      continue;
    }
  }

  free(conflict);
  conflict = NULL;
}


void generate_sub_simple(int *a, char **graph, int *weightVars){
    
  //return generate_sub(a, graph); // remove all conflict nodes

  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  

  while(hasConflictSolution(a,graph)){
    //int index = lessWeightedConflict(a, graph, weightVars);
    int index = randomConflict(a, graph);
    a[index] = -1; // remove the chosen node
    ++weightVars[index];
  }

}


void generate_sub_weighted(int *a, char **graph, int *weightVars){
    

  while(hasConflictSolution(a,graph)){

    int index = lessWeightedConflict(a, graph,weightVars);
    a[index] = -1; // remove the chosen node
    ++weightVars[index];
  }

}


/*!
 * crossover inspired by the IIS detection
 * the computational time of local search, at sametime, it
 * reduces the diversity
 * @param nbParents the number of whole population
 * @param parents the whole population
 * @param offspring carry out the created offspring
 * @param graph adjacent matrix
 * @param freqParents counter the participation number of each parent
 */

void crossover_sub(int nbParents, int** parents, int* offspring, char** graph, int* freqParents){
  
  // initialize 
  initialArray(offspring, nbSommets, -1);
  
  
  int nbCross = 4;
  int* idxParents = malloc(sizeof(int)*nbCross);
  randomParents(nbCross, idxParents, nbParents);

  int **pcopies = malloc(sizeof(int*)*nbCross);
  
  for (int i=0; i<nbCross; ++i){
    pcopies[i] = malloc(sizeof(int)*nbSommets);
    for (int j=0; j<nbSommets; ++j){
      pcopies[i][j] = parents[idxParents[i]][j];
    }
    
    generate_sub(pcopies[i], graph);
  }

  
  int ith = -1;


  for (int i=1; i<nbColor; ++i){

    // get a parent
    if (ith < nbCross-1) ++ith;
    else ith = 0;

    int jth = idxParents[ith];
    ++freqParents[jth];

    int colorIdx = maxColorClass(pcopies[ith], offspring, graph);

    for (int j=0; j<nbSommets;++j){
      if (offspring[j] < 0 && pcopies[ith][j] == colorIdx ){
	offspring[j] = i;
      }
    }
  }
  
  // find the maximal cardinality color class
  

  // complete the partial solution =============== 
  for (int i=0; i<nbSommets;++i){
    if (offspring[i] < 0){
      int col = (rand()/(float)RAND_MAX) * (nbColor) ;
      offspring[i] = col;
    }
  }

  //printf("number of partial nodes: %d\t%d\t%d\n", totalPartial, randomAssigned, conflictNb);

  for (int i=0; i<nbCross; ++i){
    free(pcopies[i]);
    pcopies[i] = NULL;
  }

  free(pcopies);
  free(idxParents);
  pcopies = NULL;
  idxParents = NULL;
  //printf("out ngood crossover\n");
}



/*!
 * mutation operator dedicated to create a subproblem based solution
 * @param a an individual
 * @param graph adjacent martrix graph
 * @return true if mutation finds consistent solution
 */
bool mutation_sub(int *a, char **graph, int removeColorNb, int *weightVars){


  for (int r=1; r<removeColorNb+1; ++r){
    //generate_sub_simple(a, graph, weightVars);
    generate_sub(a, graph);
    
    int subColor = nbColor - r;

    // resign the subproblem
    for (int i=0; i<nbSommets; ++i){
      if (a[i] < subColor ) continue;

      a[i] = 0; // simply assign the lowest index color
    }
  
  
    tabuCol(a, graph, subColor, nbLocalSearch);
    //tabuCol(a, graph, subColor, MAX_LocalSearch_Iteration);
  }

  return cost(a, graph);
}


/*!
 * @param a: individual (complete or partial
 * @param colorNb: given color number
 * @param conflictList: bring out the color class
 * @param graph: adjacent matrix of graph
 */
bool identifyColorClass(int *a, int colorNb, char *conflictList, char **graph){
  
  // initialize
  for (int i=0; i<nbSommets;++i) conflictList[i] = 0;
  
  while(hasConflictSolution(a,graph)){
    int index = randomConflict(a, graph);
    a[index] = -1; // remove the chosen node
    conflictList[index] = 1;
  }

  for (int i=0; i<nbSommets;++i){
    if (a[i] > colorNb -1) a[i] = 0;
  }
  
  bool feasible = false;
  while(!feasible){
   
 
    feasible = tabuCol_weighted(a, graph, colorNb, 
				MAX_LocalSearch_Iteration, 
				conflictList);

    
    bool weightedViolated = false;
    for (int i=0; i<nbSommets; ++i){
      if (a[i] < 0) continue;

      if(conflictList[i] && isNodeInConflict(i,a, graph)){
	weightedViolated = true;
	break;
      }
    }
    
    if (weightedViolated) break;
    
    for (int i=0; i<nbSommets; ++i){
      if (a[i] < 0) continue;
      if (!conflictList[i] && isNodeInConflict(i,a, graph)){
	conflictList[i] = 1;
      }
    }
  }

  int *partialS = malloc(sizeof(int)*nbSommets);
  // remove in conflict node
  for (int i=0; i<nbSommets; ++i){
    if(conflictList[i]){
      partialS[i] = colorNb;
    }else
      partialS[i] = -1;
    
  }

  // only to resolve one IIS 
  tabuCol(partialS, graph, colorNb+1, nbLocalSearch);
  
  // return the nodes to the subproblem with 
  // all the nodes of IIS except one node 
  for (int i=0; i<nbSommets; ++i){
    if(partialS[i] != colorNb){
      conflictList[i] = 0;
    }
  }
  
  free(partialS);
  partialS = NULL;
  
  return feasible;
}



/*!
 * mutation operator
 * @param a  carry out the mutant (input the origin)
 * @param graph adjacent matrix
 * @return true if the individual is complete and consistent
 */
bool mutation_identifyClasses(int *a, char **graph, int* varsWeights){
  
  char *colorClass = malloc(sizeof(char)*nbSommets);
  int *tmpSolution = malloc(sizeof(int)*nbSommets);
  initialArray(tmpSolution, nbSommets,-1);

  int removeColor = MAX_RemoveColors;
  ++removeColor;
 
  for (int i=1; i<removeColor; ++i){
    bool feasible = identifyColorClass(a,nbColor-i,colorClass,graph);
    for  (int j=0; j<nbSommets;++j){
      if(colorClass[j]) tmpSolution[j] = nbColor-i;
    }

    if (feasible) break;
  }
 

  for (int i=0; i<nbSommets;++i){
    if (a[i] < 0) a[i] = tmpSolution[i];
    if (a[i] < 0) a[i] = 0;
  }

  
  free(colorClass);
  colorClass = NULL;
  free(tmpSolution);
  tmpSolution = NULL;

  return !hasConflictSolution(a,graph);
}


bool mutation_iis(int *a, char **graph, int *weightVars){

  generate_sub_simple(a, graph, weightVars);
  //generate_sub_weighted(a, graph, weightVars);

  char *conflictList = malloc(sizeof(char)*nbSommets);
  for (int i=0; i<nbSommets; ++i){
    conflictList[i] = 0;
    if (a[i] < 0){
      conflictList[i] = 1;
      a[i] = 0;
    }
    
    // modify all high index color assignment
    if(a[i] > nbColor -2) a[i] = 0;
   
  }
  
  bool feasible = false;
  while(!feasible){
   
 
    feasible = tabuCol_weighted(a, graph, nbColor-1, 
				MAX_LocalSearch_Iteration, 
				conflictList);

    
    bool weightedViolated = false;
    for (int i=0; i<nbSommets; ++i){
      if (a[i] < 0) continue;

      if(conflictList[i] && isNodeInConflict(i,a, graph)){
	weightedViolated = true;
	break;
      }
    }
    
    if (weightedViolated) break;
    
    for (int i=0; i<nbSommets; ++i){
      if (a[i] < 0) continue;
      if (!conflictList[i] && isNodeInConflict(i,a, graph)){
	conflictList[i] = 1;
      }
    }

    
  }

  int *partialS = malloc(sizeof(int)*nbSommets);
  // remove in conflict node
  for (int i=0; i<nbSommets; ++i){
    if(conflictList[i]){
      a[i] = -1;
      partialS[i] = nbColor -1;
    }else{
      partialS[i] = -1;
    }
  }


  // only to resolve one IIS 
  tabuCol(partialS, graph, nbColor, nbLocalSearch);
  
  // return the nodes to the subproblem with 
  // all the nodes of IIS except one node 
  for (int i=0; i<nbSommets; ++i){
    if(a[i]<0 && partialS[i] != nbColor-1){
      a[i] = partialS[i];
      //a[i] = nbColor-1;
    }
  }
  
  // working on rest of problem with k-1 colors
  tabuCol(a, graph, nbColor-1, MAX_LocalSearch_Iteration);
  
  // complete solution
  for (int i=0; i<nbSommets; ++i){
    if(a[i]<0 ) a[i] = nbColor-1;
  }


  free(conflictList);
  conflictList = NULL;
  free(partialS);
  partialS=NULL;
  
  //return tabuCol(a, graph, nbColor, MAX_LocalSearch_Iteration);
  return !hasConflictSolution(a,graph);
}


bool mutation_weighted(int *a, char **graph, int *weightVars){

  int removeColorNb = (rand()/(float)RAND_MAX) * MAX_RemoveColors;
  ++removeColorNb;


  if (removeColorNb > nbColor/2)
    removeColorNb = 1;
  
  
  for (int l=1; l<=removeColorNb; ++l){
    generate_sub_simple(a,graph,weightVars);
    
    for (int i=0; i<nbSommets; ++i){
      if (a[i]<nbColor-l) continue;
      a[i] = nbColor -l -1;
    }

    tabuCol(a, graph, nbColor-l, nbLocalSearch);
  }

  
  //int nb = 0;  
  for (int i=0; i<nbSommets; ++i){
    if (a[i]>-1) continue;

    a[i] = nbColor -1;
    //++nb;
  }

  

  return tabuCol(a, graph, nbColor, MAX_LocalSearch_Iteration);

  generate_sub_weighted(a, graph, weightVars);

  for (int i=0; i<nbSommets; ++i){
    if (a[i]<nbColor-1) continue;
    a[i] = nbColor -2;
  }


  while(!tabuCol(a, graph, nbColor-1, nbLocalSearch)){
    int cidx = lessWeightedConflict(a, graph, weightVars);
    a[cidx] = -1; 
    ++weightVars[cidx];
  }

 

  //int nb = 0;  
  for (int i=0; i<nbSommets; ++i){
    if (a[i]>-1) continue;

    a[i] = nbColor -1;
    //++nb;
  }

  //printf("mutation remove: %d\n",nb);

  return !hasConflictSolution(a,graph);
  //return tabuCol(a, graph, nbColor, MAX_LocalSearch_Iteration, weightVars);
}



bool mutation_weighted_simple(int *a, char **graph, int *weightVars){
  
  int removeColorNb = (rand()/(float)RAND_MAX) * MAX_RemoveColors;
  ++removeColorNb;

  // define a number of colors removed
  //removeColorNb = MAX_RemoveColors;

  if (removeColorNb > nbColor/2)
    removeColorNb = 1;

  //printf("mutation remove colors: %d\n", removeColorNb);

  generate_sub_weighted(a, graph, weightVars);
  
  for (int i=0; i<nbSommets; ++i){
    if (a[i]<nbColor-(1+removeColorNb)) continue;
   
    a[i] = 0;
  }

  //printf("in mutation\n");
  while (!tabuCol(a, graph, nbColor-removeColorNb, nbLocalSearch)){
    int lastIdx = 0;
    //printf("in reduce\n");
    if (cost(a,graph)> 2){
      while(hasConflictSolution(a,graph)){
	int cidx = lessWeightedConflict(a, graph, weightVars);
	//int cidx = moreWeightedConflict(a, graph, weightVars);
	a[cidx] = -1; 
	++weightVars[cidx];
	lastIdx = cidx;
      }

      a[lastIdx] = 0;
      --weightVars[lastIdx];
    }else{
      int cidx = lessWeightedConflict(a, graph, weightVars);
      //int cidx = moreWeightedConflict(a, graph, weightVars);
      a[cidx] = -1; 
      ++weightVars[cidx];
      tabuCol(a, graph, nbColor-removeColorNb, nbLocalSearch);
     
      break;
    }
  }

  

  
  //for (int i=0; i<nbSommets; ++i){ 
  //if (a[i] < 0) a[i] = nbColor -1;
  //}
 
  return false;
  //return tabuCol(a, graph, nbColor, nbLocalSearch);
  return !hasConflictSolution(a, graph);
}


int minClass(int node, int *a, char** graph){

  int* colorArray = malloc(sizeof(int)*nbColor);
  initialArray(colorArray, nbColor, 0);
  int colorIdx = 0;
  int minWeight = -1;
  for (int i=0; i<nbSommets; ++i){
    if (graph[node][i]) ++colorArray[a[i]];
    
    if (minWeight < 0 || minWeight > colorArray[a[i]]){
      minWeight = colorArray[a[i]];
      colorIdx = a[i];
    }
  }

  free(colorArray);
  return colorIdx;
}



int minClassNb(int node, int *a, char** graph){

  int* colorArray = malloc(sizeof(int)*nbColor);
  initialArray(colorArray, nbColor, 0);
  int colorIdx = 0;
  int minWeight = -1;
  for (int i=0; i<nbSommets; ++i){
    if (graph[node][i]) ++colorArray[a[i]];
    
    if (minWeight < 0 || minWeight > colorArray[a[i]]){
      minWeight = colorArray[a[i]];
      colorIdx = a[i];
    }
  }
  
  colorIdx = colorArray[colorIdx];
  free(colorArray);
  return colorIdx;
}




bool mutation_zombie(int *a, char **graph, int** varWeights){
  return false;
}

/*!
 * Mutaion adopts the Grenade structure in VNS paper
 * diversify the population by simply changing the configuration of assignment
 */
bool mutation_grenade(int *a, char **graph, int** varWeights){

  //  printf("in mutation_grenade\n");
  char* neighborArray = malloc(sizeof(char)*nbSommets);
  for (int k=0; k<MAX_RemoveColors; ++k){
    int grenade = 0; // the grenade node
    int minWeight = -1;
  

    // less weighted node;
    /*
    for (int i=0; i<nbSommets; ++i){
      if (isNodeInConflict(i,a,graph)) ++varWeights[i];

      if (minWeight < 0 || minWeight > varWeights[i]){
	minWeight = varWeights[i];
	grenade = i;
      }
    }*/

    grenade = randomConflict(a, graph);
  
    varWeights[grenade] += nbSommets; // increase weight on grenade node

    // change the color
    a[grenade] = minClass(grenade, a, graph);


    int nbNeighbors = 0;
    for (int i=0; i<nbSommets; ++i){
      if (graph[grenade][i]){ 
	neighborArray[i] = 1;
	++nbNeighbors;
      }
      else neighborArray[i] = 0;  
    }
  
  
    // find minimal conflict node in new conflict nodes
    for (int j=0; j<nbNeighbors; ++j){

      int tmpDegree = -1;
      int minDegree = -1;
      int newConflictIdx = 0;


      for (int i=0; i<nbSommets; ++i){
	if (graph[grenade][i] && neighborArray[i]){
	  tmpDegree = minClassNb(i,a,graph);
      
	  if (minDegree < 0 || minDegree > tmpDegree){
	    minDegree = tmpDegree;
	    newConflictIdx = i;
	  }
	}
      }
      
      // change the color class of new conflict node
      a[newConflictIdx] = minClass(newConflictIdx,a,graph);
      neighborArray[newConflictIdx] = 0;
      varWeights[newConflictIdx] += nbSommets;
    }

    
  }

  free(neighborArray);
  //return !hasConflictSolution(a, graph);
  return tabuCol(a, graph, nbColor, MAX_LocalSearch_Iteration);
  return tabuCol(a, graph, nbColor, nbLocalSearch);
}


/*!
 * crossover inspired by the IIS detection
 * the computational time of local search, at sametime, it
 * reduces the diversity
 * @param nbParents the number of whole population
 * @param parents the whole population
 * @param offspring carry out the created offspring
 * @param graph adjacent matrix
 * @param freqParents counter the participation number of each parent
 */

void crossover_cardinality(int nbParents, int** parents, int* offspring, char** graph, int* freqParents){
  
  // initialize 
  initialArray(offspring, nbSommets, -1);

  
  int nbCross = (rand()/(float)RAND_MAX) * 3;
  nbCross += 2;
  int* idxParents = malloc(sizeof(int)*nbCross);

  randomParents(nbCross, idxParents, nbParents);
  //lessFreqParents(nbCross, idxParents, nbParents, parents, freqParents);

  int **pcopies = malloc(sizeof(int*)*nbCross);
  
  for (int i=0; i<nbCross; ++i){
    pcopies[i] = malloc(sizeof(int)*nbSommets);
    for (int j=0; j<nbSommets; ++j){
      pcopies[i][j] = parents[idxParents[i]][j];
    }
    
    //int tavl = (rand()/(float)RAND_MAX) * 10 ;
    //if (tavl<4) 
    generate_sub(pcopies[i], graph);

  }

  
  

  //Move* move = malloc(sizeof(Move));
  
  int crossIdx = -1;
  for (int i=1; i<nbColor; ++i){

    if (crossIdx < nbCross-1) ++crossIdx;
    else crossIdx = 0;

    int colorIdx = maxColorClass(pcopies[crossIdx], offspring, graph);
    int ith = crossIdx;
    // need more random choose?
    /*
      maxColorClasses(nbCross, pcopies, offspring, graph, move);

      int ith = move->sommet;
      int colorIdx = move->color;
    */
    
    if (colorIdx < 0){
      printf("=================================================================== problme:%d\t%d\n",ith, colorIdx);
      continue;
    }

    //printf("parent id:%d\t%d\n",idxParents[ith], freqParents[idxParents[ith]]);

    //if (ith != 0)
    ++freqParents[idxParents[ith]];

    //printf("after: ");
    for (int j=0; j<nbSommets;++j){
      if (offspring[j] < 0 && pcopies[ith][j] == colorIdx ){
	//printf("%d:%d\t",j,pcopies[ith][j]);
	offspring[j] = i;
      }
    }
    //printf("\n");
  }
  
  // find the maximal cardinality color class
  

  // complete the partial solution ===============
  if (true){
    for (int i=0; i<nbSommets;++i){
      if (offspring[i] < 0){
	int col = (rand()/(float)RAND_MAX) * (nbColor) ;
	offspring[i] = col;
      }
    }
  }

  //printf("number of partial nodes: %d\t%d\t%d\n", totalPartial, randomAssigned, conflictNb);

  for (int i=0; i<nbCross; ++i){
    free(pcopies[i]);
    pcopies[i] = NULL;
  }

  //free(move);
  free(pcopies);
  free(idxParents);
  //move = NULL;
  pcopies = NULL;
  idxParents = NULL;
  //printf("out ngood crossover\n");
}


int *crossParentsIdx = NULL;
int **parentsCopies = NULL;
Move *crossMove = NULL;
int *setSize = NULL;

void mallocCrossOverMemory(int crossParents){
  crossParentsIdx = malloc(sizeof(int)*crossParents);
  crossMove = malloc(sizeof(Move));
  parentsCopies = malloc(sizeof(int*)*crossParents);
  setSize = malloc(sizeof(int)*nbColor);

  for (int i=0; i<crossParents; ++i){
    parentsCopies[i] = malloc(sizeof(int)*nbSommets);
  }
}


void freeCrossOverMemory(int crossParents){
  free(crossParentsIdx);
  crossParentsIdx = NULL;
  
  for (int i=0; i<crossParents; ++i){
    free(parentsCopies[i]);
    parentsCopies[i] = NULL;
  }

  free(parentsCopies);
  parentsCopies = NULL;

  free(crossMove);
  crossMove = NULL;

  free(setSize);
  setSize = NULL;
}

//=============================================================== BGN
//===============================================================

/*! 
 * max independent set by function: 
 * @param nbParent number of parents participated the crossover
 * @param parents the parents participated the crossover
 * @param b the offspring in creating
 * @param graph adjacent matrix
 * @param freq 
 * @param move carry out the color class and parent index
 */
void maxIndependentSet( int nbParent, int **parents, int *b, 
			char **graph, int **freq, Move *move){
  
  
  int maxIdx = -1;
  int parentIdx = -1;
  double maxSize = 0;

  for (int i=0; i<nbParent;++i){
    
    initialArray(setSize, nbColor, 0);    

    for (int j=0; j<nbSommets;++j){
      if (b[j] > -1 || parents[i][j] <0) continue;
      
      ++setSize[parents[i][j]];
      
      double tmp = (double)setSize[parents[i][j]]
	/(double)freq[i][parents[i][j]];

      
      if (maxIdx < 0 || maxSize < tmp){
	maxIdx = parents[i][j];
	parentIdx = i;
	maxSize = tmp;

      }
    }
  }

  
  move->sommet = parentIdx;
  move->color = maxIdx;
}


/*! 
 * max independent set proposed by Galinier and Hertz: 
 * @param nbParent number of parents participated the crossover
 * @param parents the parents participated the crossover
 * @param b the offspring in creating
 * @param graph adjacent matrix
 * @param move carry out the color class and parent index
 */
void maxIndependentSetPure( int nbParent, int **parents, int *b, 
			    char **graph, Move *move){
  
  
  int maxIdx = -1;
  int parentIdx = -1;
  int maxSize = 0;
  for (int i=0; i<nbParent;++i){
    
    initialArray(setSize, nbColor, 0);    

    for (int j=0; j<nbSommets;++j){
      if (b[j] > -1 || parents[i][j] <0) continue;
      
      ++setSize[parents[i][j]];
      
      int tmp = setSize[parents[i][j]];
      
      if (maxIdx < 0 || maxSize < tmp){
	maxIdx = parents[i][j];
	parentIdx = i;
	maxSize = tmp;

      }
    }
  }

  
  move->sommet = parentIdx;
  move->color = maxIdx;
}

//===============================================================
//=============================================================== END



/*!
 * crossover inspired by the IIS detection
 * the computational time of local search, at sametime, it
 * reduces the diversity
 * @param crossParents number of parents participated in crossover
 * @param nbParents the number of whole population
 * @param parents the whole population
 * @param offspring carry out the created offspring
 * @param graph adjacent matrix
 * @param freqParents counter the participation number of each parent
 * @param removeColor maximal number of removed colors
 * @param weightVars weights on variables/nodes
 */
bool crossover_enforced2(int crossParents, int nbParents, int** parents, 
			 int* offspring, char** graph, int* freqParents, 
			 int removeColor, int *weightVars){

  //printf("in crossover\n");
  
  assert(parentsCopies != NULL && crossParentsIdx != NULL);

  int **conflictColors = malloc(sizeof(int*)*crossParents);
  
  // initialize 
  initialArray(offspring, nbSommets, -1);
  randomParents(crossParents, crossParentsIdx, nbParents);
  //lessFreqParents(crossParents, crossParentsIdx, nbParents, parents, freqParents);
  
  for (int i=0; i<crossParents; ++i){
    conflictColors[i] = malloc(sizeof(int)*nbColor);
    initialArray(conflictColors[i], nbColor, 1);

    for (int j=0; j<nbSommets; ++j){
      parentsCopies[i][j] = parents[crossParentsIdx[i]][j];

      if (isNodeInConflict(j, parents[crossParentsIdx[i]], graph)){
	for (int k = 0; k<nbSommets; ++k){
	  if (parentsCopies[i][k] > -1 && j != k && graph[j][k]){
	    if (!isNodeInConflict(k,parents[crossParentsIdx[i]], graph))
	      ++conflictColors[i][parentsCopies[i][k]];
	  }
	}
	
	//++conflictColors[i][parentsCopies[i][j]];
      }
    }

    //++freqParents[crossParentsIdx[i]];
    //generate_sub_simple(parentsCopies[i], graph, weightVars);      
    //generate_sub_disjoint(parentsCopies[i], graph);      
    //generate_sub_weighted_all(parentsCopies[i], graph, weightVars);      
  }
  

  int colorIdx = -1;
  int ith = -1;
  bool isFirst = true; // switch between the color class selection

  for (int i=1; i< nbColor; ++i){
    

     maxIndependentSet(crossParents, parentsCopies, offspring, 
      	      graph, conflictColors, crossMove);

    
     //  maxIndependentSetPure(crossParents, parentsCopies, offspring, 
     //		      graph, crossMove);	
 

    colorIdx = crossMove->color;
    ith = crossMove->sommet;


    

    if (ith < 0 || colorIdx < 0) continue;

    
    if (isFirst){
      ++freqParents[crossParentsIdx[ith]];// += 2;
         isFirst = false;
    }else
      ++freqParents[crossParentsIdx[ith]];


    for (int j=0; j<nbSommets;++j){
      if (offspring[j] < 0 && parentsCopies[ith][j] == colorIdx ){
	offspring[j] = i;
      }
    }
  }

  for (int i=0; i<crossParents;++i){
    free(conflictColors[i]);
    conflictColors[i] = NULL;
  }
  free(conflictColors);
  conflictColors = NULL;
  
  // complete the partial solution ===============
  for (int i=0; i<nbSommets;++i){
    if (offspring[i] < 0){
      offspring[i] = 0;
    }
  }

  // add the mutation on created offspring
  return tabuCol(offspring,graph,nbColor,MAX_LocalSearch_Iteration);

}



/*!
 * create the offspring based on parents
 * @param population the whole population
 * @param offspring carry out the created offspring
 * @param ngd 
 */
void selection_freq(int** population, char** graph, int* offspring, 
		    int* freqParents, bool foundBetter){
  
  
  

  int max = -1;
  int index = -1;

  int bestCnt = 0;
  for (int i=0; i<populationSize; ++i){
    if (max < 0 || max < freqParents[i]){
      index = i;
      max = freqParents[i];
      bestCnt = 1;
    }else if (max == freqParents[i]){
      ++bestCnt;
      float tval = (rand()/(float)RAND_MAX) ;
      if (tval < 1/(float)bestCnt){
	index = i;
      }
    }
  }

  // update the population 
  for (int i=0; i<nbSommets; ++i){
    population[index][i] = offspring[i];
  }

  if (foundBetter)
    freqParents[index] = - nbColor;
  else
    freqParents[index] = 0;
}

void printSolution(int iteration, int costx, int *a, FILE *f ){
  
  //return; // ignore
  
  fprintf(f,"s:\t%d\t%d",iteration, costx);
  for (int i=0; i<nbSommets; ++i){
    fprintf(f, "\t%d", a[i]); 
  }
  fprintf(f,"\n");
}




/*!
 * ea + distance
 * @param funcCrossPtr crossover operator
 * @param funcMutationPtr mutation operator
 * @param graph adjacent matrix of given graph
 * @param savefile output file
 * @param inputFile input file
 * @return true if the solution found is consistent, otherwise false 
 */
bool ea(CrossoverFuncPtr funcCrossPtr, MutationFuncPtr funcMutationPtr, 
	char** graph, char *savefile, char *inputFile){
  
  FILE *f;
  f = fopen(savefile, "a");

  //printf("initial memory\n");
  int crossParentsNb = 3;
  mallocTabuColMemory();
  mallocCrossOverMemory(crossParentsNb);

  
  //give the basic information
  fprintf(f, "r: =========================================== START\n");
  fprintf(f, "r: %s\t", inputFile);
  fprintf(f, "nbColor = %d\t", nbColor);
  fprintf(f, "populationSize = %d\t", populationSize);
  fprintf(f, "LS iterations = %d - %d\t", nbLocalSearch, MAX_LocalSearch_Iteration);
  fprintf(f, "Time Limit = %d mins\t", Nb_Generation/60);
  fprintf(f, "Max remove colors = %d\t", 3);
  fprintf(f, "Parents for crossover = %d\n", crossParentsNb);

      
  time_t endtime;
  struct tm *tmx;
  time(&endtime);

  
  if ((tmx = localtime (&endtime)) == NULL) {
    printf ("Error extracting time stuff\n");
    exit(0);
  }


  fprintf(f,"r: beginTime:\t%04d-%02d-%02d %02d:%02d:%02d\n",
	  tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
	  tmx->tm_hour, tmx->tm_min, tmx->tm_sec);


  int** population = malloc(sizeof(int*)*populationSize);  
  int* weightsLearned = malloc(sizeof(int)*nbSommets);
  int* freqParents = malloc(sizeof(int)*populationSize);  
  
  initialArray(weightsLearned, nbSommets, 0); //initialize the weights on nodes
  initialArray(freqParents,  populationSize, 0); //initialize the frequency of parents

  //printf("create random solution\n");
  bool feasibleInit = false;
  for (int i =0; i<populationSize;++i){
    population[i] = malloc(sizeof(int)*nbSommets);
    randomSolution(population[i]);


    feasibleInit = tabuCol(population[i],graph, nbColor, 
			   MAX_LocalSearch_Iteration);
    if (feasibleInit){ 
     
      for (int j=0; j<i+1; ++j){
	free(population[j]);
	population[j] = NULL;
      }

      free(population);
      population = NULL; 

      return true;
    }
  }

  //printf("finish random solution\n");

  // initialize the population
  int* bestSolution = malloc(sizeof(int)*nbSommets);
  int nbChildren = 3;
  int **tmpSolutions = malloc(sizeof(int*)*nbChildren);

  for (int i=0; i<nbChildren; ++i){
    tmpSolutions[i] = malloc(sizeof(int)*nbSommets);
  }
  
  int bCost = nbSommets;
  int tCost = -1;
  int crossCost = -1;
  
  // iterate the generation
  int cent = 0;
  int switchIteration = 10;
  int totalMutationNb = 0;
  int removeColor = 1;
  int mutationCnt = 0;

  
  

  int g = 0;
  time_t start_time;
  time_t now_time;
  time(&start_time);
  while(true){
  
    ++g;

    // re-initialize parameters
    for (int cross=0; cross<nbChildren; ++cross){
      initialArray(tmpSolutions[cross],nbSommets,-1);
    }

    //// crossover operator ==================================== BGN
    bool foundBetter = false;
    
    if (cent < switchIteration){
      ++cent;

      crossCost = 0;
      
      int bestSolutionSoFarIdx = -1;
      for (int cross=0; cross<nbChildren; ++cross){

	bool feasible = (*funcCrossPtr)(crossParentsNb, populationSize, population, 
			tmpSolutions[cross], graph, freqParents, 
			removeColor, weightsLearned);


	if (feasible){
      
	  for (int c=0; c<nbSommets;++c){
	    bestSolution[c] = tmpSolutions[cross][c];
	    //printf("s: %d\t%d\n",c,bestSolution[c]);
	    if (bestSolution[c]< 0){
	      printf("unsigned found\n");
	      exit(0);
	    }
	  }
	  printf ("consistent solution found by mut1\n");
      
	  bCost = 0;
	  break;
	}else{

	  tCost = cost(tmpSolutions[cross],graph);
	  
	  // find best solution so far
	  if (bCost > tCost){

	    foundBetter = true;
	    bestSolutionSoFarIdx = cross;
   	    bCost = tCost;
	    //cent = 0; // switch the operator
	    mutationCnt = 0;


	    for (int i = 0; i<nbSommets; ++i){
	      bestSolution[i] = tmpSolutions[cross][i];
	    }

	  } 

	}
	
      }

      if (bCost < 1){
	break;
      }

      
      for (int parentIdx = 0; parentIdx < populationSize; ++parentIdx){
	  freqParents[parentIdx] += populationSize; // oldest strategy
      }
      
      // replace the highest frequenced parent
      for (int cross=0; cross<nbChildren; ++cross){
	if (cross != bestSolutionSoFarIdx)
	  selection_freq(population, graph, tmpSolutions[cross], freqParents,false);
	else
	  selection_freq(population, graph, tmpSolutions[cross], freqParents,true);
      }

     
    }
    //// crossover operator ==================================== END

  
    //// mutation  operator =========================== BGN
    if (cent > (switchIteration-1)){

      ++totalMutationNb;
      ++mutationCnt;
      cent = 0; // switch to crossover
     
      for (int mi=0; mi<3;++mi){

	int pIdx = -1;
	int maxFreqP = 0;
	for (int p=0; p<populationSize; ++p){
	  if (pIdx < 0 || maxFreqP < freqParents[p]){
	    maxFreqP = freqParents[p];
	    pIdx = p;
	  }
	}

	// mutation operator 
	bool isConsistent = (*funcMutationPtr)(population[pIdx], graph, weightsLearned);

	if (isConsistent){
	  for (int bs=0; bs<nbSommets;++bs){
	    bestSolution[bs] = population[pIdx][bs];
	  }

	  bCost = 0;
	  
	  printf("find consistent solution by mut_iis\n");
	  break;
	}

	
	int mutationCost = cost(population[pIdx],graph);
	double diff = (double)(mutationCost -bCost)/(double)bCost;
	
	if (diff < 0.5)
	  freqParents[pIdx] = nbColor*1.5;
	else
	  freqParents[pIdx] = 0;
	
      }

      
      // reset all the learned weights
      initialArray(weightsLearned, nbSommets, 0);	      

    }

    // print best so far solution 
    if (foundBetter){  
      printf("p:");
      printf("\t%d\t%d\t%d\t%d mins\n",g,bCost,totalMutationNb, Nb_Generation/60);
      if (foundBetter){
	fprintf(f,"p:\t%d\t%d\t%d\t%d mins\n",g,bCost,totalMutationNb, Nb_Generation/60);
	fflush ( stdout );/* this line */
      }
    }
    
    printf("parents:\t%d",g);
    for (int parentIdx = 0; parentIdx < populationSize; ++parentIdx){
      //if (freqParents[parentIdx] > -1)
      //freqParents[parentIdx] += nbColor/nbChildren/2; // oldest strategy
      printf("\t%d[%d]", cost(population[parentIdx],graph), freqParents[parentIdx]);
    }
    printf("\n");

    time(&now_time);
    if (difftime(now_time, start_time) > Nb_Generation) break; // time out
    if (bCost<1) break; // best solution found
  }
  
  printf("r:\t%d\t%d\t%d\t%d\t%d\n", g, g, totalMutationNb,Nb_Generation/60, bCost);
  fprintf(f,"r: %d\t%d\t%d\t%d\t%d\n", g, g, totalMutationNb,Nb_Generation/60, bCost);
  
  
  // verify the solution
  bool consistent = true;
  if (bCost < 1){
    
    for (int i=0; i<nbSommets; ++i){
      if (bestSolution[i] < 0 || bestSolution[i] > nbColor-1){
	fprintf(f,"e: solution is partial\n");
	consistent = false;
	break;
      }

      for (int j=0; j<nbSommets; ++j){
	if (graph[i][j]){
	  //printf("%d\t%d\n",i,j);
	  if (bestSolution[i] == bestSolution[j]){
	    fprintf(f,"e: solution isn't consistent\n");
	    consistent = false;
	    break;
	  }
	}
      }
    }
  }else{
    consistent = false;
  }

  
  time(&endtime);

  
  if ((tmx = localtime (&endtime)) == NULL) {
    printf ("Error extracting time stuff\n");
    exit(0);
  }

    
  fprintf(f, "r: endtime:\t%04d-%02d-%02d %02d:%02d:%02d\n",
	  tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
	  tmx->tm_hour, tmx->tm_min, tmx->tm_sec);

  

  if (consistent){
    printSolution(g, bCost, bestSolution, f);
    //printf("feasible\n");
    fprintf(f,"r: feasible\t=========================== END\n");
  }else{
    //  printf("infeasible\n");
    fprintf(f,"r: reach infeasible\t=========================== END\n");
  }

  fflush ( stdout );/* this line */  
  fclose(f);



  // free the dynamic memory
  for (int cross=0; cross<nbChildren; ++cross){
    free(tmpSolutions[cross]);
    tmpSolutions[cross] = NULL;
  }

  
  for (int i=0; i<populationSize; ++i){
    free(population[i]);
    population[i] = NULL;
  }

  free(population);
  population = NULL;

  free(tmpSolutions);
  tmpSolutions = NULL;
  free(bestSolution);
  bestSolution = NULL;
  free(freqParents);
  freqParents = NULL;

  free(weightsLearned);
  weightsLearned = NULL;

  freeTabuColMemory();
  freeCrossOverMemory(crossParentsNb);

  return consistent;
  
}



bool testTabu(char** graph){
  int* a = malloc(sizeof(int)*nbSommets);
  randomSolution(a);

  //int *weightsLearned = malloc(sizeof(int)*nbSommets);
  mallocTabuColMemory();
   

  bool feasible = tabuCol(a, graph, nbColor, nbLocalSearch);//, weightsLearned);
  freeTabuColMemory();

  int costx = cost(a,graph);
  printf("tabu cost: %d\n", costx);

  //free(weightsLearned);
  //weightsLearned = NULL;
  return feasible;
}


bool testEA(char** graph, char *savefilename, char *inputFile){

  // define the operators
  CrossoverFuncPtr cross = crossover_enforced2;
  //FuncMutation* mutation = &mutation_identifyClasses;
  MutationFuncPtr mutation = mutation_grenade;
  //MutationFuncPtr mutation = mutation_zombie;

  return ea(cross, mutation, graph, savefilename, inputFile);
}

/*!
 * identify the critical subgraph
 * @param graph adjacency matrix
 */
void testCritical(char** graph){

  int* weightsVars = malloc(sizeof(int)*nbSommets);
  double **weightsDegrees = malloc(sizeof(double*)*nbSommets);
  int k = 3;
  double **certio = malloc(sizeof(double*)*k);
  
  // initialize the node data and node degree
  for (int i=0; i<nbSommets; ++i){
    weightsDegrees[i] = malloc(sizeof(double)*2);

    if (i<k){
      certio[i] = malloc(sizeof(double)*2);
      certio[i][0] = 0.0;
      certio[i][1] = 0.0;
    }
    for (int j=0; j<nbSommets; ++j){
      if (graph[i][j]) ++weightsDegrees[i][1]; 
    }
  }
  mallocTabuColMemory();

  int* a = malloc(sizeof(int)*nbSommets);
  randomSolution(a);
  //tabuCol(a, graph, nbColor, nbLocalSearch);
  
  
  while(!tabuColLearntWeights(a, graph, nbColor, 
			      nbLocalSearch, weightsVars)){
    
    for (int i=0; i<nbSommets; ++i){
      
      if (i<k){
	certio[i][0] = 0.0;
	certio[i][1] = 0.0;
      }

      weightsDegrees[i][0] = weightsVars[i];
      printf("%d\t", weightsVars[i]);

      double sumWeights = 0.0;
      int degreeNode = 0;
      for (int j=0; j<nbSommets; ++j){
	if (graph[i][j] && a[j]>-1){
	  ++degreeNode;
	  sumWeights += weightsVars[j];
	}
      }
      
      weightsDegrees[i][1] = sumWeights;
      
    }
    printf("\n");

    int* labels = k_means(weightsDegrees, nbSommets, 2, k, 0.0001, certio);


    double maxNeighbors = -1.0;
    double maxSelf = -1.0;
    int labelSelf = 0;
    int labelNeighbors = 0;

    for (int i=0; i<k; ++i){

      if ( maxNeighbors < 0 || certio[i][1] > maxNeighbors){
	maxNeighbors = certio[i][1];
	labelNeighbors = i;
      }
      
      if ( maxSelf < 0 || certio[i][0] > maxSelf){
	maxSelf = certio[i][0];
	labelSelf = i;
      }


      printf("%d:%f\t%f\n", i, certio[i][0], certio[i][1]);
    }

    // remove the nodes from graph
    int nbRemoved = 0;
    for (int i=0; i<nbSommets; ++i){
      weightsVars[i] = 0; // erase the weights
      printf("%d\t", labels[i]);
      if (labels[i] != labelSelf || labels[i] !=labelNeighbors){
	a[i] = -1;
	++nbRemoved;
        for (int j=0; j<nbSommets;++j){
	  graph[i][j] = 0;
	  graph[j][i] = 0;
	}
      }
    }
    
    printf("\n%d\t%d\n", nbRemoved, nbSommets);


    
    
    free(labels);
    

    break;
  }
    

  for (int i=0; i<nbSommets; ++i){
    
    if(i< nbSommets-1)
      for (int j=i+1; j<nbSommets; ++j){
	if (graph[i][j]) printf("%d--%d\n", i,j);
      }
  }

  // free memory
  freeTabuColMemory();
  for (int i=0; i<nbSommets; ++i){
    free(weightsDegrees[i]);

    
    weightsDegrees[i] = NULL;

    if(i<k){
      free(certio[i]);
      certio[i] = NULL;
    }
  }
  
  free(certio);
  certio = NULL;
  free(weightsDegrees);
  weightsDegrees = NULL;
  free(weightsVars);
  weightsVars = NULL;
}


void testShortest(char** graph){
  int** distance = malloc(sizeof(int*)*nbSommets);
  
  for (int i=0; i<nbSommets; ++i)
    distance[i] = malloc(sizeof(int)*nbSommets);
  
  floyd_warshall(graph, distance);
  
  int max = 0;
  for (int i=0; i<nbSommets; ++i){
    for (int j=i; j<nbSommets; ++j){
      if (max < distance[i][j]) max = distance[i][j];
      if (distance[i][j]>0) printf("%d\t",distance[i][j]);
    }
    printf("\n");
  }
  
  printf("longest: %d\n",max);
  
  for (int i=0; i<nbSommets; ++i){
    free(distance[i]);
    distance[i] = NULL;
  }
  
  free(distance);
}

// ============ TESTING ============
void testAlgo(char *filename, char *inNbColor, char *inPopuSize, 
	      char *inLSIter, char *inMaxLSIter, char *inGenItr,
	      char *inMaxRemoveColor, char *savefilename){

    
  nbColor = atoi(inNbColor);
  
  populationSize = atoi(inPopuSize);
  nbLocalSearch = atoi(inLSIter);
  MAX_LocalSearch_Iteration = atoi(inMaxLSIter);
  Nb_Generation = atoi(inGenItr)*60;
  MAX_RemoveColors = atoi(inMaxRemoveColor);
  StableItr =  nbLocalSearch/3 * 2;

  MAX_Gamma = nbSommets +1;

  //nbColor = 48;
  //populationSize = 10;
  //nbLocalSearch = 7000;
  //MAX_LocalSearch_Iteration = 15000;
  //Nb_Generation = 10000;

  printf("d :%s nbColor:%d\tpopulationSize:%d\tnbLocalSearch:%d - %d\tTimeLimit:%d mins\tMutationParameter:%d\n",
  	 filename, nbColor,populationSize,nbLocalSearch,MAX_LocalSearch_Iteration,
  	 Nb_Generation/60, MAX_RemoveColors);

  loadGrapheSimple(filename);

    
  time_t endtime;
  struct tm *tmx;
  time(&endtime);

  
  if ((tmx = localtime (&endtime)) == NULL) {
    printf ("Error extracting time stuff\n");
    exit(0);
  }


      
    
  /*fprintf(f, "\t%04d-%02d-%02d %02d:%02d:%02d\n",
    tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
    tmx->tm_hour, tmx->tm_min, tmx->tm_sec);*/
  printf("t: beginTime:\t%04d-%02d-%02d %02d:%02d:%02d\n",
	 tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
	 tmx->tm_hour, tmx->tm_min, tmx->tm_sec);

  

  // Test 0: all pairs shortest path
  //testShortest(tConnect);
  //return;


  // Test 1: tabuCol algorithm
  bool feasible = testTabu(tConnect);

  // Test 2: ea algorithm
  //bool feasible = testEA(tConnect, savefilename,filename);


  // Test 3: critical subgraph
  //bool feasible = false;
  //testCritical(tConnect);

  printf("d: %s nbColor:%d\tpopulationSize:%d\tnbLocalSearch:%d - %d\tTimeLimit:%d mins\tMutationParameter:%d\n",
  	 filename, nbColor,populationSize,nbLocalSearch,MAX_LocalSearch_Iteration,
  	 Nb_Generation/60, MAX_RemoveColors);


  time(&endtime);

  
  if ((tmx = localtime (&endtime)) == NULL) {
    printf ("Error extracting time stuff\n");
    exit(0);
  }


       
    
  /*fprintf(f, "\t%04d-%02d-%02d %02d:%02d:%02d\n",
    tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
    tmx->tm_hour, tmx->tm_min, tmx->tm_sec);*/
  printf("t: endTime:\t%04d-%02d-%02d %02d:%02d:%02d\n",
	 tmx->tm_year+1900, tmx->tm_mon+1, tmx->tm_mday,
	 tmx->tm_hour, tmx->tm_min, tmx->tm_sec);

  

  if (feasible)
    printf("r: feasible =================== END\n");
  else
    printf("r: found infeasible =========== END\n");

  // free dynamic memory
  for (int i=0; i<nbSommets;++i){
    free(tConnect[i]);
    tConnect[i] = NULL;
  }
  
  free(tConnect);
  tConnect = NULL;
}
