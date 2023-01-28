#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/cdev.h>
#include<linux/fs.h>
//定义一个led字符设备
static struct cdev led_cdev;
//定义一个led设备号
dev_t led_dev_num;

static struct class  *led_dev_class;
static struct device *led_dev_device;

static int led_open(struct inode *inode, struct file *file)
{
	printk("led open\n");
	return 0;
}

int led_close(struct inode *inode, struct file *file)
{
	printk("led_close\n");
	return 0;
}

ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
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
	len = len - rt;
	printk("kbuf[%s]\n",kbuf);
	printk("len[%d]\n",len);	
	
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
	//动态分配设备号
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
	
	//会在/sys/class/myled目录下去创建myled设备
	//会包含主设备号、次设备号、设备文件名
	//自动去/dev目录下创建zkleddevice设备文件（设备节点）
	led_dev_device = device_create(led_dev_class,NULL,led_dev_num,NULL,"zkleddevice");
	if (IS_ERR(led_dev_device))
    {
		//将错误码指针转换为错误码
		ret = PTR_ERR(led_dev_device);
		
		goto err_device_create;		
    }	

	return 0;
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
	cdev_del(&led_cdev);
	
	unregister_chrdev_region(led_dev_num,1);
	
}

module_init(demo_init);		// 注册模块
module_exit(demo_cleanup);	// 注销模块
MODULE_LICENSE("GPL"); 		//告诉内核该模块具有GNU公共许可证
