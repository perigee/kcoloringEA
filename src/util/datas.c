
#include "datas.h"


///========================================
/// node structure for Binary Tree
///========================================


typedef int* ElementType;
typedef int KeyType;

typedef struct TreeNode{
  KeyType key;  // the comparing key of node
  ElementType* element; 
  struct TreeNode* left;
  struct TreeNode* right;
} TreeNode;


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
  
  if (root->key => node->key) 
    node->left = insertNode(root->left, node); 
  else 
    node->right = insertNode(root->right, node);

  return root;
}

TreeNode* insertNode(TreeNode* root, KeyType key, ElementType e){
  TreeNode* tmpNode = malloc(sizeof(TreeNode));
  tmpNode->key = key;
  tmpNode->element = e;
  return insertNode(root, tmpNode);
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






