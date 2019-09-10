#ifndef BP_TREE_H
#define BP_TREE_H

#include "basic.h"
#include "TreeNode.h"
#include "buffer_manager.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <stdio.h>

using namespace std;

extern BufferManager buffer_manager;

//function: convert any type into string stream and insert it into file pointer
template<typename T>
void stringCopy(char* p, int& offset, T data)
{
	stringstream ss;
	ss << data;
	string s = ss.str();

	for (unsigned int i = 0; i < s.length(); i++)
		p[offset++] = s[i];
}

template <typename T>
class BPTree
{
public:
	string filename;	// the name of the file which stores the B+ tree
	TNode<T>* root;	
	TNode<T>* leafHead;		// the head of the leaf nodes
	int key_num;		// the number of keys in the tree
	int key_size;		// the size of a key
	int node_num;		// the number of nodes in the tree
	int level;			// the number of levels in the tree
	int fanout;			// the fanout of the B+ tree

	//function: constructor
	//initialize a B+ tree with its filename, size of key and fanout
	BPTree(string filename, int key_size, int fanout);

	//function: destructor
	~BPTree();

	//function: find the data by its key
	//return: the value of the data
	int searchKey(T& key);

	//function: find the data by the range 
	//only work for the key which the index is established by
	//return: a vector of values of the data
	void searchRange(T& key1,  T& key2, vector<int>& vals);

	//function: insert a key and set its responding value
	bool insertKey(T& key, int val);
	
	//function: insert new node into the tree and adjust it
	bool adjustTree(TNode<T>* node);

	//function: delete a key when a record is removed 
	bool deleteKey(T& key);

	//function: adjust tree after deletion
	bool adjustBranchNode(TNode<T>* node);

	//function: read in the data from p to end
	void readfromDisk(char *p, char *end);

	//function: write the B+ tree to the disk
	void writetoDisk();

	//function: delete all the node in the tree with root
	void deleteTree(TNode<T>* root);

	//function: print the tree for test
	void printTree();


};



template <typename T>
BPTree<T>::BPTree(string filename, int key_size, int fanout):
	filename(filename),
	key_size(key_size),
	fanout(fanout)
{
	// initialization
	root = new TNode<T>(fanout, true);

	leafHead = root;
	level = 1;
	node_num = 1;
	key_num = 0;
	string path = "./database/index/"+filename;

	// read the data from the disk
	FILE* f;
	fopen_s(&f, path.c_str(), "r");			//fopen(filename.c_str(), "r");
	if (f == NULL) {
		fopen_s(&f, path.c_str(), "w+");	//f = fopen(filename.c_str(), "w+");
		fclose(f);
		fopen_s(&f, path.c_str(), "r");		//f = fopen(filename.c_str(), "r");
	}
	fclose(f);
	
	// find the blocks where the file stores and read data
	// NOTE: the files are sequentially stored on blocks
	int block_num = 0;
	char *p = buffer_manager.getPage(path, block_num++);

	while (p[0]) {
		readfromDisk(p, p + PAGESIZE);
		p = buffer_manager.getPage(path, block_num++);
	}
	
}



template <typename T>
BPTree<T>::~BPTree()
{
	deleteTree(root);

	return;
}



template <typename T>
inline void BPTree<T>::deleteTree(TNode<T>* root)
{
	int i;
	if (!root) return;
	else if (!root->isLeaf) {
		i = 0;
		while (i <= root->num) {
			deleteTree(root->children[i]);
			root->children[i] = NULL;
		}
	}
	delete root;
	node_num--;
	return;
}



