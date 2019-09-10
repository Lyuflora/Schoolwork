#ifndef _INTERPRETER_H_
#define	_INTERPRETER_H_ 

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <fstream>
#include "catalog_manager.h"
#include "api.h"
#include "basic.h"

class Interpreter{
public:
    Interpreter();
	void getQuery();
    void EXEC();
    void EXEC_SELECT();
    void EXEC_DROP_TABLE();
    void EXEC_DROP_INDEX();
    void EXEC_CREATE_TABLE();
    void EXEC_CREATE_INDEX();
    void EXEC_INSERT();
    void EXEC_DELETE();
    void EXEC_SHOW();
    void EXEC_EXIT();
    void EXEC_FILE();
    
private:
    void Normalize();
    std::string query;
    std::string getWord(int pos,int &end_pos);
    std::string getLower(std::string str,int pos);
    std::string getRelation(int pos,int &end_pos);
    int getType(int pos,int &end_pos);
    int getBits(int num);
    int getBits(float num);
};

template <class Type>
Type stringToNum(const std::string& str)
{
    std::istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

#endif
