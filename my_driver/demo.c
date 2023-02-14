

//都是INT型

//定义一个原子变量并且赋值初始化和定义方法
atomic_t atomic_a = ATOMIC_INIT(0);
void atomic_set(atomic_a,8);
//获取一个原子变量的值
atomic_a=atomic_read( atomic_a);

//原子变量加/减
void atomic_add(int i,atomic_a); // atomic_add(1,atomic_a); atomic_a+1
void atomic_sub(int i, atomic_a);// atomic_sub(1,atomic_a); atomic_a-1/原子变量的自增/自减
void atomic_inc(atomic_a);// atomic_a++
void atomic_dec(atomic_a);// atomic_a--
//操作并测试  操作后 测试 判断新值
int atomic_inc_and_test(atomic_a);
int atomic_dec_and_test(atomic_a);
int atomic_sub_and_test(int i,atomic_a);


//操作并返回
int atomic_add_return(int i , atomic_a); 
int atomic_sub_return(int i, atomic_a);
int atomic_inc_return(atomic_a);
int atomic_dec_return(atomic_a);


//设置初始化BIT
void set_bit(nr, addr);

//清除位操作给addr的第nr位置0
void clear_bit(nr, addr);
//改变位给addr的第nr位置反转
void change_bit(nr, addr);
//测试位测试addr的第nr位是什么值
int test_bit(nr , addr);


//测试并操作位先测试返回之前的bit值然后再进行置1清0反转
int test_and_set_bit(nr, addr);
int test_and_clear_bit(nr, addr);
int test_and_change_bit(nr, addr);


//自旋锁
//定义一个spin锁
spinlock_t lock;
//初始化spin锁
spin_lock_init(lock);
//锁定/获取SPIN锁
spin_lock(lock);
//释放一个spin锁
spin_unlock(lock);
//尝试获得一个spin
spin_trylock(spinlock_t*lock);


//spin的衍生函数SPIN+IRQ的操作衍生函数
spin_lock_irq(spinlock_t* lock)=spin_lock(spinlock_t* lock )+local irq disable();
spin_unlock_irq( spinlock_t* lock)=spin_unlock(spinlock_t* lock )+local_irq_enable();spin_lock_irqsave(spinlock_t* lock , flags)=spin_lock(spinlock_t* lock )+local_irq_save();
spin_unlock_irqrestore(spinlock_t* lock , flags)=spin_unlock(spinlock_t* lock )+local_irq_restore();
//定义和初始化读写锁
rwlock_t my_rwlock;
rwlock_init(&my_rwlock);
//读锁定操作函数
void read_lock(rwlock_t *my_rwlock);
void read_lock_irq(rwlock _t *my_rwlock);
void read_lock_irqsave(rwlock_t *my_rwlock, flags);


//写锁定函数
void write_lock(rwlock t *my_rwlock);
void write_lock_irq(rwlock_t *my_rwlock);
void write_lock_irqsave(rwlock_t *my_rwlock, flags);
int write_trylock(wlock t *my_rwlock);//尝试进行写锁获得如果当前没有写操作就获得并且返回值为1反之返回0

//获取一个顺序锁
void write_seqlock(seqlock_t * my_seq_lock);
int write_tryseqlock(seqlock_t * my_seq_lock);
write_seqlock_irqsave(seqlock_t * my_seq_lock, flags);
write_seqlock_irq(seqlock_t * my_seq_lock);
//读开始
unsigned read_seqbegin(const seqlock_t * sl);
read_seqbegin_irqsave(seqlock_t * my_seq_lock, flags);
//重读函数
int read_seqretry (const seqcount_t * s, unsigned iv);
read_seqretry_irqrestore(seqcount_t * s, iv, flags);



//RCU
struct kindmem{
struct list_head list;
int flag;
int length;
int count;
}
LIST_HEAD( head );
plist =search(head ,id);
if(plist == NULL)
    return
qlist=kmalloc(sizeof(kindmem),GFP_KERNEL);*qlist=*plist;
qlist->flag=1;
qlist->length=2;
list_replace_rcu(&(plist- >list),&(qlist- >list));
synchronize_rcu();
kfree(plist);
 // RCU读锁定
rcu_read_lock()
//RCU读解锁
rcu_read_unlock()
//同步RCU操作，等待当前CPU对RCU保护的链表读完成操作
synchronize_rcu();
// RCU的非阻塞回调方式
void call_rcu(struct rcu_head * head, void(* func)(struct rcu_head * rcu));