template<typename T>
inline bool BPTree<T>::insertKey(T& key, int val)
{
	int i, j;

	// if there is no root
	if (!root) {
		TNode<T>* newnode = new TNode<T>(0, true);
		if (!newnode)
			return false;
		newnode->key[0] = key;
		newnode->val[0] = val;
		newnode->isLeaf = true;
		newnode->children[0] = NULL;
		newnode->num = 1;
		newnode->degree = 0;
		newnode->parent = NULL;
		newnode->next = NULL;
		root = newnode;
		leafHead = newnode;

		node_num++;
		return true;
	}
	
	// find the leaf node to insert the key
	TNode<T>* cur_node = root;
	while (!cur_node->isLeaf) {
		for (i = 0; i < cur_node->num; i++) {
			if (key == cur_node->key[i])	//the key already exits in the index tree
				return false;
			if (key < cur_node->key[i])
				break;
		}
		cur_node = cur_node->children[i];
	}

	// determine if the key already exists in leaf node
	for (i = 0; i < cur_node->num; i++)
		if (key == cur_node->key[i])
			return false;

	// if current node isn't full, insert key into it directly
	if (cur_node->num < fanout) {
		for (i = 0; i < cur_node->num; i++)
			if (key < cur_node->key[i])
				break;

		for (j = cur_node->num; j > i; j--) {
			cur_node->key[j] = cur_node->key[j - 1];
			cur_node->val[j] = cur_node->val[j - 1];
		}
		cur_node->key[i] = key;
		cur_node->val[i] = val;
		cur_node->num++;

		key_num++;
		return true;
	}

	// if current node is full, spilt it 
	int mid = fanout - 1 / 2;
	vector<T> temp_key;
	vector<int> temp_val;

	if (!&temp_key || !&temp_val)
		return false;
	for (i = 0; i < cur_node->num; i++)
		if (cur_node->key[i] > key)
			break;
	for (j = cur_node->num; j > i; j--) {
		cur_node->key[j] = cur_node->key[j - 1];
		cur_node->val[j] = cur_node->val[j - 1];
	}

	for (j = 0; j < i; j++) {
		temp_key.push_back(cur_node->key[j]);
		temp_val.push_back(cur_node->val[j]);
	}
	
	temp_key.push_back(key);
	temp_val.push_back(val);

	TNode<T>* newnode = new TNode<T>(0, true);		// create a leaf node with none child node 
	if (!newnode)
		return false;
	// assign keys and values to the new node ( index >= mid )
	for (j = 0, i = mid; i <= fanout; i++, j++) {
		newnode->key.push_back(temp_key[i]);
		newnode->val.push_back(temp_val[i]);
		newnode->num++;
	}
	newnode->parent = cur_node->parent;
	newnode->isLeaf = cur_node->isLeaf;
	newnode->parent->degree++;

	// reset the keys and values in the cur_node ( index < mid )
	for (i = 0; i < cur_node->num; i++) {
		cur_node->key[i] = (T)0;
		cur_node->val[i] = 0;
	}
	for (i = 0; i < mid; i++) {
		cur_node->key.push_back(temp_key[i]);
		cur_node->val.push_back(temp_val[i]);
	}
	cur_node->num = mid;
	newnode->next = cur_node->next;
	cur_node->next = newnode;

	//insert the new node into current's parent node 
	if (cur_node->parent) {
		for (i = 0; i < cur_node->parent->num; i++)
			if (cur_node->parent->key[i] > newnode->key[0])
				break;
		for (j = cur_node->parent->num; j > i; j--) {
			cur_node->parent->key[j] = cur_node->parent->key[j - 1];
			cur_node->parent->children[j+1] = cur_node->parent->children[j];
		}
		cur_node->parent->children[i+1] = newnode;
		cur_node->parent->key[i] = newnode->key[0];		
		cur_node->parent->num++;
		newnode->parent = cur_node->parent;
	}
	else {
		// if current node is root
		TNode<T>* newroot = new TNode<T>(0, true);
		newroot->children[0] = cur_node;
		newroot->children[1] = newnode;
		newroot->key[0] = newnode->key[0];
		newroot->num = 1;
		newroot->degree = 2;
		newroot->parent = NULL;

		newnode->parent = newroot;
		newnode->isLeaf = true;
		cur_node->parent = newroot;
		cur_node->isLeaf = true;
		root = newroot;

		node_num++;
		return true;
	}
	
	adjustTree(cur_node->parent);
	node_num++;
	return true;
}



template <typename T>
bool BPTree<T>::adjustTree(TNode<T>* node)
{
	int i, j;

	// adjust tree after insertion
	if (node->num < fanout)
	{
		return true;
	}
	else {// if current node is full, spilt it 
		int mid = fanout - 1 / 2;

		TNode<T>* newnode = new TNode<T>(0, true);
		if (!newnode)
			return false;

		// assign keys and values to the new node ( index >= mid ) 
		// and reset the keys and values in the old node ( index >= mid )
		for (j = 0, i = mid+1; i <= fanout; i++, j++) {
			newnode->key[j] = node->key[i];
			newnode->children[j] = node->children[i];
			newnode->num++;
			node->key[i] = (T)0;
			node->children[i] = NULL;
			node->num--;
		}
		newnode->children[j] = node->children[i];	// number of child nodes = key_num + 1
		node->children[i] = NULL;

		newnode->next = node->next;
		node->next = newnode;

		//insert the new node into current's parent node 
		if (node->parent) 
		{	//if node has parent
			for (i = 0; i < node->parent->num; i++)
				if (node->parent->key[i] > newnode->key[0])
					break;
			for (j = node->parent->num; j > i; j--) {
				node->parent->key[j] = node->parent->key[j - 1];
				node->parent->children[j + 1] = node->parent->children[j];
			}
			node->parent->children[i + 1] = newnode;
			node->parent->key[i] = newnode->key[0];
			node->parent->num++;
			newnode->parent = node->parent;

			return adjustTree(node->parent);
		}
		else {
			// if current node is root
			TNode<T>* newroot = new TNode<T>(0, true);
			newroot->children[0] = node;
			newroot->children[1] = newnode;
			newroot->key[0] = newnode->key[0];
			newroot->num = 1;
			newroot->degree = 2;
			newroot->parent = NULL;

			newnode->parent = newroot;
			newnode->isLeaf = true;
			node->parent = newroot;
			node->isLeaf = true;
			root = newroot;

			return true;
		}
	}
}


