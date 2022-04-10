/*************************************************************************
	> File Name: chrdevbase.c
	> Author: amoscykl
	> Mail: amoscykl@163.com 
	> Created Time: 2022年03月07日 星期一 20时41分44秒
 ************************************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define CHRDEVBASE_MAJOR 200 //主设备号
#define CHRDEVBASE_NAME "chrdevbase" //设备名

static char readbuf[100];
static char writebuf[100];
static char kerneldata[] = {"kernel data!"};


static struct file_operations chrdevbase_fops;

static int chrdevbase_open(struct inode *inode, struct file *filp)
{
    //用戶具体功能
    return 0;
}

static ssize_t chrdevbase_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue = 0;                
     //用戶具体功能
    memcpy(readbuf, kerneldata, sizeof(kerneldata));
    retvalue = copy_to_user(buf, readbuf, cnt);
    if(retvalue == 0) {
        printk("kernel senddate ok!\r\n");
    }
    else {
        printk("kernel senddata failed!\r\n");
    }
    return 0;   
}

static ssize_t chrdevbase_write(struct file *file, const char __user *buf,size_t cnt, loff_t *offt)
{
    int retvalue = 0;
    retvalue = copy_from_user(writebuf, buf, cnt);
    if(retvalue == 0) {
        printk("kernel recevdata:%s\r\n", writebuf);
    }
    else {
        printk("kernel recevdata failed!\r\n");
    }
     //用戶具体功能
    return 0;   
}
  
static int chrdevbase_release(struct inode *inode, struct file *filp)
{
    //用戶具体功能
    return 0;    
}

static struct file_operations chrdevbase_fops = {
	.owner          = THIS_MODULE,

	.open           = chrdevbase_open,
    .read 			= chrdevbase_read,
	.write          = chrdevbase_write,
	.release        = chrdevbase_release,
};

//驱动入口函数
static int __init chrdevbase_init(void)
{
    int retvalue = 0;
    
    retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(retvalue < 0) {
        //字符设备注册失败，自行处理
        printk("chrdevbase driver register failed\r\n");
    }
    printk("chrdevbase_init()\r\n");
    return 0;
}

//驱动出口函数
static void __exit chrdevbase_exit(void)
{
    //注销字符设别驱动
	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase_exit()\r\n");
}
    

//指定为驱动的入口函数和出口函数
module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

//添加LICENSE和作者信息
MODULE_LICENSE("GPL");  //GPL协议
MODULE_AUTHOR("HUANG_YE"); //作者

// #include <linux/types.h>
// #include <linux/kernel.h>
// #include <linux/delay.h>

// #include </home/ye/individual/linux_imx6u/linux/kernel/alientek_linux/include/linux/ide.h>

// #include <linux/init.h>
// #include <linux/module.h>
// /***************************************************************
// Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
// 文件名		: chrdevbase.c
// 作者	  	: 左忠凯
// 版本	   	: V1.0
// 描述	   	: chrdevbase驱动文件。
// 其他	   	: 无
// 论坛 	   	: www.openedv.com
// 日志	   	: 初版V1.0 2019/1/30 左忠凯创建
// ***************************************************************/

// #define CHRDEVBASE_MAJOR	200				/* 主设备号 */
// #define CHRDEVBASE_NAME		"chrdevbase" 	/* 设备名     */

// static char readbuf[100];		/* 读缓冲区 */
// static char writebuf[100];		/* 写缓冲区 */
// static char kerneldata[] = {"kernel data!"};

// /*
//  * @description		: 打开设备
//  * @param - inode 	: 传递给驱动的inode
//  * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
//  * 					  一般在open的时候将private_data指向设备结构体。
//  * @return 			: 0 成功;其他 失败
//  */
// static int chrdevbase_open(struct inode *inode, struct file *filp)
// {
// 	//printk("chrdevbase open!\r\n");
// 	return 0;
// }

// /*
//  * @description		: 从设备读取数据 
//  * @param - filp 	: 要打开的设备文件(文件描述符)
//  * @param - buf 	: 返回给用户空间的数据缓冲区
//  * @param - cnt 	: 要读取的数据长度
//  * @param - offt 	: 相对于文件首地址的偏移
//  * @return 			: 读取的字节数，如果为负值，表示读取失败
//  */
// static ssize_t chrdevbase_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
// {
// 	int retvalue = 0;
	
// 	/* 向用户空间发送数据 */
// 	memcpy(readbuf, kerneldata, sizeof(kerneldata));
// 	retvalue = copy_to_user(buf, readbuf, cnt);
// 	if(retvalue == 0){
// 		printk("kernel senddata ok!\r\n");
// 	}else{
// 		printk("kernel senddata failed!\r\n");
// 	}
	
// 	//printk("chrdevbase read!\r\n");
// 	return 0;
// }

// /*
//  * @description		: 向设备写数据 
//  * @param - filp 	: 设备文件，表示打开的文件描述符
//  * @param - buf 	: 要写给设备写入的数据
//  * @param - cnt 	: 要写入的数据长度
//  * @param - offt 	: 相对于文件首地址的偏移
//  * @return 			: 写入的字节数，如果为负值，表示写入失败
//  */
// static ssize_t chrdevbase_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
// {
// 	int retvalue = 0;
// 	/* 接收用户空间传递给内核的数据并且打印出来 */
// 	retvalue = copy_from_user(writebuf, buf, cnt);
// 	if(retvalue == 0){
// 		printk("kernel recevdata:%s\r\n", writebuf);
// 	}else{
// 		printk("kernel recevdata failed!\r\n");
// 	}
	
// 	//printk("chrdevbase write!\r\n");
// 	return 0;
// }

// /*
//  * @description		: 关闭/释放设备
//  * @param - filp 	: 要关闭的设备文件(文件描述符)
//  * @return 			: 0 成功;其他 失败
//  */
// static int chrdevbase_release(struct inode *inode, struct file *filp)
// {
// 	//printk("chrdevbase release！\r\n");
// 	return 0;
// }

// /*
//  * 设备操作函数结构体
//  */
// static struct file_operations chrdevbase_fops = {
// 	.owner = THIS_MODULE,	
// 	.open = chrdevbase_open,
// 	.read = chrdevbase_read,
// 	.write = chrdevbase_write,
// 	.release = chrdevbase_release,
// };

// /*
//  * @description	: 驱动入口函数 
//  * @param 		: 无
//  * @return 		: 0 成功;其他 失败
//  */
// static int __init chrdevbase_init(void)
// {
// 	int retvalue = 0;

// 	/* 注册字符设备驱动 */
// 	retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
// 	if(retvalue < 0){
// 		printk("chrdevbase driver register failed\r\n");
// 	}
// 	printk("chrdevbase init!\r\n");
// 	return 0;
// }

// /*
//  * @description	: 驱动出口函数
//  * @param 		: 无
//  * @return 		: 无
//  */
// static void __exit chrdevbase_exit(void)
// {
// 	/* 注销字符设备驱动 */
// 	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
// 	printk("chrdevbase exit!\r\n");
// }

// /* 
//  * 将上面两个函数指定为驱动的入口和出口函数 
//  */
// module_init(chrdevbase_init);
// module_exit(chrdevbase_exit);

// /* 
//  * LICENSE和作者信息
//  */
// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("zuozhongkai");

