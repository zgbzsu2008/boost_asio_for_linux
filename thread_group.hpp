#ifndef BOOST_ASIO_DETAIL_THREAD_GROUP_HPP
#define BOOST_ASIO_DETAIL_THREAD_GROUP_HPP

#include <memory>
#include <vector>
#include "thread.hpp"

namespace boost::asio::detail {
class thread_group
{
 public:
  thread_group() : first_(0) {}
  ~thread_group() { join(); }

  template <typename Function>
  void create_thread(Function&& func)
  {
    first_ = new item(std::forward<Function>(func), first_);
  }

  template <typename Function>
  void create_thread(Function&& func, std::size_t num_threads)
  {
    for (std::size_t i = 0; i < num_threads; ++i) {
      create_thread(std::forward<Function>(func));
    }
  }

  void join()
  {
    while (first_) {
      first_->thread_.join();
      item* tmp = first_;
      first_ = first_->next_;
      delete tmp;
    }
  }

 private:
  struct item
  {
    template <typename Function>
    explicit item(Function&& func, item* next = 0) : thread_(std::forward<Function>(func)), next_(next)
    {}
    detail::thread thread_;
    item* next_;
  };

  item* first_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_THREAD_GROUP_HPP