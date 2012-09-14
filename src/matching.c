// ====================================================
// Generic TabuCol Algorithm
// ====================================================




// initialize the matrix memory for tabucol
void mallocTabuColMemory(int nbNodes, int nbColors
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

  assert(tTabu != NULL && tGamma != NULL && tTmpColor != NULL);
  
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

  if(bestObj < 1) return true;
  
  return false;
}


/*!========================================================================
 * 

  ========================================================================*/



// projection between N(x) to matrix

void partitionMatch(){
  
  //


}
