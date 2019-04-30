#ifndef BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP
#define BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP

#include <type_traits>

#include "executor.hpp"
#include "system_executor.hpp"

namespace boost::asio
{
namespace detail
{
template <typename> struct associated_executor_check {
  typedef void type;
};

template <typename T, typename E, typename = void> struct associated_executor_impl {
  typedef E type;

  static type get(const T&, const E& e) { return e; }
};

template <typename T, typename E>
struct associated_executor_impl<T, E, typename associated_executor_check<typename T::executor_type>::type> {
  typedef typename T::executor_type type;

  static type get(const T& t, const E&) { return t.get_executor(); }
};
}  // namespace detail

template <typename T, typename Executor = system_executor> struct associated_executor {
  typedef typename detail::associated_executor_impl<T, Executor>::type type;

  static type get(const T& t, const Executor& ex = Executor())
  {
    return detail::associated_executor_impl<T, Executor>::get(t, ex);
  }
};

template <typename T> inline typename associated_executor<T>::type get_associated_executor(const T& t)
{
  return associated_executor<T>::get(t);
}

template <typename T, typename Executor>
inline typename associated_executor<T, Executor>::type get_associated_executor(
    const T& t, const Executor& ex,
    typename std::enable_if<std::is_base_of<executor, Executor>::value ||
                            std::is_base_of<system_executor, Executor>::value>::type* = 0)
{
  return associated_executor<T, Executor>::get(t, ex);
}

template <typename T, typename ExecutionContext>
inline typename associated_executor<T, typename ExecutionContext::executor_type>::type get_associated_executor(
    const T& t, ExecutionContext& ctx,
    typename std::enable_if<std::is_base_of<ExecutionContext&, execution_context&>::value>::type* = 0)
{
  return associated_executor<T, typename ExecutionContext::executor_type>::get(t, ctx.get_executor());
}

template <typename T, typename Executor = system_executor>
using associated_executor_t = typename associated_executor<T, Executor>::type;

}  // namespace boost::asio

#endif  // !BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP