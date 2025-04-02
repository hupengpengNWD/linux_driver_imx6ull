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
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>

#define BUF_LEN 128
#define NEXT_POS(x) ((x+1) % BUF_LEN)

struct gpio_key{
	int gpio_pin_num;		/* gpio编号 */
	struct gpio_desc *gpiod;/* gpio句柄 */
	int gpio_act_lev;		/* gpio有效电平 */
	int gpio_irq_num;		/* gpio中断编号 */
} ;

/* 按键对象 */
static struct gpio_key *gpio_keys_100ask;

/* 主设备号 */
static int major = 0;
static struct class *gpio_key_class;

/* 环形缓冲区 */
static int g_keys[BUF_LEN];
static int r, w;

/* 使用工作队列的阻塞方式

宏展开为：
wait_queue_head_t gpio_key_wait = {
    .lock = { 
        .raw_lock = { 0 }  // 未锁定的自旋锁
    },
    .task_list = {
        .next = &(gpio_key_wait.task_list),
        .prev = &(gpio_key_wait.task_list)
    }
};
*/
static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

static ssize_t gpio_key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset);

static struct file_operations gpio_key_drv = {
	.owner	 = THIS_MODULE,
	.read    = gpio_key_drv_read,
};

static int is_key_buf_empty(void)
{
	return (r == w);//为空返回1
}

static int is_key_buf_full(void)
{
	return (r == NEXT_POS(w));
}

static void put_key(int key)
{
	if (!is_key_buf_full())
	{
		g_keys[w] = key;
		w = NEXT_POS(w);
	}
}

static int get_key(void)
{
	int key = 0;
	if (!is_key_buf_empty())
	{
		key = g_keys[r];
		r = NEXT_POS(r);
	}
	return key;
}

/* 驱动读函数 */
static ssize_t gpio_key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	int key;
	
	/* 缓冲区不为空时 is_key_buf_empty（）返回0，!0=1
	   缓冲区为空时 is_key_buf_empty（）返回1， !1=0
	   不为空时继续向下执行，返回按键值
	   为空时，使当前进程进入睡眠状态
	*/

	/* 休眠进程 */
	wait_event_interruptible(gpio_key_wait, !is_key_buf_empty());
	key = get_key();
	err = copy_to_user(buf, &key, 4);
	
	return 4;
}

/* 按键中断回调函数 */
static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;
	int key;
	
	val = gpiod_get_value(gpio_key->gpiod);
	key = (gpio_key->gpio_irq_num << 8) | val;
	put_key(key);

	/* 唤醒进程 */
	wake_up_interruptible(&gpio_key_wait);
	
	return IRQ_HANDLED;
}

/* 驱动和设备树平台设备节点匹配后的设备创建函数 */
static int gpio_key_probe(struct platform_device *pdev)
{
	int err;
	int count;
	int i;
	enum of_gpio_flags flag;

	/* 找到设备树节点 */
	struct device_node *node = pdev->dev.of_node;
	
	/* 得到gpio按键个数 */
	count = of_gpio_count(node);
	if (!count)
	{
		return -1;
	}

	/* 分配内存保存count个struct gpio_key实例信息*/
	gpio_keys_100ask = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);
	for (i = 0; i < count; i++)
	{
		/* 保存gpio有小电平（决定写的时候写1的输出什么电平，读的时候什么电平触发中断，与原理图中按键常开是什么状态相反） */
		gpio_keys_100ask[i].gpio_irq_num = of_get_gpio_flags(node, i, &flag);
		if (gpio_keys_100ask[i].gpio_irq_num < 0)
		{
			return -1;
		}
		/* 保存gpio句柄 */
		gpio_keys_100ask[i].gpiod = gpio_to_desc(gpio_keys_100ask[i].gpio_irq_num);
		/* 保存gpio电平有效（只保留低电平有效，原理图中按键常开的时候是高电平，所以是低电平有效） */
		gpio_keys_100ask[i].gpio_act_lev = flag & OF_GPIO_ACTIVE_LOW;
		/* 保存gpio中断号，该中断号是虚拟中断号，由内核动态分配 */
		gpio_keys_100ask[i].gpio_irq_num  = gpio_to_irq(gpio_keys_100ask[i].gpio_irq_num);
	}

	for (i = 0; i < count; i++)
	{
		/* 注册中断处理函数，中断触发的类型设备树中没有指定，而是通过此函数的参数指定的*/
		err = request_irq(gpio_keys_100ask[i].gpio_irq_num,
		/* 中断回调函数 */	gpio_key_isr, 
		/* 中断触发类型 */  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
		/* 中断名字 */	   "100ask_gpio_key", 
		/* 中断回调参数 */	&gpio_keys_100ask[i]);
	}

	/* 注册file_operations 	*/
	major = register_chrdev(0, "100ask_gpio_key", &gpio_key_drv);   

	/* 创建类 */
	gpio_key_class = class_create(THIS_MODULE, "100ask_gpio_key_class");
	if (IS_ERR(gpio_key_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "100ask_gpio_key");
		return PTR_ERR(gpio_key_class);
	}
	
	/* 创建设备 */
	device_create(gpio_key_class, NULL, MKDEV(major, 0), NULL, "100ask_gpio_key"); /* /dev/100ask_gpio_key */
        
    return 0;
    
}

static int gpio_key_remove(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;

	device_destroy(gpio_key_class, MKDEV(major, 0));
	class_destroy(gpio_key_class);
	unregister_chrdev(major, "100ask_gpio_key");

	count = of_gpio_count(node);
	for (i = 0; i < count; i++)
	{
		free_irq(gpio_keys_100ask[i].gpio_irq_num, &gpio_keys_100ask[i]);
	}
	kfree(gpio_keys_100ask);
    return 0;
}


/* 匹配列表，用来喝设备树的平台设备匹配 */
static const struct of_device_id ask100_keys[] = {
    { .compatible = "100ask,gpio_key" },
};

/* 平台驱动 */
static struct platform_driver gpio_keys_driver = {
    .probe      = gpio_key_probe,
    .remove     = gpio_key_remove,
    .driver     = {
        .name   = "100ask_gpio_key",
        .of_match_table = ask100_keys,
    },
};

/* 装载函数 */
static int __init gpio_key_init(void)
{
    int err;
    
	/* 注册平台驱动设备
	 硬件抽象层：通过设备树（如 100ask,gpio_key）匹配硬件，获取 GPIO 和中断资源。
	 探测硬件：在 probe() 函数中初始化硬件（配置 GPIO、注册中断等）。
	 与内核基础设施集成：依赖 Linux 的 Platform 框架管理设备生命周期。 
	 简而言之，通过平台驱动设备可以匹配到设备节点定义好的硬件资源，获得真正的硬件

	 然后通过匹配和执行probe函数
	 再注册的字符设备驱动，这个字符设备驱动可以在其注册的操作函数（比如read）中使用这些硬件，
	*/
    err = platform_driver_register(&gpio_keys_driver); 
	
	return err;
}

/* 卸载函数 */
static void __exit gpio_key_exit(void)
{
	/* 卸载平台驱动 */
    platform_driver_unregister(&gpio_keys_driver);
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);
MODULE_LICENSE("GPL");


