#include <linux/init.h>
#include <linux/module.h>
#include<linux/mm.h>
#include<linux/slab.h>
#include<linux/list.h>


struct num
{
	int i;
	struct list_head list;
};


static int __init hello_init(void){

	int i=0;
	LIST_HEAD(test_list);
	/*插入节点操作*/
	struct num *num;
	struct num *num1;
	printk("\n插入5o个元素到链表.\n\n");
	for(i=1;i<=50;i++)
	{
		num=( struct num* )kmalloc(sizeof(struct num),GFP_KERNEL);
		num->i=i;
		list_add_tail(&num->list,&test_list);
	}
	/*链表输入操作*/i=0;
	printk("输出链表所有节点的值为:\n");
	list_for_each_entry (num,&test_list,list){

		printk( "%3d" , num->i);
	}
	/*删除操作*/
	list_for_each_entry_safe( num , num1,&test_list,list){
		list_del( &num->list);
		kfree(num);
	}
	if(list_empty(&test_list))
	{
		printk("\n释放链表所有节点成功. \n\n" );
	}




	return 0;
}


static void __exit hello_exit(void){

}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("kz.cc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("alloc and free pages fron buddy systen");

