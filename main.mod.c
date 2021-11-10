#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x9de7765d, "module_layout" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x67c0c54c, "pv_ops" },
	{ 0x999e8297, "vfree" },
	{ 0x7260f26b, "put_disk" },
	{ 0xe0814095, "blk_cleanup_queue" },
	{ 0x2060bb78, "del_gendisk" },
	{ 0x89384a02, "device_add_disk" },
	{ 0x306e02c, "__alloc_disk_node" },
	{ 0x5c30bc72, "blk_queue_logical_block_size" },
	{ 0x7ed29774, "blk_mq_init_sq_queue" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0x69acdf38, "memcpy" },
	{ 0x56470118, "__warn_printk" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x70df2d13, "blk_mq_end_request" },
	{ 0x9f54e77b, "blk_mq_start_request" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "163AF533F1D2689B528D66E");
