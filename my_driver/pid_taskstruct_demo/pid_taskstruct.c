#include <linux/init.h>
#include <linux/module.h>
#include<linux/sched.h>
#include<linux/delay.h>
#include<linux/gfp.h>
#include<linux/highmem.h>

struct task_struct *pts_thread=NULL;
int MyFunc_ThreadFunc(void *argc){


	int iData=-1;
	printk("调用内核线程函数: MyFunc_ThreadFunc(...).\n");
	printk("打印当前进程的pid的值为:d\n" , current->pid);
	//显示父进程的状态
	printk("初始化函数状态为:%ld\n",pts_thread->state);
	iData=wake_up_process(pts_thread); 


	//打印输出函数调用之后的父进程状态
	printk("调用wake_up_process之后的函数状态为:ld\n", pts_thread->state);
	printk("调用wake_up_process函数返回结果为:%d\n",iData);
	printk("退出内核线程函数: MyFunc_ThreadFunc( .. .).\n");
	return 0;
}


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
	struct task_struct *ttask=pid_task(kernelpid,PIDTYPE_PID);
	printk("打印任务当前的状态为:%ld\n",ttask->state);
	printk("打印任务当前的进程号为:%d\n",ttask->pid);
	printk("打印任务当前的进程号为:%s\n",ttask->comm);
	// 4 : _task_pid_nr_ns(...)
	pid_t rest=__task_pid_nr_ns(ttask,PIDTYPE_PID,kernelpid->numbers[kernelpid->level].ns);
	printk("调用__task_pid_nr_ns(...)函数，输出结果为: %d \n ",rest);


	int result_data=-1;//保存wake_up_process结果
	char cName[]="wakeup.c";
	struct task_struct *pResult=NULL;
	long time_out;
	//等待队列元素
	wait_queue_head_t head;
	wait_queue_entry_t data;
	printk("调用wakeupprocess_functest_init(...)函数. \n");//指定存储节点，创建一个新的内核线程
	pResult=kthread_create_on_node(MyFunc_ThreadFunc,NULL,-1,cName);

	printk("打印新的内核线程的PID值为:%d \n ",pResult->pid);
	printk("打印当前进程的PID值为:%d\n",current->pid);
	init_waitqueue_head(&head);//初始化等待队列的头元素init_waitqueue_entry (&data,current);
	add_wait_queue(&head ,&data);
	pts_thread=current;//保存当前进程的数据信息
	result_data=wake_up_process(pResult);//唤醒刚创建新线程
	printk("调用wake_up_process(...)函数，唤醒新线程之后的结果为: %d \n" , result_data);
	//让当前进程进入睡眼状态
	time_out=schedule_timeout_uninterruptible( 2000*10);
	//唤醒当前进程,让当前进程处于RUNNING状态
	result_data=wake_up_process( current);
	printk("唤醒当前进程为RUNNING状态之后线程结果为:%d \n" , result_data);
	//输出time_out的值
	printk("调用sched_tiMeout_uninterruptible( ...)函数，
	返回的值为∶%ld\n" ,time_out) ;



	printk("退出wakeupprocess_functest_init(...)函数.\n");


	return 0;
}


static void __exit hello_exit(void){

}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");
