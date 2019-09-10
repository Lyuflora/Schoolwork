#include <iostream>
#include "interpreter.h"
#include "buffer_manager.h"
using namespace std;

BufferManager buffer_manager;

int main(int argc, const char * argv[]) {
    std::cout<<">>> Welcome to MiniSQL"<<std::endl;
    while(1){
        Interpreter query;
        query.getQuery();
        query.EXEC();
    }
    return 0;
}