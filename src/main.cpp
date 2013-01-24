
//#include <boost/thread/thread.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
//#include <boost/move/move.hpp>

void hello(int i)
{
  std::cout <<   "I'm a thread! "
        << i << std::endl;
}

int main(int argc, char* argv[])
{

  std::vector<std::thread> workers;

  for (int i=0; i<9; ++i){
    auto th = std::thread(&hello, i);
    workers.push_back(std::move(th));

  }

  std::cout << "Hi from main\n" ;

  std::for_each(workers.begin(),workers.end(), [](std::thread & th){th.join();});




    return 0;
}
