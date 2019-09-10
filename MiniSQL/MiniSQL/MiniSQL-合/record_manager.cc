#include "record_manager.h"

extern indexManager index_manager;

void RecordManager::createTableFile(std::string table_name) {
    table_name = "./database/data/" + table_name;
    FILE* f = fopen(table_name.c_str() , "w");
    fclose(f);
}

void RecordManager::dropTableFile(std::string table_name) {
    table_name = "./database/data/" + table_name;
    remove(table_name.c_str());
}

void RecordManager::insertRecord(std::string table_name , Tuple& tuple) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    std::vector<Data> v = tuple.getData();
    for (int i = 0;i < v.size();i++) {
        if (v[i].type != attr.type[i])
            throw tuple_type_conflict();
    }
    Table table = selectRecord(tmp_name);
    std::vector<Tuple>& tuples = table.getTuple();
    if (attr.primary_key >= 0) {
        if (isConflict(tuples , v , attr.primary_key) == true) 
            throw primary_key_conflict();
    }
    for (int i = 0;i < attr.num;i++) {
        if (attr.unique[i] == true) {
            if (isConflict(tuples , v , i) == true)
                throw unique_conflict();
        }
    }

    int block_num = getBlockNum(table_name);
    if (block_num <= 0)
        block_num = 1;
    char* p = buffer_manager.getPage(table_name , block_num - 1);
    int i;
    for (i = 0;p[i] != '\0' && i < PAGESIZE;i++) ;
    int j;
    int len = 0;
    for (j = 0;j < v.size();j++) {
        Data d = v[j];
        switch(d.type) {
            case -1:{
                int t = getDataLength(d.datai);
                len += t;
            };break;
            case 0:{
                float t = getDataLength(d.dataf);
                len += t;
            };break;
            default:{
                len += d.datas.length();
            };
        }
    }
    len += v.size() + 7;
    int block_offset;
    if (PAGESIZE - i >= len) {
        block_offset = block_num - 1;
        insertRecord1(p , i , len , v);
        int page_id = buffer_manager.getPageId(table_name , block_num - 1);
        buffer_manager.modifyPage(page_id);
    }
    else {
        block_offset = block_num;
        char* p = buffer_manager.getPage(table_name , block_num);
        insertRecord1(p , 0 , len , v);
        int page_id = buffer_manager.getPageId(table_name , block_num);
        buffer_manager.modifyPage(page_id);
    }

    indexManager index_manager(tmp_name);
    for (int i = 0;i < attr.num;i++) {
        if (attr.has_index[i] == true) {
            std::string attr_name = attr.name[i];
            std::string file_path = "INDEX_FILE_" + attr_name + "_" + tmp_name;
            std::vector<Data> d = tuple.getData();
            index_manager.insertIndex(file_path , d[i] , block_offset);
        }
    }
}

int RecordManager::deleteRecord(std::string table_name) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    int block_num = getBlockNum(table_name);
    if (block_num <= 0)
        return 0;
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    indexManager index_manager(tmp_name);
    int count = 0;
    for (int i = 0;i < block_num;i++) {
        char* p = buffer_manager.getPage(table_name , i);
        char* t = p;
        while(*p != '\0' && p < t + PAGESIZE) {
            Tuple tuple = readTuple(p , attr);
            for (int j = 0;j < attr.num;j++) {
                if (attr.has_index[j] == true) {
                    std::string attr_name = attr.name[i];
                    std::string file_path = "INDEX_FILE_" + attr_name + "_" + tmp_name;
                    std::vector<Data> d = tuple.getData();
                    index_manager.deleteIndexbyKey(file_path , d[j]);
                }
            }
            p = deleteRecord1(p);
            count++;
        }
        int page_id = buffer_manager.getPageId(table_name , i);
        buffer_manager.modifyPage(page_id);
    }
    return count;
}

int RecordManager::deleteRecord(std::string table_name , std::string target_attr , Where where) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;     
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    int index = -1;
    bool flag = false;
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == target_attr) {
            index = i;
            if (attr.has_index[i] == true) 
                flag = true;
            break;
        }
    }
    if (index == -1) {
        throw attribute_not_exist();
    }
    else if (attr.type[index] != where.data.type) {
        throw data_type_conflict();
    }

    int count = 0;
    if (flag == true && where.relation_character != NOT_EQUAL) {
        std::vector<int> block_ids;
        searchWithIndex(tmp_name , target_attr , where , block_ids);
        for (int i = 0;i < block_ids.size();i++) {
            count += conditionDeleteInBlock(tmp_name , block_ids[i] , attr , index , where);
        }
    }
    else {
        int block_num = getBlockNum(table_name);
        if (block_num <= 0)
            return 0;
        for (int i = 0;i < block_num;i++) {
            count += conditionDeleteInBlock(tmp_name , i , attr , index , where);
        }
    }
    return count;
}

