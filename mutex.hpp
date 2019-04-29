#ifndef BOOST_ASIO_DETAIL_MUTEX_HPP
#define BOOST_ASIO_DETAIL_MUTEX_HPP

#include <mutex>

#include "noncopyable.hpp"
#include "scoped_lock.hpp"

namespace boost::asio::detail
{
class event;
class mutex : private noncopyable
{
 public:
  using scoped_lock = detail::scoped_lock<mutex>;
  
  void lock() { mutex_.lock(); }
  void unlock() { mutex_.unlock(); }

 private:
  friend class event;
  std::mutex mutex_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_MUTEX_HPP