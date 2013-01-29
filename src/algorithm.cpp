


#include "algorithm.h"





struct Algo_Tabu
{


  Algo_Tabu(size_t nodesNb):_nodesN(nodesNb)
  {
    //    _gammaTable.reserve(nodesNb);
    //    _tabuTable.reserve(nodesNb*nodesNb);
  }

  void initMem(AdjMatrix& graph, size_t k 
	       SolutionVec::iterator begin, 
	       SolutionVec::iterator end)
  {
    
  }

  // @return true consistent, 
  bool search(AdjMatrix& graph, size_t k
	      SolutionVec::iterator begin, 
	      SolutionVec::iterator end)
  {
    bool isConsistent = false;
    init(graph, k, begin,end);
    
    

    return isConsistent;
  }


private:


  

  void init(AdjMatrix& graph, size_t k
	       SolutionVec::iterator begin, 
	       SolutionVec::iterator end)
  {
    
  }

  size_t _nodesN;
  std::vector<int> _gammaTable;
  std::vector<int> _tabuTable;
};
