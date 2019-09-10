#pragma once
#ifndef TREE_NODE_H
#define TREE_NODE_H
#include <iostream>
#include <vector>
#include "basic.h"
#include <sstream>

using namespace std;

//template class Tnode, which is the tree node for the B+ tree
//T can be integer, float or string.
template <typename T>
class TNode {
public:
	int degree;				// the degree of the node, <=fanout
	int num;				// the number of keys in one node, <= fanout-1
	bool isLeaf;			// the sign for leaf nodes
	TNode* parent;			// pointer which points to the parent node
	TNode* next;			// pointer which points to the next node 

	vector<T> key;			// the keys in the node
	vector<int> val;		// the values of the leaf nodes i.e. the address of the data
	vector<TNode*> children;//the pointer of its children 
	
	//function: constructor
	TNode(int degree, bool isLeaf){
		this->degree = degree;
		this->isLeaf = isLeaf;
		key.resize(100);
		val.resize(100);
		children.resize(130);
		parent = NULL;
		next = NULL;
		num = 0;
	};
	//function: destructor
	~TNode() {};
	//function: judge whether it is a root node
	bool isRoot() {
		if (parent == NULL) return true;
		else return false;
	}
};

#endif // 
