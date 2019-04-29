#ifndef BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP
#define BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP

#include "conditionally_enabled_mutex.hpp"
#include "event.hpp"

namespace boost::asio::detail
{
class conditionally_enabled_event : private noncopyable
{
 public:
  using scoped_lock = conditionally_enabled_mutex::scoped_lock;
  void signal_all(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      event_.signal_all(lock);
    }
  }

  void signal(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      event_.signal(lock);
    }
  }

  void unlock_and_signal_one(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      event_.unlock_and_signal_one(lock);
    }
  }

  bool maybe_unlock_and_signal_one(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      return event_.maybe_unlock_and_signal_one(lock);
    }
    return false;
  }

  void clear(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      event_.clear(lock);
    }
  }

  void wait(scoped_lock& lock)
  {
    if (lock.mutex_.enabled_) {
      event_.wait(lock);
    } else {
      std::this_thread::sleep_until(std::chrono::steady_clock::time_point::max());
    }
  }

  bool wait_for_usec(scoped_lock& lock, long usec)
  {
    if (lock.mutex_.enabled_) {
      return event_.wait_for_usec(lock, usec);
    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(usec));
      return true;
    }
  }

 private:
  detail::event event_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP