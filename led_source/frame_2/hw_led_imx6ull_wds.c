
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

#include "hw_led_imx6ull_wds.h"


static void hw_led_imx6ull_wds_release(struct device *dev);

/* 定义平台设备资源 */
static struct resource platform_device_led_obj_resources_obj[] = {
	{
			.start = HW_LED_IMX6ULL_WDS_GROUP_PIN(3,1),
			.flags = IORESOURCE_IRQ,
			.name = "100ask_led_pin",
	},
	{
			.start = HW_LED_IMX6ULL_WDS_GROUP_PIN(5,8),
			.flags = IORESOURCE_IRQ,
			.name = "100ask_led_pin",
	},
};


/* 定义平台设备 */
static struct platform_device platform_device_led_obj = {
	.name = "100ask_led",
	.num_resources = ARRAY_SIZE(platform_device_led_obj_resources_obj),
	.resource = platform_device_led_obj_resources_obj,/* 设备资源 */
	.dev = {
			.release = hw_led_imx6ull_wds_release,/* 设备注销时的清理函数 */
	 },
};


static void hw_led_imx6ull_wds_release(struct device *dev)
{

}

/* 模块加载 */
static int __init hw_led_imx6ull_wds_platform_device_init(void)
{
    int err;
    /* 注册一个设备 */
    err = platform_device_register(&platform_device_led_obj);   
    
    return 0;
}

/* 模块卸载 */
static void __exit hw_led_imx6ull_wds_platform_device_exit(void)
{
    /* 注销一个设备 */    
    platform_device_unregister(&platform_device_led_obj);
}

module_init(hw_led_imx6ull_wds_platform_device_init);
module_exit(hw_led_imx6ull_wds_platform_device_exit);
MODULE_LICENSE("GPL");



