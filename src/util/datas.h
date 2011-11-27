#ifndef MY_DATAS_H
#define MY_DATAS_H

#include <stdio.h>
#include <stdlib.h>


typedef int* ElementType ;
typedef int KeyType ;

struct node ;
typedef struct node TreeNode;

TreeNode* createNode(KeyType key, ElementType e) ;
TreeNode* insertNode(TreeNode *root,TreeNode *node) ;
void deleteTree(TreeNode* root) ;

#endif
