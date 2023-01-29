

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





