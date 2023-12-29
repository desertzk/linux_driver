#include <linux/init.h>
#include <linux/module.h>
#include<linux/sched.h>
#include<linux/gfp.h>
#include<linux/highmem.h>



static int __init hello_init(void){
	// 1 : find_get_pid(...)
	struct pid *kernelpid=find_get_pid(current->pid);
	printk("打印进程描述符count的值:%d\n",kernelpid->count);
	printk("打印进程描述符1evel的值:%d\n",kernelpid->level);
	printk("打印进程描述符nnumbers的值: %d\n" ,kernelpid->numbers[kernelpid->level].nr);
	// 2 : pid_nr( ..)
	int iNr=pid_nr(kernelpid);
	printk("打印进程描述符的全局进程编号为:%d\n",iNr);
	printk("打印进程描述符的当前线程组编号为:%d\n",current->tgid); 
	// 3 : pid_task(...)
	struct task_struct *ttask=pid_task(kernepid,PIDTYPE_PID);
	printk("打印任务当前的状态为:%ld\n",ttask->state);
	printk("打印任务当前的进程号为:%d\n",ttask->pid);
	printk("打印任务当前的进程号为:%s\n",ttask->comm);
	// 4 : _task_pid_nr_ns(...)
	pid_t rest=__task_pid_nr_ns(ttask,PIDTYPE_PID,kernelpid->numbers[kernelpid->level].ns);
	printk("调用__task_pid_nr_ns(...)函数，输出结果为: %d \n ",rest);


	return 0;
}


static void __exit hello_exit(void){

}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");

