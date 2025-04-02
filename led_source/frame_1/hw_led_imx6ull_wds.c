
#include "hw_led_imx6ull_wds.h"

static struct hw_led_imx6ull_wds_resource hw_led_imx6ull_wds_resource_obj = {
	.gpio_pad = HW_LED_IMX6ULL_WDS_GROUP_PIN(5,3),
};


struct hw_led_imx6ull_wds_resource* hw_led_imx6ull_wds_get_resouce(void){

	return &hw_led_imx6ull_wds_resource_obj;
}


