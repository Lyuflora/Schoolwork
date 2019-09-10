//screenshot_bmp.h
//yam
#ifndef _SCREENSHOT_BMP_H_
#define _SCREENSHOT_BMP_H_
#include <GL/glew.h>
#include <iostream>
#include <Windows.h>
#include <gl/glut.h>
#define Header_Length 54 //文件头14字节+位图信息头40字节

using namespace std;

struct BITMAP_FILE_HEADER; //位图文件头
struct BITMAP_INFO_HEADER; //位图信息头

typedef struct BITMAP_FILE_HEADER* BITMAP_FILE;
typedef struct BITMAP_INFO_HEADER* BITMAP_INFO;

bool Write_bmp(string filename, unsigned char *pdata);
void Screen_shot();

#endif // !_FILE_PROCESS_H_
