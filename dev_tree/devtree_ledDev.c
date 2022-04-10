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


struct dtsled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    
};

struct dtsled_dev dtsled;




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

    filp->private_data = &dtsled; /* 设置私有数据 */
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

    //devtree led
    int ret = 0;
    u32 regdata[14] = {0};
    const char *str = NULL;
    struct property *proper = NULL;
    //devtree led

    //old leddev
	// ret = register_chrdev(LED_DEV_MAJOR, LED_DEV_NAME, &led_fops);
	// if (ret < 0) {
	// 	printk(LED_DEV_NAME": could not get major number\n");
	// 	return ret;
	// }


    //dev tree
    //1.获取设备节点：alphaled
    dtsled.nd = of_find_node_by_path("/alphaled");
    if(dtsled.nd == NULL) {
        printk("alphaled node can not found!\r\n");
    }
    //2.提取属性
    proper = of_find_property(dtsled.nd, "compatible", NULL);
    if(NULL == proper) {
        printk("find property failed!\r\n");
    }
    else {
        printk("compatible value = %s\r\n", (char *)proper->value);
    }
    //3.读取字符串值
    ret = of_property_read_string(dtsled.nd, "status", &str);
    if(0 == ret) {
        printk("read string success!\r\n");
    }
    else {
        printk("read string failed!\r\n");
    }
    //4.读取u32类型的数组数据
    ret = of_property_read_u32_array(dtsled.nd, "reg", regdata, 10);
    if(ret != 0) {
        printk("read array failed!\r\n");
    }
    else {
        int i = 0;
        for(i = 0; i < 8; i++) {
            printk("regdata[%d] = %d\r\n", i, regdata[i]);
        }
    }

#if 1
    ccm_ccgr0 = of_iomap(dtsled.nd, 0);
    mux_gpio1_3 = of_iomap(dtsled.nd, 1);
    pad_gpio1_3 = of_iomap(dtsled.nd, 2);
    gpio_dr = of_iomap(dtsled.nd, 3);
    gpio_gdir = of_iomap(dtsled.nd, 4);
#else
    ccm_ccgr0 = of_iomap(regdata[0], regdata[1]);
    mux_gpio1_3 = of_iomap(regdata[2], regdata[3]);
    pad_gpio1_3 = of_iomap(regdata[4], regdata[5]);
    gpio_dr = of_iomap(regdata[6], regdata[7]);
    gpio_gdir = of_iomap(regdata[8], regdata[9]);
#endif


    //与newled相比，dtsled将寄存器的值写到了设备树里，然后出设备树中读取
    // ccm_ccgr0 = ioremap(CCM_CCGR0, 4);
    // mux_gpio1_3 = ioremap(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03, 4);
    // pad_gpio1_3 = ioremap(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03, 4);
    // gpio_dr = ioremap(GPIO1_DR, 4);
    // gpio_gdir = ioremap(GPIO1_GDIR, 4);

    writel(0xffffffff, ccm_ccgr0);
    writel(0x5, mux_gpio1_3);
    writel(0X10B0, pad_gpio1_3);


    reg_val |= (1<<3);
    writel(reg_val, gpio_gdir);

    reg_val = 0;
    reg_val &= (1<<3);
    writel(reg_val, gpio_dr);

//----------------------------------新字符设备驱动led--------------------------------------
    
    //注册字符设备号
    if(dtsled.major) {
        dtsled.devid = MKDEV(dtsled.major, 0);
        register_chrdev_region(dtsled.devid, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
    } 
    else {
        alloc_chrdev_region(&dtsled.devid, 0, NEW_LED_DEV_CNT, NEW_LED_DEV_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }
    printk("dtsled major = %d, minor = %d\r\n", dtsled.major, dtsled.minor);
    

    //注册字符设备
    //初始化cdev
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &newchrled_fops);
    //添加一个cdev
    cdev_add(&dtsled.cdev, dtsled.devid, NEW_LED_DEV_CNT);


    //自动创建设备节点
    //创建类
    dtsled.class = class_create(THIS_MODULE, NEW_LED_DEV_NAME);
    if(IS_ERR(dtsled.class)) {
        return PTR_ERR(dtsled.class);
    }
    //创建设备
    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, NEW_LED_DEV_NAME);
    if(IS_ERR(dtsled.device)) {
        return PTR_ERR(dtsled.device);
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
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, NEW_LED_DEV_CNT);

    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
    
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");


