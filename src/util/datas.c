#include "datas.h"


///========================================
/// node structure for Binary Tree
///========================================



struct node{
  KeyType key;  // the comparing key of node
  ElementType element; 
  struct node* left;
  struct node* right;
};


/*!
 * Create tree node
 */
TreeNode* createNode(KeyType key, ElementType e){
  TreeNode* tempNode;
  tempNode = malloc(sizeof(TreeNode));
  tempNode->key = key;
  tempNode->element = e;
  tempNode->left = NULL;
  tempNode->right = NULL;
  return tempNode;
}



/*!
 * Insert the node in the tree by order
 * @return the root node
 */
TreeNode* insertNode(TreeNode* root,TreeNode* node){
  
  if(root == NULL)
    return node;
  
  if (root->key >= node->key) 
    node->left = insertNode(root->left, node); 
  else 
    node->right = insertNode(root->right, node);

  return root;
}


/*!
 * Traveral the tree
 */



/*!
 * Delete entire tree
 */
void deleteTree(TreeNode* root){
  if(root->left != NULL){
    deleteTree(root->left);
  }

  if(root->right != NULL){
    deleteTree(root->right);
  }
  
  free(root);
  root = NULL;
}






