#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include <iostream>
#include <map>
#include "basic.h"
#include "BPTree.h"

using namespace std;

typedef std::map<std::string, BPTree<int> *> intMap;
typedef std::map<std::string, BPTree<std::string> *> stringMap;
typedef std::map<std::string, BPTree<float> *> floatMap;

class indexManager
{
public:
	indexManager(string tableName);
	~indexManager();
	void createIndex(string Addr, int type);
	void insertIndex(string Addr, Data data, int val);
	void dropIndex(string Addr, int type);
	int findbyKey(string Addr, Data key);
	bool findbyRange(string Addr, Data key1, Data key2, vector<int>& vals);
	bool deleteIndexbyKey(string Addr, Data key);

private:
	map<string, BPTree<int>*> index_INT;
	map<string, BPTree<float>*> index_FLOAT;
	map<string, BPTree<string>*> index_STR;
};

#endif // !INDEX_MANAGER_H