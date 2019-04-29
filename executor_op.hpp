#ifndef BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP
#define BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP

#include "fenced_block.hpp"
#include "handler_alloc_helpers.hpp"
#include "scheduler_operation.hpp"

namespace boost::asio::detail
{
template <typename Handler, typename Alloc, typename Operation = scheduler_operation>
class executor_op : public Operation
{
 public:
  BOOST_ASIO_DEFINE_HANDLER_ALLOCATOR_PTR(executor_op)

  template <typename H>
  executor_op(H&& h, const Alloc& a) : Operation(&executor_op::do_complete), handler_(h), alloc_(a)
  {
  }

  static void do_complete(void* owner, Operation* base, const std::error_code&, std::size_t)
  {
    executor_op* o(static_cast<executor_op*>(base));
    Alloc a(o->alloc_);
    ptr p = {std::addressof(a), o, o};
    Handler handler(std::move(o->handler_));
    p.reset();
    if (owner) {
      std_fenced_block b(std_fenced_block::half);
      handler();
    }
  }

 private:
  Handler handler_;
  Alloc alloc_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_EXECUTOR_OP_HPP