template<typename T>
int BPTree<T>::searchKey(T& key)
{
	int i;

	if (!root)
		return -1;
	else {
		// find the leaf node
		TNode<T>* cur_node = root;
		while (!cur_node->isLeaf) {
			for (i = 0; i < cur_node->num; i++) {
				if (key < cur_node->key[i])	break;
			}
			cur_node = cur_node->children[i];
		}

		for (i = 0; i < cur_node->num; i++)
			if (key == cur_node->key[i])
				return cur_node->val[i];
		if (i == cur_node->num)						// the key don't exit in the tree	
			return -1;
	}
	return 0;
}



template<typename T>
void BPTree<T>::searchRange(T& key1, T& key2, vector<int>& vals)
{
	int i;

	// find the leaf node including key1
	int index = 0;
	TNode<T>* cur_node = root;

	while (!cur_node->isLeaf) {
		for (i = 0; i < cur_node->num; i++) {
			if (key1 < cur_node->key[i])	break;
		}
		cur_node = cur_node->children[i];
	}

	if (key1 <= leafHead->key[0])
		key1 = leafHead->key[0];

	for (i = 0; i < cur_node->num; i++)
		if (key1 == cur_node->key[i]) {
			vals.push_back(cur_node->val[i]);
			break;
		}
	if (i == cur_node->num)	// key1 doesn't exit in the tree	
		return ;

	/*
	// determine whether key2 exits in the tree
	if (searchKey(key2) <= 0) 
	{	// key2 doesn't exit in the tree
		cout << "ERROR! The range is mistake." << endl;
		return;
	}*/

	// search the values of keys in the leaf nodes, which are not larger than key2
	bool end = false;
	while (cur_node->next&&!end) {
		cur_node = cur_node->next;
		for (int i = 0; i < cur_node->num; i++) {
			if (cur_node->key[i] == key2) {
				vals.push_back(cur_node->val[i]);
				end = true;
				break;
			}
			else if (cur_node->key[i] > key2) {
				end = true;
				break;
			}
			else // current key < key2
				vals.push_back(cur_node->val[i]);
		}
	}

	return;
}



