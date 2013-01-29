#ifndef IO_HPP
#define IO_HPP


//#include "graph.hpp"

#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>

#include <boost/dynamic_bitset.hpp>

// define adjacent matrix
typedef std::vector<boost::dynamic_bitset<> > AdjMatrix;




AdjMatrix read_graph(const std::string& filepath)
{

    AdjMatrix graph;
    int varSize = 0;
    std::ifstream fp(filepath.c_str());
    for (std::string line; getline(fp, line); )
    {

      if (line.size() < 2 ) continue;
     
      if (line[0] == 'p'){
	std::istringstream str(line.substr(1));
	std::string dummy;
	//int varSize;
	str >> dummy >> varSize; 
	// initialize the entire graph 
	//	std::cout << "nodes size: " << varSize << std::endl;	

	for (int i=0; i<varSize; ++i){
	  boost::dynamic_bitset<> op(varSize);
	  graph.push_back(op);
	}

	continue;
      }
      
        if (line[0] == 'e')
        {
            std::istringstream str(line.substr(1));
            int fi, si;
            str >> fi >> si;
            //std::cout << "fi: " << fi << "   si: " << si << '\n';
	    graph[--fi][--si]= 1;
	    graph[si][fi]= 1;

	    continue;
        }
    }
    

    return graph;
}



void write_graph(const AdjMatrix& graph, size_t n, 
                 std::string& filename){
  
  // bitset will be printed in reverse order
    for (int i=0; i<n; ++i)
      std::cout << graph[i] << std::endl;
}


#endif
