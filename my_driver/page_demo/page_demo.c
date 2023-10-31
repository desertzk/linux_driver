#include <linux/init.h>
#include <linux/module.h>
#include<linux/mm.h>
#include<linux/gfp.h>
#define PAGE_ORDER 1
struct page *page;
unsigned long int virt_addr;
static int __init hello_init(void){
	page =alloc_pages(GFP_KERNEL,PAGE_ORDER);
	printk("page frane no:%lx\n",page_to_pfn(page));
	printk("physicl addr:%llx",page_to_phys(page));
	printk("virtual addr:%llx\n",(unsigned int)page_address(page));
	virt_addr =(unsigned long)page_to_virt(page);
	printk("virtual addr:%llx\n",virt_addr);
	return 0;
}


static void __exit hello_exit(void){
	free_pages(virt_addr,PAGE_ORDER);
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");

