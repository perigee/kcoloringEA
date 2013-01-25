#ifndef GRAPH_HPP
#define GRAPH_HPP



#include <boost/dynamic_bitset.hpp>
#include <vector>

typedef std::vector<boost::dynamic_bitset<> > AdjMatrix;


void create_graph(AdjMatrix& graph, size_t n){
  graph.reserve(n); // fix the size of graph

  for (int i=0; i< n; ++i){

  }
}


#endif
