#include <stdio.h> 
#include <stdlib.h> 
#include "logo.h"

#define SYSVINIT_PATH 	"/sbin/init.sysvinit"
#define SYSVINIT_PARAM 	"init.sysvinit"

#define DEV_FB 		    "/dev/fb0"                              //设备节点
#define LOGO_PATH 		"/usr/app/hsae-reverse-ctrl/bootlogo"   //logo路径
#define LOGO_COUNT 		15                                      //logo数量


//启动文件系统
int execl_sysvinit()
{
	if(execl(SYSVINIT_PATH,SYSVINIT_PARAM,NULL) < 0){
        perror("execl error!");
    }
    return 0;
}


int main()
{
#if 1
    printf("logo +++\n");
    char path[100];
    //char buff[256];
    int devfb = fb_init(DEV_FB);
	if(devfb != -1){
        #if 1
		for(int i = 0; i < LOGO_COUNT; i++){
			memset(path, 0, sizeof(path));
			sprintf(path,"%s/DA-1280-10fps_000%02d.bmp", LOGO_PATH, i);
			bmp_show(path);
            //png_show(path);
            //fgets(buff, 255, stdin);

		}
        #endif
		fb_uninit(devfb);
	}
    printf("logo ---\n");
    #endif
    execl_sysvinit();
    return 0;
}

