#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include<linux/interrupt.h>
#include<mach/devices.h> 
#include<linux/sched.h>


#define CMD_KEY_ALL _IOR('K',0,unsigned int)


struct key_event{
	int code;
	int value;
}


//定义一个key字符设备
static struct cdev key_cdev;
//定义一个key设备号

static dev_t key_dev_num;

static unsigned int key_press_flag=0;
static unsigned int key_val=0;
static wait_queue_head_t key_wq;
static struct class  *key_dev_class;
static struct device *key_dev_device;
static struct fasync_struct *fasync;
static struct tasklet_struct mytasklet;



//K2 gpioa28
static void __iomem *gpioa_va=NULL;
static void __iomem *gpioa_out_va=NULL;
static void __iomem *gpioa_outenb_va=NULL;    //input op '0'
static void __iomem *gpioa_pad_va=NULL;    //input op  0 low 1 high
static void __iomem *gpioa_altfn0_va=NULL;    
static void __iomem *gpioa_altfn1_va=NULL;    // input op '00'




static int key_open(struct inode *inode, struct file *file)
{
	printk("key_open\n");
		unsigned int v=0;
	
	//配置gpioa28引脚为GPIO模式
	v = ioread32(gpioa_altfn1_va);
	v &=~(3<<24);
	iowrite32(v,gpioa_altfn1_va);
	
	//配置gpioa28引脚为input模式
	v = ioread32(gpioa_outenb_va);
	v &=~(1<<28);
	iowrite32(v,gpioa_outenb_va);	
	
	

	return 0;
}

int key_close(struct inode *inode, struct file *file)
{
	unsigned int v;
	//熄灭D7灯
	v = ioread32(gpioa_out_va);
	v |=1<<13;
	iowrite32(v,gpioa_out_va);
	printk("key_close\n");
	return 0;
}

ssize_t key_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
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
				v = ioread32(gpioa_out_va);
				v &=~(1<<13);
				iowrite32(v,gpioa_out_va);				
				printk("led on %x %p\n",v,gpioa_out_va);
			}
				
			else
			{
				//灭灯
				v = ioread32(gpioa_out_va);
				v |=1<<13;
				iowrite32(v,gpioa_out_va);				
				printk("led off %x %p\n",v,gpioa_out_va);
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

static ssize_t key_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	

		int rt=0;
	
	if(buf == NULL)
	{
		printk("[key_read]buf == NULL\n");
		return -EINVAL;		//学会返回错误码：参数非法
	}
		
	if(file->f_flags & O_NONBLOCK && !key_press_flag) // open in non block mode
	{
		return -EAGAIN;
	}
//在需要等待(没有数据)的时候，进行休眠
	printk("block read");
	wait_event_interruptible(key_wq,key_press_flag); 
	printk("have data can read now");
	
/*	if(len > sizeof (buf))
	{
		printk("[key_read]len==%d\n",len);
		return -EINVAL;		//学会返回错误码：参数非法
		
	}*/
			
	//内核空间向用户空间拷贝数据
	rt = copy_to_user(buf, &key_val, sizeof(key_val));
	key_press_flag=0;
		//获取成功拷贝的字节数
	len = len - rt;	

	
	
	return len;
}

unsigned int key_drv_poll(struct file *filp, struct poll_table_struct *pts)
{
	//返回一个mask值
	unsigned int mask;
	printk("before poll_wait");
	//调用poll_wait,将当前到等待队列注册系统中
	poll_wait(filp,&key_wq, pts);
	printk("after poll_wait");
	// 1,当没有数据到时候返回一个0
	if(!key_press_flag)
		mask = 0;
	// 2,有数据返回一个POLLIN
	if(key_press_flag)
		mask |= POLLIN;
	return mask;
}



static long key_ioctl(struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *) arg;
	

	
	int rt=0;

	switch(cmd)
    {
	case CMD_KEY_ALL:
        {
			// key_val|=gpio_get_value(PAD_GPIO_B+9);
			// key_val|=gpio_get_value(PAD_GPIO_B+30)<<1;	
			// key_val|=gpio_get_value(PAD_GPIO_A+28)<<2;	
			// key_val|=gpio_get_value(PAD_GPIO_B+31)<<3;
			printk("before wait_event_interruptible\n");
			//等待队列且条件成立
			wait_event_interruptible(key_wq,key_press_flag);
			key_press_flag=0;				
			printk("after wait_event_interruptible\n");
			//将按键值拷贝给用户空间
			rt = copy_to_user(argp,(void *)&key_val,sizeof key_val);
			//按键值清零
			key_val=0;
			
			if(rt)
				return -EFAULT;
			
		}break;
		
		
		default:
			return -ENOIOCTLCMD;
		
	}
	
	return 0;
	
}


static struct file_operations key_fops={
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_close,
	.write = key_write,
	.read = key_read,
    .unlocked_ioctl	= key_ioctl,
	.poll = key_drv_poll,
	.fasync = key_drv_fasync,
};


int key_drv_fasync(int fd, struct file *filp, int on)
{
	//只需要调用一个函数记录信号该发送给谁
	return fasync_helper(fd, filp, on, &fasync);
}

void tasklet_bottom_half(unsigned long data)
{
	printk("tasklet_bottom_half called\n");
		//唤醒等待队列且条件成立
		wake_up(&key_wq);
		
		key_press_flag=1;

		//send signal to userspace
		kill_fasync(&fasync,SIGIO,POLLIN);
}

