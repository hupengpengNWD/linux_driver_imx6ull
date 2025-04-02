#include <linux/module.h>
#include <linux/poll.h>

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


struct gpio_key{
	int gpio;
	struct gpio_desc *gpiod;
	int flag;
	int irq;
} ;

static struct gpio_key *gpio_keys_100ask;

static int major = 0;
static struct class *gpio_key_class;

#define BUF_LEN 128
static int g_keys[BUF_LEN];
static int r, w;

#define NEXT_POS(x) ((x+1) % BUF_LEN)


static unsigned int gpio_key_drv_poll(struct file *fp, poll_table * wait);
static ssize_t gpio_key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset);


static struct file_operations gpio_key_drv = {
	.owner	 = THIS_MODULE,
	.read    = gpio_key_drv_read,
	.poll    = gpio_key_drv_poll,
};

static int is_key_buf_empty(void)
{
	return (r == w);
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

/* 定义一个等待队列，用于存放因 poll() 或 read() 而休眠的进程 */
static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

static ssize_t gpio_key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	int key;
	
	/* 缓冲区不为空时 is_key_buf_empty（）返回0，!0=1
	   缓冲区为空时 is_key_buf_empty（）返回1， !1=0
	   不为空时，不休眠，继续向下执行，返回按键值
	   为空时，使当前进程进入睡眠状态
	*/

/*
	下面的进程指的是gpio_key_wait等待队列中的进程（载gpio_key_drv_poll函数中会向这个队列里添加进程）
	当 !is_key_buf_empty() == 1（真）：
		条件满足，进程不会休眠，直接继续执行后续代码 
	当 !is_key_buf_empty() == 0（假）：
		条件不满足，进程进入休眠，直到满足下列条件之一：
		 a、其他代码调用 wake_up_interruptible(&gpio_key_wait) 且 !is_key_buf_empty() 变为真。
		 b、收到信号（如 Ctrl+C）中断休眠。
		进程退出休眠继续执行后续代码
*/
	wait_event_interruptible(gpio_key_wait, !is_key_buf_empty());
	key = get_key();
	err = copy_to_user(buf, &key, 4);
	
	return 4;/* 返回四个字节，key是int类型占四个字节 */
}

static unsigned int gpio_key_drv_poll(struct file *fp, poll_table * wait)
{


	/*
		驱动不会休眠，它只是通过 poll_wait() 向内核注册一个唤醒回调机制：
		当用户程序（如 button_test）调用 poll() 时，
		内核会调用驱动的 gpio_key_drv_poll。
		poll_wait的作用：
		“如果未来有事件（比如按键按下），请唤醒那些正在等待 gpio_key_wait 队列的进程”。
		（但此时进程还未休眠，只是注册了唤醒路径。）
	*/

	/*  将当前进程加入 gpio_key_wait 队列（当前进程指的是测试程序） */
	poll_wait(fp, &gpio_key_wait, wait);
	
	
	/* 是否有按键值，有就返回 POLLIN | POLLRDNORM，表示有按键类型的事件发生了
	   没有就返回0*/
	return is_key_buf_empty() ? 0 : POLLIN | POLLRDNORM;
}

static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;
	int key;
	
	val = gpiod_get_value(gpio_key->gpiod);
	
	key = (gpio_key->gpio << 8) | val;
	put_key(key);

	/* 唤醒那些正在等待 gpio_key_wait 队列的进程 */
	wake_up_interruptible(&gpio_key_wait);
	
	return IRQ_HANDLED;
}


static int gpio_key_probe(struct platform_device *pdev)
{
	int err;
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;
	enum of_gpio_flags flag;
		

	count = of_gpio_count(node);
	if (!count){
		return -1;
	}

	gpio_keys_100ask = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);
	for (i = 0; i < count; i++)
	{
		gpio_keys_100ask[i].gpio = of_get_gpio_flags(node, i, &flag);
		if (gpio_keys_100ask[i].gpio < 0)
		{
			return -1;
		}
		gpio_keys_100ask[i].gpiod = gpio_to_desc(gpio_keys_100ask[i].gpio);
		gpio_keys_100ask[i].flag = flag & OF_GPIO_ACTIVE_LOW;
		gpio_keys_100ask[i].irq  = gpio_to_irq(gpio_keys_100ask[i].gpio);
	}

	for (i = 0; i < count; i++){
		err = request_irq(gpio_keys_100ask[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "100ask_gpio_key", &gpio_keys_100ask[i]);
	}

	major = register_chrdev(0, "100ask_gpio_key", &gpio_key_drv);  

	gpio_key_class = class_create(THIS_MODULE, "100ask_gpio_key_class");
	if (IS_ERR(gpio_key_class)) {
		unregister_chrdev(major, "100ask_gpio_key");
		return PTR_ERR(gpio_key_class);
	}

	device_create(gpio_key_class, NULL, MKDEV(major, 0), NULL, "100ask_gpio_key"); 
        
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
	for (i = 0; i < count; i++){
		free_irq(gpio_keys_100ask[i].irq, &gpio_keys_100ask[i]);
	}
	kfree(gpio_keys_100ask);
    return 0;
}


static const struct of_device_id ask100_keys[] = {
    { .compatible = "100ask,gpio_key" },
    { },
};

static struct platform_driver gpio_keys_driver = {
    .probe      = gpio_key_probe,
    .remove     = gpio_key_remove,
    .driver     = {
        .name   = "100ask_gpio_key",
        .of_match_table = ask100_keys,
    },
};

static int __init gpio_key_init(void)
{
    int err;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
    err = platform_driver_register(&gpio_keys_driver); 
	
	return err;
}

static void __exit gpio_key_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&gpio_keys_driver);
}



module_init(gpio_key_init);
module_exit(gpio_key_exit);
MODULE_LICENSE("GPL");


