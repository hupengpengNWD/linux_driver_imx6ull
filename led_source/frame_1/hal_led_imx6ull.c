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


#include "hal_led_imx6ull.h"
#include "hw_led_imx6ull_wds.h"


static struct hw_led_imx6ull_wds_resource*  resource_ptr;

static int hal_led_imx6ull_init (int which){	

	if (!resource_ptr){
		resource_ptr = hw_led_imx6ull_wds_get_resouce();
	}

	switch(HW_LED_IMX6ULL_WDS_GROUP(resource_ptr->gpio_pad))
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

static int hal_led_imx6ull_ctl (int which, char status) {
    
	switch(HW_LED_IMX6ULL_WDS_GROUP(resource_ptr->gpio_pad))
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

static struct hal_led_imx6ull_operations operations_obj = {
	.init = hal_led_imx6ull_init,
	.ctl  = hal_led_imx6ull_ctl,
};

struct hal_led_imx6ull_operations* hal_led_imx6ull_get_operations(void){

	return &operations_obj;
}

