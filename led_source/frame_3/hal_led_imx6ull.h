#ifndef _HAL_LED_IMX6ULL_H_
#define _HAL_LED_IMX6ULL_H_

/* GPIO3_0 */
/* bit[31:16] = group */
/* bit[15:0]  = which pin */
#define HAL_LED_IMX6ULL_GROUP(x) (x>>16)			/* 得到高16位，也就是gpio group的值 */
#define HAL_LED_IMX6ULL_PIN(x)   (x&0xFFFF)		/* 得到低16位，也就是gpio pin的值*/
#define HAL_LED_IMX6ULL_GROUP_PIN(g,p) ((g<<16) | (p))


struct hal_led_imx6ull_operations {
	int (*init) (int which);                /* 初始化LED, which-哪个LED */       
	int (*ctl) (int which, char status);    /* 控制LED, which-哪个LED, status:1-亮,0-灭 */
};

struct hal_led_imx6ull_operations* hal_led_imx6ull_get_led_operations_obj(void);


#endif

