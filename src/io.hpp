#ifndef IO_HPP
#define IO_HPP


//#include "graph.hpp"

#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>

typedef int AdjMatrix;




AdjMatrix read_graph(const std::string& filepath)
{
    AdjMatrix graph;


    // read nodes number


    //create_graph(graph, nbNodes);

    std::ifstream fp(filepath.c_str());
    for (std::string line; getline(fp, line); )
    {

      if (line.size() < 2 ) continue;
     
      if (line[0] == 'p'){
	std::istringstream str(line.substr(1));
	int dummy, varSize;
	str >> dummy >> varSize; 
	// initialize the entire graph 
	std::cout << "nodes size: " << varSize << std::endl;	
	continue;
      }
      
        if (line[0] == 'a')
        {
            std::istringstream str(line.substr(1));
            int fi, si;
            str >> fi >> si;
            std::cout << "fi: " << fi << "   si: " << si << '\n';
	    //      graph[--fi][--si]= 1;
	    continue;
        }
    }
    return graph;
}



void write_graph(const AdjMatrix& graph, size_t n, 
                 std::string& filename){



}


#endif
