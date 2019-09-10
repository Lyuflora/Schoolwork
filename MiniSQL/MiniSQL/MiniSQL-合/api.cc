#include "api.h"
#include "template_function.h"
#include <algorithm>
#include <vector>
#include <iterator>

API::API(){}

API::~API(){}

Table API::selectRecord(std::string table_name, std::vector<std::string> target_attr, std::vector<Where> where, char operation)
{
	if (target_attr.size() == 0) {
		return record.selectRecord(table_name);
	} else if (target_attr.size() == 1) {
        return record.selectRecord(table_name, target_attr[0], where[0]);
    } else {
		Table table1 = record.selectRecord(table_name, target_attr[0], where[0]);
		Table table2 = record.selectRecord(table_name, target_attr[1], where[1]);

		if (operation)
			return joinTable(table1, table2, target_attr[0], where[0]);
		else
			return unionTable(table1, table2, target_attr[0], where[0]);
	}
}

int API::deleteRecord(std::string table_name , std::string target_attr , Where where)
{
    int result;
	if (target_attr == "")
		result = record.deleteRecord(table_name);
	else
		result = record.deleteRecord(table_name, target_attr, where);
	return result;
}

void API::insertRecord(std::string table_name , Tuple& tuple)
{
	record.insertRecord(table_name, tuple);
	return;
}

bool API::createTable(std::string table_name, Attribute attribute, int primary, Index index)
{
	record.createTableFile(table_name);
	catalog.createTable(table_name, attribute, primary, index);

	return true;
}

bool API::dropTable(std::string table_name)
{
	record.dropTableFile(table_name);
	catalog.dropTable(table_name);

	return true;
}

bool API::createIndex(std::string table_name, std::string index_name, std::string attr_name)
{
    indexManager index(table_name);
    
	std::string file_path = "INDEX_FILE_" + attr_name + "_" + table_name;
	int type;

	catalog.createIndex(table_name, attr_name, index_name);
	Attribute attr = catalog.getAttribute(table_name);
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == attr_name) {
			type = (int)attr.type[i];
			break;
		}
	}
	index.createIndex(file_path, type);
	record.createIndex(index , table_name, attr_name);

	return true;
}

bool API::dropIndex(std::string table_name, std::string index_name)
{
    indexManager index(table_name);
    
	std::string attr_name = catalog.IndextoAttr(table_name, index_name);
	std::string file_path = "INDEX_FILE_" + attr_name + "_" + table_name;
	int type;

	Attribute attr = catalog.getAttribute(table_name);
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == attr_name) {
			type = (int)attr.type[i];
			break;
		}
	}
	index.dropIndex(file_path, type);
	catalog.dropIndex(table_name, index_name);
	
	file_path = "./database/index/" + file_path;
    remove(file_path.c_str());
	return true;
}

Table API::unionTable(Table &table1, Table &table2, std::string target_attr, Where where)
{
	Table result_table(table1);
    std::vector<Tuple>& result_tuple = result_table.getTuple();
	std::vector<Tuple> tuple1 = table1.getTuple();
	std::vector<Tuple> tuple2 = table2.getTuple();
    result_tuple = tuple1;


    int i;
    Attribute attr = table1.getAttr();
    for (i = 0; i < 32; i++)
        if (attr.name[i] == target_attr)
            break;
    
    for (int j = 0; j < tuple2.size(); j++)
        if (!isSatisfied(tuple2[j], i, where))
            result_tuple.push_back(tuple2[j]);
    
    std::sort(result_tuple.begin(), result_tuple.end(), sortcmp);
    return result_table;
	
}

Table API::joinTable(Table &table1, Table &table2, std::string target_attr, Where where)
{
	Table result_table(table1);
    std::vector<Tuple>& result_tuple = result_table.getTuple();
	std::vector<Tuple> tuple1 = table1.getTuple();
	std::vector<Tuple> tuple2 = table2.getTuple();
	
	int i;
    Attribute attr = table1.getAttr();
    for (i = 0; i < 32; i++)
        if (attr.name[i] == target_attr)
            break;
    
	result_tuple.clear();
	for (int j = 0; j < tuple2.size(); j++) {
		if (isSatisfied(tuple2[j], i, where)) {
			result_tuple.push_back(tuple2[j]);
		}			
	}
    
	std::sort(result_tuple.begin(), result_tuple.end(), sortcmp);
    return result_table;

}

bool sortcmp(const Tuple &tuple1, const Tuple &tuple2)
{
	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

    switch (data1[0].type) {
        case -1: return data1[0].datai < data2[0].datai;
        case 0: return data1[0].dataf < data2[0].dataf;
        default: return data1[0].datas < data2[0].datas;
    }
}


bool calcmp(const Tuple &tuple1, const Tuple &tuple2)
{
	int i;

	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

    for (i = 0; i < data1.size(); i++) {
        bool flag = false;
        switch (data1[0].type) {
            case -1: {
                if (data1[0].datai != data2[0].datai)
                    flag = true;
            }break;
            case 0: {
                if (data1[0].dataf != data2[0].dataf)
                    flag = true;
            }break;
            default: {
                if (data1[0].datas != data2[0].datas)
                    flag = true;
            }break;
        }
		if (flag)
			break;
    }


	if (i == data1.size())
		return true;
	else
		return false;
}

bool isSatisfied(Tuple& tuple, int target_attr, Where where)
{
    std::vector<Data> data = tuple.getData();
    
    switch (where.relation_character) {
        case LESS : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai < where.data.datai); break;
                case 0 : return (data[target_attr].dataf < where.data.dataf); break;
                default: return (data[target_attr].datas < where.data.datas); break;
            }
        } break;
        case LESS_OR_EQUAL : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai <= where.data.datai); break;
                case 0 : return (data[target_attr].dataf <= where.data.dataf); break;
                default: return (data[target_attr].datas <= where.data.datas); break;
            }
        } break;
        case EQUAL : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai == where.data.datai); break;
                case 0 : return (data[target_attr].dataf == where.data.dataf); break;
                default: return (data[target_attr].datas == where.data.datas); break;
            }
        } break;
        case GREATER_OR_EQUAL : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai >= where.data.datai); break;
                case 0 : return (data[target_attr].dataf >= where.data.dataf); break;
                default: return (data[target_attr].datas >= where.data.datas); break;
            }
        } break;
        case GREATER : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai > where.data.datai); break;
                case 0 : return (data[target_attr].dataf > where.data.dataf); break;
                default: return (data[target_attr].datas > where.data.datas); break;
            }
        } break;
        case NOT_EQUAL : {
            switch (where.data.type) {
                case -1 : return (data[target_attr].datai != where.data.datai); break;
                case 0 : return (data[target_attr].dataf != where.data.dataf); break;
                default: return (data[target_attr].datas != where.data.datas); break;
            }
        } break;
        default:break;
    }
            
    return false;
}
