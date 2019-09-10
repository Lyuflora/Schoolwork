//
//  basic.cpp
//  base
//
//  Created by Sr on 2017/5/25.
//  Copyright © 2017年 Sr. All rights reserved.
//

#include "basic.h"
#include <string>

Tuple::Tuple(const Tuple &tuple_in){
    for(int index=0;index<tuple_in.data_.size();index++)
    {
        this->data_.push_back(tuple_in.data_[index]);
    }
	this->isDeleted_ = tuple_in.isDeleted_;
}

inline int Tuple::getSize(){
    return (int)data_.size();
}

//新增数据
void Tuple::addData(Data data_in){
    this->data_.push_back(data_in);
}

bool Tuple::isDeleted() {
    return isDeleted_;
}

void Tuple::setDeleted() {
    isDeleted_ = true;
}

std::vector<Data> Tuple::getData() const{
    return this->data_;
}

void Tuple::showTuple(){
    for(int index=0;index<getSize();index++){
        if(data_[index].type==-1)
            std::cout<<data_[index].datai<<'\t';
        else if(data_[index].type==0)
            std::cout<<data_[index].dataf<<'\t';
        else
            std::cout<<data_[index].datas<<'\t';
    }
    std::cout<<std::endl;
}

Table::Table(std::string title,Attribute attr){
    title_=title;
    attr_=attr;
    index_.num=0;
}

Table::Table(const Table &table_in){
    attr_=table_in.attr_;
    index_=table_in.index_;
    title_=table_in.title_;
    for(int index=0;index<table_in.tuple_.size();index++)
        tuple_.push_back(table_in.tuple_[index]);
}

int Table::setIndex(short index,std::string index_name){
    short tmpIndex;
    for(tmpIndex=0;tmpIndex<index_.num;tmpIndex++){
        if(index==index_.location[tmpIndex])  
        {
            std::cout<<"Illegal Set Index: The index has been in the table."<<std::endl;
            return 0;
        }
    }
    for(tmpIndex=0;tmpIndex<index_.num;tmpIndex++){
        if(index_name==index_.indexname[tmpIndex]) 
        {
            std::cout<<"Illegal Set Index: The name has been used."<<std::endl;
            return 0;
        }
    }
    index_.location[index_.num]=index;  
    index_.indexname[index_.num]=index_name;
    index_.num++;
    return 1;
}

int Table::dropIndex(std::string index_name){
    short tmpIndex;
    for(tmpIndex=0;tmpIndex<index_.num;tmpIndex++){
        if(index_name==index_.indexname[tmpIndex])  
            break;
    }
    if(tmpIndex==index_.num)
    {
        std::cout<<"Illegal Drop Index: No such a index in the table."<<std::endl;
        return 0;
    }

    index_.indexname[tmpIndex]=index_.indexname[index_.num-1];
    index_.location[tmpIndex]=index_.location[index_.num-1];
    index_.num--;
    return 1;
}


std::string  Table::getTitle(){
    return title_;
}
Attribute Table::getAttr(){
    return attr_;
}
std::vector<Tuple>& Table::getTuple(){
    return tuple_;
}
Index Table::getIndex(){
    return index_;
}


void Table::showTable(){
    for(int index=0;index<attr_.num;index++)
        std::cout<<attr_.name[index]<<'\t';
    std::cout<<std::endl;
    for(int index=0;index<tuple_.size();index++)
        tuple_[index].showTuple();
}

void Table::showTable(int limit) {
    for(int index=0;index<attr_.num;index++)
        std::cout<<attr_.name[index]<<'\t';
    std::cout<<std::endl;
    for(int index=0;index<limit&&index<tuple_.size();index++)
        tuple_[index].showTuple();
}
