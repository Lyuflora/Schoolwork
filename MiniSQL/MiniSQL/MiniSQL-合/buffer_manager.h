#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_ 

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include "const.h"

class Page {
public:
        Page();
        void initialize();
        void setFileName(std::string file_name);
        std::string getFileName();
        void setBlockId(int block_id);
        int getBlockId();
        void setPinCount(int pin_count);
        int getPinCount();
        void setDirty(bool dirty);
        bool getDirty();
        void setRef(bool ref);
        bool getRef();
        void setAvaliable(bool avaliable);
        bool getAvaliable();
        char* getBuffer();
private:
        char buffer_[PAGESIZE];
        std::string file_name_;
        int block_id_;
        int pin_count_;
        bool dirty_;
        bool ref_;
        bool avaliable_;
};

class BufferManager {
    public: 
        BufferManager();
        BufferManager(int frame_size);
        ~BufferManager();
        char* getPage(std::string file_name , int block_id);
        void modifyPage(int page_id);
        void pinPage(int page_id);
        int unpinPage(int page_id);
        int flushPage(int page_id , std::string file_name , int block_id);
        int getPageId(std::string file_name , int block_id);
    private:
        Page* Frames;
        int frame_size_;
        int current_position_;
        void initialize(int frame_size);
        int getEmptyPageId();
        int loadDiskBlock(int page_id , std::string file_name , int block_id);
};

#endif