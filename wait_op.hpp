#ifndef BOOST_ASIO_DETAIL_WAIT_OP_HPP
#define BOOST_ASIO_DETAIL_WAIT_OP_HPP

#include "scheduler_operation.hpp"

namespace boost::asio::detail {

class wait_op : public scheduler_operation
{
 public:
  std::error_code ec_;
  wait_op(func_type func) : scheduler_operation(func) {}
};

}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_WAIT_OP_HPP
