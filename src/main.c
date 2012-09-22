#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util/gfile.h"
//#include "graphe.h"
//#include "ealgo.h"
#include "matching.h"

typedef int (*funcDef)(int,int, int*);


int changeInt(int a, int b, int* c){
  c[0] = 8;
  return a+b;

}


int main (int argc, char * const argv[]) {
  
  /*
  int *changeOne = malloc(sizeof(int));
  changeOne[0] = 1;

  funcDef myFunc = &changeInt;

  int tmp = (*myFunc)(5,6, changeOne);
  printf("result: %d\t%d\n",tmp,changeOne[0]);
  return 0;*/


  //================================== IIS inspired algorithm ================ BGN


  srand((unsigned)time(NULL));
  printf("d: Procedure de calcul ======================== \n");
  
  /*
  if(argc<5){
    printf("Erreur d'utilisation !!\n");
    exit(-1);
  }*/

  //bool feasible = partitionMatch(argv[1],argv[2],argv[3]);
  bool feasible = multiPlaneAnalysisSimple(argv[1],argv[2],argv[3]);
  
  printf("\n");

  
  // free dynamic memory
  for (int i=0; i<nbSommets;++i){
    free(tConnect[i]); tConnect[i] = NULL;
  }
  
  free(tConnect); tConnect = NULL;


  //testAlgo(argv[1], argv[2],argv[3],argv[4],argv[5],
  //	   argv[6], argv[7], argv[8]);


   return 0;
  //================================== IIS inspired algorithm ================ END


}
