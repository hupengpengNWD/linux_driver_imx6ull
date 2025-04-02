
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
#include <linux/of.h>
#include <linux/slab.h>//提供kaclloc
#include <linux/io.h>   // 提供 readl/writel
#include <linux/ioport.h> // 资源管理

#include "hal_led_imx6ull.h"
#include "drv_led.h"

/* i.MX6ULL GPIO Bank 基址（参考手册） */ 
#define GPIO1_BASE 0x0209C000
#define GPIO2_BASE 0x020A0000
#define GPIO3_BASE 0x020A4000
#define GPIO4_BASE 0x020A8000
#define GPIO5_BASE 0x020AC000

/* gpio寄存器描述 */
struct gpio_bank {
    void __iomem *base;     /* 寄存器基址 */ 
    u32 dr_offset;          /* 数据寄存器偏移 */ 
    u32 gdir_offset;        /* 方向寄存器偏移 */ 
};
/* led信息描述 */
struct custom_led {
    const char *label;
    struct gpio_bank *bank;
    u8 pin;
	int num;
    bool active_low;
    struct list_head list;
};

/* led链表头 */ 
static LIST_HEAD(led_list); 

/* gpio寄存器描述实例 */
static struct gpio_bank banks[] = {
    [1] = { .base = (void __iomem*)GPIO1_BASE, .dr_offset = 0x00, .gdir_offset = 0x04 },
    [2] = { .base = (void __iomem*)GPIO2_BASE, .dr_offset = 0x00, .gdir_offset = 0x04 },
    [3] = { .base = (void __iomem*)GPIO3_BASE, .dr_offset = 0x00, .gdir_offset = 0x04 },
    [4] = { .base = (void __iomem*)GPIO4_BASE, .dr_offset = 0x00, .gdir_offset = 0x04 },
    [5] = { .base = (void __iomem*)GPIO5_BASE, .dr_offset = 0x00, .gdir_offset = 0x04 },
};

static int hal_led_imx6ull_led_init (int which);
static int hal_led_imx6ull_led_ctl(int which, char status);

static int hal_led_imx6ull_platform_driver_led_probe(struct platform_device *pdev);
static int hal_led_imx6ull_platform_driver_led_remove(struct platform_device *pdev);


/* 定义一个led操作对象 */
static struct hal_led_imx6ull_operations led_operations_obj = {
	.init = hal_led_imx6ull_led_init,
	.ctl  = hal_led_imx6ull_led_ctl,
};

/* 定义一个设备树匹配列表 */
static const struct of_device_id hal_led_imx6ull_led[] = {
    { .compatible = "hpp,generic-led" },/* 与设备树定义的平台资源节点适配名字需要一致 */
};

/* 定义一个平台驱动 */
static struct platform_driver hal_led_imx6ull_platform_driver_led_obj= {
    .probe      = hal_led_imx6ull_platform_driver_led_probe,
    .remove     = hal_led_imx6ull_platform_driver_led_remove,
    .driver     = {
        .name   = "100ask_led",
		.of_match_table = hal_led_imx6ull_led,
    },
};



