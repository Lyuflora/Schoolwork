#include "buffer_manager.h"

Page::Page() {
    initialize();
}

void Page::initialize() {
    file_name_ = "";
    block_id_ = -1;
    pin_count_ = -1;
    dirty_ = false;
    ref_ = false;
    avaliable_ = true;
    for (int i = 0;i < PAGESIZE;i++) 
        buffer_[i] = '\0';
}


inline void Page::setFileName(std::string file_name) {
    file_name_ = file_name;
}

inline std::string Page::getFileName() {
    return file_name_;
}

inline void Page::setBlockId(int block_id) {
    block_id_ = block_id;
}

inline int Page::getBlockId() {
    return block_id_;
}

inline void Page::setPinCount(int pin_count) {
    pin_count_ = pin_count;
}

inline int Page::getPinCount() {
    return pin_count_;
}

inline void Page::setDirty(bool dirty) {
    dirty_ = dirty;
}

inline bool Page::getDirty() {
    return dirty_;
}

inline void Page::setRef(bool ref) {
    ref_ = ref;
}

inline bool Page::getRef() {
    return ref_;
}

inline void Page::setAvaliable(bool avaliable) {
    avaliable_ = avaliable;
}

inline bool Page::getAvaliable() {
    return avaliable_;
}

inline char* Page::getBuffer() {
    return buffer_;
}


BufferManager::BufferManager() {
    initialize(MAXFRAMESIZE);
}

BufferManager::BufferManager(int frame_size) {
    initialize(frame_size);
}


BufferManager::~BufferManager() {
    for (int i = 0;i < frame_size_;i++) {
        std::string file_name;
        int block_id;
        file_name = Frames[i].getFileName();
        block_id = Frames[i].getBlockId();
        flushPage(i , file_name , block_id);
    }
}


void BufferManager::initialize(int frame_size) {
    Frames = new Page[frame_size];
    frame_size_ = frame_size;
    current_position_ = 0;
}


char* BufferManager::getPage(std::string file_name , int block_id) {
    int page_id = getPageId(file_name , block_id);
    if (page_id == -1) {
        page_id = getEmptyPageId();
        loadDiskBlock(page_id , file_name , block_id);
    }
    Frames[page_id].setRef(true);
    return Frames[page_id].getBuffer();
}

void BufferManager::modifyPage(int page_id) {
    Frames[page_id].setDirty(true);
}

void BufferManager::pinPage(int page_id) {
    int pin_count = Frames[page_id].getPinCount();
    Frames[page_id].setPinCount(pin_count + 1);
}

int BufferManager::unpinPage(int page_id) {
    int pin_count = Frames[page_id].getPinCount();
    if (pin_count <= 0) 
        return -1;
    else
        Frames[page_id].setPinCount(pin_count - 1);
    return 0;
}


int BufferManager::loadDiskBlock(int page_id , std::string file_name , int block_id) {
    Frames[page_id].initialize();
    FILE* f = fopen(file_name.c_str() , "r");
    if (f == NULL)
        return -1;
    fseek(f , PAGESIZE * block_id , SEEK_SET);
    char* buffer = Frames[page_id].getBuffer();
    fread(buffer , PAGESIZE , 1 , f);
    fclose(f);
    Frames[page_id].setFileName(file_name);
    Frames[page_id].setBlockId(block_id);
    Frames[page_id].setPinCount(1);
    Frames[page_id].setDirty(false);
    Frames[page_id].setRef(true);
    Frames[page_id].setAvaliable(false);
    return 0;
}


int BufferManager::flushPage(int page_id , std::string file_name , int block_id) {
    FILE* f = fopen(file_name.c_str() , "r+");
    if (f == NULL)
        return -1; 
    fseek(f , PAGESIZE * block_id , SEEK_SET);
    char* buffer = Frames[page_id].getBuffer();
    fwrite(buffer , PAGESIZE , 1 , f);
    fclose(f);
    return 0;
}

int BufferManager::getPageId(std::string file_name , int block_id) {
    for (int i = 0;i < frame_size_;i++) {
        std::string tmp_file_name = Frames[i].getFileName();
        int tmp_block_id = Frames[i].getBlockId();
        if (tmp_file_name == file_name && tmp_block_id == block_id)
            return i;
    }
    return -1;
}

int BufferManager::getEmptyPageId() {
    for (int i = 0;i < frame_size_;i++) {
        if (Frames[i].getAvaliable() == true)
            return i;
    }
    while (1) {
        if (Frames[current_position_].getRef() == true) {
            Frames[current_position_].setRef(false);
        }
        else if (Frames[current_position_].getPinCount() == 0) {
            if (Frames[current_position_].getDirty() == true) {
                std::string file_name = Frames[current_position_].getFileName();
                int block_id = Frames[current_position_].getBlockId();
                flushPage(current_position_ , file_name , block_id);
            }
            Frames[current_position_].initialize();
            return current_position_;
        }
        current_position_ = (current_position_ + 1) % frame_size_;
    }
}