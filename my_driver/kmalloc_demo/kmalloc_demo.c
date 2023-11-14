#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include<linux/vmalloc.h>
#include<linux/io.h>

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
//

void *q;
void *vp;


static int __init hello_init(void){
	p =kmalloc(sizeof(struct student),GFP_KERNEL);
	printk("p=%llx\n",p);
	if(!p)
		return -ENOMEM;
	printk("p=%llx\n",p);
	printk("sizeof student %ld\n",sizeof(*p));
	printk("p phy %llx\n",virt_to_phys(p));


	struct mm_struct init_mm = *(current->mm);
	printk("phys_base %lx\n",phys_base);
	printk("page_offset_base %lx\n",page_offset_base);
	printk("vmalloc_base %lx\n",vmalloc_base);
	printk("VMALLOC_START %lx\n",VMALLOC_START);
	printk("VMALLOC_END %lx\n",VMALLOC_END);
	printk("vmemmap_base %lx\n",vmemmap_base);
	printk("current pid %lx\n",current->pid);
	printk("current_mm kernel code start  %lx end %lx \n",init_mm.start_code,init_mm.end_code);
	printk("current_mm kernel data start %lx end %lx \n",init_mm.start_data,init_mm.end_data);
	printk("PAGE_OFFSET %lx  __START_KERNEL_map %lx  \n",PAGE_OFFSET,__START_KERNEL_map);
	
/*	    printk(KERN_INFO "Kernel Text (Code): 0x%lx - 0x%lx\n", (unsigned long)_stext, (unsigned long)_etext);
    printk(KERN_INFO "Kernel Data: 0x%lx - 0x%lx\n", (unsigned long)_sdata, (unsigned long)_edata);
    printk(KERN_INFO "Init Task: 0x%lx\n", (unsigned long)&init_task);
    printk(KERN_INFO "Kernel Stack (current): 0x%lx\n", (unsigned long)current);*/


    printk(KERN_INFO "Kernel Stack (current): 0x%lx\n", (unsigned long)current);






    unsigned int phys_addr;
    unsigned int pfn;
    int i;
    vp=vmalloc(5*1024*1024);
    printk("sizeof vp %x",sizeof(vp));
    if(!vp){
	    printk("vmalloc failed\n");
	    return -ENOMEM;
    }else{
	    for(i=0;i<100;i++){
		    vp=vp+4096;//test:change 1024 to 4096 8192…
		    pfn =vmalloc_to_pfn(vp);
		    phys_addr =(pfn<<12)|((unsigned long)vp &0xfff);
		    printk("virt_addr:%lx pfn:%x   phys_addr:%x\n",
				    (unsigned long)vp,pfn,phys_addr);
	    }
    }
    printk("…-----------------------------------------------------------------\n");
    q=kmalloc(4*1024*1024,GFP_KERNEL);
    printk("sizeof q %x",sizeof(q));
    if(!q){
	    printk("knalloc failed\n");return -ENOMEM;
    }else{
	    for(i=0;i<100;i++){
		    q=q+4096;
		    phys_addr =virt_to_phys(q);
		    pfn =(unsigned long)phys_addr >>12;
		    printk("virt_addr =%lx  pfn=%x    physaddr =%lx\n",
				    (unsigned long)q,pfn,phys_addr);
	    }
    }









	return 0;

}

static void __exit hello_exit(void){
	kfree(p);
	if(q)
		kfree(q);
	vfree(vp);
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");
