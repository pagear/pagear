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
#include <asm/io.h>

//lednew
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>

//dts dev_tree
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

//
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>

#define NEW_LED_DEV_CNT   1
#define NEW_LED_DEV_NAME "beep"  //决定了 /dev/led 的名字


struct gpioled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    
    int led_gpio;
};

struct gpioled_dev gpioled;


static int led_open(struct inode *inode, struct file *filp) {

    filp->private_data = &gpioled; /* 设置私有数据 */
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos) {

    int ret = 0;
    struct gpioled_dev *dev = filp->private_data;
    unsigned char ledbuf[1];
    ledbuf[0] = 0;

    ret = copy_from_user(ledbuf, buf, count);

    if(ledbuf[0] == 1) {
        gpio_set_value(dev->led_gpio, 1);    
    }
    else {
        gpio_set_value(dev->led_gpio, 0);   
    }
    

    return 0;
}    

static int led_release(struct inode *inode, struct file *filp) {

    return 0;
}



static const struct file_operations newchrled_fops = {
	.owner	= THIS_MODULE,
    .open	= led_open,
	.write	= led_write,
	.release= led_release,
};


static int __init led_init(void)
{

    int ret = 0;

    //1.获取设备节点
    gpioled.nd = of_find_node_by_path("/beep");
    if(gpioled.nd == NULL) {
        printk("gpioled node can not found!\r\n");
        return -1;
    }
    else {
        printk("gpioled node has been found!\r\n");
    }
    //2.获取设备树中的gpio属性，得到LED所使用的LED编号   "beep-gpio保存了编号信息"
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "beep-gpio", 0);
    if(gpioled.led_gpio < 0) {
        printk("can not get led-gpio");
        return -1;
    }
    printk("led-gpio num = %d\r\n", gpioled.led_gpio);
    //3.用于申请一个 GPIO 管脚，可以防止IO冲突     "led-gpio"名字是随便起的
    ret = gpio_request(gpioled.led_gpio, "led-gpio");
    if(ret) {
        printk("Failed to request the led gpio\r\n");
    }
    //4.设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0) {
        printk("can not set gpio!\r\n");
    }

    gpio_set_value(gpioled.led_gpio, 0);
    
//----------------------------------新字符设备驱动led--------------------------------------
    
    //注册字符设备号
    if(gpioled.major) {
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
    } 
    else {
        alloc_chrdev_region(&gpioled.devid, 0, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    printk("gpioled major = %d, minor = %d\r\n", gpioled.major, gpioled.minor);
    

    //注册字符设备
    //初始化cdev
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &newchrled_fops);
    //添加一个cdev
    cdev_add(&gpioled.cdev, gpioled.devid, NEW_LED_DEV_CNT);


    //自动创建设备节点
    //创建类
    gpioled.class = class_create(THIS_MODULE, NEW_LED_DEV_NAME);
    if(IS_ERR(gpioled.class)) {
        return PTR_ERR(gpioled.class);
    }
    //创建设备
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, NEW_LED_DEV_NAME);
    if(IS_ERR(gpioled.device)) {
        return PTR_ERR(gpioled.device);
    }

//----------------------------------新字符设备驱动led--------------------------------------

    return 0;
}
static void __exit led_exit(void)
{

    gpio_set_value(gpioled.led_gpio, 1);
    //注销字符设备
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, NEW_LED_DEV_CNT);

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);

    gpio_free(gpioled.led_gpio);
    
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");


