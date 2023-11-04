#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>


struct student{
	int id;
	int age;
	float score;
	void(*print_score)(int id);
	void(*print_age)(int id);
};
struct kmem_cache *stu_cache;
struct student *p;
void stu_ctor(void *p)
{
}

//init objects here
static int __init hello_init(void){
	p =kmalloc(sizeof(struct student),GFP_KERNEL);
	printk("p=%llx\n",p);
	if(!p)
		return -ENOMEM;
	printk("p=%llx\n",p);
	printk("sizeof student %ld\n",sizeof(*p));
	printk("p phy %llx\n",virt_to_phys(p));

	printk("phys_base %lx\n",phys_base);
	printk("page_offset_base %lx\n",page_offset_base);
	printk("vmalloc_base %lx\n",vmalloc_base);
	printk("VMALLOC_START %lx\n",VMALLOC_START);
	printk("VMALLOC_END %lx\n",VMALLOC_END);
	printk("vmemmap_base %lx\n",vmemmap_base);
	printk("PAGE_OFFSET %lx  __START_KERNEL_map %lx  \n",PAGE_OFFSET,__START_KERNEL_map);
/*	    printk(KERN_INFO "Kernel Text (Code): 0x%lx - 0x%lx\n", (unsigned long)_stext, (unsigned long)_etext);
    printk(KERN_INFO "Kernel Data: 0x%lx - 0x%lx\n", (unsigned long)_sdata, (unsigned long)_edata);
    printk(KERN_INFO "Init Task: 0x%lx\n", (unsigned long)&init_task);
    printk(KERN_INFO "Kernel Stack (current): 0x%lx\n", (unsigned long)current);*/


    printk(KERN_INFO "Kernel Stack (current): 0x%lx\n", (unsigned long)current);
	return 0;

}

static void __exit hello_exit(void){
	kfree(p);
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");
