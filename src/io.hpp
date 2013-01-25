#ifndef IO_HPP
#define IO_HPP


#include "graph.hpp"

#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>






AdjMatrix read_graph(const std::string& filepath)
{
    AdjMatrix graph;


    // read nodes number


    create_graph(graph, nbNodes);

    std::ifstream fp(filepath.c_str());
    for (std::string line; getline(fp, line); )
    {
        if (line.size() > 1 && line[0] == 'a')
        {
            std::istringstream str(line.substr(1));
            int fi, si;
            str >> fi >> si;
            std::cout << "fi: " << fi << "   si: " << si << '\n';
            graph[--fi][--si]= 1;
        }
    }
    return graph;
}



void write_graph(const AdjMatrix& graph, size_t n, 
                 std::string& filename){



}


#endif
