#ifndef BOOST_ASIO_SYSTEM_EXECUTOR_HPP
#define BOOST_ASIO_SYSTEM_EXECUTOR_HPP

#include <functional>
#include "executor_op.hpp"
#include "global.hpp"
#include "system_context.hpp"

namespace boost::asio {
class system_executor
{
 public:
  system_context& context() const { return detail::global<system_context>(); }

  void on_work_started() const {}
  void on_work_finished() const {}

  template <typename Function, typename Alloc>
  void dispatch(Function&& func, const Alloc& a) const
  {
    typename std::decay<Function>::type tmp(std::forward<Function>(func));
    std::invoke(tmp);
  }

  template <typename Function, typename Alloc>
  void post(Function&& func, const Alloc& a) const
  {
    using func_type = typename std::decay<Function>::type;
    system_context& ctx = detail::global<system_context>();

    using op = detail::executor_op<func_type, Alloc>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::forward<Function>(func), a);

    ctx.scheduler_.post_immediate_completion(p.p, false);
    p.v = p.p = 0;
  }

  template <typename Function, typename Alloc>
  void defer(Function&& func, const Alloc& a) const
  {
    using func_type = typename std::decay<Function>::type;
    system_context& ctx = detail::global<system_context>();

    using op = detail::executor_op<func_type, Alloc>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::forward<Function>(func), a);

    ctx.scheduler_.post_immediate_completion(p.p, true);
    p.v = p.p = 0;
  }

  friend bool operator==(const system_context&, const system_context&) { return true; }
  friend bool operator!=(const system_context&, const system_context&) { return false; }
};
}  // namespace boost::asio

#endif  // BOOST_ASIO_SYSTEM_EXECUTOR_HPP