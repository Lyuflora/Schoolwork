#ifndef _API_H_
#define _API_H_ 

#include "basic.h"
#include "record_manager.h"
#include "buffer_manager.h"

class API {
public:
	API();
	~API();
	Table selectRecord(std::string table_name, std::vector<std::string> target_attr, std::vector<Where> where, char operation);
	int deleteRecord(std::string table_name, std::string target_attr , Where where);
	void insertRecord(std::string table_name, Tuple& tuple);
	bool createTable(std::string table_name, Attribute attribute, int primary, Index index);
	bool dropTable(std::string table_name);
	bool createIndex(std::string table_name, std::string index_name, std::string attr_name);
	bool dropIndex(std::string table_name, std::string index_name);
private:
    Table unionTable(Table &table1, Table &table2, std::string target_attr, Where where);
	Table joinTable(Table &table1, Table &table2, std::string target_attr, Where where);

private:
	RecordManager record;
	CatalogManager catalog;
};

bool sortcmp(const Tuple &tuple1, const Tuple &tuple2);
bool calcmp(const Tuple &tuple1, const Tuple &tuple2);
bool isSatisfied(Tuple& tuple, int target_attr, Where where);
#endif
