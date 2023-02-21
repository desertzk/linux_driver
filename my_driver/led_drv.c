#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/uaccess.h>

//定义一个led字符设备
static struct cdev led_cdev;
//定义一个led设备号

dev_t led_dev_num;

static void __iomem *gpioe_va=NULL;
static void __iomem *gpioe_out_va=NULL;
static void __iomem *gpioe_outenb_va=NULL;
static void __iomem *gpioe_altfn0_va=NULL;

static struct class  *led_dev_class;
static struct device *led_dev_device;

static int led_open(struct inode *inode, struct file *file)
{
	printk("led open\n");
		unsigned int v=0;
	
	//配置GPIOE13引脚为GPIO模式
	v = ioread32(gpioe_altfn0_va);
	v &=~(3<<26);
	iowrite32(v,gpioe_altfn0_va);
	
	//配置GPIOE13引脚为输出模式
	v = ioread32(gpioe_outenb_va);
	v |=1<<13;
	iowrite32(v,gpioe_outenb_va);	
	
	
	//熄灭D7灯
	v = ioread32(gpioe_out_va);
	v |=1<<13;
	iowrite32(v,gpioe_out_va);
	return 0;
}

int led_close(struct inode *inode, struct file *file)
{
	unsigned int v;
	//熄灭D7灯
	v = ioread32(gpioe_out_va);
	v |=1<<13;
	iowrite32(v,gpioe_out_va);
	printk("led_close\n");
	return 0;
}

ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	//kbuf[0]:指定哪个led灯，7~10
	//kbuf[1]:指定对应灯的亮灭，1：点亮；0：熄灭
	char kbuf[64]={0};
	int rt=0;
	if(buf==NULL)
		return -EINVAL;

	if(len > sizeof (kbuf))
		return -EINVAL;		//学会返回错误码：参数非法	
	//获取用户空间传递的数据
	//将用户空间buf数据拷贝给内核空间的kbuf
	//返回没有被拷贝成功的字节数
	rt=copy_from_user(kbuf,buf,len);
	printk("kbuf[%s]\n",kbuf);
	printk("len[%d]\n",len);	
	unsigned int v=0;
	switch(kbuf[0])
	{
		case '1':
		{		
			if(kbuf[1]=='1')
			{
				//点灯
				v = ioread32(gpioe_out_va);
				v &=~(1<<13);
				iowrite32(v,gpioe_out_va);				
				printk("led on %x %p\n",v,gpioe_out_va);
			}
				
			else
			{
				//灭灯
				v = ioread32(gpioe_out_va);
				v |=1<<13;
				iowrite32(v,gpioe_out_va);				
				printk("led off %x %p\n",v,gpioe_out_va);
			}
				
		}break;
		
		case 8:
		{	

		}break;
		
		
		default:break;
	}
	
	//获取成功拷贝的字节数
	//应用传递了32字节,len=32
	//没有被拷贝成功有10字节，rt=10
	//成功拷贝的字节数 = len - rt = 22
	len = len - rt;
	

	
	return len;

}

static ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	char kbuf[5]={'1','2','2'};

		int rt=0;
	
	if(buf == NULL)
	{
		printk("[led_read]buf == NULL\n");
		return -EINVAL;		//学会返回错误码：参数非法
	}
		
	
	
	if(len > sizeof (kbuf))
	{
		printk("[led_read]len==%d\n",len);
		return -EINVAL;		//学会返回错误码：参数非法
		
	}
			
	//内核空间向用户空间拷贝数据
	rt = copy_to_user(buf, kbuf, 5);
		//获取成功拷贝的字节数
	len = len - rt;	

	
	
	return len;
}



static struct file_operations led_fops={
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
	.read = led_read
};



static int __init demo_init(void)
{
	printk("Hello, World! from the kernel space...\n");
	int ret =0;
#if 0
	//静态设备号
	led_dev_num = MKDEV(255,0);

	ret = register_chrdev_region(led_dev_num,1,"myled");
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_register_chrdev_region;
	}
#else
	//动态分配设备号 如果设备号有效则在/proc/devices文件中可以找到myled
	ret = alloc_chrdev_region(&led_dev_num,0,1,"myled");
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_register_chrdev_region;
	}
	printk("major %d minor %d\n",MAJOR(led_dev_num),MINOR(led_dev_num));
#endif
	cdev_init(&led_cdev,&led_fops);
	//将字符设备加入到内核
	ret = cdev_add(&led_cdev,led_dev_num,1);
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_cdev_add;
	}

	//自动创建设备类，若创建成功，则在/sys/class目录下创建myled文件夹   
	led_dev_class=class_create(THIS_MODULE,"zkled");
	if(IS_ERR(led_dev_class))
	{
		//将错误码指针转换为错误码
		ret = PTR_ERR(led_dev_class);
		
		goto err_class_create;
		
	}
	
	//会在/sys/class/zkled目录下去创建zkleddevice设备
	//会包含主设备号、次设备号、设备文件名
	//自动去/dev目录下创建zkleddevice设备文件（设备节点）
	led_dev_device = device_create(led_dev_class,NULL,led_dev_num,NULL,"zkleddevice");
	if (IS_ERR(led_dev_device))
    {
		//将错误码指针转换为错误码
		ret = PTR_ERR(led_dev_device);
		
		goto err_device_create;		
    }	

		//申请io内存
	//GPIOE13，以0xC001E000作为起始地址，连续申请0x24字节，申请成功在iomem文件显示"gpioe"  cat /proc/iomem
	ret=request_mem_region(0xC001E000,0x24,"gpioe");
	
	if(ret == NULL)
	{
		ret = -ENOMEM;
		
		goto err_request_mem_region;
		
		
	}

	//将物理地址映射（转换）为虚拟地址
	gpioe_va = ioremap(0xC001E000,0x24);
	if(gpioe_va == NULL)
	{
		ret = -ENOMEM;
		
		goto err_ioremap;
		
		
	}

	printk("gpioe_va = %p \n",gpioe_va);
	
	//得到相应寄存器的虚拟地址
	gpioe_out_va = gpioe_va+0x00;
	gpioe_outenb_va = gpioe_va+0x04;	
	gpioe_altfn0_va = gpioe_va+0x20;

	return 0;




err_ioremap:
	//释放io内存
	release_mem_region(0xC001E000,0x24);

err_request_mem_region:
	device_destroy(led_dev_class,led_dev_num);

err_device_create:	
	class_destroy(led_dev_class);
	
err_class_create:
	cdev_del(&led_cdev);	


err_cdev_add:
	unregister_chrdev_region(led_dev_num,1);
	
err_register_chrdev_region:
	return ret;
}

static void __exit demo_cleanup(void)
{
	printk("Good Bye, World! leaving kernel space...\n");
		//解除内存映射
	iounmap(gpioe_va);
release_mem_region(0xC001E000,0x24);
device_destroy(led_dev_class,led_dev_num);
class_destroy(led_dev_class);
	cdev_del(&led_cdev);
	
	unregister_chrdev_region(led_dev_num,1);
	
}

module_init(demo_init);		// 注册模块
module_exit(demo_cleanup);	// 注销模块
MODULE_LICENSE("GPL"); 		//告诉内核该模块具有GNU公共许可证
