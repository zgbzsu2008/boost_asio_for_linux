#include <functional>
#include <iostream>
#include <queue>

#include "conditionally_enabled_event.hpp"
#include "conditionally_enabled_mutex.hpp"
#include "object_pool.hpp"
#include "op_queue.hpp"
#include "thread_group.hpp"

#include <sys/socket.h>
#include <unistd.h>

using namespace boost::asio;

class object
{
 public:
  object(int fd) : fd_(fd) {}
  int fd() const { return fd_; }
  void destroy() {}

  object* next_;
  object* prev_;

 private:
  friend class scheduler;
  friend class op_queue_access;
  friend class object_pool_access;
  int fd_;
};

class scheduler
{
 private:
  using mutex = detail::conditionally_enabled_mutex;
  using event = detail::conditionally_enabled_event;

 public:
  std::size_t do_run()
  {
    mutex::scoped_lock lock(mutex_);
    std::size_t i;
    for (i = 0; do_run_one(lock); lock.lock()) {
      if (i < std::numeric_limits<std::size_t>::max()) {
        ++i;
      }
    }
    return i;
  }

  int do_run_one(mutex::scoped_lock& lock)
  {
    while (true) {
      if (op_queue_.empty()) {
        std::cout << "tid=" << std::this_thread::get_id() << " wait....\n";
        event_.clear(lock);
        event_.wait(lock);
      } else {
        object* top = op_queue_.front();
        op_queue_.pop();
        if (!op_queue_.empty()) {
          event_.unlock_and_signal_one(lock);
        } else {
          lock.unlock();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "tid= " << std::this_thread::get_id() << " \tpop " << top->fd() << '\n';
        pool_.free(top);
        return 1;
      }
    }
  }

  void post(int fd)
  {
    mutex::scoped_lock lock(mutex_);
    op_queue_.push(pool_.alloc(fd));
    if (!event_.maybe_unlock_and_signal_one(lock)) {
      lock.unlock();
    }
  }

 private:
  event event_;
  mutex mutex_;
  detail::object_pool<object> pool_;
  detail::op_queue<object> op_queue_;
};

inline int boost_asio_event()
{
  scheduler s;
  detail::thread_group group;
  group.create_thread(std::bind(&scheduler::do_run, std::ref(s)),
                      detail::thread::hardware_concurrency() + 2);
  for (int i = 0; i < 100; ++i) {
    std::cout << "tid= " << std::this_thread::get_id() << " push " << i << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    s.post(i);
  }
  return 0;
}