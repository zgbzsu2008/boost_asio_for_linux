#ifndef BOOST_ASIO_DETAIL_WORK_DISPATCHER_HPP
#define BOOST_ASIO_DETAIL_WORK_DISPATCHER_HPP

#include "associated_allocator.hpp"
#include "associated_executor.hpp"
#include "executor_work_guard.hpp"

namespace boost::asio::detail {
template <typename Handler>
class work_dispatcher
{
 public:
  using work_type = executor_work_guard<typename associated_executor<Handler>::type>;

  work_dispatcher(Handler& handler) : work_(get_associated_executor(handler)), handler_(handler) {}

  work_dispatcher(const work_dispatcher& other) : work_(other.work_), handler_(other.handler_) {}

  work_dispatcher(work_dispatcher&& other)
      : work_(std::forward<work_type>(other.work_)), handler_(std::forward(other.handler_))
  {}

  void operator()()
  {
    using alloc_type = typename associated_allocator<Handler>::type;
    alloc_type alloc(get_associated_allocator(handler_));
    work_.get_executor().dispatch(handler_, alloc);
    work_.reset();
  }

 private:
  work_type work_;
  Handler handler_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_WORK_DISPATCHER_HPP