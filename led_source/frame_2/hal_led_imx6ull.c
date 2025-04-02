
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>

#include "hal_led_imx6ull.h"
#include "hw_led_imx6ull_wds.h"
#include "drv_led.h"



static int g_ledpins[100];
static int g_ledcnt = 0;

static int hal_led_imx6ull_led_init (int which);
static int hal_led_imx6ull_led_ctl (int which, char status);

static int hal_led_imx6ull_platform_driver_led_probe(struct platform_device *pdev);
static int hal_led_imx6ull_platform_driver_led_remove(struct platform_device *pdev);


/* 定义一个led操作对象 */
static struct hal_led_imx6ull_operations led_operations_obj = {
	.init = hal_led_imx6ull_led_init,
	.ctl  = hal_led_imx6ull_led_ctl,
};


/* 定义一个平台驱动 */
static struct platform_driver hal_led_imx6ull_platform_driver_led_obj= {
    .probe      = hal_led_imx6ull_platform_driver_led_probe,
    .remove     = hal_led_imx6ull_platform_driver_led_remove,
    .driver     = {
        .name   = "100ask_led",
    },
};

/* led初始化操作 */
static int hal_led_imx6ull_led_init (int which){	

	switch(HW_LED_IMX6ULL_WDS_GROUP(g_ledpins[which]))
	{
		case 0:
		{
			printk("init pin of group 0 ...\n");
			break;
		}
		case 1:
		{
			printk("init pin of group 1 ...\n");
			break;
		}
		case 2:
		{
			printk("init pin of group 2 ...\n");
			break;
		}
		case 3:
		{
			printk("init pin of group 3 ...\n");
			break;
		}
	}
	
	return 0;
}

/* led控制操作 */
static int hal_led_imx6ull_led_ctl (int which, char status) {
    
	switch(HW_LED_IMX6ULL_WDS_GROUP(g_ledpins[which]))
	{
		case 0:
		{
			printk("set pin of group 0 ...\n");
			break;
		}
		case 1:
		{
			printk("set pin of group 1 ...\n");
			break;
		}
		case 2:
		{
			printk("set pin of group 2 ...\n");
			break;
		}
		case 3:
		{
			printk("set pin of group 3 ...\n");
			break;
		}
	}

	return 0;
}

/* 平台设备的proe成员函数，用于获取资源创建设备 */
static int hal_led_imx6ull_platform_driver_led_probe(struct platform_device *pdev)
{
    struct resource *res_ptr;
    int i = 0;

    while (1)
    {
        /* 获取资源 --->根据名字匹配后可以得到平台设备的资源*/
        res_ptr = platform_get_resource(pdev, IORESOURCE_IRQ, i++);
        if (!res_ptr)
            break;
        /* 保存资源的起始地址 */
        g_ledpins[g_ledcnt] = res_ptr->start;

        /*  leddrv.c中定义的,根据资源创建设备，参数次设备号 */
        drv_led_class_create_device(g_ledcnt);
        g_ledcnt++;
    }
    return 0;
    
}

/* 平台设备的remove成员函数，用于获取资源销毁设备 */
static int hal_led_imx6ull_platform_driver_led_remove(struct platform_device *pdev)
{
    struct resource *res;
    int i = 0;

    while (1)
    {
        /* 获取资源 */
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if (!res)
            break;
		
        /* leddrv.c中定义的,根据资源销毁设备，参数次设备号 */
        drv_led_class_destroy_device(i);
        i++;
        g_ledcnt--;
    }
    return 0;
}

/*  led操作对象获取函数，供外部调用 */
struct hal_led_imx6ull_operations* hal_led_imx6ull_get_led_operations_obj(void){

	return &led_operations_obj;
}

/* 模块加载 */
static int __init hal_led_imx6ull_init(void)
{
    int err;
    /* 注册一个驱动 */
    err = platform_driver_register(&hal_led_imx6ull_platform_driver_led_obj); 

    /* leddrv.c中定义的，将led的init和ctrl接口函数传递给leddrv，有下层调用上层的路径！ */
    drv_led_get_imx6ull_operations(&led_operations_obj);
    
    return 0;
}

/* 模块卸载 */
static void __exit hal_led_imx6ull_exit(void)
{
    /* 注销一个驱动 */
    platform_driver_unregister(&hal_led_imx6ull_platform_driver_led_obj);
}

module_init(hal_led_imx6ull_init);
module_exit(hal_led_imx6ull_exit);

MODULE_LICENSE("GPL");