template<typename T>
bool BPTree<T>::deleteKey(T& key)
{
	int i, j;

	if (!root)
		return false;
	else {
		// find the leaf node
		TNode<T>* cur_node = root;
		while (!cur_node->isLeaf) {
			for (i = 0; i < cur_node->num; i++) {
				if (key < cur_node->key[i])	break;
			}
			cur_node = cur_node->children[i];
		}

		for (i = 0; i < cur_node->num; i++)
			if (key == cur_node->key[i])
				break;
		if (i == cur_node->num)						// the key don't exit in the tree	
			return false;

		// delete the key and the value in leaf node
		cur_node->key[i] = (T)0;
		cur_node->val[i] = 0;
		for (j = i; j < cur_node->num - 1; j++) {
			cur_node->key[j] = cur_node->key[j + 1];
			cur_node->val[j] = cur_node->val[j + 1];
		}
		cur_node->num--;

		int mid = (fanout - 1) / 2;
		if (cur_node->num >= mid)
			return true;

		// adjust the leaf nodes
		TNode<T>* parent = cur_node->parent;
		TNode<T>* brother = NULL;
		int index = -1;
		int bro_index = -1;

		// if it's a root
		if (!parent) {
			
			if (cur_node->num <= 0) {
				root = NULL;
				delete cur_node;
				leafHead = NULL;
			}
			return true;
		}

		// select the neighbor node which we will borrow a node from
		for (index = 0; index <= parent->num; index++)
			if (cur_node == parent->children[index])
				break;

		if (index == 0) {
			brother = parent->children[1];
			bro_index = 1;
		}
		else {
			brother = parent->children[index - 1];
			bro_index = index - 1;
		}

		if (brother->num > mid)
		{	
			// if its brother has spare nodes
			if (bro_index < index)
			{
				// move the last node in the brother node to the current node
				parent->key[bro_index] = brother->key[brother->num - 1];
				for (i = cur_node->num - 1; i >= 0; i--) {
					cur_node->key[i + 1] = cur_node->key[i];
					cur_node->val[i + 1] = cur_node->val[i];
				}
				cur_node->key[0] = brother->key[brother->num - 1];
				cur_node->val[0] = brother->val[brother->num - 1];
				cur_node->num++;
				brother->num--;
			}
			else {
				// move the first node in the brother node to the current node
				parent->key[index] = brother->key[1];
				cur_node->key[cur_node->num] = brother->key[0];
				cur_node->val[cur_node->num] = brother->val[0];
				cur_node->num++;
				for (i = 1; i <= brother->num - 1; i++) {
					brother->key[i - 1] = brother->key[i];
					brother->val[i - 1] = brother->val[i];
				}
				brother->num--;
			}
			return true;
		}
		else
		{
			// if brother node doesn't have enough spare nodes,
			// merge brother node and current node into one node
			if (bro_index < index)
			{
				// move keys and values in the current node to brother node
				for (i = 0; i < cur_node->num; i++) {
					brother->key[brother->num] = cur_node->key[i];
					brother->val[brother->num] = cur_node->val[i];
					brother->num++;
				}

				// adjust parent node
				for (i = index; i < parent->num; i++) {
					parent->key[i - 1] = parent->key[i];
					parent->children[i] = parent->children[i + 1];
				}
				parent->num--;

				if (cur_node->next)
					brother->next = cur_node->next;

				delete cur_node;
			}
			else 
			{
				// move keys and values in the brother node to current node
				for (i = 0; i < brother->num; i++) {
					cur_node->key[cur_node->num] = brother->key[i];
					cur_node->val[cur_node->num] = brother->val[i];
					cur_node->num++;
				}

				// adjust parent node
				for (i = bro_index; i < parent->num; i++) {
					parent->key[i - 1] = parent->key[i];
					parent->children[i] = parent->children[i + 1];
				}
				parent->num--;

				if (brother->next)
					cur_node->next = brother->next;

				delete brother;
			}
			
			adjustBranchNode(parent);

		}
		node_num--;
	}
}

template<typename T>
bool BPTree<T>::adjustBranchNode(TNode<T>* node)
{
	int i;

	int min_num = fanout / 2 - 1;		// the lowest number of leaf nodes

	while (node) {
		if (node->num >= min_num)
			break;

		TNode<T>* parent = node->parent;
		TNode<T>* brother = NULL;
		int bro_index = -1;
		int index = -1;

		// if it's a root
		if (!parent) {
			if (node->num <= 0) {
				node->children[0]->parent = NULL;
				root = node->children[0];
				delete node;
			}
			break;
		}

		// if it isn't a root
		// select the neighbor node which we will borrow a node from
		for (index = 0; index <= parent->num; index++)
			if (node == parent->children[index])
				break;
		if (index == 0) {
			// if current node is the first child node of its parent
			brother = parent->children[1];
			bro_index = 0;
		}
		else {
			brother = parent->children[index - 1];
			bro_index = index- 1;
		}

		if (brother->num > min_num)
		{	
			// if brother node has spare nodes
			if (bro_index < index)
			{	
				// borrow a node from left
				T temp = parent->key[bro_index];
				parent->key[bro_index] = brother->key[brother->num - 1];
				for (i = node->num - 1; i >= 0; i--) {
					node->key[i + 1] = node->key[i];
					node->children[i + 2] = node->children[i+1];
				}
				node->children[i + 2] = node->children[i + 1];
				node->key[0] = temp;
				if (brother->children[brother->num])
					brother->children[brother->num]->parent = node;
				node->children[0] = brother->children[brother->num];
				node->num++;
				brother->num--;
			}
			else {
				// move the first node in the brother node to the current node
				T temp = parent->key[index];
				parent->key[index] = brother->key[0];
				node->key[node->num] = temp;
				if (brother->children[0])
					brother->children[0]->parent = node;
				node->children[++node->num] = brother->children[0];
				
				for (i = 1; i < brother->num; i++) {
					brother->key[i - 1] = brother->key[i];
					brother->children[i-1] = brother->children[i];
				}
				brother->children[i-1] = brother->children[i];

				brother->num--;
			}
			return true;
		}
		else {
			// merge brother node and current node 
			if (bro_index < index)
			{
				// merge to the left side i.e. brother node
				brother->key[brother->num] = parent->key[bro_index];
				brother->num++;

				for (i = 0; i < node->num; i++)
				{
					brother->key[brother->num] = node->key[i];
					brother->children[brother->num] = node->children[i];
					TNode<T>* p = brother->children[brother->num];
					if (p)
						p->parent = brother;
					brother->num++;
				}
				brother->children[brother->num] = node->children[i];

				if (brother->children[brother->num])
					brother->children[brother->num]->parent = brother;

				for (i = bro_index + 1; i < parent->num; i++)
						parent->key[i - 1] = parent->key[i];
				for (i = index + 1; i <= parent->num; i++)
					parent->children[i - 1] = parent->children[i];

				parent->num--;
				delete node;
			}
			else //merge to the right side i.e. current node
			{
				node->key[node->num++] = parent->key[index];
				
				for (i = 0; i < brother->num; i++) {
					node->key[node->num] = brother->key[i];
					node->children[node->num] = brother->children[i];
					TNode<T>* p = node->children[node->num];
					if (p)
						p->parent = node;
					node->num++;
				}
				node->children[node->num] = brother->children[i];
				if (node->children[node->num])
					node->children[node->num]->parent = node;

				for (i = index + 1; i < parent->num; i++) 
					parent->key[i - 1] = parent->key[i];
				for (i = bro_index + 1; i <= parent->num; i++)
					parent->children[i - 1] = parent->children[i];
				
				parent->num--;

				delete brother;

			}
			node = parent;
		}
	}
	
	return true;
}



