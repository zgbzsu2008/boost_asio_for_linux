#ifndef BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP
#define BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP

#include <functional>
#include <type_traits>
#include "fenced_block.hpp"
#include "handler_alloc_helpers.hpp"
#include "scheduler_operation.hpp"

namespace boost::asio::detail {
template <typename Handler, typename Alloc, typename Operation = scheduler_operation>
class executor_op : public Operation
{
 public:
  BOOST_ASIO_DEFINE_HANDLER_ALLOCATOR_PTR(executor_op);

  template <typename Function>
  executor_op(Function&& func, const Alloc& a)
      : Operation(&executor_op::do_complete), handler_(std::forward(func)), alloc_(a)
  {
    static_assert(std::is_convertible<Function&, Handler&>::value);
  }

  static void do_complete(void* owner, Operation* base, const std::error_code&, std::size_t)
  {
    executor_op* o(static_cast<executor_op*>(base));
    Alloc a(o->alloc_);
    ptr p = {std::addressof(a), o, o};
    Handler hander(std::move(o->handler_));
    p.reset();
    if (owner) {
      detail::fenced_block b(detail::fenced_block::half);
      boost_asio_handler_invoke_helpers::invoke(handler, handler);
    }
  }

 private:
  Handler handler_;
  Alloc alloc_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP
