
//////////////////////////////////////////////////////////////////////////////////////////////
/////////// EA with IIS 
/////////// author: Jun HU
//////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "util/gfile.h"
#include "ealgo.h"



float lambdaValue = 0.6;
int LValue = 10;



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
 * TabuCol c implementation
 * @return true: if the solution is consistent
 * false: if the solution is inconsistent
 * @parameter graph: adjacent list of graph
 */



typedef struct assignment{
  int sommet;
  int color;
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


/*!
 * initialize the Gamma table
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @param tGamma gamma table for total incremental objective function computing n*k
 * @return the number of violated edges
 */
int initGammaTable(int* a, char** graph, int** tGamma){
  /// determine les conflits entre les noeuds
  int nbConflict=0;
	
  // check the link
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0) continue;

    for (int j=i; j<nbSommets; ++j){
      if (a[j] < 0) continue;
	    
      if( graph[i][j]){ // only verify assigned node

	if (a[i] == a[j]) ++nbConflict;

	// increase gamma
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
	  double tval=(rand()/(double)RAND_MAX);

	  //if (tval > 100/bestCnt){
	  if (tval < 1/(double)bestCnt){
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
	double tval=(rand()/(double)RAND_MAX);

	if (tval < 1/(double)bestVarCnt){
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
		int** tGamma, char** graph, int* ind){
  
  for (int i=0; i< nbSommets; ++i){
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
bool tabuCol(int* a, char** graph, int colorNB, int maxIteration){

 
  int** tGamma = malloc(sizeof(int*)*nbSommets);
  int** tTabu = malloc(sizeof(int*)*nbSommets);


  int* tTmpColor = malloc(sizeof(int)*nbSommets);// store the temp color assignment
  
  int maxNoImpIteration = maxIteration;
 

  // init Tabu and Gamma Tables
  for (int i=0; i<nbSommets; ++i) {
   //copy color assignment     
    tTmpColor[i] = a[i];

    tGamma[i] = malloc(sizeof(int)*colorNB);
    tTabu[i] = malloc(sizeof(int)*colorNB);
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
  
  Move* move = malloc(sizeof(Move));
  // a always records best so far solution
  for (int i=0;  i< maxNoImpIteration ; ++i){
    
    move->sommet = -1;
    move->color = -1;
    if(bestObj < 1) break; // find consistent solution

    // find best move based on gamma table
    int delta = bestMove(move, tGamma, tTabu, tTmpColor, colorNB);

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
    }else if (bestObj == obj+delta){
      
      int tval = (rand()/(float)RAND_MAX) * 10 ;
      if (tval > 4){
	for (int j=0; j<nbSommets; ++j){
	  a[j] = tTmpColor[j];
	}

	a[move->sommet] = move->color;
      }
    }

    // update move
    updateMove(move->sommet, tTmpColor[move->sommet], move->color, tGamma, graph, tTmpColor);
 
    
    // calculate the nbVariable in conflict
    int nbConflict = 0;
    for (int j=0; j<nbSommets; ++j){
      if (tGamma[j][tTmpColor[j]]>0)
	nbConflict += tGamma[j][tTmpColor[j]];
    }
    
    int rdx=(rand()/(float)RAND_MAX) * LValue;
    tTabu[move->sommet][tTmpColor[move->sommet]] = rdx + lambdaValue*nbConflict/2; // tabu duration

    tTmpColor[move->sommet] = move->color;
    obj += delta;
    

  }
  

 
  // free all dynamic memory before return 
  for (int i=0; i<nbSommets; ++i){
    free(tGamma[i]);
    free(tTabu[i]);
    tGamma[i] = NULL;
    tTabu[i] = NULL;
  }

  free(tGamma);
  free(tTabu);
  free(tTmpColor);
  free(move);

  tGamma = NULL;
  tTabu = NULL;
  tTmpColor = NULL;
  move = NULL;

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
  
  register int *classSize = malloc(sizeof(int)*nbColor);
  
  initialArray(classSize, nbColor, 0);
  
  int maxIdx = -1;
  int maxSize = 0;
  for (int j=0; j<nbSommets;++j){
      if (b[j] > -1 || a[j] <0) continue;
      
      ++classSize[a[j]];
      if (maxIdx < 0 || maxSize < classSize[a[j]]){
	maxIdx = a[j];
	maxSize = classSize[a[j]]; 
      }
  }


  free(classSize);
  classSize = NULL;
  
  return maxIdx;
}


void maxColorsClasses( int nbParent, int *chosenParent,  int **parents, 
		       int *b, char **graph, Move *move){
  
  register int *classSize = malloc(sizeof(int)*nbColor);
  

  
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
  
  register int *classSize = malloc(sizeof(int)*nbColor);
  

  
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
 * calculate the number of violated edges
 * @param a individual color table
 * @param graph adjacent matrix of graph
 * @return the number of violated edges
 */
int cost(int* a, char** graph){
  register int nbConflict = 0;
  register char* conflict = malloc(sizeof(char)*nbSommets);
  
  for (int i=0; i<nbSommets; ++i){
    conflict[i] = 0;
  }

  for (int i=0; i<nbSommets; ++i){
    if (a[i] < 0 || conflict[i]) continue;

    for (int j=0; j<nbSommets; ++j){
      if (graph[i][j] && a[i] == a[j]){
        if (!conflict[i]){
	  conflict[i] = 1;
	  ++nbConflict;
	}
	
	if (!conflict[j]){
	  conflict[j] = 1;
	  ++nbConflict;
	}
      }
    }
    
    
    
  }
  
  free(conflict);
  conflict = NULL;
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
void crossover_maximal(int nbParents, int** parents, int* offspring, char** graph, int* freqParents){

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
      int val = (rand()/(float)RAND_MAX) * 1000 ;
      if (idx < 0 || val > 1000/cnt)
	idx = i;
    }
  }

  return idx;
}




/*!
 * crossover operator - improve the solution and decrease 
 * the computational time of local search, at sametime, it
 * reduces the diversity
 * @param nbParents the number of whole population
 * @param parents the whole population
 * @param offspring carry out the created offspring
 * @param graph adjacent matrix
 * @param freqParents counter the participation number of each parent
 */

void crossover_nogood(int nbParents, int** parents, int* offspring, char** graph, int* freqParents){
  

  //printf("in ngood crossover\n");
  
  

  initialArray(offspring, nbSommets, -1);
  
  // randomly choose two nogoods nodes from 2 parents
  int totalPartial = 0;
  for (int p=0; p<3; ++p){
  
    int jth = (rand()/(float)RAND_MAX) * nbParents ;
    ++freqParents[jth];

    /*printf("jth is: %d\n", jth);
  
    for (int x = 0; x<nbSommets; ++x){
      if (parents[jth][x] < 0){
	printf("1.1. parent has null\n");
	break;
      }
    }*/
    
    int indx = randomConflict(parents[jth],graph);

    //printf("random conflict is: %d\n", indx);

    // randomly generate initial solution for subproblem which consists of nogood 
    for (int i=0; i<nbSommets;++i){
      if (graph[indx][i] && offspring[i] < 0){
	int col = (rand()/(float)RAND_MAX) * (nbColor-1) ;
	offspring[i] = col;
	++totalPartial;
      }
    }
  }

  

  // tabuCol attempts to find consistent partial soltuion with  nbColor-1
  //printf("before tabu partial : %d\n", totalPartial);
  tabuCol(offspring, graph, nbColor-1, nbLocalSearch);
  int afterTabu = cost(offspring, graph);
  //printf("after tabu partial\n");

  /*
  totalPartial = 0;
  for (int c=0; c<nbSommets;++c){
    if (offspring[c] > -1) ++totalPartial;
  }
  
  printf("partial: %d\t%d\n", totalPartial, afterTabu);*/
  
  
  int* classSize = malloc(sizeof(int)*nbColor);

  for (int i=0; i<nbSommets;++i){
    if (offspring[i] < 0) continue;

    ++classSize[offspring[i]];
  }

  int maxSize = 0;
  int colorIdx = -1;
  
  for (int i=0; i<nbColor;++i){
    if (maxSize < classSize[i]){
      maxSize = classSize[i];
      colorIdx = i;
    }
  }

  for (int i=0; i<nbSommets;++i){
    if (offspring[i] != colorIdx) offspring[i] = -1;
    else offspring[i] = 0;
  }

  
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
  
  int ith = -1;
  for (int i=1; i<nbColor; ++i){
    initialArray(classSize,nbColor,0);

    
    // get a parent
    if (ith < nbCross-1) ++ith;
    else ith = 0;

    int jth = idxParents[ith];
    //++freqParents[jth];
    
    /*printf("2.5 choosen ith and  jth is: %\d\t%d\n", ith, jth);
    for (int x = 0; x<nbSommets; ++x){
      if (parents[jth][x] < 0){
	printf("3. parent has null %d\n", jth);
	break;
      }
    }*/
    
    
    for (int j=0; j<nbSommets;++j){
      if (offspring[j] > -1) continue;
      
      ++classSize[parents[jth][j]];   
    }

    maxSize = 0;
    colorIdx = -1;
    
    for (int k=0; k<nbColor;++k){
      if (maxSize < classSize[k]){
	maxSize = classSize[k];
	colorIdx = k;
      }    
    }



    for (int j=0; j<nbSommets;++j){
      if (offspring[j] < 0 && parents[jth][j] == colorIdx ){
	  offspring[j] = i;
      }
    }




  }
  
  // find the maximal cardinality color class
  
  
  int conflictNb = cost(offspring, graph);


  // complete the partial solution =============== working
 
  for (int i=0; i<nbSommets;++i){
    if (offspring[i] < 0){
      int col = (rand()/(float)RAND_MAX) * (nbColor) ;
      offspring[i] = col;
    
    }
  }

  //printf("number of partial nodes: %d\t%d\t%d\n", totalPartial, randomAssigned, conflictNb);

  free(classSize);
  free(idxParents);
  classSize = NULL;
  idxParents = NULL;
  //printf("out ngood crossover\n");
}


void generate_sub(int *a, char **graph){
    
  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  
  int nbConflict = 1;
  
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

    if(a[i] < (nbColor -2)) continue;
    
    a[i] = nbColor-3; 
  }

  /*
  while(hasConflictSolution(a,graph)){
    int index = randomConflict(a, graph);
    a[index] = -1; // remove the chosen node
  }

  // redo the highest index color nodes
  for (int i=0; i<nbSommets; ++i){
    if (a[i] < (nbColor -1)) continue;
    
    a[i] = nbColor-2; 
  }*/

  // try to find a partial consistent solution 
  // with k-1 colors
  //tabuCol(a, graph, nbColor-1, MAX_LocalSearch_Iteration);
  tabuCol(a, graph, nbColor-2, nbLocalSearch);
  free(conflict);
  conflict = NULL;
}

void generate_sub_simple(int *a, char **graph){
    
  // randomly remove the conflict nodes one by one, 
  // until a consistent partial solution 
  

  while(hasConflictSolution(a,graph)){
    int index = randomConflict(a, graph);
    a[index] = -1; // remove the chosen node
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
bool mutation_sub(int *a, char **graph, int removeColorNb){

  //generate_sub(a, graph);


  int subColor = nbColor - removeColorNb;

  for (int i=0; i<nbSommets; ++i){
    if (a[i] < subColor ) continue;

    a[i] = subColor-1; // simply assign the highest index color
  }
  

  bool feasible = tabuCol(a, graph, subColor, MAX_LocalSearch_Iteration);


  return feasible;
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

void crossover_iis(int nbParents, int** parents, int* offspring, char** graph, int* freqParents){
  
  // initialize 
  initialArray(offspring, nbSommets, -1);

  /*
  int *iisParent = malloc(sizeof(int)*nbSommets);
  int pIdx = (rand()/(float)RAND_MAX) * nbParents;

  for (int i=0; i<nbSommets; ++i){
    iisParent[i] = parents[pIdx][i];
  }

  generate_sub_simple(iisParent, graph);

  
  
  // force first parent
  int removeColorNb = (rand()/(float)RAND_MAX) * 2;
  ++removeColorNb;


  if (removeColorNb > nbColor-1)
    removeColorNb = 1;



  printf("remove color nb: %d\n",removeColorNb);
  //removeColorNb = 1;
  mutation_sub(iisParent,graph,removeColorNb);
  generate_sub_simple(iisParent, graph);

  int colorIdxIIS = maxColorClass(iisParent, offspring, graph);

  for (int i=0; i<nbSommets; ++i){
    if(iisParent[i] !=colorIdxIIS) continue;

    offspring[i] = nbColor-1;
  }

  free(iisParent);
  */
  
  int nbCross = (rand()/(float)RAND_MAX) * 3;
  nbCross += 2;
  int* idxParents = malloc(sizeof(int)*nbCross);

  //randomParents(nbCross, idxParents, nbParents);
  lessFreqParents(nbCross, idxParents, nbParents, parents, freqParents);

  int **pcopies = malloc(sizeof(int*)*nbCross);
  
  for (int i=0; i<nbCross; ++i){
    pcopies[i] = malloc(sizeof(int)*nbSommets);
    for (int j=0; j<nbSommets; ++j){
      pcopies[i][j] = parents[idxParents[i]][j];
    }
    
    int tavl = (rand()/(float)RAND_MAX) * 10 ;
    if (tavl<4) 
      generate_sub_simple(pcopies[i], graph);

  }

  
  

  //Move* move = malloc(sizeof(Move));
  
  int crossIdx = -1;
  for (int i=1; i<nbColor-1; ++i){

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







/*!
 * create the offspring based on parents
 * @param population the whole population
 * @param offspring carry out the created offspring
 * @param ngd 
 */
void selection_freq(int** population, char** graph, int* offspring, int* freqParents){
  
  

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
      int tval = (rand()/(float)RAND_MAX) * 10 ;
      if (tval < 5){
	index = i;
      }
    }
  }

  // update the population 
  for (int i=0; i<nbSommets; ++i){
    population[index][i] = offspring[i];
  }

  freqParents[index] = 0;
}

void printSolution(int cost, int *a){
  
  return; // ignore
  
  printf("s: %d",cost);
  for (int i=0; i<nbSommets; ++i){
    printf("\t%d", a[i]); 
  }
  printf("\n");
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
    tabuCol(population[i],graph, nbColor, MAX_LocalSearch_Iteration);
  }
  
  int* freqParents = malloc(sizeof(int)*populationSize);
  
  initialArray(freqParents,  populationSize, 0);

  // initialize the population

  bool setBest = false;
  int* bestSolution = malloc(sizeof(int)*nbSommets);
  int* tmpSolution = malloc(sizeof(int)*nbSommets);
  int bCost = -1;
  int tCost = -1;
  int crossCost = -1;
  

  // iterate the generation
  int cent = 0;
  bool switchX = true; // should be true
  int gen = 0; // crossover number
  int switchIteration = populationSize/2;

  int MaxRemoveColor = 5;
  int MinRemoveColor = 0;
  int removeColor = MinRemoveColor;

  for (int g = 0; g < Nb_Generation; ++g){
    
    initialArray(tmpSolution,nbSommets,-1);

      

    //// crossover operator ==================================== BGN
    //if (true){
    if (cent < switchIteration){
      ++cent;
      //crossover_maximal(populationSize, population, tmpSolution, graph, freqParents);    
      //crossover_nogood(populationSize, population, tmpSolution, graph, freqParents);
      //crossover_sub(populationSize, population, tmpSolution, graph, freqParents);
      //crossover_sub_simple(populationSize, population, tmpSolution, graph, freqParents);
      crossover_iis(populationSize, population, tmpSolution, graph, freqParents);

      crossCost = cost(tmpSolution, graph);

      // mutation after crossover
      int tval = (rand()/(float)RAND_MAX) * 10 ;
      int mutation_iteration = nbLocalSearch;
      //if (tval < 2)
	mutation_iteration = MAX_LocalSearch_Iteration;
    
      if (tabuCol(tmpSolution,graph,nbColor,mutation_iteration)){
      
 	for (int c=0; c<nbSommets;++c){
	  bestSolution[c] = tmpSolution[c];
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

	tCost = cost(tmpSolution,graph);
      
	if (!setBest || bCost > tCost){

	  if (!setBest) setBest = true;
	  for (int i = 0; i<nbSommets; ++i){
	    bestSolution[i] = tmpSolution[i];
	    bCost = tCost;
	  }
	  // print best solution so far 
	  printSolution(bCost, bestSolution);
	  //cent = 0;
	} 
      }
      
      printf("costb:");
    for (int i=0; i<populationSize;++i){
      int cx = cost(population[i],graph);
      printf("\t%d[%d]",cx,freqParents[i]);
    }
    printf("\n");
      
      // replace the highest frequenced parent
      selection_freq(population, graph, tmpSolution, freqParents);
      
      //printf("i: Crossover operator\n");
    }

    //// crossover operator ==================================== END

  
    //// mutation  operator =========================== BGN
   
    
    
    // mutation operator (subproblem )============================
    
    //if (false){
    if (cent > switchIteration -1){

      // remove 1-3 colors
      int removeColor = (rand()/(float)RAND_MAX) * 3 ;
      ++removeColor;

      
      if (removeColor > nbColor-1)
	removeColor = 1;

      cent = 0;
      //if (mutation_iteration != MAX_LocalSearch_Iteration && tval > 4){
      //printf("in mutation sub\n");

      for (int mi=0; mi<populationSize/2;++mi){
      
	int jth;// = (rand()/(float)RAND_MAX) * populationSize;
      
	int freq = -1;
	for (int i = 0; i<populationSize; ++i){
	  if (freq < 0 || freq < freqParents[i]){
	    freq = freqParents[i];
	    jth = i;
	  }
	}

	freqParents[jth] = 0;
	//int costx = cost(population[jth], graph);
	//printf("i: mutation operator\n");
	crossCost = cost(population[jth], graph);
      
	bool mutFeasible =false;

	
	int tval = 10;
	if (tval > 6){
	  // re-introduce best solution found
	  for (int c=0; c<nbSommets;++c){
	    population[jth][c] = bestSolution[c];
	  }
	}else{

	  
	  mutFeasible = mutation_sub(population[jth], graph, removeColor);
	}
      
	tCost = cost(population[jth], graph);

	if (!setBest || bCost > tCost){

	  if (!setBest) setBest = true;
	  for (int i = 0; i<nbSommets; ++i){
	    bestSolution[i] = population[jth][i];
	    bCost = tCost;
	  }
	  // print best solution so far 
	  printSolution(bCost, bestSolution);
	  //cent = 0;
	}

	if (mutFeasible){
	
	  for (int c=0; c<nbSommets;++c){
	    bestSolution[c] = population[jth][c];
	    if (bestSolution[0]<0){
	      printf("found partial problem\n");
	      exit(0);
	    }
	  }
	
	  int bestC = cost(bestSolution, graph);
	  printf("consistent solution found by mut2: %d\n",bestC);
	  bCost = 0;
	  break;
	}

      }

    }

    /*
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
    printf("i: parents in conflict:%d\n",nb);*/
    //// mutation (tabuCol) operator =========================== END
    
    //// selection operator ============================ BGN
    //// selection operator: update population

    //// selection operator ============================ BGN
    
    //printf("selection\n");
    


    //// selection operator ============================ END


    // print info
    
    printf("costp:");
    for (int i=0; i<populationSize;++i){
      int cx = cost(population[i],graph);
      printf("\t%d[%d]",cx,freqParents[i]);
    }
    printf("\n");
    printf("costg:\t%d\t%d\t%d\t%d\t%d\n",g+1,++gen,crossCost,tCost,bCost);
  }


  

  // print best solution so far 
  printSolution(bCost, bestSolution);


  // free the dynamic memory


  free(tmpSolution);

  free(bestSolution);

  free(freqParents);


  if (bCost != 0)
    return false;


  
  // verify the solution
  bool consistent = true;
  for (int i=0; i<nbSommets; ++i){
    if (bestSolution[i] < 0 || bestSolution[i] > nbColor-1){
      printf("solution is partial");
      consistent = false;
      break;
    }

    for (int j=0; j<nbSommets; ++j){
      if (graph[i][j]){
	//printf("%d\t%d\n",i,j);
	if (bestSolution[i] == bestSolution[j]){
	  printf("solution isn't consistent");
	  consistent = false;
	  break;
	}
      }
    }
  }

  return consistent;

  //return true;
  
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



bool testTabu(char** graph){
  int* individual = malloc(sizeof(int)*nbSommets);
  randomSolution(individual);
  bool feasible = tabuCol(individual, graph, nbColor, nbLocalSearch);
  int costx = cost(individual, graph);
  printf("tabu cost: %d\n", costx);
  return feasible;
}


bool testEA(char** graph){
  return ea(graph);
}


bool testHeuristic(char** graph){
  // initialize an individual
   int* individual = malloc(sizeof(int)*nbSommets);
   initialArray(individual, nbSommets,-1);
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

// ============ UNIT TESTING ============
void testAlgo(char *filename, char *inNbColor, char *inPopuSize, 
	      char *inLSIter, char *inMaxLSIter, char *inGenItr){

    
  nbColor = atoi(inNbColor);
  
  populationSize = atoi(inPopuSize);
  //nbLocalSearch = atoi(inLSIter);
  //MAX_LocalSearch_Iteration = atoi(inMaxLSIter);
  //Nb_Generation = atoi(inGenItr);

  //nbColor = 48;
  //populationSize = 10;
  nbLocalSearch = 7000;
  MAX_LocalSearch_Iteration = 15000;
  Nb_Generation = 10000;

  printf("d: nbColor:%d\tpopulationSize:%d\tnbLocalSearch:%d - %d\tNbGeneration:%d\n",
	 nbColor,populationSize,nbLocalSearch,MAX_LocalSearch_Iteration,Nb_Generation);

  

  loadGrapheSimple(filename);


  // Test 0: all pairs shortest path
  //testShortest(tConnect);
  //return;


  // Test 1: tabuCol algorithm
  //bool feasible = testTabu(tConnect);

  // Test 2: ea algorithm
  bool feasible = testEA(tConnect);
  

  if (feasible)
    printf("feasible\n");
  else
    printf("found infeasible\n");


  // free dynamic memory
  for (int i=0; i<nbSommets;++i){
    free(tConnect[i]);
    tConnect[i] = NULL;
  }
  
  free(tConnect);
  tConnect = NULL;
}