Table RecordManager::selectRecord(std::string table_name , std::string result_table_name) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;    
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    int block_num = getBlockNum(table_name);
    if (block_num <= 0)
        block_num = 1;
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    Table table(result_table_name , attr);
    std::vector<Tuple>& v = table.getTuple();
    for (int i = 0;i < block_num;i++) {
        char* p = buffer_manager.getPage(table_name , i);
        char* t = p;
        while (*p != '\0' && p < t + PAGESIZE) {
            Tuple tuple = readTuple(p , attr);
            if (tuple.isDeleted() == false)
               v.push_back(tuple);
            int len = getTupleLength(p);
            p = p + len;
        }
    }
	//table.showTable();
    return table;
}

Table RecordManager::selectRecord(std::string table_name , std::string target_attr , Where where , std::string result_table_name) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    int index = -1; 
    bool flag = false;
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == target_attr) {
            index = i;
            if (attr.has_index[i] == true)
                flag = true;
            break;
        }
    }
    if (index == -1) {
        throw attribute_not_exist();
    }
    else if (attr.type[index] != where.data.type) {
        throw data_type_conflict();
    }
    Table table(result_table_name , attr);
    std::vector<Tuple>& v = table.getTuple();
    if (flag == true && where.relation_character != NOT_EQUAL) {
        std::vector<int> block_ids;
        searchWithIndex(tmp_name , target_attr , where , block_ids);
        for (int i = 0; i < block_ids.size();i++) {
            conditionSelectInBlock(tmp_name , block_ids[i] , attr , index , where , v);
        }
    }
    else {
        int block_num = getBlockNum(table_name);
        if (block_num <= 0)
            block_num = 1;
        for (int i = 0;i < block_num;i++) {
            conditionSelectInBlock(tmp_name , i , attr , index , where , v);
        }
    }
	//table.showTable();
    return table;
}

void RecordManager::createIndex(indexManager& index_manager , std::string table_name , std::string target_attr) {
    std::string tmp_name = table_name;
    table_name = "./database/data/" + table_name;
    CatalogManager catalog_manager;
    if (!catalog_manager.hasTable(tmp_name)) {
        throw table_not_exist();
    }
    Attribute attr = catalog_manager.getAttribute(tmp_name);
    int index = -1;
    for (int i = 0;i < attr.num;i++) {
        if (attr.name[i] == target_attr) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        throw attribute_not_exist();
    }
    int block_num = getBlockNum(table_name);
    if (block_num <= 0)
        block_num = 1;
    std::string file_path = "INDEX_FILE_" + target_attr + "_" + tmp_name;
    for (int i = 0;i < block_num;i++) {
        char* p = buffer_manager.getPage(table_name , i);
        char* t = p;
        while (*p != '\0' && p < t + PAGESIZE) {
            Tuple tuple = readTuple(p , attr);
            if (tuple.isDeleted() == false) {
                std::vector<Data> v = tuple.getData();
                index_manager.insertIndex(file_path , v[index] , i);
            }
            int len = getTupleLength(p);
            p = p + len;
        }
    }
}

int RecordManager::getBlockNum(std::string table_name) {
    char* p;
    int block_num = -1;
    do {
        p = buffer_manager.getPage(table_name , block_num + 1);
        block_num++;
    } while(p[0] != '\0');
    return block_num;
}

void RecordManager::insertRecord1(char* p , int offset , int len , const std::vector<Data>& v) {
    std::stringstream stream;
    stream << len;
    std::string s = stream.str();
    while (s.length() < 4) 
        s = "0" + s;
    for (int j = 0;j < s.length();j++,offset++)
        p[offset] = s[j];
    for (int j = 0;j < v.size();j++) {
        p[offset] = ' ';
        offset++;
        Data d = v[j];
        switch(d.type) {
            case -1:{
                copyString(p , offset , d.datai);
            };break;
            case 0:{
                copyString(p , offset , d.dataf);
            };break;
            default:{
                copyString(p , offset , d.datas);
            };
        }
    }
    p[offset] = ' ';
    p[offset + 1] = '0';
    p[offset + 2] = '\n';
}

char* RecordManager::deleteRecord1(char* p) {
    int len = getTupleLength(p);
    p = p + len;
    *(p - 2) = '1';
    return p;
}

Tuple RecordManager::readTuple(const char* p , Attribute attr) {
    Tuple tuple;
    p = p + 5;
    for (int i = 0;i < attr.num;i++) {
        Data data;
        data.type = attr.type[i];
        char tmp[100];
        int j;
        for (j = 0;*p != ' ';j++,p++) {
            tmp[j] = *p;
        }
        tmp[j] = '\0';
        p++;
        std::string s(tmp);
        switch(data.type) {
            case -1:{
                std::stringstream stream(s);
                stream >> data.datai;
            };break;
            case 0:{
                std::stringstream stream(s);
                stream >> data.dataf;
            };break;
            default:{
                data.datas = s;
            }
        }
        tuple.addData(data);
    }
    if (*p == '1')
        tuple.setDeleted();
    return tuple;
}

