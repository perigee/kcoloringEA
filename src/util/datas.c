#include "datas.h"


///========================================
/// node structure for Binary Tree
///========================================



struct node{
  KeyType key;  // the comparing key of node
  ElementType element; 
  struct node* smaller;
  struct node* bigger;
};


/*!
 * Create tree node or double-linked node
 */
Node* createNode(KeyType key, ElementType e){
  Node* tempNode;
  tempNode = malloc(sizeof(Node));
  tempNode->key = key;
  tempNode->element = e;
  tempNode->smaller = NULL;
  tempNode->bigger = NULL;
  return tempNode;
}



/*!
 * Insert the node in the tree by order
 * @return the root node
 */
Node* insertTree(Node* root,Node* node){
  
  if(root == NULL)
    return node;
  
  if (root->key >= node->key) 
    node->smaller = insertTree(root->smaller, node); 
  else 
    node->bigger = insertTree(root->bigger, node);

  return root;
}


/*!
 * Traveral the tree
 */



/*!
 * Delete entire tree
 */
void deleteTree(Node* root){
  if(root->bigger != NULL){
    deleteTree(root->bigger);
  }

  if(root->smaller != NULL){
    deleteTree(root->smaller);
  }
  
  free(root);
  root = NULL;
}






/*!
 * join a and b, insert big before small if big is bigger than small
 * @return the big
 */ 
static Node* insertBefore(Node* big, Node* small) {
  big->bigger = small->bigger;
  big->smaller =  small;
  small->bigger = big;
  return big;
}


/*!
 * Insert node in the double-linked list
 */
Node* insertList(Node* root, Node* node){
  if(root == NULL)
    return node;

  if(root->key >= node->key){
    root->smaller = insertList(root->smaller,node);
  }else{
    return insertBefore(node, root);
  }

  return root;
}

/*!
 * delete double-linked list
 */
void deleteList(Node* root){

  if(root->smaller != NULL)
    deleteList(root->smaller);

  free(root);
  root = NULL;
}









