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