//RCU的链表头操作函数
//给RCU保护的指针赋新值链表的   写端使用  发布
rcu_assign_pointer(old , new);

//获取一个rcu保护指针函数链表的   读端使用  订阅
rcu_dereference(new);

//RCU的链表操作
//把链表元素new插入到RCU保护的链表head的开头
static inline void list_add_rcu(struct list_head * new, struct list_head *head);
//插入链表末尾
static inline void list_add_tail_rcu(struct list_head * new, struct list_head * head)
//删除一个rcu保护链表的节点
static inline void list_del_rcu(struct list_head * entry)
list_del_rcu(&qlist->list);
//用干用新的链表元素new取代旧的链表元索old
static inline void list_replace_rcu(struct list_head * old, struct list_head * new)

//对于RCU保护的链表的遍历函数
static inline void list_for_each_rcu(pos, head);



//获取互斥体
void mutex_lock(struct mutex * lock);
int mutex_lock_interruptible(struct mutex * lock);
int mutex_trylock(struct mutex * lock);
//释放互斥体
void mutex_unlock(struct mutex * lock);


//完成量
//定义一个完成量
struct completion kindlemem_completion;
//初始化完成量  =windows keinitializeEvent
init_completion(&kindlemem_completion);
//等待完成量 = windows waitforsingleobject
wait_for_completion(struct completion * kindlemem_completion);
//唤醒完成量
//只唤醒第一个结点 =kesetEvent
complete(struct completion* kindlemem_completion);
//唤醒等待队列的所有结点
complete_all(struct completion * kindlemem_completion);



//定义信号量
struct semaphore kindlemem_sem;
//初始化信号量
void sema_init(struct semaphore * kindlemem_sem, int val);
//获取信号量
void down(struct semaphore * kindlemem_sem);
int down_interruptible(struct semaphore *kindlemem_sem);
if ( down_interruptible(&kindlemem_sem))
{
return -ERESTARTSYs;
}
//释放信号量
void up(struct semaphore * kindlemem_sem);







//操作顶部中断函数 也就是中断上半部 不费时的操作
//如何在内核中注册一个中断
int request_irq(unsigned int irq, irq _handler_t handler , unsigned long flags ,const char * devname, void * dev_id)
// dev_id设备结构体
//取消一个中断
free_irq(unsigned int irq , void * dev_id)
//带自动回收的内核中断注册函数
int devm_request_irq(struct device *dev , unsigned int irq, irq_handler_t handler , l/unsigned long irqflags, const char* devname, void* dev_id)
//devm_在高版本内核是一种类似于JAVA中的垃圾回收机制的框架函数分段中断处理函数
//底部中断函数的实现方式
// tasklet机制
void kindlemem__tasklet_func(unsigned long);
//定义 赋值 初始化
DECLARE_TASKLET(name, func, data);
tasklet_schedule(struct tasket_struct * t);


//工作队列的方式
struct work_struct kindlemem_work;
void kindlemem_work_func(struct work_struct * work);
INIT_WORK(&kindlemem_work, kindlemem_work_func);
schedule_work(struct work_struct * work);


// soft irq是底部中断的机制TIMER_SOFTIRQTIMER_SOFTIRQ
//开启一个软中断
open_softirq(TIMER_SOFTIRQ, run_timer_softirq, NULL);
static void run_timer softirq(struct softirq_action *h)
struct softirq _action
//触发一个软中断
void fastcall raise_softirq(unsigned int TIMER_SOFTIRQ);


//线程中断threaded_irq
//利用线程的方式来进行底部中断的调用
int request_threaded_irq(unsigned int irq, irq_handler_t handler , irq_handler_t thread_fn, l
unsigned long irqflags, const char * devname, void* dev_id)
request_threaded_irq(pm860x- >irq[ i], NULL,
pm860x_codec_handler, IRQF_ONESHOT,pm860x- >name[i, pm860x);
IRQF_ONESHOT //这个flags的含义是默认的系统顶部中断irq_defaut primary_handler
//服务函数handler返回值为IRQ_WAKE_THREAD,那么就会调用底部中断函数的线程thread_fn/H/ IRQ HANDLED
int request_irq(unsigned int irq, irq_handler_t handler , unsigned long flags, \
const char * devname, void * dev_id)

