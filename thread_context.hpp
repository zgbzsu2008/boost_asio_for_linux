#ifndef BOOST_ASIO_DETAIL_THREAD_CONTEXT_HPP
#define BOOST_ASIO_DETAIL_THREAD_CONTEXT_HPP

#include "call_stack.hpp"
#include "thread_info_base.hpp"

namespace boost::asio::detail
{
class thread_context
{
 public:
  using thread_call_stack = call_stack<thread_context, thread_info_base>;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_THREAD_CONTEXT_HPP
