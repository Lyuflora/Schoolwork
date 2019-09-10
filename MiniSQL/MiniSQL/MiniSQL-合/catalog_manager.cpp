#include "catalog_manager.h"
using namespace std;
extern BufferManager buffer_manager;

void CatalogManager::createTable(string table_name, Attribute attribute, int primary, Index index) {

	if (hasTable(table_name)) {
		throw table_exist();
	}
	// 读入表名、属性数量
	string str_tmp = "";
	str_tmp += "0000 ";
	str_tmp = str_tmp + table_name + " ";
	int attr_num = attribute.num;
	str_tmp = str_tmp + num2str(attr_num, 2) + " ";
	//读入各个属性
	for (int attr_i = 0; attr_i < attr_num; attr_i++) {
		string attr_type = num2str(attribute.type[attr_i], 3);
		string attr_name = attribute.name[attr_i];
		string uni = "";
		uni = (attribute.unique[attr_i] == true ? "1" : "0");

		str_tmp = str_tmp + attr_type + " " + attr_name + " " + uni + " ";
	}
	// 读入主键
	str_tmp += num2str(primary, 2);
	str_tmp = str_tmp + " " + ";";

	// 读入索引数量
	str_tmp = str_tmp + num2str(index.num,2);
	for (int index_i = 0; index_i < index.num; index_i++) {
		int index_loc = index.location[index_i];
		string index_nam = index.indexname[index_i];
		str_tmp = str_tmp + " " + num2str(index_loc, 2) + " " + index_nam;
	}
	str_tmp = str_tmp + "\n" + "#";
	// 该行的长度
	string str_len = num2str((int)str_tmp.length() - 1, 4);
	str_tmp = str_len + str_tmp.substr(4, str_tmp.length() - 4);

	// 储存到块中
	int block_num = getBlockNum(TABLE_MANAGER_PATH) / PAGESIZE;
	if (block_num < 1) block_num = 1;

	for (int block_i = 0; block_i < block_num; block_i++) {
		char *buffer_block = buffer_manager.getPage(TABLE_MANAGER_PATH, block_i);
		int rear = 0;
		// 确定行的末尾位置
		while (buffer_block[rear] != '\0' && buffer_block[rear] != '#' && rear < PAGESIZE)
			rear++;
		// 检验加上该行后是否超出Pagesize
		int len_table = (int)str_tmp.length();
		if (rear + len_table < PAGESIZE) {
			if (rear != 0 && buffer_block[rear] == '#')
				buffer_block[rear] = '\0';
			else if (buffer_block[rear - 1] == '#')
				buffer_block[rear - 1] = '\0';
			strcat(buffer_block, str_tmp.c_str());
			return;
		}
	}
	char* buffer_ = buffer_manager.getPage(TABLE_MANAGER_PATH, block_num);
	int page_id = buffer_manager.getPageId(TABLE_MANAGER_PATH, block_num);
	strcat(buffer_, str_tmp.c_str());
	buffer_manager.modifyPage(page_id);

}

void CatalogManager::dropTable(string table_name) {
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	// 得到表所在的块，和表在块中的开始位置

	int table_block;
	int start = getTablePlace(table_name, table_block);

	// 读出块的信息，块对应的页，以及表在块中的结束位置
	char* buffer_block = buffer_manager.getPage(TABLE_MANAGER_PATH, table_block);
	int block_page = buffer_manager.getPageId(TABLE_MANAGER_PATH, table_block);
	string buffer_check(buffer_block);
	int end = start + str2num(buffer_check.substr(start, 4));

	// 执行删除
	int index = 0, current = 0;
	do {
		if (index < start || index >= end)
			buffer_block[current++] = buffer_block[index];
		index++;
	} while (buffer_block[index] != '#');
	buffer_block[current++] = '#';
	buffer_block[current] = '\0';
	//刷新页面
	buffer_manager.modifyPage(block_page);

}