/**
 * @brief 解析设备树中的 LED 节点
 * @param np 父节点指针
 * @return 成功返回 0，失败返回错误码
 */
 static int parse_led_dt(struct platform_device *pdev, struct device_node *np)
 {
	 struct device_node *child;
	 int ret = 0;
	 struct custom_led *led;
	 
	 /* 遍历所有子节点 */
	 for_each_child_of_node(np, child) {
		 
		 u32 bank_num, pin;
		 int num;
		 /* 分配 LED 结构体 */
		 led = kzalloc(sizeof(*led), GFP_KERNEL);
		 if (!led) {
			 ret = -ENOMEM;
			 goto err_out;
		 }
		 
		 /* 读取必要属性 */
		 if (of_property_read_u32(child, "bank", &bank_num)) {
			 dev_err(&pdev->dev, "Missing 'bank' property\n");
			 ret = -EINVAL;
			 goto err_free_led;
		 }
		 
		 if (of_property_read_u32(child, "pin", &pin)) {
			 dev_err(&pdev->dev, "Missing 'pin' property\n");
			 ret = -EINVAL;
			 goto err_free_led;
		 }

		 if (of_property_read_u32(child, "num", &num)) {
			dev_err(&pdev->dev, "Missing 'num' property\n");
			ret = -EINVAL;
			goto err_free_led;
		}
		 
		 of_property_read_string(child, "label", &led->label);
		 if (!led->label){
			led->label = "unnamed-led";
		 }

		 /* 验证 bank 编号有效性 */
		 if (bank_num < 1 || bank_num > ARRAY_SIZE(banks) || !banks[bank_num].base) {
			 dev_err(&pdev->dev, "Invalid bank number %u\n", bank_num);
			 ret = -EINVAL;
			 goto err_free_led;
		 }
		 
		 /* 填充 LED 描述信息 */
		 led->bank = &banks[bank_num];
		 led->pin = pin;
		 led->num = num;
		 led->active_low = of_property_read_bool(child, "active-low");
		  
		 /* 初始化 GPIO */
		 {
			 void __iomem *gdir = led->bank->base + led->bank->gdir_offset;
			 u32 val = readl(gdir);
			 /* 设置为输出模式 */ 
			 writel(val | (1 << led->pin), gdir);  
		 }
		 
		 /* 组织led链表 */
		 INIT_LIST_HEAD(&led->list);
		 /* 将led链表挂到led链表头下 */
		 list_add(&led->list, &led_list);
		 /* 注册设备 */
		 drv_led_class_create_device(led->num );
	 }
	 
	 return 0;
	 
 err_free_led:
	 kfree(led);
 err_out:
	 of_node_put(child);
	 return ret;
 }


 /**
 * @brief 清理函数
 */
static void cleanup_leds(void)
{
    struct custom_led *led, *tmp;
    
    list_for_each_entry_safe(led, tmp, &led_list, list) {
        /* 关闭 LED */
        void __iomem *dr = led->bank->base + led->bank->dr_offset;
        u32 val = readl(dr);
        writel(val & ~(1 << led->pin), dr);
        
        /* 从链表移除并释放 */
        list_del(&led->list);
        kfree(led);
		/* 销毁设备 */
		drv_led_class_destroy_device(led->num);
    }
}


/**
 * @brief 设置 LED 亮/灭状态
 * @param led LED 设备指针
 * @param on true=点亮, false=熄灭
 */
 void led_set(struct custom_led *led, bool on)
 {
	 void __iomem *dr_reg = led->bank->base + led->bank->dr_offset;
	 u32 val = readl(dr_reg);
	 
	 /* 处理 active-low 逻辑 */
	 if (led->active_low)
		 on = !on;
	 
	 /* 更新 GPIO 输出 */
	 if (on)
		 val |= (1 << led->pin);    
	 else
		 val &= ~(1 << led->pin);   
	 
	 writel(val, dr_reg);
 }

 /**
 * @brief 通过编号查找 LED 设备
 * @param num 要查找的编号
 * @return 找到返回 LED 指针，否则返回 NULL
 */
struct custom_led *led_find_by_num(int num)
{
    struct custom_led *led;
    
    list_for_each_entry(led, &led_list, list) {
        if (led->num == num)
            return led;
    }
    return NULL; // 未找到
}

/* led初始化操作 */
static int hal_led_imx6ull_led_init (int which){	
	
	// /* 查找编号为0的LED */ 
	// struct custom_led *led = led_find_by_num(which); 
	// led_set(led,0);
	return 0;
}

/* led控制操作 */
static int hal_led_imx6ull_led_ctl (int which, char status) {

	/* 查找编号为0的LED */ 
	struct custom_led *led = led_find_by_num(which); 
	if(NULL != led){
		led_set(led,((bool)status));
	}
	return 0;
}

/* 平台设备的proe成员函数，用于获取资源创建设备 */
static int hal_led_imx6ull_platform_driver_led_probe(struct platform_device *pdev)
{
    struct device_node *np;

	/* 从自定义的设备树找平台资源 */
    np = pdev->dev.of_node;
    if (!np){
        return -1;
	}
	/* 提取设备树节硬件的节点信息初始化硬件 */
	parse_led_dt(pdev, np); 

    return 0;
    
}

/* 平台设备的remove成员函数，用于获取资源销毁设备 */
static int hal_led_imx6ull_platform_driver_led_remove(struct platform_device *pdev)
{
    struct device_node *np;
    np = pdev->dev.of_node;
    if (!np){
        return -1;
	}

	cleanup_leds();
    
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
