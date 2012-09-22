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




int** mallocTabuColTable(int nbNodes, int nbColors){
  
  int** table = malloc(sizeof(int*)*nbNodes);
  //tGamma = malloc(sizeof(int*)*nbNodes);


  
  for (int i=0; i<nbNodes;++i){
    table[i] = malloc(sizeof(int)*nbColors);
    //tGamma[i]= malloc(sizeof(int)*nbColors);
  }

  return table;
}


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

  //  printf("in init\n");
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

  //printf("after init\n");
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

  assert(tTabu != NULL && tGamma != NULL);
  
  

  // init the memory 
  Move* tabuMove = malloc(sizeof(Move));
  int* tTmpColor = malloc(sizeof(int)*nbNodes);
  
  int maxNoImpIteration = maxIteration;

 
  //  printf("in tabucol\n");
 
  // init Tabu and Gamma Tables
  for (int i=0; i<nbNodes; ++i) {
    //copy color assignment     
    tTmpColor[i] = a[i];
  
    // clean the gamma table
    for (int j=0; j<colorNB; ++j) {
      tTabu[i][j]=-1;
      tGamma[i][j] = 0;
    }
  }
    
  
  int obj = initGammaTable(a,graph,tGamma,nbNodes); // init gamma table

  //printf("after obj\n");

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
      for (int j=0; j<nbNodes; ++j){
	a[j] = tTmpColor[j];
      }
      
      i = 0; // reset i if found best so far
      a[tabuMove->sommet] = tabuMove->color;
    }else if (bestObj == obj+delta){
      
      int tval = (rand()/(float)RAND_MAX) * 10 ;
      if (tval > 4){
	for (int j=0; j<nbNodes; ++j){
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
  
  printf("subgraph ================ BGN\n");
  printf("graph G\{ node [shape=point];\n");


  for (int i=0; i<subNb; ++i){
    for (int j=i; j<subNb; ++j){
      if (graph[sub[i]][sub[j]]){
	table[i][j] = 1;
	table[j][i] = 1;
	printf("%d -- %d;\n",i,j);
      }else{
	table[i][j] = 0;
	table[j][i] = 0;
      }
    }
  }

  printf("\}\n");
  printf("subgraph ================ END\n");
  
  
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

int generateCft(int nbNodes, int* solution,  char** graph, char* conflictList){

  int nb = 0;

 for (int i=0; i < nbNodes; ++i)   conflictList[i] = 0;

  

  for (int i=0; i < nbNodes; ++i){    
    if (conflictList[i]) continue;

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
  int* subSol;
  int color;
} Projection;


void createSublist(int nbNodes, char* conflictList, int conflictNb,  int* projection){
  int j=-1;
  for (int i=0; i<nbNodes; ++i){
    if (conflictList[i]){
      ++j;
      projection[j] = i;
    }
  }

  //printf("j %d and conflictnb %d\n", j+1, conflictNb);
  assert(j == (conflictNb-1));
}


void randomSolution(int nbNodes, int nbColors, int* a){
  for (int i=0; i<nbNodes; ++i){
    a[i] = (rand()/(float)RAND_MAX) * nbColors;
    //printf("%d:[%d]\t",i,a[i]);
  }
  //printf("\n");
}


bool solveSub(int idxCft, Projection* subProb,
	      char** graph, int maxIter){
  
  
  //======================================================= Neighbor ======== BGN

  //
  


  
  char** subgraph = createSubgraph(graph, subProb->nb, subProb->sub);

  assert(subgraph != NULL); 


  //    printf("before malloc sub\n");
  int** subTabu = malloc(sizeof(int*)*(subProb->nb));
  int** subGamma = malloc(sizeof(int*)*(subProb->nb));

  for (int i= 0; i<subProb->nb; ++i){
    subTabu[i] = malloc(sizeof(int)*(subProb->color));
    subGamma[i] = malloc(sizeof(int)*(subProb->color));
  }


			//int** subGamma = mallocTabuColTable(subProb->nb, subProb->color); 
			//int** subTabu = mallocTabuColTable(subProb->nb, subProb->color); 
  //mallocTabuColMemory(nbNeighbors, nbColor-1, subTabu, subGamma);

  randomSolution(subProb->nb, subProb->color, subProb->subSol);

  bool subfeasible = tabuCol(subProb->subSol, subgraph, subProb->nb, 
			     subProb->color, maxIter, subGamma,
			     subTabu);

  //freeTabuColMemory(subProb->nb, subTabu, subGamma);
  //    printf("after free\n")
  for (int i= 0; i<subProb->nb; ++i){
    free(subTabu[i]); subTabu[i] = NULL;
    free(subGamma[i]); subGamma[i] = NULL;
  }

  free(subTabu); subTabu = NULL;
  free(subGamma); subGamma = NULL;
  
  assert(subTabu == NULL);

  for (int i= 0; i<subProb->nb; ++i){
    free(subgraph[i]); subgraph[i] = NULL;
  }
  
  free(subgraph); subgraph = NULL;
  assert(subgraph == NULL);



  return subfeasible;

}

/*
bool maxMatching(){
  
  //========== loop begin ==== verify the connectivity


  // verify the connection among the partition
  // remove the neighbors nodes 
  
  

  int tabCnt = 0;
  for (int c=0; c < subProb->color; ++c){

    

    if(tabCnt>=5){
        printf("\n");
	tabCnt = 0;
    }

    ++tabCnt;

    for (int ni=0; ni < subProb->color; ni++){
      colorTable[ni] = 0;
    }

        int nbCp = 0;

	int nbNp = 0;
  for (int i= 0; i< ( subProb->nb); i++){

    if(subSol[i]!=c) continue;

    ++nbNp;

    // specific
    for (int j=0; j< nbSommets; ++j){
      if (graph[subProb->sub[i]][j] && !graph[idxCft][j]){
	colorTable[graphSol[j]]=1;
      }
    }

 }

  
    for (int ni=0; ni < nbColor; ni++){
      if (colorTable[ni]) ++nbCp;
    }
    printf("color[%d,%d] -- %d\t", c,nbNp, nbCp);
  }
  printf("\n");
  
  
  //========== loop end

  
  
  // free the memory
  free(colorTable); colorTable = NULL;


}*/




// main solver of coloring problem
bool partitionMatch(char *filename, char* inNbColor, char *inMaxIter){

  nbColor = atoi(inNbColor);
  int maxIter =  atoi(inMaxIter);
  loadGrapheSimple(filename);


  char** graph = tConnect;
  // generate a tabu assignment
  // initialize two matrix for entire graph
  int** graphGamma =  malloc(sizeof(int*)*nbSommets); 
  int** graphTabu = malloc(sizeof(int*)*nbSommets); 
  int* graphSol = malloc(sizeof(int)*nbSommets);

  
   
  printf("nbColor = %d\n", nbColor);
  //mallocTabuColMemory(nbSommets, nbColor, graphTabu, graphGamma);

  
  for (int i= 0; i<nbSommets; ++i){
    graphGamma[i] =  malloc(sizeof(int*)*nbColor); 
    graphTabu[i] =  malloc(sizeof(int*)*nbColor); 
  }

  //graphGamma = mallocTabuColTable(nbSommets, nbColor);
  //graphTabu = mallocTabuColTable(nbSommets, nbColor);
  
  printf("nbSommets = %d\n",nbSommets);
  randomSolution(nbSommets, nbColor, graphSol);
  
  
  printf("maxIter  = %d\n", maxIter);
  bool feasible = tabuCol(graphSol, graph, nbSommets, 
			  nbColor, maxIter, graphGamma, 
			  graphTabu); 
  printf("before malloc = \n");
  if(feasible) return true;
  

  char* graphCft = malloc(sizeof(char)*nbSommets);
  char* tmpDisNodes = malloc(sizeof(char)*nbSommets);

  for (int i=0; i<nbSommets; ++i){
    tmpDisNodes[i] = 0;
    graphCft[i] = 0;
  }
  

  // create the subgraph based on N(x) 
  int nbCft = generateCft(nbSommets, graphSol, graph, graphCft);
  
  // find a conflict node idxCft
  int idxCft = 0;
  int nbNeighbors = 0; 
  int nbDisjoints = 0;


  for (int i=0; i<nbSommets; ++i){
    if (graphCft[i]){
      idxCft = i;

      for (int j=0; j<nbSommets; ++j){
	if (graph[idxCft][j]) ++nbNeighbors;
	else{
	  if (idxCft != j){ 
	    ++nbDisjoints;
	    tmpDisNodes[j] = 1;
	  }
	}
      }
      
      break;
    }
  }

  // get partial assignment on  disjoint nodes
  int* partialSol = malloc(sizeof(int)*nbSommets);
  for (int i=0; i<nbSommets; ++i){
    partialSol[i] = -1;
    if (!graph[idxCft][i]) partialSol[i] = graphSol[i];
  }
  

  printf("r: conflict nodes number = %d\n", nbCft);
  printf("r: neighbor nodes number = %d\n", nbNeighbors);
  printf("r: disjoint nodes number = %d\n", nbDisjoints);


  // generate the neighbor subgraph and solve it ==========================
  Projection* neighborSub = malloc(sizeof(Projection));
  neighborSub->nb = nbNeighbors;
  neighborSub->color = nbColor-1;
  assert(nbNeighbors != 0);

  neighborSub->sub = malloc(sizeof(int)*nbNeighbors);
  neighborSub->subSol = malloc(sizeof(int)*nbNeighbors);

  createSublist(nbSommets, graph[idxCft],neighborSub->nb, neighborSub->sub);
  bool feasibleNeighbor = solveSub(idxCft, neighborSub, graph, maxIter);

    // ignore the satisfiability of the subgraph
  if (feasibleNeighbor)
    printf("sub neighbor feasible\n");
  else
    printf("sub neighbor infeasible\n");



  // generate the disjoint subgraph and solve it ===========================
  
  Projection* disjointSub = malloc(sizeof(Projection));
  disjointSub->nb = nbDisjoints;
  disjointSub->color = nbColor;
  assert(nbDisjoints != 0);
  

  disjointSub->sub = malloc(sizeof(int)*nbDisjoints);
  disjointSub->subSol = malloc(sizeof(int)*nbDisjoints);
  
  createSublist(nbSommets, tmpDisNodes, disjointSub->nb, disjointSub->sub);
  bool feasibleDisjoint = solveSub(idxCft, disjointSub, graph, maxIter);

  // ignore the satisfiability of the subgraph
  if (feasibleDisjoint)
    printf("sub disjoint feasible\n");
  else
    printf("sub disjoint infeasible\n");

  

  
  // create disjoint nodes' color table
  int* cpDisjoint = malloc(sizeof(int)*nbSommets);
  for (int i=0; i<nbSommets; ++i) cpDisjoint[i] = -1;

  char* colorTable = malloc(sizeof(char)*nbColor);


  for (int i=0; i< disjointSub->nb; ++i)
    cpDisjoint[disjointSub->sub[i]] = disjointSub->subSol[i];


  // preserve origin solution
  //for (int i=0; i<nbSommets; ++i) cpDisjoint[i] = partialSol[i];


  double totalCnt = 0.0;
  for (int i=0; i<nbColor-1; ++i){


    for (int ij=0; ij<nbColor; ++ij) colorTable[ij] = 0;

    int cnt = 0; 


    for (int j=0; j<neighborSub->nb; ++j){
      if (neighborSub->subSol[j] != i) continue;
      
      for (int k=0; k<nbSommets; ++k){
	if (graph[neighborSub->sub[j]][k] && cpDisjoint[k]>-1){
	  if (!colorTable[cpDisjoint[k]]){
	    colorTable[cpDisjoint[k]] = 1;
	    ++cnt;
	  }
	}
      }      
    }

    //printf("[%d]:%d\t", i, cnt);
    totalCnt += (double)cnt; 
  }

  totalCnt /= (double)(nbColor - 1);
  printf("\nr: density per color: %f", totalCnt);
  totalCnt = 0.0;

  for (int i=0; i< neighborSub->nb; ++i){

   for (int ij=0; ij<nbColor; ++ij) colorTable[ij] = 0;
    int cnt = 0; 



      
      for (int k=0; k<nbSommets; ++k){
	if (graph[neighborSub->sub[i]][k] && cpDisjoint[k]>-1){
	  if (!colorTable[cpDisjoint[k]]){
	    colorTable[cpDisjoint[k]] = 1;
	    ++cnt;
	  }
	}
       

	
      }      

      //printf("[%d]:%d\t", neighborSub->sub[i], cnt);
	totalCnt+= cnt;
    
  }
  
  totalCnt /= (double)(neighborSub->nb);
  printf("\nr: density per node: %f\n", totalCnt);


  // combine the partial solution with neighbor solution in brute way
  
  graphSol[nbColor-1] = nbColor - 1;
  for (int i=0; i<neighborSub->nb; ++i)
    graphSol[neighborSub->sub[i]] = neighborSub->subSol[i];

  
  nbCft = generateCft(nbSommets, graphSol, graph, graphCft);
  printf("before final conflict nb: %d\n", nbCft);

  
  feasible = tabuCol(graphSol, graph, nbSommets, 
			  nbColor, maxIter, graphGamma, 
			  graphTabu); 

  if (!feasible){
    nbCft = generateCft(nbSommets, graphSol, graph, graphCft);
    printf("r: final conflict nb: %d\n", nbCft);
  }else
    printf("r: success\n");



  free(colorTable); colorTable = NULL;
  free(cpDisjoint); cpDisjoint = NULL;


  // free memory ========================================================== 
  assert(neighborSub != NULL);
  assert(disjointSub != NULL);
  assert(tmpDisNodes != NULL);
  

  free(disjointSub->sub); disjointSub->sub = NULL; 
  free(disjointSub->subSol);disjointSub->subSol = NULL;
  free(disjointSub); disjointSub  = NULL;


  free(neighborSub->sub); neighborSub->sub = NULL;
  free(neighborSub->subSol); neighborSub->subSol = NULL;
  free(neighborSub); neighborSub  = NULL;




  free(tmpDisNodes); tmpDisNodes = NULL;
  
  
  for (int i= 0; i<nbSommets; ++i){
    free(graphGamma[i]); graphGamma[i] = NULL;
    free(graphTabu[i]); graphTabu[i] = NULL;
  }

  free(graphCft); graphCft = NULL;
  free(graphGamma); graphGamma = NULL;
  free(graphTabu); graphTabu = NULL;
  free(graphSol); graphSol = NULL;
  free(partialSol); partialSol = NULL;
  
  graph = NULL;

  
  //======================================================= Neighbor ======== BGN

  //  printf("exists disjoint edges\n");
  return true;

}



//============================================================
// Multi-plane algorithm 
//============================================================

int degree(int idx, int size,  char* neighbors){
  int cnt = 0;
  for (int i=0; i<size; ++i)
    if (neighbors[i]) ++cnt;

  return cnt;
}


int cliqueFinder(int nbNodeInClique, char* clique,  char* candidate,
	   int* inWeightConjoint, char** graph){
  
  // find max weight adjacent nodes
  int maxWeight = -1; 
  int idx = -1;


  if (nbNodeInClique < 1){
    for (int i=0; i<nbSommets; ++i){
      if (!candidate[i]) continue;

      if (maxWeight < inWeightConjoint[i]){
	maxWeight = inWeightConjoint[i];
	idx = i;
      }
    }
    return idx;
  }
  

  for (int i=0; i<nbSommets; ++i){
    if (!candidate[i]) continue;
    
    if (idx < 0 || maxWeight < inWeightConjoint[i]){
      bool isClique = true;
      for (int j=0; j<nbSommets; ++j){
	if (clique[j] && !graph[i][j]){
	  isClique = false;
	  break;
	}
      }
      if (isClique){
	maxWeight = inWeightConjoint[i];
	idx = i;
      }
      
    }
  }
  return idx;
}


bool multiPlaneAnalysis(char *filename, char* inNbColor, char *inMaxIter){
  
  nbColor = atoi(inNbColor);
  int maxIter =  atoi(inMaxIter);
  loadGrapheSimple(filename);

  printf("%s\t", filename);


  char** graph = tConnect;


  // find max degree node
  int maxDegreeNodeIdx = -1;
  int degreeTmp = 0; 
  
  for (int i= 0; i<nbSommets; ++i){
    int d=degree(i, nbSommets, graph[i]);
    if (degreeTmp < d){
      degreeTmp = d; 
      maxDegreeNodeIdx = i;
    }
  }

  assert(maxDegreeNodeIdx != -1);

  // get N(x)
  

  // compute the disjoint counter and conjoint counter
  int* weightConjoint = malloc(sizeof(int)*nbSommets);

  for (int i= 0; i<nbSommets; ++i) weightConjoint[i] = 0;
  

  for (int i=0; i<nbSommets; ++i){
    // ignore the non-neighors and non-self
    if (!graph[maxDegreeNodeIdx][i] && maxDegreeNodeIdx != i) continue;
    
    for (int j=0; j<nbSommets; ++j){
	if (graph[i][j]) ++weightConjoint[j];
    } 
  }


  int minConjoint = -1;
  int minConjointIdx = - 1;
  for (int i= 0; i<nbSommets; ++i){

    if (weightConjoint[i] < 1) continue; // ignore the zero one

    if (minConjoint < 0 || minConjoint > weightConjoint[i]){
      minConjoint = weightConjoint[i];
      minConjointIdx = i;
    }
  }
      
  assert(minConjointIdx != -1);
    
  weightConjoint[minConjointIdx] = 0;
  int candidateCnt = 0;
  int maxConjoint = -1;
  int maxConjointIdx = -1;
  char* candidate = malloc(sizeof(char)*nbSommets);
  char* clique = malloc(sizeof(char)*nbSommets);
  
  for (int i= 0; i<nbSommets; ++i){
     candidate[i] = 0;
     clique[i] = 0;
  }

  for (int i= 0; i<nbSommets; ++i){

    if (graph[minConjointIdx][i])
      weightConjoint[i] = 0;
    else{

      if (maxConjoint < weightConjoint[i]){
	maxConjoint = weightConjoint[i];
	maxConjointIdx = i;
      }
      ++candidateCnt;
      candidate[i] = 1;
    }
  }
  
  //printf("candidateCnt: %d weight edges: %d\n", candidateCnt, maxConjoint);



  int defCliqueSize = maxIter;
  int* cliqueVec = malloc(sizeof(int)*defCliqueSize);
  
  for (int cliqueNb = 0; cliqueNb < defCliqueSize; ++cliqueNb){

    int idx = cliqueFinder(cliqueNb, clique, candidate, weightConjoint, graph);
    assert(idx != -1);
    
    candidate[idx] = 0;
    clique[idx] = 1;
    cliqueVec[cliqueNb] = idx;

  }

  int Nc = 0;
  int Dis = 0;

  for (int i=0; i<nbSommets; ++i){
    if (clique[i]) continue;
    
    int tmpCnt = 0;
    for (int j=0; j<defCliqueSize; ++j){
      if (graph[i][cliqueVec[j]]) ++tmpCnt;
    }
    
    if (tmpCnt == defCliqueSize) ++Nc;
    if (tmpCnt < 1) ++Dis;
    
  }

  printf("%d, %d, %d, %d, ",defCliqueSize, Nc, nbSommets-defCliqueSize-Nc-Dis, Dis);

  // find a 3-clique by seeking in N(maxConjointIdx)
  
  
  free(cliqueVec); cliqueVec = NULL;
  free(clique); clique = NULL;
  free(candidate); candidate = NULL;

  
  free(weightConjoint); weightConjoint = NULL;
  graph = NULL;

  return true;
}


bool multiPlaneAnalysisSimple(char *filename, char* inNbColor, char *inMaxIter){
  
  nbColor = atoi(inNbColor);
  int maxIter =  atoi(inMaxIter);
  loadGrapheSimple(filename);

  printf("%s\t", filename);


  char** graph = tConnect;


  // compute the disjoint counter and conjoint counter
  int* weightConjoint = malloc(sizeof(int)*nbSommets);
  
  for (int i= 0; i<nbSommets; ++i)
    weightConjoint[i]=degree(i, nbSommets, graph[i]);


  // find min connection node
  
  int minConjoint = -1;
  int minConjointIdx = - 1;
  for (int i= 0; i<nbSommets; ++i){

    if (weightConjoint[i] < 1) continue; // ignore the zero one

    if (minConjoint < 0 || minConjoint > weightConjoint[i]){
      minConjoint = weightConjoint[i];
      minConjointIdx = i;
    }
  }
  
  assert(minConjointIdx != -1);


  int candidateCnt = 0;
  int maxConjoint = -1;
  int maxConjointIdx = -1;
  char* candidate = malloc(sizeof(char)*nbSommets);
  char* clique = malloc(sizeof(char)*nbSommets);
  
  for (int i= 0; i<nbSommets; ++i){

    if (i != minConjointIdx && !graph[minConjointIdx][i])
     candidate[i] = 1;
    else
     candidate[i] = 0;

     clique[i] = 0;
  }

  int defCliqueSize = maxIter;
  int* cliqueVec = malloc(sizeof(int)*defCliqueSize);
  
  for (int cliqueNb = 0; cliqueNb < defCliqueSize; ++cliqueNb){

    int idx = cliqueFinder(cliqueNb, clique, candidate, weightConjoint, graph);
    assert(idx != -1);
    
    candidate[idx] = 0;
    clique[idx] = 1;
    cliqueVec[cliqueNb] = idx;

  }

  int Nc = 0;
  int Dis = 0;

  for (int i=0; i<nbSommets; ++i){
    if (clique[i]) continue;
    
    int tmpCnt = 0;
    for (int j=0; j<defCliqueSize; ++j){
      if (graph[i][cliqueVec[j]]) ++tmpCnt;
    }
    
    if (tmpCnt == defCliqueSize) ++Nc;
    if (tmpCnt < 1) ++Dis;
    
  }

  printf("%d, %d, %d, %d, ",defCliqueSize, Nc, nbSommets-defCliqueSize-Nc-Dis, Dis);

  // find a 3-clique by seeking in N(maxConjointIdx)
  
  
  free(cliqueVec); cliqueVec = NULL;
  free(clique); clique = NULL;
  free(candidate); candidate = NULL;

  
  free(weightConjoint); weightConjoint = NULL;
  graph = NULL;

  return true;
}
