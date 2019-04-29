# boost_asio_for_linux
#### noncopyable
不可复制

#### mutex
简单封装std::mutex
```
void lock();
void unlock();
```

#### conditionally_enabled_mutex
简单封装mutex，添加是否开启功能

#### scoped_lock
类似std::unique_ptr提供更灵活的lock()/unlock()以及锁的状态locked()
```
void lock();
void unlock();
bool locked();
```

#### event
使用std::condition_variable实现事件通知
````
  void signal_all(Lock& lock);
  void signal(Lock& lock);
  void unlock_and_signal_one(Lock& lock);
  bool maybe_unlock_and_signal_one(Lock& lock);
  void wait(Lock& lock);
  bool wait_for_usec(Lock& lock, long usec);
````

#### conditionally_enabled_event
简单封装event，添加是否开启功能

#### thread
简单封装std::thread
```
template <typename Function> thread(Function f, unsigned int = 0)
void join()
static std::size_t hardware_concurrency()
```

#### thread_groud
创建thread线程组，自动加入，使用单链表存储
```
void create_thread(Function f)
void create_thread(Function&& f, std::size_t num_threads)
void join()
```

#### op_queue
队列 双向链表实现
```
Operation* front()
void pop()
void push(Operation* h)
void push(op_queue<OtherOperation>& q)
bool is_enqueued(Operation* o) const
bool empty()
friend class op_queue_access;
```

#### object_pool
对象池 维护2个双向链表，live_list_，free_list_
```
Object* first() 
Object* alloc()
Object* alloc(Arg arg)
void free(Object* o)
void destroy_list(Object* list)
```

#### global
线程安全的全局变量唯一 std::once_flag std::call_once实现线程安全静态变量
```
static std::once_flag init_once_;
std::call_once(global_impl<T>::init_once_, &global_impl<T>::do_init);
```

#### fenced_block
简单封装std::atomic_thread_fence栅栏，流程控制
```
std::atomic_thread_fence(std::memory_order_acquire);
std::atomic_thread_fence(std::memory_order_release);
```

#### throw_exception
抛出异常
```
void throw_exception(const Exception& e);
```

#### error_code
扩展std::error_code，自定义的error_code
```
enum class error_code;
inline std::error_code make_error_code(error_code code);
template <> struct is_error_code_enum<error_code> : true_type
```

#### select_interrupter
中断轮询 一般添加一个event fd，然后::write(fd)，fd可读则轮询中断
```
fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
ssize_t result = ::write(fd_, &n, sizeof(uint64_t));
ssize_t result = ::read(fd_, &n, sizeof(uint64_t));
```

#### thread_info_base
allocate流程 
1，先判断是否有足够空间，没有就分配,，有的话判断是否足够大
2，分配空间时将大小保存在尾部，回收时把大小存在头部多分配1个字节，
3，最大保存255*4字节，小于这个数的进行回收，超过丢弃
```
void* allocate(thread_info_base* this_thread, std::size_t size)
void deallocate(thread_info_base* this_thread, void* pointer, std::size_t size)
```

####scheduler_operation
封装一个调度器操作完成函数 complete
```
void (*)(void*, scheduler_operation*, const std::error_code&, std::size_t);
void complete(void*, const std::error_code&, std::size_t)
unsigned int task_result_; // 任务操作结果，比如read/write传送字节，epoll events
```

#### reactor_op
反应器操作，继承scheduler_operation，则拥有2个函数
1，处理函数 perform
2，调度完成函数 complete
例如：perform()异步读写完成-->complete()传输数据
epoll 设置events完成-->complete()传输events值
```
std::size_t bytes_transferred_;
status perform() { return perform_func_(this); }
```


