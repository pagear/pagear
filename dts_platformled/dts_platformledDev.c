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

#define NEW_LED_DEV_CNT   1
#define NEW_LED_DEV_NAME "platform_led"  //决定了 /dev/led 的名字


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


// struct platform_driver {
// 	int (*probe)(struct platform_device *);
// 	int (*remove)(struct platform_device *);
// 	void (*shutdown)(struct platform_device *);
// 	int (*suspend)(struct platform_device *, pm_message_t state);
// 	int (*resume)(struct platform_device *);
// 	struct device_driver driver;
// 	const struct platform_device_id *id_table;
// 	bool prevent_deferred_probe;
// };
// struct device_driver {
// 	const char		*name;
// 	struct bus_type		*bus;

// 	struct module		*owner;
// 	const char		*mod_name;	/* used for built-in modules */

// 	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */

// 	const struct of_device_id	*of_match_table;
// 	const struct acpi_device_id	*acpi_match_table;

// 	int (*probe) (struct device *dev);
// 	int (*remove) (struct device *dev);
// 	void (*shutdown) (struct device *dev);
// 	int (*suspend) (struct device *dev, pm_message_t state);
// 	int (*resume) (struct device *dev);
// 	const struct attribute_group **groups;

// 	const struct dev_pm_ops *pm;

// 	struct driver_private *p;
// };
// struct of_device_id {
// 	char	name[32];
// 	char	type[32];
// 	char	compatible[128];
// 	const void *data;
// };


struct of_device_id	of_match_table[] = {
    {.compatible = "atkalpha-gpioled"},
    {/*sentinel*/}
};

//上面的of_match_table 要与设备树的接节点下的compatible = "atkalpha-gpioled"的匹配成功下面的函数才会执行
int led_probe(struct platform_device *platform_device_led)
{
    printk("led_probe is running!\r\n");
    int ret = 0;

    //1.获取设备节点
    gpioled.nd = platform_device_led->dev.of_node;
//    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL) {
        printk("gpioled node can not found!\r\n");
        return -1;
    }
    else {
        printk("gpioled node has been found!\r\n");
    }
    //2.获取设备树中的gpio属性，得到LED所使用的LED编号
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
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
    return 0;
}
int led_remove(struct platform_device *platform_device_led)
{
    printk("led_remove is running!\r\n");
    gpio_set_value(gpioled.led_gpio, 1);
    //注销字符设备
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, NEW_LED_DEV_CNT);

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);

    gpio_free(gpioled.led_gpio);
    return 0;
}
static struct platform_driver led_driver = {
	.driver		= {                                             
		.name	= "imx6ull_led_platform", //驱动的名字成功加载之后可以在/sys/bus/platform中看到
        .of_match_table = of_match_table,
	},

    .probe		= led_probe,
	.remove		= led_remove,

};
static int __init leddriver_init(void) 
{
    platform_driver_register(&led_driver);
    return 0;
}
static void __exit leddriver_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");


