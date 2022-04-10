/*************************************************************************
	> File Name: chrdevbaseApp.c
	> Author: amoscykl
	> Mail: amoscykl@163.com 
	> Created Time: 2022年03月07日 星期一 20时46分37秒
 ************************************************************************/
/*
* @description : main 主程序
* @param - argc : argv 数组元素个数
* @param - argv : 具体参数
* @return
*/

//open function
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//read write
#include <unistd.h>

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

static char usrdata[] = {"usr data!"};

int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    char readbuf[100], writebuf[100];
    
    if(argc != 3) {
        printf("Error Usage!\r\n");
        return -1;
    }
    
    filename = argv[1]; //第一个参数：文件名称
    
    //打开驱动文件
    fd = open(filename, O_RDWR);
    if(fd < 0) {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }
    
    if(atoi(argv[2]) == 1) {
        retvalue = read(fd, readbuf, 50); //从fd文件读取50个数据保存到readbuf里
        if(retvalue < 0) {
            printf("read file %s failed!\r\n", filename);
        }
        else {
            //读取成功，打印出读取成功的数据
            printf("read data:%s\r\n", readbuf);
        }
    }
    else if(atoi(argv[2]) == 2) {
        //向设备驱动写数据
        memcpy(writebuf, usrdata, sizeof(usrdata));
        retvalue = write(fd, writebuf, 50);
        if(retvalue < 0) {
            printf("write file %s failed!\r\n", filename);
        }
    }
    
   	//关闭设备
    retvalue = close(fd);
    if(retvalue < 0) {
        printf("Can't close file %s\r\n", filename);
        return -1;
    }
    
    return 0;
}
