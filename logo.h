#ifndef __LOGO_H__
#define __LOGO_H__

#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>    
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>



//文件头结构体
typedef struct 
{ 
    unsigned char	bfType[2];		//文件类型
    unsigned long	bfSize;			//位图大小
    unsigned short 	bfReserved1;	//位0 
    unsigned short 	bfReserved2;	//位0
    unsigned long 	bfOffBits;		//到数据偏移量
}__attribute__((packed)) BitMapFileHeader;	//使编译器不优化，其大小为14字节 

//信息头结构体
typedef struct 
{ 
    unsigned long biSize;			// BitMapFileHeader 字节数
    long biWidth; 					//位图宽度 
    long biHeight; 					//位图高度，正位正向，反之为倒图 
    unsigned short biPlanes;		//为目标设备说明位面数，其值将总是被设为1
    unsigned short biBitCount; 		//说明比特数/象素，为1、4、8、16、24、或32。 
    unsigned long biCompression;	//图象数据压缩的类型没有压缩的类型：BI_RGB 
    unsigned long biSizeImage;		//说明图象的大小，以字节为单位 
    long biXPelsPerMeter;			//说明水平分辨率 
    long biYPelsPerMeter;			//说明垂直分辨率 
    unsigned long biClrUsed;		//说明位图实际使用的彩色表中的颜色索引数
    unsigned long biClrImportant;	//对图象显示有重要影响的索引数，0都重要。 
}__attribute__((packed)) BitMapInfoHeader; 

//24位深像素点结构体
typedef struct 
{ 
    unsigned char Blue;			//该颜色的蓝色分量 
    unsigned char Green;		//该颜色的绿色分量 
    unsigned char Red;			//该颜色的红色分量 
}__attribute__((packed)) RgbQuad24;
//32位深像素点结构体
typedef struct 
{ 
    unsigned char Blue;			//该颜色的蓝色分量 
    unsigned char Green;		//该颜色的绿色分量 
    unsigned char Red;			//该颜色的红色分量 
    unsigned char Reserved;		//保留值（亮度）   
}__attribute__((packed)) RgbQuad32;

int fb_init(char *dev);
void fb_uninit();
int bmp_show(char *path);
int png_show(char *path);
#endif /* __LOGO_H__ */
