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
/* 定义led个数  */
#define LED_NUM 1

/* 定义驱动模块名字 */
#define DRV_LED_MODULE_NAME "drv_led_module_hpp"

/* 定义主设备号（保存系统随机分配的值） */
static int drv_led_major = 0;

/* 定义设备类（在用户层创建设备时需要使用） */
static struct class*    drv_led_class_ptr;

/* 声明驱动接口函数 */
static int drv_led_close (struct inode *node, struct file *file);
static int drv_led_open (struct inode *node, struct file *file);
static ssize_t drv_led_read (struct file *file, char __user *buf, size_t size, loff_t *offset);
static ssize_t drv_led_write (struct file *file, const char __user *buf, size_t size, loff_t *offset);

/* 芯片操作对象 */
struct hal_led_imx6ull_operations*  led_imx6ull_operations_ptr;

/* 驱动操作对象 */
static struct file_operations drv_file_operations_obj = {
	.owner	 = THIS_MODULE,
	.open    = drv_led_open,
	.read    = drv_led_read,
	.write   = drv_led_write,
	.release = drv_led_close,
};

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
void drv_led_class_create_device(int minor)
{
	/* 设备注册 */
	device_create(drv_led_class_ptr, NULL, MKDEV(drv_led_major, minor), NULL, "drv_led_module_hpp%d", minor); 
}

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
void drv_led_class_destroy_device(int minor)
{
	/* 设备销毁 */
	device_destroy(drv_led_class_ptr, MKDEV(drv_led_major, minor));
}

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
void drv_led_get_imx6ull_operations(struct hal_led_imx6ull_operations *opr)
{
	led_imx6ull_operations_ptr = opr;
}
EXPORT_SYMBOL(drv_led_class_create_device);
EXPORT_SYMBOL(drv_led_class_destroy_device);
EXPORT_SYMBOL(drv_led_get_imx6ull_operations);
/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static ssize_t drv_led_read (struct file *file, char __user *buf, size_t size, loff_t *offset){

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}
/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static ssize_t drv_led_write (struct file *file, const char __user *buf, size_t size, loff_t *offset){

	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/* 根据次设备号和status控制LED */
	led_imx6ull_operations_ptr->ctl(minor, status);
	
	return 1;
}

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static int drv_led_open (struct inode *node, struct file *file){

	int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	/* 根据次设备号初始化LED */
	led_imx6ull_operations_ptr->init(minor);
	
	return 0;
}

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static int drv_led_close (struct inode *node, struct file *file){

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}



/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static int __init drv_led_init(void){

	int err;

	drv_led_major = register_chrdev(0, DRV_LED_MODULE_NAME, &drv_file_operations_obj);  
	
	drv_led_class_ptr = class_create(THIS_MODULE, "drv_led_module_hpp_class");
	err = PTR_ERR(drv_led_class_ptr);
	if (IS_ERR(drv_led_class_ptr)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(drv_led_major, DRV_LED_MODULE_NAME);
		return -1;
	}
	
	return 0;
}

/**-------------------------------------------------------
  * @fn     	
  * @brief	   
  * @param		
  * @return		
  * @remark  		
  */
static void __exit drv_led_exit(void){

	class_destroy(drv_led_class_ptr);
	unregister_chrdev(drv_led_major, DRV_LED_MODULE_NAME);
}



module_init(drv_led_init);
module_exit(drv_led_exit);

MODULE_LICENSE("GPL");