template<typename T>
inline void BPTree<T>::readfromDisk(char* p, char* end)
{
	T key;
	int val;
	int i;

	// read the key and value from disk 
	//*–Ë“™”Î record manager   ≈‰
	for (i = 0; i < PAGESIZE; i++)
	{
		if (p[i] != '#')
			return;
		else {
			i = i + 2;
			char temp[100];
			int j = 0;
			while (i < PAGESIZE && p[i] != ' ') 
				temp[j++] = p[i++];
			temp[j] = 0;
			string s_key(temp);
			stringstream stream_key(s_key);
			stream_key>>key;

			memset(temp, 0, sizeof(temp));	// initialize tmp

			i++;
			j = 0;
			while (i < PAGESIZE && p[i] != ' ')
				temp[j++] = p[i++];
			temp[j] = 0;
			string s_val(temp);
			stringstream stream_val(s_val);
			stream_val>>val;
			
			insertKey(key, val);
			key_num++;
		}
	}
}



template<typename T>
void BPTree<T>::writetoDisk()
{
	// read the data from the disk
	FILE* f;
	string path = "./database/index/" + filename;
	fopen_s(&f, path.c_str(), "r");
	if (f == NULL) {
		fopen_s(&f, path.c_str(), "w+");
		fclose(f);
		fopen_s(&f, path.c_str(), "r");
	}
	fclose(f);
	
	// get the number of blocks
	char* p;
	int block_num = -1;
	do {
		p = buffer_manager.getPage(path, block_num + 1);
		block_num++;
	} while (p[0]);

	// 
	TNode<T>* cur = leafHead;
	int j = 0, i = 0;
	int offset;

	while (cur) {
		p = buffer_manager.getPage(path, block_num);
		offset = 0;

		// initialize as empty
		memset(p, 0, PAGESIZE);

		for (i = 0; i < cur->num; i++) {
			p[offset++] = '#';
			p[offset++] = ' ';

			stringCopy(p, offset, cur->key[i]);
			p[offset++] = ' ';
			stringCopy(p, offset, cur->val[i]);
			p[offset++] = ' ';
		}

		p[offset] = '\0';

		// get the id of the page
		// set the page as dirty by id

		cur = cur->next;
		j++;
		int page_id = buffer_manager.getPageId(path, block_num);
		buffer_manager.modifyPage(page_id);
	}

	// set other page as empty too
	//char* p = buffer_manager.getPage(path, block_num);
	/*int page_id = buffer_manager.getPageId(path, block_num);
	buffer_manager.modifyPage(page_id);*/
}



template<typename T>
void BPTree<T>::printTree()
{
	TNode<T>* p = leafHead;
	while (p) {
		cout << "----------"<<filename << "----------" << endl;
		for (int i = 0; i < p->num; i++){
			cout << "key:" << p->key[i] << " val:" << p->val[i] << endl;
			}
		p = p->next;
	}
}



#endif // ! BP_TREE_H