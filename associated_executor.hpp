#ifndef BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP
#define BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP

#include "executor.hpp"
#include "is_executor.hpp"
#include "system_executor.hpp"

namespace boost::asio {
namespace detail {

template <typename> struct associated_executor_check
{
  using type = void;
};

template <typename T, typename E, typename = void> struct associated_executor_impl
{
  using type = E;
  static type get(const T&, const E& e) { return e; }
};

template <typename T, typename E>
struct associated_executor_impl<T, E, typename associated_executor_check<typename T::executor_type>::type>
{
  using type = typename T::executor_type;

  static type get(const T& t, const E&) { return const_cast<T*>(&t)->get_executor(); }
};
}  // namespace detail

template <typename T, typename Executor = system_executor> struct associated_executor
{
  using type = typename detail::associated_executor_impl<T, Executor>::type;
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
    const T& t, const Executor& ex, typename std::enable_if<detail::is_executor<Executor>::value>::type* = 0)
{
  return associated_executor<T, Executor>::get(t, ex);
}

template <typename T, typename Context>
inline typename associated_executor<T, typename Context::executor_type>::type get_associated_executor(
    const T& t, Context& ctx,
    typename std::enable_if<std::is_convertible<Context&, execution_context&>::value>::type* = 0)
{
  return associated_executor<T, typename Context::executor_type>::get(t, ctx.get_executor());
}

template <typename T, typename Executor = system_executor>
using associated_executor_t = typename associated_executor<T, Executor>::type;

}  // namespace boost::asio
#endif  // !BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP