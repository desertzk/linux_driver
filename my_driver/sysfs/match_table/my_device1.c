#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include "hello.h"


struct hello_device hello_dev1 = {
    .name = "wit1",
    .id   = 2,
};

static int __init hello_device_init(void)
{
    return hello_device_register(&hello_dev1);
}

static void __exit hello_device_exit(void)
{
    hello_device_unregister(&hello_dev1);
}

module_init(hello_device_init);
module_exit(hello_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wit@zhaixue.cc");
MODULE_DESCRIPTION("create and register a hello device");








