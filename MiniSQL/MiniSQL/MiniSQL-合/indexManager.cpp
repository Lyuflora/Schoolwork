#include "basic.h"
#include "indexManager.h"
#include "catalog_manager.h"

#include "BPTree.h"
#include <map>

using namespace std;

//enum TREE_TYPE{TYPE_INT, TYPE_FLOAT, TYPE_STR};
#define TYPE_INT -1
#define TYPE_FLOAT 0

indexManager::indexManager(string tableName)
{
	CatalogManager catalog;//to get the attributes of the table by functions of class catalog  
	Attribute attributes = catalog.getAttribute(tableName);
	int i;

	for (i = 0; i < attributes.num; i++)
	{
		if (attributes.has_index[i]) 
			createIndex("INDEX_FILE_" + attributes.name[i] + "_" +tableName, attributes.type[i]); 
	}
	
	//createIndex("index_of_" + tableName + "_on_" + "key", 1); // for test
}

indexManager::~indexManager()	// include result's output
{
	//write data to the disk
	for (intMap::iterator it = index_INT.begin(); it != index_INT.end(); it++)
	{
		if (it->second) {
			it->second->printTree();	// for test
			it->second->writetoDisk();
			delete it->second;
		}
	}
	for (floatMap::iterator it = index_FLOAT.begin(); it != index_FLOAT.end(); it++)
	{
		if (it->second) {
			it->second->printTree();	// for test
			it->second->writetoDisk();
			delete it->second;
		}
	}
	for (stringMap::iterator it = index_STR.begin(); it != index_STR.end(); it++)
	{
		if (it->second) {
			it->second->printTree();	// for test
			it->second->writetoDisk();
			delete it->second;
		}
	}
}

void indexManager::createIndex(string Addr, int type)
{
	int keysize;	// the size of a key
	int fanout;		// the maximum number of children in a parent node

	switch (type) {
		case TYPE_INT: {
			keysize = sizeof(int);
			fanout = (PAGESIZE - sizeof(int)) / (keysize + sizeof(int));
			if (fanout % 2 == 0) fanout = fanout - 1;	// let fan out of a B+ tree be an odd number

			//cout << "key size = " << keysize << "; fanout = " << fanout << "; filename = " << Addr << endl;	// for test

			BPTree<int> *T = new BPTree<int>(Addr, keysize, fanout);
			index_INT[Addr] = T;
			break;
		}
		case TYPE_FLOAT: {
			keysize = sizeof(float);
			fanout = (PAGESIZE - sizeof(int)) / (keysize + sizeof(int));
			if (fanout % 2 == 0) fanout = fanout - 1;

			//cout << "key size = " << keysize << "; fanout = " << fanout << "; filename = " << Addr << endl;	// for test

			BPTree<float> *T = new BPTree<float>(Addr, keysize, fanout);
			index_FLOAT[Addr] = T;
			break;
		}
		default: {	//TYPE_STR
			keysize = sizeof(string);
			fanout = (PAGESIZE - sizeof(int)) / (keysize + sizeof(int));
			if (fanout % 2 == 0) fanout = fanout - 1;

			//cout << "key size = " << keysize << "; fanout = " << fanout << "; filename = " << Addr << endl;	// for test

			BPTree<string> *T = new BPTree<string>(Addr, keysize, fanout);
			index_STR[Addr] = T;
			break;
		}
	}
	return;

}

void indexManager::dropIndex(string Addr, int type)
{
	switch(type) {
		case TYPE_INT: {
			intMap::iterator it = index_INT.find(Addr);
			if (it == index_INT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;	
				throw(index_not_exist());
				return;
			}
			else {
				delete it->second;
				index_INT.erase(it);
			}
			break; 
		}
		case TYPE_FLOAT: {
			floatMap::iterator it = index_FLOAT.find(Addr);
			if (it == index_FLOAT.end()){
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return;
			}
			else {
				delete it->second;
				index_FLOAT.erase(it);
			}
			break; 
		}
		default: {	//TYPE_STR
			stringMap::iterator it = index_STR.find(Addr);
			if (it == index_STR.end()){
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return;
			}
			else {
				delete it->second;
				index_STR.erase(it);
			}
			break; 
		}
	}
}

void indexManager::insertIndex(string Addr, Data data, int val)
{
	switch (data.type) {
		case TYPE_INT: {
			intMap::iterator it = index_INT.find(Addr);
			if (it == index_INT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return;
			}
			else 
				it->second->insertKey(data.datai, val);
			break; 
		}
		case TYPE_FLOAT: {
			floatMap::iterator it = index_FLOAT.find(Addr);
			if (it == index_FLOAT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return;
			}
			else 
				it->second->insertKey(data.dataf, val);
			break; 
		}
		default: {	//TYPE_STR
			stringMap::iterator it = index_STR.find(Addr);
			if (it == index_STR.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return;
			}
			else 
				it->second->insertKey(data.datas, val);
			break; 
		}
	}
}

int indexManager::findbyKey(string Addr, Data key)
{
	switch (key.type) {
		case TYPE_INT: {
			intMap::iterator it = index_INT.find(Addr);
			if (it == index_INT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return 0;
			}
			else
				return it->second->searchKey(key.datai);
			break;
		}
		case TYPE_FLOAT: {
			floatMap::iterator it = index_FLOAT.find(Addr);
			if (it == index_FLOAT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return 0;
			}
			else
				return it->second->searchKey(key.dataf);
			break;
		}
		default: {	//TYPE_STR
			stringMap::iterator it = index_STR.find(Addr);
			if (it == index_STR.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return 0;
			}
			else
				return it->second->searchKey(key.datas);
			break;
		}
	}
	return 0;
}

bool indexManager::findbyRange(string Addr, Data key1, Data key2, vector<int>& vals)
{
	switch (key1.type) {
		case TYPE_INT: {
			intMap::iterator it = index_INT.find(Addr);
			if (it == index_INT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				if (key1.datai == key2.datai)	it->second->searchKey(key1.datai);
				else it->second->searchRange(key1.datai, key2.datai, vals);
			break;
		}
		case TYPE_FLOAT: {
			floatMap::iterator it = index_FLOAT.find(Addr);
			if (it == index_FLOAT.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				if (key1.dataf == key2.dataf)	it->second->searchKey(key1.dataf);
				else it->second->searchRange(key1.dataf, key2.dataf, vals);
			break;
		}
		default: {	//TYPE_STR
			stringMap::iterator it = index_STR.find(Addr);
			if (it == index_STR.end()) {
				//cout << "ERROR: no such index file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				if (key1.datas == key2.datas) it->second->searchKey(key1.datas);
				else it->second->searchRange(key1.datas, key2.datas, vals);
			break;
		}
	}
	return true;
}

bool indexManager::deleteIndexbyKey(string Addr, Data key)
{
	switch (key.type) {
		case TYPE_INT: {
			intMap::iterator it = index_INT.find(Addr);
			if (it == index_INT.end()) {
				//cout << "ERROR: no such file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				it->second->deleteKey(key.datai);
			break;
		}
		case TYPE_FLOAT: {
			floatMap::iterator it = index_FLOAT.find(Addr);
			if (it == index_FLOAT.end()) {
				//cout << "ERROR: no such file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				it->second->deleteKey(key.dataf);
			break;
		}
		default: {	//TYPE_STR
			stringMap::iterator it = index_STR.find(Addr);
			if (it == index_STR.end()) {
				//cout << "ERROR: no such file exits!" << endl;
				throw(index_not_exist());
				return false;
			}
			else
				it->second->deleteKey(key.datas);
			break;
		}
	}
	return true;
}


