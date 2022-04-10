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

//gpio driver
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>

//platform
#include <linux/platform_device.h>

//misc
#include <linux/miscdevice.h>

// #define NEW_LED_DEV_CNT   1
// #define NEW_LED_DEV_NAME "platform_led"  //决定了 /dev/led 的名字
#define MISCBEEP_MINOR 30

struct miscbeep_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    
    int beep_gpio;
};

struct miscbeep_dev miscbeep;



static int beep_open(struct inode *inode, struct file *filp) {

    filp->private_data = &miscbeep; /* 设置私有数据 */
    return 0;
}

static ssize_t beep_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos) {

    int ret = 0;
    struct miscbeep_dev *dev = filp->private_data;
    unsigned char beepbuf[1];
    beepbuf[0] = 0;

    ret = copy_from_user(beepbuf, buf, count);

    if(beepbuf[0] == 1) {
        gpio_set_value(dev->beep_gpio, 1);    
    }
    else {
        gpio_set_value(dev->beep_gpio, 0);   
    }
    

    return 0;
}    

static int beep_release(struct inode *inode, struct file *filp) {

    return 0;
}



static const struct file_operations miscbeep_fops = {
	.owner	= THIS_MODULE,
    .open	= beep_open,
	.write	= beep_write,
	.release= beep_release,
};


static struct miscdevice beep_miscdev = {
    .minor = MISCBEEP_MINOR,
    .name = "beep_misc_dev",
    .fops = &miscbeep_fops,
};

struct of_device_id	of_match_table[] = {
    {.compatible =  "atkalpha-beep"},
    {/*sentinel*/}
};

//上面的of_match_table 要与设备树的接节点下的compatible = "atkalpha-gpiobeep"的匹配成功下面的函数才会执行
int beep_probe(struct platform_device *platform_device_beep)
{
    int ret = 0;

    printk("beep_probe is running!\r\n");
    

    //1.获取设备节点
//    miscbeep.nd = platform_device_beep->dev.of_node;
    miscbeep.nd = of_find_node_by_path("/beep");
    if(miscbeep.nd == NULL) {
        printk("miscbeep node can not found!\r\n");
        return -1;
    }
    else {
        printk("miscbeep node has been found!\r\n");
    }
    //2.获取设备树中的gpio属性，得到beep所使用的beep编号
    miscbeep.beep_gpio = of_get_named_gpio(miscbeep.nd, "beep-gpio", 0);
    if(miscbeep.beep_gpio < 0) {
        printk("can not get beep-gpio");
        return -1;
    }
    printk("beep-gpio num = %d\r\n", miscbeep.beep_gpio);
    //3.用于申请一个 GPIO 管脚，可以防止IO冲突     "beep-gpio"名字是随便起的
    ret = gpio_request(miscbeep.beep_gpio, "beep-gpio");
    if(ret) {
        printk("Failed to request the beep gpio\r\n");
    }
    //4.设置GPIO1_IO03为输出，并且输出高电平，默认关闭beep灯
    ret = gpio_direction_output(miscbeep.beep_gpio, 1);
    if(ret < 0) {
        printk("can not set gpio!\r\n");
    }

    gpio_set_value(miscbeep.beep_gpio, 0);
    
//----------------------------------misc杂项设备驱动beep--------------------------------------
    
    misc_register(&beep_miscdev);
    return 0;
}
int beep_remove(struct platform_device *platform_device_led)
{
    printk("led_remove is running!\r\n");
    gpio_set_value(miscbeep.beep_gpio, 1);
    //注销字符设备
    misc_deregister(&beep_miscdev);
    return 0;
}
static struct platform_driver beep_driver = {
	.driver		= {                                             
		.name	= "imx6ull_beep_misc_platform", //驱动的名字成功加载之后可以在/sys/bus/platform中看到
        .of_match_table = of_match_table,
	},

    .probe		= beep_probe,
	.remove		= beep_remove,

};
static int __init beep_misc_driver_init(void) 
{
    platform_driver_register(&beep_driver);
    return 0;
}
static void __exit beep_misc_driver_exit(void)
{
    platform_driver_unregister(&beep_driver);
}

module_init(beep_misc_driver_init);
module_exit(beep_misc_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");


