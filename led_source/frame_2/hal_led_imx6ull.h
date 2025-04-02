#ifndef _HAL_LED_IMX6ULL_H_
#define _HAL_LED_IMX6ULL_H_

struct hal_led_imx6ull_operations {
	int (*init) (int which);                /* 初始化LED, which-哪个LED */       
	int (*ctl) (int which, char status);    /* 控制LED, which-哪个LED, status:1-亮,0-灭 */
};

struct hal_led_imx6ull_operations* hal_led_imx6ull_get_led_operations_obj(void);


#endif

