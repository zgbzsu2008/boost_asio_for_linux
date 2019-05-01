#ifndef BOOST_ASIO_DETAIL_EVENT_HPP
#define BOOST_ASIO_DETAIL_EVENT_HPP

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <thread>
#include "noncopyable.hpp"

namespace boost::asio::detail {
class event : private noncopyable
{
 public:
  template <typename Lock>
  void signal_all(Lock& lock)
  {
    assert(lock.locked());
    (void)lock;
    state_ |= 1;
    cond_.notify_all();
  }

  template <typename Lock>
  void signal(Lock& lock)
  {
    this->signal_all(lock);
  }

  template <typename Lock>
  void unlock_and_signal_one(Lock& lock)
  {
    assert(lock.locked());
    state_ |= 1;
    bool have_waiters = (state_ > 1);
    lock.unlock();
    if (have_waiters) {
      cond_.notify_one();
    }
  }

  template <typename Lock>
  bool maybe_unlock_and_signal_one(Lock& lock)
  {
    assert(lock.locked());
    state_ |= 1;
    bool have_waiters = (state_ > 1);
    if (have_waiters) {
      lock.unlock();
      cond_.notify_one();
      return true;
    }
    return false;
  }

  template <typename Lock>
  void clear(Lock& lock)
  {
    assert(lock.locked());
    (void)lock;
    state_ &= ~std::size_t(1);
  }

  template <typename Lock>
  void wait(Lock& lock)
  {
    assert(lock.locked());
    std::unique_lock<std::mutex> ulock(lock.mutex().mutex_, std::adopt_lock);
    while ((state_ & 1) == 0) {
      waiter w(state_);
      cond_.wait(ulock);  // 阻塞直到(state_&1)==1
    }
  }

  template <typename Lock>
  bool wait_for_usec(Lock& lock, long usec)
  {
    assert(lock.locked());
    std::unique_lock<std::mutex> ulock(lock.mutex().mutex_, std::adopt_lock);
    while ((state_ & 1) == 0) {
      waiter w(state_);  // 阻塞直到(state_&1)==1或者超时
      cond_.wait_for(ulock, std::chrono::microseconds(usec));
    }
    return (state_ & 1) != 0;
  }

 private:
  class waiter
  {
   public:
    explicit waiter(std::size_t& state) : state_(state) { state_ += 2; }
    ~waiter() { state_ -= 2; }

   private:
    std::size_t& state_;
  };

  std::size_t state_ = 0;
  std::condition_variable cond_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_EVENT_HPP