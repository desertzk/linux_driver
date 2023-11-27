#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/mutex.h>
#include<linux/gfp.h>
#include<linux/mm.h>
#include<linux/io.h>


#define MEM_MINORCNT 4
#define KINDLEMEM_SIZE 1024
#define KINDLE_MEM_MAGIC 'k'
#define KINDLE_MEM_CLEAR _IO(KINDLE_MEM_MAGIC,0)

#define PAGE_ORDER 0
#define MAX_SIZE 4096

static struct page *page=NULL;
static char *hello_buf=NULL;

static int major;
static unsigned long flags;

struct KINDLEMEM{
	struct cdev cdev;
	unsigned char mem[KINDLEMEM_SIZE];
	spinlock_t kindlemem_spin_lock;
	//rwlock_t kindlemem_rwlock;
	struct mutex kindlemem_mutex;
};

struct KINDLEMEM *kindlemem_devp;

static int kindlemem_open_count=0;

static struct class  *led_dev_class;
static struct device *led_dev_device;
dev_t led_dev_num;
static struct cdev led_cdev;

#define phys_to_pfn(p) ((p) >> PAGE_SHIFT)


static int  page_content(unsigned long pfn)
{
    struct page *page;
    
    int i = 0;

    // Get the page structure for the given PFN
    page = pfn_to_page(pfn);

    // Read the content of the physical frame
    pr_info("Content of Physical Frame %lu:\n", pfn);
    unsigned char *data = page_address(page);
    printk("Page content in hex:\n");

    for (i = 0; i < PAGE_SIZE; ) {
        if (i % 16 == 0)
            printk("\n");

        printk("i %d   %02x %02x %02x %02x  %02x %02x %02x %02x",i, data[i++],data[i++],data[i++],data[i++]
		,data[i++],data[i++],data[i++],data[i++]);
    }

    printk("\n");


    return 0;
}


static int  get_phys_content(unsigned long phys_addr)
{
    struct page *page;
    
    int i = 0;
    unsigned long pfn = phys_to_pfn(phys_addr);
    // Get the page structure for the given PFN
    page = pfn_to_page(pfn);

    // Read the content of the physical frame
    pr_info("Content of Physical Frame %lu:\n", pfn);

	unsigned long offset = (phys_addr & ~PAGE_MASK);

    unsigned char *data = page_address(page)+offset;
    printk("Page content in hex:\n");



        printk("%02x %02x %02x %02x  %02x %02x %02x %02x", data[i++],data[i++],data[i++],data[i++]
		,data[i++],data[i++],data[i++],data[i++]);


printk("Page content in string %s\n",data);


    return 0;
}



// Function to convert user space virtual address to physical address
unsigned long virt_to_phys_pgt(unsigned long virt_addr) {
    pgd_t *pgd;
		p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page;
    unsigned long phys_addr = 0;

    // Get the current process's page global directory
    pgd = pgd_offset(current->mm, virt_addr);

    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_ALERT "Error: PGD entry not found.\n");
        return 0;
    }

	p4d = p4d_offset(pgd, virt_addr);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return 0;
    // Get the page upper directory
    pud = pud_offset(p4d, virt_addr);

    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_ALERT "Error: PUD entry not found.\n");
        return 0;
    }

    // Get the page middle directory
    pmd = pmd_offset(pud, virt_addr);

    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_ALERT "Error: PMD entry not found.\n");
        return 0;
    }

    // Get the page table entry
    pte = pte_offset_map(pmd, virt_addr);

    if (!pte || pte_none(*pte)) {
        printk(KERN_ALERT "Error: PTE entry not found.\n");
        return 0;
    }

    // Get the physical address from the page table entry
    page = pte_page(*pte);
    phys_addr = page_to_phys(page) + (virt_addr & ~PAGE_MASK);
printk("Physical Address: 0x%lx",phys_addr);
    pte_unmap(pte);

    return phys_addr;
}



