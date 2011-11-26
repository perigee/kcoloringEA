#ifndef MY_DATAS_H
#define MY_DATAS_H

#include <stdio.h>



struct TreeNode;
TreeNode* createNode(KeyType key, ElementType e);
TreeNode* insertNode(TreeNode* root,TreeNode* node);
TreeNode* insertNode(TreeNode* root, KeyType key, ElementType e);
void deleteTree(TreeNode* root);

#endif
