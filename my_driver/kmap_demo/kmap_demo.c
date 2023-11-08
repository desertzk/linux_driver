#include <linux/init.h>
#include <linux/module.h>
#include<linux/mm.h>
#include<linux/gfp.h>
#include<linux/highmem.h>

#define PAGE_ORDER 1
struct page *page,*page_highmem;
unsigned long int virt_addr,kvirt_addr,kphys_addr,hvirt_addr,hphys_addr;

static int __init hello_init(void){
	page =alloc_pages(GFP_KERNEL,PAGE_ORDER);//low memory
	printk("page frane no:%lx\n",page_to_pfn(page));
	printk("physicl addr:%llx",page_to_phys(page));
	printk("virtual addr:%llx\n",(unsigned int)page_address(page));
	virt_addr =(unsigned long)page_to_virt(page);
	printk("virtual addr:%llx\n",virt_addr);
	kvirt_addr =kmap(page); //low memory dont need to map because it's already mapped
	kphys_addr =page_to_pfn(page)<<12;
	printk("phys_addr:%x virt_addr:%lx\n",kphys_addr,kvirt_addr);

	page_highmem =alloc_page(__GFP_HIGHMEM);//high mem need to map because it's unmap before
	if(!page_highmem)
		printk("alloc page failed\n");
	hvirt_addr =kmap(page_highmem);
	hphys_addr =page_to_pfn(page_highmem)<<12;
	printk("phys_addr:%x virt_addr:%lx\n",hphys_addr,(unsigned int)hvirt_addr);

	return 0;
}


static void __exit hello_exit(void){
	kunmap(page);
	free_pages(page,PAGE_ORDER);

	kunmap(page_highmem);
	free_page(page_highmem);
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");