void print_page_tables(struct mm_struct *mm,unsigned long addr) {
    pgd_t *pgd;
	p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    
//(pgd + pgd_index(address));
    pgd = pgd_offset(mm, addr);
	printk("(mm)->pgd 0x%x *pgd 0x%x pgd_index %x",pgd,*pgd,pgd_index(addr));
	/*
	if (!pgtable_l5_enabled())
		return (p4d_t *)pgd;
	return (p4d_t *)pgd_page_vaddr(*pgd) + p4d_index(address);
	*/
	p4d = p4d_offset(pgd, addr);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return;
printk("p4d 0x%x *pgd 0x%x pgd_page_vaddr(*pgd) 0x%x p4d_index %x",p4d,*pgd,pgd_page_vaddr(*pgd),p4d_index(addr));
	
	
	//p4d_pgtable(*p4d) + pud_index(address);
	pud = pud_offset(p4d, addr);
	if (pud_none(*pud) || pud_bad(*pud))
		return;
printk("pud 0x%x *p4d 0x%x p4d_pgtable(*p4d) 0x%x p4d_index %x",pud,*p4d,p4d_pgtable(*p4d),pud_index(addr));

	//pud_pgtable(*pud) + pmd_index(address);
	pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return;
printk("pmd 0x%x *pud 0x%x pud_pgtable(*pud) 0x%x pmd_index %x",pmd,*pud,pud_pgtable(*pud),pmd_index(addr));

//return (pte_t *)pmd_page_vaddr(*pmd) + pte_index(address);
	pte = pte_offset_map(pmd, addr);
	if (pte_none(*pte))
		return;
printk("pte 0x%x *pmd 0x%x pmd_page_vaddr(*pmd) 0x%x pte_index %x",pte,*pmd,pmd_page_vaddr(*pmd),pte_index(addr));

/*
	phys_addr_t pfn = pte_val(pte);
	pfn ^= protnone_mask(pfn);
	return (pfn & PTE_PFN_MASK) >> PAGE_SHIFT;
*/
	unsigned long pfn = pte_pfn(*pte);

	printk("Virtual Address: 0x%lx,pgd_val %lx p4d_val %lx pud_val %lx pmd_val %lx PTE Value: 0x%lx pfn 0x%lx\n"
	, addr,pgd_val(*pgd),p4d_val(*p4d),pud_val(*pud),pmd_val(*pmd), pte_val(*pte),pfn);


	page_content(pfn);

	pte_unmap(pte);
    //}
}

static int hello_open(struct inode *inode,struct file *file)
{
	
	printk("hello open\n");
	return 0;
}


static int hello_release(struct inode *inode,struct file *file){
	__free_pages(page,PAGE_ORDER);

	return 0;
}

unsigned long virt2phys(unsigned long virt_addr)
{

        phys_addr_t phys_addr;
        struct page *page;

        page = virt_to_page((void *)virt_addr);
        phys_addr = page_to_phys(page) + offset_in_page(virt_addr);

        printk(KERN_INFO "Virtual Address: 0x%lx, Physical Address: 0x%llx\n", virt_addr, (unsigned long long)phys_addr);
        return phys_addr;
}



static ssize_t kindlemem_read(struct file *file,char __user *buf,size_t count,loff_t *ppos)
{
	int len;
	int ret;
	len =PAGE_SIZE-*ppos;
	if(count>len)
		count =len;

	printk("kindle read bufaddr %lx\n",buf);
	ret =copy_to_user(buf,hello_buf +*ppos,count);
	printk("kindle read %d\n",ret);
	//virt2phys(buf);
	print_page_tables(current->mm,buf);
	unsigned long phys=virt_to_phys_pgt(buf);
get_phys_content(phys);
	virt2phys(hello_buf);
	if(ret)
		return -EFAULT;
	*ppos +=count;
	return count;
}



static ssize_t kindlemem_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int len=0;
	int ret;
	len=PAGE_SIZE-*ppos;
	if(count>len)
		count =len;
	ret =copy_from_user(hello_buf +*ppos,buf,count);
	printk("kindle write %d\n",ret);
	virt2phys(hello_buf);
	printk("write_buf virt_addr;%lx content %s\n",(unsigned int)hello_buf,hello_buf);
	if(ret)
		return -EFAULT;
	*ppos +=count;
	return count;

}

vm_fault_t hello_fault(struct vm_fault *vmf)
{
	dump_stack();
	page =alloc_pages(GFP_KERNEL,PAGE_ORDER);
	if(!page){
		printk("alloc_page failed\n");
		return -ENOMEM;
	}
	hello_buf =page_to_virt(page);
	printk("data_buf phys_addr:%lx,virt_addr;%lx\n",page_to_phys(page),(unsigned int)hello_buf);
#if 1
	vmf->page =page;
	get_page(vmf->page);
#endif
	return 0;
}

struct vm_operations_struct hello_vm_ops ={
	.fault = hello_fault
};

static int hello_mmap(struct file *file,struct vm_area_struct *vma)
{
	printk("hello_mmap call vma start %lx vma end %lx \n",vma->vm_start,vma->vm_end);
	vma->vm_ops= &hello_vm_ops;
	return 0;
}