Attribute CatalogManager::getAttribute(string name) {
	//表不存在，异常
	if (!hasTable(name)) {
		throw attribute_not_exist();
	}
	//表对应的块和它在块中的位置
	int find_block;
	int start_index = getTablePlace(name, find_block);
	//得到块的信息
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, find_block);
	string buffer_check(buffer);

	//end_index 确定表名的位置
	int end_index = 0;
	string table_name = getTableName(buffer_check, start_index, end_index);

	Attribute table_attr;
	start_index = end_index + 1;
	//从Attribute类中得到关于属性的信息
	string attr_num = buffer_check.substr(start_index, 2);
	table_attr.num = str2num(attr_num);
	start_index += 3;
	for (int index = 0; index < table_attr.num; index++) {
		//读取一系列attribute的类型和名字
		if (buffer_check[start_index] == '-') {
			table_attr.type[index] = -1; // int
			start_index += 5; //多了"-"
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		else if (str2num(buffer_check.substr(start_index, 3)) == 0) {
			table_attr.type[index] = 0; // float
			start_index += 4;
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		else {
			table_attr.type[index] = str2num(buffer_check.substr(start_index, 3)); // 正整数为字符类型的长度
			start_index += 4;
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		start_index += 2;
	}

	// 主键信息
	if (buffer_check[start_index] == '-')
		table_attr.primary_key = -1;
	else
	{
		table_attr.primary_key = str2num(buffer_check.substr(start_index, 2));
	}

	// 索引信息
	Index index_record = getIndex(table_name);
	for (int i = 0; i < 32; i++) {
		table_attr.has_index[i] = false;
	}
	for (int i = 0; i < index_record.num; i++)
	{
		//在这些位置的属性有索引
		int have_index = index_record.location[i];
		table_attr.has_index[have_index] = true;
	}
	return table_attr;
}
bool CatalogManager::hasAttribute(string table_name, string attr_name) {
	// 首先检验有没有表
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	// 有表，检验每一个属性的名字
	// 首先获取表的整体属性信息
	Attribute find_attr = getAttribute(table_name);
	int tmp_index;
	for (tmp_index = 0; tmp_index < find_attr.num; tmp_index++) {
		if (attr_name == find_attr.name[tmp_index]) {
			//找到了对应的属性
			return true;
		}
	}
	return false;

}
//通过索引名找到它对应的属性
string CatalogManager::IndextoAttr(string table_name, string index_name) {
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	// 首先获取表的整个索引信息，找到目标索引的序号
	Index index_record = getIndex(table_name);
	int find = -1;
	for (int i = 0; i < index_record.num; i++) {
		if (index_record.indexname[i] == index_name) {
			find = i;
			break;
		}
	}
	// 找不到索引名对应的索引，报错
	if (find < 0) {
		throw index_not_exist();
	}
	// 获取表的整个属性信息，目标索引的location指明了是第几个属性
	Attribute attr = getAttribute(table_name);
	return attr.name[index_record.location[find]];
}

void CatalogManager::createIndex(string table_name, string attr_name, string index_name) {
	// 首先检验有没有表
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	if (!hasAttribute(table_name, attr_name)) {
		throw attribute_not_exist();
	}
	//现有的索引，检查是否索引数量超出上界
	Index index = getIndex(table_name);
	int index_num = index.num;
	int Maxindex = 10;
	if (index_num >= Maxindex) {
		throw index_full();
	}

	//获取表的属性，确定是否可以在指定的属性上建立索引
	Attribute attribute = getAttribute(table_name);
	int attr_index = -1;
	for (int attr_i = 0; attr_i < attribute.num; attr_i++) {
		if (index.indexname[attr_i] == index_name) {
			//重名
			throw index_exist();
		}
		if (attribute.name[attr_i] == attr_name) {
			attr_index = attr_i;
		}
			
	}

	for (int index_i = 0; index_i < index.num; index_i++) {
		if (index.location[index_i] == attr_index) {
			// 找到指定的属性，检查是否已经有索引
			
				throw index_exist();
		}
	}


	// 修改索引信息
	index.indexname[index.num] = index_name;
	for (int attr_i = 0; attr_i < attribute.num; attr_i++) {
		if (attribute.name[attr_i] == attr_name) {
			index.location[index.num] = attr_i;
		}
	}

	index.num++;
	dropTable(table_name);
	int pri = attribute.primary_key;
	createTable(table_name, attribute, pri, index);


}


void CatalogManager::dropIndex(string table_name, string index_name) {
	//如果不存在指定名字的索引，异常
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	// 获取索引的全部信息
	Index index = getIndex(table_name);
	// 获取属性信息
	Attribute attr = getAttribute(table_name);
	// 遍历属性，找到目标索引
	int hasfind = -1;
	for (int i = 0; i < index.num; i++) {
		if (index.indexname[i] == index_name) {
			hasfind = i;
			break;
		}
	}
	if (hasfind == -1) {
		throw index_not_exist();
	}
	// 删除索引：与最后一个位置的索引换位置，删除最后一个
	index.indexname[hasfind] = index.indexname[index.num - 1];
	index.location[hasfind] = index.location[index.num - 1];
	index.num--;
	// 用修改后的索引信息重建表
	dropTable(table_name);
	createTable(table_name, attr, attr.primary_key, index);
}
void CatalogManager::showTable(string table_name) {

	//如果不存在该表，则异常
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	//打印表的名字
	cout << "Table name:" << table_name << endl;
	Attribute attribute = getAttribute(table_name);
	Index index_record = getIndex(table_name);
	// 设置宽度
	int width=0;

	for (int attr_i = 0; attr_i < attribute.num; attr_i++) {
		int length = (int)attribute.name[attr_i].length();
		if (length > width) {
			width = length;
		}
	}

	//打印属性, 设置列宽
	string type;
	cout << "Attribute:" << endl;
	cout << "Num|" << "Name" << setw(width + 2) << "|Type" << type << setw(6) << "|" << "Unique|Primary Key" << endl;
	for (int index_out = 0; index_out < width + 35; index_out++)
		cout << "-";
	cout << endl;
	for (int attr_i = 0; attr_i < attribute.num; attr_i++) {
		switch (attribute.type[attr_i]) {
		case -1:
			type = "int";
			break;
		case 0:
			type = "float";
			break;
		default:
			type = "char(" + num2str(attribute.type[attr_i] - 1, 3) + ")";
			break;
		}
		int name_len = (int)attribute.name[attr_i].length();
		int type_len = (int)type.length();
		cout << attr_i << setw(3 - attr_i / 10) << "|" << attribute.name[attr_i] << setw(width - name_len + 2) << "|" << type << setw(10 - type_len) << "|";
		if (attribute.unique[attr_i])
			cout << "unique" << "|";
		else
			cout << setw(7) << "|";	

		// 主键
		if (attribute.primary_key == attr_i) {
			cout << "primary key";
		}
		cout << endl;
	}

	for (int index_out = 0; index_out < width + 35; index_out++)
		cout << "-";

	cout << endl;

	//打印索引
	cout << "Index:" << endl;
	cout << "Num|Location|Name" << endl;
	width = -1;
	for (int index_out = 0; index_out < index_record.num; index_out++) {
		if ((int)index_record.indexname[index_out].length() > width)
			width = (int)index_record.indexname[index_out].length();
	}
	for (int index_out = 0; index_out < ((width + 14) > 18 ? (width + 14) : 18); index_out++)
		cout << "-";
	cout << endl;
	for (int index_out = 0; index_out < index_record.num; index_out++) {
		cout << index_out << setw(3 - index_out / 10) << "|" << index_record.location[index_out] << setw(8 - index_record.location[index_out] / 10) << "|" << index_record.indexname[index_out] << endl;
	}
	for (int index_out = 0; index_out < ((width + 14) > 18 ? (width + 14) : 18); index_out++)
		cout << "-";
	cout << endl << endl;

}
// 是否已存在表
bool CatalogManager::hasTable(string table_name) {
	// 算块的数量
	int block_num = getBlockNum(TABLE_MANAGER_PATH) / PAGESIZE;
	if (block_num <= 0) {
		block_num = 1;
	}
	// 遍历块
	for (int i = 0; i < block_num; i++) {
		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, i);

		string buffer_check(buffer);
		string str_tmp = "";
		int start = 0; int end = 0;
		do {
			//“#” 表示是块的末尾，跳出检查下一块
			if (buffer_check[0] == '#') {
				break;
			}
			else if (getTableName(buffer, start, end) == table_name) {
				return true;
			}
			else {
				// 读这行记录的长度（前4个字符），跳过这段长度读下一个table
				start += str2num(buffer_check.substr(start, 4));
				if (start == 0)
					break;
			}

		} while (buffer_check[start] != '#');
	}
	return false;
}
string CatalogManager::num2str(int num, short bit) {
	//初始化一个空字符串
	string str = "";
	if (num < 0) {
		str += "-";
		num = -num;
	}
	int d = pow(10, bit - 1);
	for (int i = 0; i < bit; i++) {
		str += (num / d % 10 + '0');
		d /= 10;
	}
	return str;
}
int CatalogManager::str2num(string str) {
	return atoi(str.c_str());
}
// 已知表在块中的某行，返回表名
string CatalogManager::getTableName(string buffer, int start, int &end) {
	string tmp_string = "";
	end = 0;
	if (buffer == "") {
		return buffer;
	}
	// 行的第5个字符起是表名，从该位置遍历，直到空格表示表名的字符串结束
	while (buffer[start + end + 5] != ' ') {
		end++;
	}
	tmp_string = buffer.substr(start + 5, end);
	end = start + 5 + end;
	return tmp_string;
}
// 获得表所在的块的编号和在块中的位置
int CatalogManager::getTablePlace(string name, int& find_block) {
	// 获取块的总数以便遍历
	int block_num = getBlockNum(TABLE_MANAGER_PATH);
	if (block_num <= 0)
		block_num = 1;
	//遍历所有的块
	for (find_block = 0; find_block < block_num; find_block++) {

		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, find_block);
		string buffer_check(buffer);
		string str_tmp = "";
		int start = 0, rear = 0;
		do {
			//是空的块#，则跳到下一块
			if (buffer_check[0] == '#') {
				break;
			}
			// 调用getTableName找到对应的表名,返回表在块中的位置
			if (getTableName(buffer, start, rear) == name) {
				return start;
			}
			else {
				// 读取该行的长度（前4个字符），跳到下一个表
				int length = str2num(buffer_check.substr(start, 4));
				start += length;
			}
		} while (buffer_check[start] != '#');
	}
	return -1;
}
Index CatalogManager::getIndex(string table_name) {
	Index index;
	// 先找到表的位置，所在的块
	int right_block;
	int start_index = getTablePlace(table_name, right_block);
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, right_block);
	// start_index对齐索引开始的位置
	string buffer_check(buffer);
	while (buffer_check[start_index] != ';') {
		start_index++;
	}
	// start_index 为索引开始的下标
	start_index++;
	index.num = str2num(buffer_check.substr(start_index, 2));

	for (int index_tmp = 0; index_tmp < index.num; index_tmp++) {
		// 跳到下一个信息
		start_index += 3;
		// 读取两个字符，记录了该索引对应属性的位置
		index.location[index_tmp] = str2num(buffer_check.substr(start_index, 2));
		start_index += 3;
		while (buffer_check[start_index] != ' ' && buffer_check[start_index] != ';' && buffer_check[start_index] != '\n')
		{
			// 读入索引的名字
			index.indexname[index_tmp] += buffer_check[start_index++];
		}
		start_index -= 2;

	}
	return index;
}
//获得文件大小/占几块
int CatalogManager::getBlockNum(string table_name) {
	char* p;
	int block_num = -1;
	do {
		p = buffer_manager.getPage(table_name, block_num + 1);    //Buffer Manager调用getbuffer返回指向页内容的指针
		block_num++;
	} while (p[0] != '\0');
	return block_num;
}