//
//  basic.h
//  base
//
//  Created by Sr on 2017/5/25.
//  Copyright © 2017年 Sr. All rights reserved.
//

#ifndef _BASIC_H_
#define _BASIC_H_ 

#include <iostream>
#include <vector>
#include <string>


typedef enum {
	LESS,
	LESS_OR_EQUAL,
	EQUAL,
	GREATER_OR_EQUAL,
	GREATER,
	NOT_EQUAL
}WHERE;


struct Data{
	int type;
    int datai;
    float dataf;
	std::string datas;
};


struct Where{
    Data data; 
    WHERE relation_character;   
};


struct Attribute{
    int num;  
    std::string name[32];  
    short type[32];  
    bool unique[32];  
    short primary_key;  
    bool has_index[32]; 
};


struct Index{
    int num;  
    short location[10]; 
    std::string indexname[10];  
};


class Tuple{
private:
    std::vector<Data> data_; 
    bool isDeleted_;
public:
    Tuple() : isDeleted_(false) {};
    Tuple(const Tuple &tuple_in);  
    void addData(Data data_in); 
    std::vector<Data> getData() const;  
    int getSize();  
    bool isDeleted();
    void setDeleted();
    void showTuple();  
};

class Table{
private:
    std::string title_;  
    std::vector<Tuple> tuple_;  
    Index index_;  
public:
    Attribute attr_;  
    Table(){};
    Table(std::string title,Attribute attr);
    Table(const Table &table_in);

    // int DataSize();  

    int setIndex(short index,std::string index_name);  
    int dropIndex(std::string index_name);  

    std::string getTitle();
    Attribute getAttr();
    std::vector<Tuple>& getTuple();
    Index getIndex();
    short gethasKey();

    void showTable(); 
    void showTable(int limit);
};


#endif