/*
static int hello_mmap(struct file *file,struct vm_area_struct *vma)
{
	struct mm_struct *mm;
	unsigned long size;
	unsigned long pfn;
	int ret;
	mm =current->mm;
	pfn =page_to_pfn(page);
	size =vma->vm_end -vma->vm_start;
	if(size>MAX_SIZE){
		printk("map size too large,max size is ox%x\n",MAX_SIZE);
		return -EINVAL;
	}
	// map va to physical page
	ret =remap_pfn_range(vma,vma->vm_start,pfn,size,vma->vm_page_prot);
	if(ret)
	{
		printk("remap_pfn_range failed\n");
		return -EAGAIN;
	}
	return ret;
}
*/


static long loctl_kindlemem(struct file *file, unsigned int cmd, unsigned long arg)
{
//获得设置好的文件私有数据
struct KINDLEMEM* TPdev = file->private_data;
switch ( cmd )
	{
	case KINDLE_MEM_CLEAR:
		memset( TPdev->mem,0,KINDLEMEM_SIZE);
		printk(KERN_INFO "Kindlemem is set to ioctl clear to zero\n");
		break;
	default:
		return -EINVAL;
		break;
	}
return 0;
}


static loff_t hello_llseek(struct file *file,loff_t offset,int whence)
{
	loff_t pos;
	switch(whence){
		case SEEK_SET:
			pos =offset;
			break;
		case SEEK_CUR:
			pos =file->f_pos +offset;break;
		case SEEK_END:
			pos =MAX_SIZE +offset;break;
		default:
			return -EINVAL;
	}
	if(pos<0 || pos>MAX_SIZE)
		return -EINVAL;
	file->f_pos =pos;
	return pos;
}

static struct file_operations kindlemem_fops={
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_release,
	.write = kindlemem_write,
	.read = kindlemem_read,
	.unlocked_ioctl = loctl_kindlemem,
	.mmap = hello_mmap,
	.llseek = hello_llseek
};


void setup_kindlemem_cdev(struct KINDLEMEM *dev,int minor)
{
	int error;
	dev_t devno= MKDEV(major,minor);
	cdev_init(&dev->cdev,&kindlemem_fops);
	dev->cdev.owner=THIS_MODULE;
	error=cdev_add(&dev->cdev,devno,1);
	if(error<0)
	{
		printk("cdev_add error %d",error);
		//goto err_cdev_add;
	}
}


static int __init Init_Kindlemem(void)
{
	int ret;
	led_dev_num=MKDEV(major,0);
	if(major)
	{
		//create cdev
		ret=register_chrdev_region(led_dev_num,MEM_MINORCNT,"kindlemem");

	}else{
		ret=alloc_chrdev_region(&led_dev_num,0,MEM_MINORCNT,"kindlemem"); 
		major=MAJOR(led_dev_num);
	}
	if(ret<0)
		printk("register_chrdev_region failed\n");
	// Register the character device driver
	cdev_init(&led_cdev, &kindlemem_fops);
	led_cdev.owner = THIS_MODULE;
	cdev_add(&led_cdev, led_dev_num, 1);


	//自动创建设备类，若创建成功，则在/sys/class目录下创建kindmem文件夹 cat /proc/devices 也会多出新建的
	led_dev_class=class_create(THIS_MODULE,"kindlemem");
	if(IS_ERR(led_dev_class))
	{
		//将错误码指针转换为错误码
		ret = PTR_ERR(led_dev_class);
		printk("class_create failed %d",ret);	
		goto err_class_create;

	}

	//会在/sys/class/myled目录下去创建myled设备
	//会包含主设备号、次设备号、设备文件名
	//自动去/dev目录下创建zkkindlemem设备文件（设备节点）
	led_dev_device = device_create(led_dev_class,NULL,led_dev_num,NULL,"zkkindlemem");
	if (IS_ERR(led_dev_device))
	{
		//将错误码指针转换为错误码
		ret = PTR_ERR(led_dev_device);
		printk("device_create failed %d",ret);
		goto err_device_create;		
	}

	return ret;



err_device_create:	
	class_destroy(led_dev_class);

err_class_create:
	cdev_del(&led_cdev);	

err_cdev_add:
	unregister_chrdev_region(led_dev_num,MEM_MINORCNT);
	return ret;
}


static void __exit Exit_Kindlemem(void)
{
	dev_t devno= MKDEV(major,0);
	printk("Good Bye, World! leaving kernel space...\n");
device_destroy(led_dev_class,led_dev_num);
class_destroy(led_dev_class);
	cdev_del(&led_cdev);
	
	unregister_chrdev_region(led_dev_num,1);
	
}


module_init(Init_Kindlemem);		// 注册模块
module_exit(Exit_Kindlemem);	// 注销模块
MODULE_LICENSE("GPL"); 		//告诉内核该模块具有GNU公共许可证
