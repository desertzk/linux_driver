#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include<linux/sched/signal.h>

static int pid_mem = 1;
module_param(pid_mem, int, S_IRUGO);
MODULE_PARM_DESC(pid_mem, "PID of the process for which one need tist VMAs");

static void print_mem(struct task_struct *task)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    int count = 0;
    mm = task->mm;
    pr_info("\nThis mm struct has %d vmas.\n", mm->map_count);
    for (vma = mm->mmap; vma; vma = vma->vm_next)
    {
        printk("\nVma number %d:\n", ++count);
	struct file *vmfile = vma ->vm_file;
	if(vmfile)
        	printk(" Starts at θx%lx,Ends at ox%lx flag %lx  file %s\n", vma->vm_start, vma->vm_end,vma->vm_flags ,vmfile->f_path.dentry->d_name.name);
	else
        	printk(" Starts at θx%lx,Ends at ox%lx flag %lx  file %s\n", vma->vm_start, vma->vm_end,vma->vm_flags ,"null");
    }

    pr_info("\nCode Segment start = 0x%lx,end = 0x%lx \n"
    "Data Segment start = 0x%lx,end = 0x%lx\n"
    "Stack Segment start ox%lx\n",mm->start_code, mm->end_code,mm->start_data, mm->end_data,mm->start_stack);

}

static int mm_exp_load(void)
{
    struct task_struct *task;
    pr_info("\nGot the process id to look up as %d.\n", pid_mem);
    for_each_process(task)
    {
        if (task->pid == pid_mem)
        {
            printk("%s[%d]\n", task->comm, task->pid);
            print_mem(task);
        }
    }
    return 0;
}
static void mm_exp_unload(void)
{
    printk("\nPrint segment information module exiting.\n");
}
module_init(mm_exp_load);
module_exit(mm_exp_unload);
MODULE_AUTHOR("John Madieu <john.madieuagmail.com>");
MODULE_DESCRIPTION("Print segment information");
MODULE_LICENSE("GPL");
