#ifndef _DRV_ELD_H_
#define _DRV_ELD_H_


void drv_led_class_create_device(int minor);
void drv_led_class_destroy_device(int minor);
void drv_led_get_imx6ull_operations(struct hal_led_imx6ull_operations *opr);
#endif /* _LEDDRV_H */