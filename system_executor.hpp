#ifndef BOOST_ASIO_SYSTEM_EXECUTOR_HPP
#define BOOST_ASIO_SYSTEM_EXECUTOR_HPP

#include "global.hpp"
#include "system_context.hpp"

namespace boost::asio
{
class system_executor
{
 public:

  system_context& context() const { return detail::global<system_context>(); }

  void on_work_started() const {}
  void on_work_finished() const {}

  // @func type = void()
  template <typename Function, typename Alloc> void dispatch(Function&& func, const Alloc& a) const { func(); }
  template <typename Function, typename Alloc> void post(Function&& func, const Alloc& a) const {}
  template <typename Function, typename Alloc> void defer(Function&& func, const Alloc& a) const {}

  friend bool operator==(const system_context&, const system_context&) { return true; }
  friend bool operator!=(const system_context&, const system_context&) { return false; }
};
}  // namespace boost::asio

#endif  // BOOST_ASIO_SYSTEM_EXECUTOR_HPP