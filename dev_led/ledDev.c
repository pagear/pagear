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

#define LED_DEV_MAJOR 200
#define LED_DEV_NAME "led"


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



static const struct file_operations led_fops = {
	.owner	= THIS_MODULE,
    .open	= led_open,
	.write	= led_write,
	.release= led_release,
};


static int __init led_init(void)
{
    int ret = 0;
    unsigned int reg_val = 0;

	ret = register_chrdev(LED_DEV_MAJOR, LED_DEV_NAME, &led_fops);
	if (ret < 0) {
		printk(LED_DEV_NAME": could not get major number\n");
		return ret;
	}

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
    
    
    return 0;
}
static void __exit led_exit(void)
{
    unsigned int reg_val = 0;
    reg_val = 0;
    reg_val |= (1<<3);
    writel(reg_val, gpio_dr);

    unregister_chrdev(LED_DEV_MAJOR, LED_DEV_NAME);
    iounmap(ccm_ccgr0);
    iounmap(mux_gpio1_3);
    iounmap(pad_gpio1_3);
    iounmap(gpio_dr);
    iounmap(gpio_gdir);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HUANGYE");



// void led_on(void) // 0
// {
//     GPIO1->GPIO_DR &= ~(1<<3);
// }

// void led_off(void) // 1
// {
//     GPIO1->GPIO_DR |= (1<<3);
// }

