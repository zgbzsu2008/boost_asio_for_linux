#ifndef BOOST_ASIO_DETAIL_THREAD_GROUP_HPP
#define BOOST_ASIO_DETAIL_THREAD_GROUP_HPP

#include <memory>
#include <vector>
#include "thread.hpp"

namespace boost::asio::detail
{
class thread_group
{
 public:
  thread_group() : first_(0) {}
  ~thread_group() { join(); }

  template <typename Function> void create_thread(Function f)
  {
    first_ = new item(std::move(f), first_);
  }

  template <typename Function> void create_thread(Function&& f, std::size_t num_threads)
  {
    for (std::size_t i = 0; i < num_threads; ++i) {
      create_thread(std::move(f));
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
  struct item {
    template <typename Function>
    explicit item(Function f, item* next = 0) : thread_(std::move(f)), next_(next)
    {
    }
    detail::thread thread_;
    item* next_;
  };

  item* first_;
};

}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_THREAD_GROUP_HPP
