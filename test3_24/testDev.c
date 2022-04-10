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


#define DEV_CNT 1
#define DEV_NAME "led"

static struct gpioled {

    struct device_node *nd;
    dev_t dev_id;
    struct cdev cdev;
    struct class *class;
    struct device *device;

    int major;
    int minor;

    int led_gpio; 
} gpioled;


static int led_open(struct inode *inode, struct file *filp)
{
    //filp->private_data = &gpioled;
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf,
			size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
    int ret = 0;
    char ledbuf[1] = {0};
    //struct gpioled *dev = filp->private_data;
    ret = copy_from_user(ledbuf, buf, count);
    if(ret) {
        printk("failed to copy from user!\r\n");
    }
    if(ledbuf[0] == 1) {
        gpio_set_value(gpioled.led_gpio, 1);
    }
    else {
        gpio_set_value(gpioled.led_gpio, 0);
    }

    return 0;
}

static int led_close(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations gpioled_fops = {
	.owner		= THIS_MODULE,
	.read		= led_read,
	.write		= led_write,
	.open		= led_open,
	.release	= led_close,

};


static int __init led_init(void)
{
    int ret = 0;

    gpioled.nd = of_find_node_by_path("/gpioled");
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    ret = gpio_request(gpioled.led_gpio, "gpio_led");
    if(ret) {
        printk("failed to request gpio!\r\n");
    }
    gpio_direction_output(gpioled.led_gpio, 1);
    gpio_set_value(gpioled.led_gpio, 0);


    //1.自动获取设备号
    ret = alloc_chrdev_region(&gpioled.dev_id, 0, DEV_CNT, DEV_NAME);
	if (ret) {
        printk("failed to get dev_id\r\n");
    }

	gpioled.major = MAJOR(gpioled.dev_id);
    gpioled.minor = MINOR(gpioled.dev_id);
    printk("major = %d, minor = %d\r\n", gpioled.major, gpioled.minor);
    
    //2.创建设备
    cdev_init(&gpioled.cdev, &gpioled_fops);
	ret = cdev_add(&gpioled.cdev, gpioled.dev_id, DEV_CNT);
	if (ret) {
        printk("failed to create dev\r\n");
    }

    //3.自动挂载设备
    gpioled.class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(gpioled.class)) {
		ret = PTR_ERR(gpioled.class);
        printk("failed to create class\r\n");
	}
    gpioled.device = device_create(gpioled.class, NULL, gpioled.dev_id, NULL, DEV_NAME);
	if (IS_ERR(gpioled.device)) {
		ret = PTR_ERR(gpioled.device);
		printk("failed to create device\r\n");
	}

    return 0;
}

static void __exit led_exit(void)
{
    gpio_set_value(gpioled.led_gpio, 1);
    

    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.dev_id, DEV_CNT);//注意顺序否则卸载驱动会报错

    device_destroy(gpioled.class, gpioled.dev_id);//注意两个顺序否则卸载驱动会报错
    class_destroy(gpioled.class);

    gpio_free(gpioled.led_gpio);
}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("huangye");
MODULE_LICENSE("GPL");

