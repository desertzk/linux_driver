#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include<linux/fs.h>

#define MEM_MINORCNT 4
#define KINDLEMEM_SIZE 1024
#define KINDLE_MEM_MAGIC 'k'
#define KINDLE_MEM_CLEAR _IO(KINDLE_MEM_MAGIC,0)

static int major;

struct KINDLEMEM{
	struct cdev cdev;
	unsigned char mem[KINDLEMEM_SIZE];
}KINDLEMEM_STRU;

struct KINDLEMEM_STRU *kindlemem_devp;

static struct file_operations kindlemem_fops={
	.owner = THIS_MODULE,
	.open = kindlemem_open,
	.release = kindlemem_release,
	.write = kindlemem_write,
	.read = kindlemem_read,
	.unlocked_ioctl = loctl_kindlemem
};

static int kindlemem_release(struct inode *inode, struct file *file)
{
	printk("kindlemem release\n");
	return 0;
}

//file *  fs.h
static int kindlemem_open(struct inode *inode, struct file *file)
{
	printk("kindlemem open\n");
	//获取当前对应结构体成员的结构体指针
	struct KINDLEMEM *devp = container_of(inode->i_cdev,struct KINDLEMEM,cdev);
	file->private_data=devp;
	//file->private_data=kindlemem_devp;
	return 0;
}

static inline loff_t llseek(struct file *file, off_t offset,int origin)
{
loff_t ret =0;
}


static ssize_t kindlemem_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	struct KINDLEMEM_STRU *TPdev=file->private_data;
		int rt=0;
	
	if(buf == NULL)
	{
		printk("[led_read]buf == NULL\n");
		return -EINVAL;		//学会返回错误码：参数非法
	}
		
	
	
	if(len > sizeof (TPdev->mem))
	{
		printk("[led_read]len==%d\n",len);
		return -EINVAL;		//学会返回错误码：参数非法
		
	}
			
	//内核空间向用户空间拷贝数据
	if(rt = copy_to_user(buf, TPdev->mem+*offset, len))
	{
		ret= -EFAULT;
	}else{
		//获取成功拷贝的字节数
		len = len - rt;	
		printk(KERN_INFO"read size is %lu bytes from %lu \n",len,*offset);
	}

	
	return len;

}


static ssize_t kindlemem_write(struct file *file, char __user *userbuf, size_t count, loff_t *offset)
{
	unsigned long pos=*off;
	int ret=0;
	struct KINDLEMEM_STRU *TPdev=file->private_data;

	if(copy_from_user(TPdev->mem)+pos,userbuf,count)
	{
		ret= -EFAULT;
	}else{
		*off+=size;
		ret=size;
		printk(KERN_INFO"write size is %lu bytes from %lu \n",size,*off);
	}
	return ret;
}

static long loctl_kindlemem(struct file *file, unsigned int cmd, unsigned long arg)
{
//获得设置好的文件私有数据
struct KINDLEMEM_STRU* TPdev = file- >private_data;
switch ( cmd )
	{
	case KINDLE_MEM_CLEAR:
		memset( TPdev- >mem,o,KINDLEMEM_SIZE);
		printk(KERN_INFO "Kindlemem is set to ioctl clear to zero\n");
		break;
	default:
		return -EINVAL;
		break;
	}
return 0;
}



void setup_kindlemem_cdev(struct kindlemem_dev *dev,int minor)
{
	int error;
	dev_t devno= MKDEV(major,minor);
	cdev_init(&dev->cdev,&kindlemem_fops);
	dev->cdev.owner=THIS_MODULE;
	error=cdev_add(&dev->cdev,dev_t,1);
    if(error<0)
	{
		printk("cdev_add error %d",error);
		//goto err_cdev_add;
	}
}


static int __init Init_Kindlemem(void)
{
    int ret;
    dev_t devno=MKDEV(major,0);
    if(major)
    {
        //create cdev
        ret=register_chrdev_region(devno,MEM_MINORCNT,"kindlemem");

    }else{
		ret=alloc_chrdev_region(&devno,0,MEM_MINORCNT,"kindlemem"); //走了这个应该就不用mknod了 这个函数会在/dev/下新建kindlemem
		major=MAJOR(devno);
	}
	if(ret<0)
		goto fail;

	kindlemem_devp=kzalloc(sizeof(struct KINDLEMEM_STRU),GFP_KERNEL);
	if(!kindlemem_devp)
	{
		ret=-1;
		goto err_cdev_add;
	}
	setup_kindlemem_cdev(kindlemem_devp,0);
	return ret;
err_cdev_add:
	unregister_chrdev_region(devno,MEM_MINORCNT);
	return ret;
}


static void __exit Exit_Kindlemem(void)
{
	dev_t devno= MKDEV(major,0);
	printk("Good Bye, World! leaving kernel space...\n");
	cdev_del(&kindlemem_devp->cdev);
	kfree(kindlemem_devp);
	unregister_chrdev_region(devno,MEM_MINORCNT);
	
}


module_init(Init_Kindlemem);		// 注册模块
module_exit(Exit_Kindlemem);	// 注销模块
MODULE_LICENSE("GPL"); 		//告诉内核该模块具有GNU公共许可证