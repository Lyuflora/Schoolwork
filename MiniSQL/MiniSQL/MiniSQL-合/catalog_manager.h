#ifndef _CATALOG_MANAGER_H_
#define _CATALOG_MANAGER_H_ 1

#include <iostream>
#include <cstring>
#include "const.h"
#include <math.h>
#include <iomanip>
#include "basic.h"
#include "exception.h"

using namespace std;
#ifndef TABLE_MANAGER_PATH
#define TABLE_MANAGER_PATH "./database/catalog/catalog_file"    //?
#endif

#include "buffer_manager.h"
#include "basic.h"
extern class BufferManager buffer_manager;

class CatalogManager {
public:
	// 接口:
	// 创建表 creatTable
	// 输入：表名，属性信息，主键序号，索引信息
	// 功能：创建表和其他相关的定义
	// 异常：如果指定名字的表已经存在，抛出table_exist()
	void createTable(string table_name, Attribute attribute, int primary, Index index);

	// 删除表 dropTable
	// 输入：表名
	// 功能：删除表
	// 异常：如果不存在该名字的表，抛出 table_not_exit()
	void dropTable(string table_name);

	// 判断表是否已经存在 hasTable
	// 输入：表名
	// 输出：已经存在->true; 否则->false
	// 功能：在执行创建/删除表、插入/删除记录、创建/删除索引等命令前检查是否存在相关的表以及是否要抛出table_not_exit()
	bool hasTable(string table_name);

	// 判断指定表的属性是否已存在 hasAttribute
	// 输入：表名，属性名
	// 输出：已经存在->true; 否则->false
	// 异常：如果不存在该名字的表，抛出 table_not_exit()
	bool hasAttribute(string table_name, string attr_name);

	// 获取表的属性信息 getAttribute
	// 输入：表名
	// 输出：属性信息
	// 异常：如果不存在该名字的表，抛出 table_not_exit()
	Attribute getAttribute(string table_name);

	// 创建索引语句 createIndex
	// 输入：表名，索引对应的属性名，索引名
	// 功能：在一个属性上建立索引，通过重新建表更新索引的信息
	// 异常：1.如果不存在该名字的表，抛出 table_not_exit()
	//      2.如果不存在指定的属性，抛出throw attribute_not_exit()
	//      3.如果索引数目超出上限，抛出throw index_full()
	//      4.如果索引名重复，抛出index_exist()
	//      5.如果指定的属性已有索引，抛出 index_exist()
	void createIndex(string table_name, string arrt_name, string index_name);

	// 找到索引对应的属性 IndextoAttr
	// 输入：表名，索引名
	// 输出：属性名
	// 功能：调用getAttribute获得属性名
	// 异常：1.如果不存在该名字的表，抛出 table_not_exit()
	//      2.如果不存在该名字的索引，抛出index_not_exist()
	string IndextoAttr(string table_name, string index_name);

	// 删除索引 dropIndex
	// 输入：表名，索引名
	// 功能：把索引的最后一个调到被删除的索引位置，然后删除最后一个索引记录
	// 异常：1.如果不存在该名字的表，抛出 table_not_exit()
	//      2.如果不存在该名字的索引，抛出index_not_exist()
	void dropIndex(string table_name, string index_name);

	// 打印表
	// 输入：表名
	// 输出：cout属性、主键、索引
	void showTable(string table_name);

private:
	// 类型转换 
	// 数字转为字符串，从右往左数某几位
	string num2str(int num, short bit);
	// 字符串转数字
	int str2num(string str);

	// 得到该行表的名字
	// （读到的字符串）
	string getTableName(string buffer, int start, int &rear);

	//返回该表的位置，引用传出该表的所在的块的位置，返回在块中的位置，如果未找到，则返回-1
	// （遍历所有的块，buffer manager指出页的头地址，读取字符确定表start的位置）
	int getTablePlace(string name, int &suitable_block);

	// 返回该表的index
	// （从buffer manager得到表的位置和对应的块，找到索引开始的位置，
	// 从记录中读取索引的信息：数量、对应的属性序号、索引名，读完一个内容跳跃几个字节读取下一个）
	Index getIndex(string table_name);

	// 文件大小
	int getBlockNum(string table_name);

};

#endif