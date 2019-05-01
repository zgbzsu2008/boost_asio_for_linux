#ifndef BOOST_ASIO_DETAIL_THREAD_HPP
#define BOOST_ASIO_DETAIL_THREAD_HPP

#include <thread>

#include "noncopyable.hpp"

namespace boost::asio::detail {
class thread : private noncopyable
{
 public:
  template <typename Function>
  thread(Function f, unsigned int = 0) : thread_(f)
  {}

  ~thread() { join(); }

  void join()
  {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  static std::size_t hardware_concurrency() { return std::thread::hardware_concurrency(); }

 private:
  std::thread thread_;
};

}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_THREAD_HPP