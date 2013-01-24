#ifndef GRAPH_HPP
#define GRAPH_HPP



#include <boost/dynamic_bitset.hpp>
#include <vector>

typedef std::vector<boost::dynamic_bitset<> > AdjMatrix;


void create_graph(AdjMatrix& graph, size_t n){
  graph.reserve(n); // fix the size of graph
 
}


bool isConnected(AdjMatrix& graph, size_t n, size_t m){
  return graph[n]
}

#endif
