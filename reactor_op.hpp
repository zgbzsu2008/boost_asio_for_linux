#ifndef BOOST_ASIO_DETAIL_REACTOR_OP_HPP
#define BOOST_ASIO_DETAIL_REACTOR_OP_HPP

#include "scheduler_operation.hpp"

namespace boost::asio::detail
{
class reactor_op : public scheduler_operation
{
 public:
  enum status { not_done, done, done_and_exhausted };

  std::error_code ec_;
  std::size_t bytes_transferred_;

  status perform() { return perform_func_(this); }

 protected:
  using perform_func_type = status (*)(reactor_op*);

  reactor_op(perform_func_type perform_func, func_type complete_func)
      : scheduler_operation(complete_func), bytes_transferred_(0), perform_func_(perform_func)
  {
  }

 private:
  perform_func_type perform_func_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_REACTOR_OP_HPP