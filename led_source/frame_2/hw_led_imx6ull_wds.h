#ifndef _HW_LED_IMX6ULL_WDS_
#define _HW_LED_IMX6ULL_WDS_


/* GPIO3_0 */
/* bit[31:16] = group */
/* bit[15:0]  = which pin */
#define HW_LED_IMX6ULL_WDS_GROUP(x) (x>>16)			/* 得到高16位，也就是gpio group的值 */
#define HW_LED_IMX6ULL_WDS_PIN(x)   (x&0xFFFF)		/* 得到低16位，也就是gpio pin的值*/
#define HW_LED_IMX6ULL_WDS_GROUP_PIN(g,p) ((g<<16) | (p))



#endif

