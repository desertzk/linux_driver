#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include<linux/vmalloc.h>
#include<linux/io.h>
#include <linux/kallsyms.h>

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

unsigned long virt2phys(unsigned long virt_addr)
{

	phys_addr_t phys_addr;
	struct page *page;

	page = virt_to_page((void *)virt_addr);
	phys_addr = page_to_phys(page) + offset_in_page(virt_addr);

	printk(KERN_INFO "Virtual Address: 0x%lx, Physical Address: 0x%llx\n", virt_addr, (unsigned long long)phys_addr);
	return phys_addr;
}

static int __init hello_init(void){
	p =kmalloc(sizeof(struct student),GFP_KERNEL);
	printk("p=%llx\n",p);
	if(!p)
		return -ENOMEM;
	printk("p=%llx\n",p);
	printk("sizeof student %ld\n",sizeof(*p));
	printk("p phy %llx\n",virt_to_phys(p));


	struct mm_struct current_mm = *(current->mm);
	printk("phys_base %lx\n",phys_base);
	printk("page_offset_base %lx\n",page_offset_base);
	printk("vmalloc_base %lx\n",vmalloc_base);
	printk("VMALLOC_START %lx\n",VMALLOC_START);
	printk("VMALLOC_END %lx\n",VMALLOC_END);
	printk("vmemmap_base %lx\n",vmemmap_base);
	printk("PAGE_OFFSET %lx  __START_KERNEL_map %lx  \n",PAGE_OFFSET,__START_KERNEL_map);
	printk("current pid %lx\n",current->pid);
    printk(KERN_INFO "Kernel Stack (current): 0x%lx\n", (unsigned long)current);
	printk("current_mm kernel code start  %lx end %lx \n",current_mm.start_code,current_mm.end_code);
	printk("current_mm kernel data start %lx end %lx \n",current_mm.start_data,current_mm.end_data);
struct mm_struct *pinit_mm=kallsyms_lookup_name("init_mm");

printk("pinit_mm %lx \n",pinit_mm);

struct mm_struct init_mm=*pinit_mm;
/*phys_addr_t phys_addr;

phys_addr = virt_to_phys((void *)virt_addr);*/
	printk("init_mm kernel code start  %lx phys %lx  end %lx phys %lx \n",pinit_mm->start_code,virt_to_phys(pinit_mm->start_code),pinit_mm->end_code,virt_to_phys(pinit_mm->end_code));
	virt2phys(pinit_mm->start_code);
	virt2phys(pinit_mm->end_code);

	printk("init_mm kernel data start  %lx phys %lx  end %lx phys %lx \n",pinit_mm->start_data,virt2phys(pinit_mm->start_data),pinit_mm->end_data,virt2phys(pinit_mm->end_data));
	printk("init_mm kernel brk start  %lx phys %lx  end %lx phys %lx \n",pinit_mm->start_brk,virt2phys(pinit_mm->start_brk),pinit_mm->brk,virt2phys(pinit_mm->brk));






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
