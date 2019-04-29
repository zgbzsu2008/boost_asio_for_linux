#ifndef BOOST_ASIO_DETAIL_SCHEDULER_OPERATION_HPP
#define BOOST_ASIO_DETAIL_SCHEDULER_OPERATION_HPP

#include <system_error>

#include "op_queue.hpp"

namespace boost::asio::detail
{
class scheduler;
class scheduler_operation
{
 public:
  using func_type = void (*)(void*, scheduler_operation*, const std::error_code&, std::size_t);

  scheduler_operation(func_type func) : task_result_(0), next_(0), func_(func) {}

  void complete(void* owner, const std::error_code& ec, std::size_t bytes_transferred)
  {
    func_(owner, this, ec, bytes_transferred);
  }

  void destroy() { func_(this, 0, std::error_code(), 0); }

 protected:
  friend class scheduler;
  friend class op_queue_access;
  unsigned int task_result_;

 private:
  scheduler_operation* next_;
  func_type func_;
};
using operation = scheduler_operation;
}  // namespace boost::asio::detail


#endif  // !BOOST_ASIO_DETAIL_SCHEDULER_OPERATION_HPP