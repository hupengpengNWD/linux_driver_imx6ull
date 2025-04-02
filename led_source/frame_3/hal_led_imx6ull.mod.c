#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8d42f378, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb9347d89, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x4624e53d, __VMLINUX_SYMBOL_STR(drv_led_get_imx6ull_operations) },
	{ 0x22016404, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x68c6b359, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x1bec770f, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x83851feb, __VMLINUX_SYMBOL_STR(drv_led_class_create_device) },
	{ 0x5d527252, __VMLINUX_SYMBOL_STR(of_find_property) },
	{ 0x74153d9d, __VMLINUX_SYMBOL_STR(of_property_read_string) },
	{ 0x3fcedc52, __VMLINUX_SYMBOL_STR(of_property_read_variable_u32_array) },
	{ 0x5363d73f, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xbd601bbe, __VMLINUX_SYMBOL_STR(of_get_next_child) },
	{ 0xe1f1a5fc, __VMLINUX_SYMBOL_STR(drv_led_class_destroy_device) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x822137e2, __VMLINUX_SYMBOL_STR(arm_heavy_mb) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=drv_led";

