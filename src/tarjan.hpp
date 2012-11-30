#include <vector>
#include <memory>


/*!
 * Node class for algorithm Tarjan
 */
struct Node{
  
  Node():_id(-1), _index(-1),
	 _llink(-1),_isStack(false),
	 _group(-1){}

  int _id;
  int _index;
  int _llink;
  bool _isStack;
  int _group;
};

typedef std::shared_ptr<Node> NodePtr;

std::vector<NodePtr> NodeVec; // store the nodes
std::vector<NodePtr> STACK; // stack used in tarjan


/*!
 * Algorithm Tarjan to find strongly connected components
 */
void tarjan(NodePtr x){
  
  x->_index = INDEX;
  x->_llink = INDEX;
  STACK.push_back(x); // put into STACK
  x->_isStack = true; // put into STACK


  ++INDEX;

  for (std::vector<NodePtr>::iterator i = NodeVec.begin(); 
       i != NodeVec.end(); ++i){

    NodePtr node = *i;
    
    if (!graph[x->_id][node->_id]) continue;
   
    if (node->_index <0){
      tarjan(node);
      x->_llink = std::min(x->_llink,node->_llink);
    }else if(node->_isStack){
      x->_llink = std::min(x->_llink,node->_index);
    }   
  
  }
    
  if (x->_index == x->_llink){
    while(true){
      NodePtr node = STACK.back();
      STACK.pop_back();
      node->_isStack = false;
      node->_group = GROUP;
      if (node->_id == x->_id){
	++GROUP;
	break;
      }
    }
  }

}

