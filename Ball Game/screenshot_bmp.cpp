//screenshot_bmp.cpp
//yam
#include "screenshot_bmp.h"

#include <string>
#include <stdlib.h>
#include <windows.h>


#pragma pack(1)

char sc_num = '1';
const long Window_Width = 1024;
const long Window_Height = 768;

struct BITMAP_FILE_HEADER {
	WORD    bfType;      //BM,0x4D42
	DWORD   bfSize;      // 结构体的字节大小
	WORD    bfReserved1;   //必须为0
	WORD    bfReserved2;  //必须为0
	DWORD   bfOffBits;     //位图数据偏移量
};

struct BITMAP_INFO_HEADER {
	DWORD  biSize;
	LONG   biWidth;       //宽度
	LONG   biHeight;       //高度
	WORD   biPlanes;       //平面数目1
	WORD   biBitCount;       //颜色位数
	DWORD  biCompression;
	DWORD  biSizeImage;     //位图数据的字节大小
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
};

bool Write_bmp(string filename, unsigned char *pdata)
{
	//填充bmp文件头
	BITMAP_FILE bit_file;
	bit_file = (BITMAP_FILE)malloc(sizeof(struct BITMAP_FILE_HEADER));
	if (bit_file == NULL)
	{
		cout << "Cannot alloc space for bitmap_file pointer." << endl;
		return false;
	}
	
	int size = Window_Width * Window_Height * 3;
	bit_file->bfSize = Header_Length + size;
	bit_file->bfType = 0x4d42;
	bit_file->bfReserved1 = 0;
	bit_file->bfReserved2 = 0;
	bit_file->bfOffBits = Header_Length;

	//填充bmp信息头
	BITMAP_INFO bit_info;
	bit_info = (BITMAP_INFO)malloc(sizeof(struct BITMAP_INFO_HEADER));
	if (bit_info == NULL)
	{
		cout << "Cannot alloc space for bitmap_info pointer." << endl;
		return false;
	}
	
	bit_info->biSize = 40L;
	bit_info->biWidth = Window_Width;
	bit_info->biHeight = Window_Height;
	bit_info->biPlanes = 1L;
	bit_info->biBitCount = 24L;
	bit_info->biCompression = BI_RGB;
	bit_info->biSizeImage = Window_Width * Window_Height * 3;
	bit_info->biXPelsPerMeter = 0;
	bit_info->biYPelsPerMeter = 0;
	bit_info->biClrUsed = 0;
	bit_info->biClrImportant = 0;

	FILE *fp;
	fp = fopen(filename.c_str(), "wb"); //二进制只写
	if (fp == NULL)
	{
		cout << "ERROR: Cannot open file" + filename << endl;
		return false;
	}

	fwrite(bit_file, sizeof(*bit_file), 1, fp);
	fwrite(bit_info, sizeof(*bit_info), 1, fp);
	fwrite(pdata, bit_info->biSizeImage, 1, fp);

	fclose(fp);
	free(bit_file);
	free(bit_info);
	return true;
}

void Screen_shot()
{
	GLuint pdata_length;
	string filename;
	string filename_1 = "sc_", filename_2 = ".bmp";
	unsigned char *pdata;
	
	pdata_length = Window_Width * Window_Height * 3;
	pdata = (unsigned char *)malloc(pdata_length);
	memset(pdata, 0, pdata_length);

	filename = filename_1 + sc_num + filename_2;
	sc_num++;
	if (sc_num > '9') { sc_num = '1'; } //最多容纳九张截图

	glReadPixels(0, 0, Window_Width, Window_Height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pdata);
	if (Write_bmp(filename, (unsigned char *)pdata))
	{
		cout << "Screenshot successfully saved in " << filename << "." << endl;
	}

	free(pdata);
	return;
}
