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

#define NEW_LED_DEV_CNT   1
#define NEW_LED_DEV_NAME "led"


#define CCM_CCGR0 (0X020C406C)

#define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03  (0x020E0068)    //multi

#define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03  (0x020E02F4)

#define GPIO1_DR                          (0x0209C000)
#define GPIO1_GDIR                        (0x0209C004)

static void __iomem *ccm_ccgr0 = NULL;
static void __iomem *mux_gpio1_3 = NULL;
static void __iomem *pad_gpio1_3 = NULL;
static void __iomem *gpio_dr = NULL;
static void __iomem *gpio_gdir = NULL;


struct newchrled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    
};

struct newchrled_dev newchrled;




static void led_switch(char val)
{
    unsigned int reg_val = 0;

    if(val == 0) {
        reg_val = 0;
        reg_val &= (1<<3);
        writel(reg_val, gpio_dr);
    }
    else if(val ==1) {
        reg_val = 0;
        reg_val |= (1<<3);
        writel(reg_val, gpio_dr);
    }
}

static int led_open(struct inode *inode, struct file *filp) {

    filp->private_data = &newchrled; /* 设置私有数据 */
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos) {

    int ret = 0;
    unsigned char ledbuf[1];
    ledbuf[0] = 0;

    ret = copy_from_user(ledbuf, buf, count);

    
    if(ret == 0) {
        printk("kernel recevdata success!\r\n");
    }
    else {
        printk("kernel recevdata failed!\r\n");
    }

    led_switch(ledbuf[0]);

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
    //int ret = 0;
    unsigned int reg_val = 0;

	// ret = register_chrdev(LED_DEV_MAJOR, LED_DEV_NAME, &led_fops);
	// if (ret < 0) {
	// 	printk(LED_DEV_NAME": could not get major number\n");
	// 	return ret;
	// }

    ccm_ccgr0 = ioremap(CCM_CCGR0, 4);
    mux_gpio1_3 = ioremap(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03, 4);
    pad_gpio1_3 = ioremap(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03, 4);
    gpio_dr = ioremap(GPIO1_DR, 4);
    gpio_gdir = ioremap(GPIO1_GDIR, 4);

    writel(0xffffffff, ccm_ccgr0);
    writel(0x5, mux_gpio1_3);
    writel(0X10B0, pad_gpio1_3);


    reg_val |= (1<<3);
    writel(reg_val, gpio_gdir);

    reg_val = 0;
    reg_val &= (1<<3);
    writel(reg_val, gpio_dr);

//----------------------------------新字符设备驱动led--------------------------------------
    
    //注册字符设备
    if(newchrled.major) {
        newchrled.devid = MKDEV(newchrled.major, 0);
        register_chrdev_region(newchrled.devid, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
    } 
    else {
        alloc_chrdev_region(&newchrled.devid, 0, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }
    printk("newchrled major = %d, minor = %d\r\n", newchrled.major, newchrled.minor);
    
    //初始化cdev
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);

    //添加一个cdev
    cdev_add(&newchrled.cdev, newchrled.devid, NEW_LED_DEV_CNT);

    //创建类
    newchrled.class = class_create(THIS_MODULE, NEW_LED_DEV_NAME);
    if(IS_ERR(newchrled.class)) {
        return PTR_ERR(newchrled.class);
    }

    //创建设备
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEW_LED_DEV_NAME);
    if(IS_ERR(newchrled.device)) {
        return PTR_ERR(newchrled.device);
    }

//----------------------------------新字符设备驱动led--------------------------------------

    return 0;
}
static void __exit led_exit(void)
{
    unsigned int reg_val = 0;
    reg_val = 0;
    reg_val |= (1<<3);
    writel(reg_val, gpio_dr);

    //unregister_chrdev(LED_DEV_MAJOR, LED_DEV_NAME);
    iounmap(ccm_ccgr0);
    iounmap(mux_gpio1_3);
    iounmap(pad_gpio1_3);
    iounmap(gpio_dr);
    iounmap(gpio_gdir);

    //注销字符设备
    cdev_del(&newchrled.cdev);
    unregister_chrdev_region(newchrled.devid, NEW_LED_DEV_CNT);

    device_destroy(newchrled.class, newchrled.devid);
    class_destroy(newchrled.class);
    
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");


