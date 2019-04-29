#ifndef BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_MUTEX_HPP
#define BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_MUTEX_HPP

#include "mutex.hpp"

namespace boost::asio::detail
{
class conditionally_enabled_mutex : private noncopyable
{
 public:
  class scoped_lock : private noncopyable
  {
   public:
    enum adopt_lock_t { adopt_lock };

    scoped_lock(conditionally_enabled_mutex& m, adopt_lock_t) : mutex_(m), locked_(m.enabled_) {}

    explicit scoped_lock(conditionally_enabled_mutex& m) : mutex_(m)
    {
      if (m.enabled_) {
        mutex_.mutex_.lock();
        locked_ = true;
      } else {
        locked_ = false;
      }
    }

    ~scoped_lock()
    {
      if (locked_) {
        mutex_.mutex_.unlock();
        locked_ = false;
      }
    }

    void lock()
    {
      if (mutex_.enabled_ && !locked_) {
        mutex_.mutex_.lock();
        locked_ = true;
      }
    }

    void unlock()
    {
      if (locked_) {
        mutex_.unlock();
        locked_ = false;
      }
    }

    bool locked() const { return locked_; }

    detail::mutex& mutex() { return mutex_.mutex_; }

   private:
    friend class conditionally_enabled_event;
    conditionally_enabled_mutex& mutex_;
    bool locked_;
  };

  explicit conditionally_enabled_mutex(bool on=true) : enabled_(on) {}

  void lock()
  {
    if (enabled_) {
      mutex_.lock();
    }
  }

  void unlock()
  {
    if (enabled_) {
      mutex_.unlock();
    }
  }

  bool enabled() const { return enabled_; }

 private:
  friend class scoped_lock;
  friend class conditionally_enabled_event;
  detail::mutex mutex_;
  bool enabled_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_CONDITIONALLY_ENABLED_MUTEX_HPP