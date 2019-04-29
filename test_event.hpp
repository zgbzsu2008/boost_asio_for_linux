#include <functional>
#include <iostream>
#include <queue>

#include "conditionally_enabled_event.hpp"
#include "conditionally_enabled_mutex.hpp"
#include "thread_group.hpp"

using namespace boost::asio;

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
      if (queue_.empty()) {
        std::cout << "tid=" << std::this_thread::get_id() << " wait....\n";
        event_.clear(lock);
        event_.wait(lock);
      } else {
        int top = queue_.front();
        queue_.pop();
        if (!queue_.empty()) {
          event_.unlock_and_signal_one(lock);
        } else {
          lock.unlock();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "tid= " << std::this_thread::get_id() << " \tpop " << top << '\n';
        return 1;
      }
    }
  }

  void post(int value)
  {
    mutex::scoped_lock lock(mutex_);
    queue_.push(value);
    if (!event_.maybe_unlock_and_signal_one(lock)) {
      lock.unlock();
    }
  }

 private:
  event event_;
  mutex mutex_;
  std::queue<int> queue_;
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