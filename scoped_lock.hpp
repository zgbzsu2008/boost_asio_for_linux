#ifndef BOOST_ASIO_DETAIL_SCOPED_LOCK_HPP
#define BOOST_ASIO_DETAIL_SCOPED_LOCK_HPP

#include "noncopyable.hpp"

namespace boost::asio::detail
{
template <typename T> class scoped_lock : private noncopyable
{
 public:
  enum adopt_lock_t { adopt_lock };

  scoped_lock(T& m, adopt_lock_t) : mutex_(m), locked_(true) {}

  explicit scoped_lock(T& m) : mutex_(m)
  {
    mutex_.lock();
    locked_ = true;
  }

  ~scoped_lock()
  {
    if (locked_) {
      mutex_.unlock();
    }
  }

  void lock()
  {
    if (!locked_) {
      mutex_.lock();
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

  T& mutex() { return mutex_; }

 private:
  T& mutex_;
  bool locked_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_SCOPED_LOCK_HPP