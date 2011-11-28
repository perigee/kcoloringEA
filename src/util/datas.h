#ifndef MY_DATAS_H
#define MY_DATAS_H

#include <stdio.h>
#include <stdlib.h>


typedef int* ElementType ;
typedef int KeyType ;

struct node ;
typedef struct node Node;

Node* createNode(KeyType key, ElementType e) ;

// binary tree
Node* insertTree(Node* root,Node* node) ;
void deleteTree(Node* root) ;

// double-linked list
Node* insertList(Node* root, Node* node);
void deleteList(Node* root) ;

#endif