int RecordManager::getTupleLength(char* p) {
    char tmp[10];
    int i;
    for (i = 0;p[i] != ' ';i++) 
        tmp[i] = p[i];
    tmp[i] = '\0';
    std::string s(tmp);
    int len = stoi(s);
    return len;
}

bool RecordManager::isConflict(std::vector<Tuple>& tuples , std::vector<Data>& v , int index) {
    for (int i = 0;i < tuples.size();i++) {
        if (tuples[i].isDeleted() == true)
            continue;
        std::vector<Data> d = tuples[i].getData();
        switch(v[index].type) {
            case -1:{
                if (v[index].datai == d[index].datai)
                    return true;
            };break;
            case 0:{
                if (v[index].dataf == d[index].dataf)
                    return true;
            };break;
            default:{
                if (v[index].datas == d[index].datas)
                    return true;
            };
        }
    }
    return false;
}

void RecordManager::searchWithIndex(std::string table_name , std::string target_attr , Where where , std::vector<int>& block_ids) {
    indexManager index_manager(table_name);
	//index_manager(table_name);
    Data tmp_data;
    std::string file_path = "INDEX_FILE_" + target_attr + "_" + table_name;
    if (where.relation_character == LESS || where.relation_character == LESS_OR_EQUAL) {
        if (where.data.type == -1) {
            tmp_data.type = -1;
            tmp_data.datai = -INF;
        }
        else if (where.data.type == 0) {
            tmp_data.type = 0;
            tmp_data.dataf = -INF;
        }
        else {
            tmp_data.type = 1;
            tmp_data.datas = "";
        }
        index_manager.findbyRange(file_path , tmp_data , where.data , block_ids);
    }
    else if (where.relation_character == GREATER || where.relation_character == GREATER_OR_EQUAL) {
        if (where.data.type == -1) {
            tmp_data.type = -1;
            tmp_data.datai = INF;
        }
        else if (where.data.type == 0) {
            tmp_data.type = 0;
            tmp_data.dataf = INF;
        }
        else {
            tmp_data.type = where.data.type;
        }
        index_manager.findbyRange(file_path , where.data , tmp_data , block_ids);
    }
    else {
        //index_manager.findbyRange(file_path , where.data , where.data , block_ids);
		block_ids.push_back(index_manager.findbyKey(file_path, where.data));
    }
}

int RecordManager::conditionDeleteInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where) {
    table_name = "./database/data/" + table_name;//新增
    char* p = buffer_manager.getPage(table_name , block_id);
    char* t = p;
    int count = 0;
    while (*p != '\0' && p < t + PAGESIZE) {
        Tuple tuple = readTuple(p , attr);
        std::vector<Data> d = tuple.getData();
        switch(attr.type[index]) {
            case -1:{
                if (isSatisfied(d[index].datai , where.data.datai , where.relation_character) == true) {
                    p = deleteRecord1(p);
                    count++;
                }
                else {
                    int len = getTupleLength(p);
                    p = p + len;
                }
            };break;
            case 0:{
                if (isSatisfied(d[index].dataf , where.data.dataf , where.relation_character) == true) {
                    p = deleteRecord1(p);
                    count++;
                }
                else {
                    int len = getTupleLength(p);
                    p = p + len;
                }
            };break;
            default:{
                if (isSatisfied(d[index].datas , where.data.datas , where.relation_character) == true) {
                    p = deleteRecord1(p);
                    count++;
                }
                else {
                    int len = getTupleLength(p);
                    p = p + len;
                }
            }
        }
    }
    int page_id = buffer_manager.getPageId(table_name , block_id);
    buffer_manager.modifyPage(page_id);
    return count;
}

void RecordManager::conditionSelectInBlock(std::string table_name , int block_id , Attribute attr , int index , Where where , std::vector<Tuple>& v) {
    table_name = "./database/data/" + table_name;
    char* p = buffer_manager.getPage(table_name , block_id);
    char* t = p;
    while (*p != '\0' && p < t + PAGESIZE) {
        Tuple tuple = readTuple(p , attr);
        if (tuple.isDeleted() == true) {
            int len = getTupleLength(p);
            p = p + len;
            continue;
        }
        std::vector<Data> d = tuple.getData();
        switch(attr.type[index]) {
            case -1:{
                if (isSatisfied(d[index].datai , where.data.datai , where.relation_character) == true) {
                    v.push_back(tuple);
                }
            };break;
            case 0:{
                if (isSatisfied(d[index].dataf , where.data.dataf , where.relation_character) == true) {
                    v.push_back(tuple);
                }
            };break;
            default:{
                if (isSatisfied(d[index].datas , where.data.datas , where.relation_character) == true) {
                    v.push_back(tuple);
                }
            };
        }
        int len = getTupleLength(p);
        p = p + len;
    }
}
