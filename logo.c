#include <stdio.h> 
#include <stdlib.h>    
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>
#include <linux/mxcfb.h>
#include <png.h>
#include "logo.h"


struct fb_var_screeninfo scrinfo;
unsigned long screensize;
char *fbp;

int fb_init(char *dev)
{
    int devfb;
    //打开设备
    devfb = open(dev, O_RDWR);
    if(!devfb){
        printf("devfb open error!\r\n");
        return -1;
    }

    //获取屏幕信息
    //若屏幕显示区域大小不合适，可用ioctl(devfb, FBIOPUT_VSCREENINFO, &scrinfo)设置
    if(ioctl(devfb, FBIOGET_VSCREENINFO, &scrinfo)){
        printf("get screen infomation error!\r\n");
        return -1;
    }

    //struct mxcfb_gamma fb_gamma;
    //fb_gamma.enable = 1;
    //fb_gamma.constk[0] = 0;
    //fb_gamma.slopek[0] = 0;
    //ioctl(devfb, MXCFB_SET_GAMMA, &fb_gamma);


    //printf(".xres=%d, .yres=%d, .bit=%d\r\n",scrinfo.xres, scrinfo.yres, scrinfo.bits_per_pixel);
    //printf(".xres_virtual=%d, .yres_virtual=%d\r\n",scrinfo.xres_virtual, scrinfo.yres_virtual);
    if(32 != scrinfo.bits_per_pixel){
        printf("screen infomation.bits %d \r\n",scrinfo.bits_per_pixel);
        //  return -1;
    }

    //计算需要的映射内存大小
    screensize = scrinfo.xres_virtual * scrinfo.yres_virtual * scrinfo.bits_per_pixel / 8;
    //printf("screensize=%lu!\r\n", screensize);

    //内存映射
    fbp = (char *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, devfb, 0);
    if(-1 == (int)fbp){
        printf("mmap error!\r\n");
        return -1;
    }
    return devfb;    
}


void fb_uninit(int devfb)
{
    //取消映射，关闭文件
    munmap(fbp, screensize);
    close(devfb);
}



int bmp_show(char *path)
{
    if(NULL == fbp || NULL == path)
        return -1;

    BitMapFileHeader FileHead;
    BitMapInfoHeader InfoHead;
    RgbQuad24 *rgb24 = NULL;
    RgbQuad32 *rgb32 = NULL;
    unsigned long tmp = 0;

    unsigned long location = 0;

    //打开.bmp文件
    FILE *fb = fopen(path, "rb");
    if (fb == NULL){
        printf("fopen bmp error\r\n");
        return -1;
    }

    //读文件信息
    if (1 != fread( &FileHead, sizeof(BitMapFileHeader),1, fb)){
        printf("read BitMapFileHeader error!\n");
        fclose(fb);
        return -1;
    }
    if (memcmp(FileHead.bfType, "BM", 2) != 0){
        printf("it's not a BMP file\n");
        fclose(fb);
        return -1;
    }

    //读位图信息
    if (1 != fread( (char *)&InfoHead, sizeof(BitMapInfoHeader),1, fb)){
        printf("read BitMapInfoHeader error!\n");
        fclose(fb);
        return -1;
    }

    //跳转至数据区
    fseek(fb, FileHead.bfOffBits, SEEK_SET);

    int len = InfoHead.biBitCount / 8;                  //原图一个像素占几字节
    int bits_len = scrinfo.bits_per_pixel / 8;          //屏幕一个像素占几字节
    int bmpsize = InfoHead.biWidth*InfoHead.biHeight;   //原图W*H

    //读取位图数据
    if(len == 3){
        rgb24 = malloc(sizeof(RgbQuad24)*bmpsize);
        if(!rgb24){
            printf("rgb24 is NULL\n"); 
            return -1;  
        }
        if (len*bmpsize != fread((char *)rgb24, 1, len*bmpsize, fb)){
            printf("read 24 RGB data error!\n");
            fclose(fb);
            return -1;
        }
    }else if(len == 4){
        rgb32 = malloc(sizeof(RgbQuad32)*bmpsize);
        if(!rgb32){
            printf("rgb32 is NULL\n"); 
            return -1;  
        }
        if (len*bmpsize != fread((char *)rgb32, 1, len*bmpsize, fb)){
            printf("read 32 RGB data error!\n");
            fclose(fb);
            return -1;
        }
    }

    //刷图(居中显示)
    //计算屏幕上要显示的区域
    int sSH = ((int)scrinfo.yres_virtual-InfoHead.biHeight)/2;  //屏幕H开始显示位置
    int sSW = ((int)scrinfo.xres_virtual-InfoHead.biWidth)/2;   //屏幕W开始显示位置
    int sEH = abs(sSH)+InfoHead.biHeight;                       //屏幕H结束显示位置
    int sEW = abs(sSW)+InfoHead.biWidth;                        //屏幕W结束显示位置
    sSH = (sSH < 0)?0:sSH;                                                  //图片H比屏幕H大，屏幕H从0开始显示,否则从sSH位置显示
    sSW = (sSW < 0)?0:sSW;                                                  //图片W比屏幕W大，屏幕H从0开始显示,否则从sSW位置显示
    sEH = (sEH < (int)scrinfo.yres_virtual)?sEH:(int)scrinfo.yres_virtual;  //图片H比屏幕H大，屏幕H从yres结束,否则从sEH位置结束
    sEW = (sEW < (int)scrinfo.xres_virtual)?sEW:(int)scrinfo.xres_virtual;  //图片W比屏幕W大，屏幕W从xres结束,否则从sEW位置结束
    //printf("sSH:%4d, sSW:%4d, sEH:%4d sEW:%4d \n", sSH, sSW, sEH, sEW);

    //计算图片上要显示的区域
    int pSH = (InfoHead.biHeight-(int)scrinfo.yres_virtual)/2;
    int pSW = (InfoHead.biWidth-(int)scrinfo.xres_virtual)/2;
    int pEH = abs(pSH)+scrinfo.yres_virtual;
    int pEW = abs(pSW)+scrinfo.xres_virtual;
    pSH = (pSH < 0)?0:pSH;  //图片H比屏幕小 
    pSW = (pSW < 0)?0:pSW;  //图片W比屏幕小
    pEH = (pEH < InfoHead.biHeight)?pEH:InfoHead.biHeight;
    pEW = (pEW < InfoHead.biWidth)?pEW:InfoHead.biWidth;
    //printf("pSH:%4d, pSW:%4d, pEH:%4d pEW:%4d\n", pSH, pSW, pEH, pEW);


    //屏幕的起始位置为:左上
    //图片的起始位置为:左下
    //计算图片上的位置
    int p = (pEH-1)*InfoHead.biWidth + pSW;       

    for(int h = 0; h < scrinfo.yres_virtual; h++){
        for(int w = 0; w < scrinfo.xres_virtual; w++){
            tmp = 0;
            location = (h*scrinfo.xres_virtual + w)*bits_len;
            if((h < sSH)||(h >= sEH)||(w < sSW)||(w >= sEW)){
                //空的地方刷黑色
                tmp |= 0XFF << 24; 
                //刷白  
                //tmp |= 0XFFFFFFFF;
            }else{
                if(p <0){
                    break; 
                }
                if(len == 3){
                    tmp |= 0XFF << 24 | rgb24[p].Red << 16 | rgb24[p].Green << 8 | rgb24[p].Blue;
                }else if(len == 4){
                    tmp |= rgb32[p].Reserved << 24 | rgb32[p].Red << 16 | rgb32[p].Green << 8 | rgb32[p].Blue;
                }
                p++;
                if((w == sEW-1)&&(h != sEH-1)){
                    p -= (InfoHead.biWidth +(sEW-sSW));
                }
            }
            *((unsigned long *)(fbp + location)) = tmp;    
        }
    }

    if(len == 3){
        free(rgb24); 
    }else if(len == 4){
        free(rgb32); 
    }
    fclose(fb);
    return 0;
}