//irq top half
irqreturn_t key_irq_handler(int irq_num, void *dev){
    //printk("key_irq_handler irq_num %d %x\n",irq_num,*gpioa_pad_va);
	//gpioa_pad_va bit 28 press 0 release 1
	int v = ioread32(gpioa_pad_va);
	printk(KERN_NOTICE"key_irq_handler irq_num %d *gpioa_pad_va 0x%x v 0x%x\n",irq_num,(*(unsigned int *)gpioa_pad_va),v);
    key_val|= (v & (1<<28))?0:1;
	printk(KERN_NOTICE"key_val irq_num %d  v 0x%x\n",key_val,v);
    //若有按键按下
	if(key_val)
	{
		//start bottom half
		tasklet_schedule(&mytasklet);
		printk("start bottom half \n");
	}

    return IRQ_HANDLED;
}


static int __init mykey_init(void)
{
	printk("Hello, World! from the kernel space...\n");
	int ret =0;

#if 0
	//静态设备号
	key_dev_num = MKDEV(255,0);

	ret = register_chrdev_region(key_dev_num,1,"myled");
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_register_chrdev_region;
	}
#else
	//动态分配设备号 如果设备号有效则在/proc/devices文件中可以找到mykey
	ret = alloc_chrdev_region(&key_dev_num,0,1,"mykey");
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_register_chrdev_region;
	}
	printk("major %d minor %d\n",MAJOR(key_dev_num),MINOR(key_dev_num));
#endif
	cdev_init(&key_cdev,&key_fops);
	//将字符设备加入到内核
	ret = cdev_add(&key_cdev,key_dev_num,1);
	if(ret<0)
	{
		printk("register_chrdev_region error");
		goto err_cdev_add;
	}

	//自动创建设备类，若创建成功，则在/sys/class目录下创建myled文件夹   
	key_dev_class=class_create(THIS_MODULE,"zkkey");
	if(IS_ERR(key_dev_class))
	{
		//将错误码指针转换为错误码
		ret = PTR_ERR(key_dev_class);
		
		goto err_class_create;
		
	}
	
	//会在/sys/class/zkled目录下去创建zkleddevice设备
	//会包含主设备号、次设备号、设备文件名
	//自动去/dev目录下创建zkleddevice设备文件（设备节点）
	key_dev_device = device_create(key_dev_class,NULL,key_dev_num,NULL,"zkkeydevice");
	if (IS_ERR(key_dev_device))
    {
		//将错误码指针转换为错误码
		ret = PTR_ERR(key_dev_device);
		
		goto err_device_create;		
    }	

		//申请io内存
	//GPIOA28，以0xC001A000作为起始地址，连续申请0x28字节，申请成功在iomem文件显示"gpioa"  cat /proc/iomem
	ret=request_mem_region(0xC001A000,0x28,"gpioa");
	
	if(ret == NULL)
	{
		ret = -ENOMEM;
		
		goto err_request_mem_region;
		
		
	}

	//将物理地址映射（转换）为虚拟地址
	gpioa_va = ioremap(0xC001A000,0x28);
	if(gpioa_va == NULL)
	{
		ret = -ENOMEM;
		
		goto err_ioremap;
		
		
	}

	printk("gpioa_va = %p \n",gpioa_va);
	
	//得到相应寄存器的虚拟地址
	gpioa_out_va = gpioa_va+0x00;
	gpioa_outenb_va = gpioa_va+0x04;	
	gpioa_pad_va = gpioa_va+0x018;
	gpioa_altfn0_va = gpioa_va+0x20;
	gpioa_altfn1_va = gpioa_va+0x24;

	//free_irq(IRQ_GPIO_A_START+28,NULL);
    //为K2按键(GPIOA28)申请中断
	//中断号：IRQ_GPIO_A_START+28 或 gpio_to_irq(PAD_GPIO_A+28)  after success you can cat /proc/interrupts
    ret = request_irq(IRQ_GPIO_A_START+28,key_irq_handler,IRQF_TRIGGER_FALLING,"irq_gpioa28",NULL);
    if(ret<0)
	{ 
		printk("request_irq error %d",ret);
		//goto err_ioremap;
	}

    init_waitqueue_head(&key_wq);

	tasklet_init(&mytasklet,tasklet_bottom_half,45);

	return 0;




err_ioremap:
	//释放io内存
	release_mem_region(0xC001A000,0x28);

err_request_mem_region:
	device_destroy(key_dev_class,key_dev_num);

err_device_create:	
	class_destroy(key_dev_class);
	
err_class_create:
	cdev_del(&key_cdev);	


err_cdev_add:
	unregister_chrdev_region(key_dev_num,1);
	
err_register_chrdev_region:
	return ret;
}

static void __exit mykey_cleanup(void)
{
	printk("Good Bye, World! leaving kernel space...\n");
	tasklet_kill(&mytasklet);
    free_irq(IRQ_GPIO_A_START+28,NULL);
		//解除内存映射
	iounmap(gpioa_va);
release_mem_region(0xC001A000,0x28);
device_destroy(key_dev_class,key_dev_num);
class_destroy(key_dev_class);
	cdev_del(&key_cdev);
	
	unregister_chrdev_region(key_dev_num,1);
	
}

module_init(mykey_init);		// 注册模块
module_exit(mykey_cleanup);	// 注销模块
MODULE_LICENSE("GPL"); 		//告诉内核该模块具有GNU公共许可证
