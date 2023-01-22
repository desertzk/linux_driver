



static struct file_operations kindlemem_fops={
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
	.read = led_read
};


void setup_kindlemem_cdev(kindlemem_dev *dev,int minor)
{
	int error;
	dev_t = MKDEV(major,minor);
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

    }
}