int png_show(char *path)
{
    png_structp png_ptr;
    png_infop info_ptr;
    int color_type = 0; 
    int image_width, image_height;
    unsigned long tmp = 0;
    unsigned long location = 0;
    int bits_len = scrinfo.bits_per_pixel / 8;          //屏幕一个像素占几字节
    FILE *p_fp;

    if ((p_fp = fopen(path, "rb")) == NULL){
        fprintf(stderr, "%s open fail!!!\n", path);
        return -1;
    }

    /* 创建一个png_structp结构体，如果不想自定义错误处理，后3个参数可以传NULL */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);    
    if(png_ptr == NULL){
        fclose(p_fp);
        return -1;
    }

    /* 创建一个info_ptr */  
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL){
        fclose(p_fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return -1;
    }

    /* 如果上面png_create_read_struct没有自定义错误处理 */
    if(setjmp(png_jmpbuf(png_ptr))){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(p_fp);
        return -1;
    }

    /* 初始化文件IO */
    png_init_io(png_ptr, p_fp);

    /* 
     * 函数功能：读取png图片信息 
     * 说明：可以参数3来改变不同的读取方式，这里只读取RGB，抛弃了ALPHA
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, NULL);

    if((png_ptr != NULL) && (info_ptr != NULL)){
        /* 获取图像的宽高 */
        image_width = png_get_image_width(png_ptr, info_ptr);       
        image_height = png_get_image_height(png_ptr, info_ptr); 
        /* 获取图像颜色类型 */
        color_type = png_get_color_type(png_ptr, info_ptr); 
    }

    /* 获取所有的图像数据 */
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    /* RGB格式 */
    if(PNG_COLOR_TYPE_RGB == color_type){
        //刷图
        for(int i = 0; i < image_height; i++){
            for(int j = 0; j < (image_width * 3); j += 3){
                tmp = 0;
                location = (i*scrinfo.xres_virtual + j/3)*bits_len;
                tmp |= 0XFF << 24 | row_pointers[i][j] << 16 | row_pointers[i][j+1] << 8 | row_pointers[i][j+2];
                *((unsigned long *)(fbp + location)) = tmp; 
            }
        }
    }   

    /* 清理图像，释放内存 */
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(p_fp);    

    return 0;
}
