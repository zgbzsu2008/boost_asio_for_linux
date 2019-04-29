#ifndef BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP
#define BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP

#include <type_traits>

#include "system_executor.hpp"

namespace boost::asio
{
namespace detail
{
template <typename T, typename E, typename...> struct associated_executor_impl {
  using type = E;
  static type get(const T&, const E& e) { return e; }
};

template <typename T, typename E> struct associated_executor_impl<T, E, typename T::executor_type> {
  using type = typename T::executor_type;
  static type get(const T& t, const E& e) { return t.get_executor(); }
};
}  // namespace detail

template <typename T, typename E = system_executor> struct associated_executor {
  using type = typename detail::associated_executor_impl<T, E>::type;
  static type get(const T& t, const E& ex = E()) { return detail::associated_executor_impl<T, E>::get(t, ex); }
};

// �ػ�E=system_executor
template <typename T> typename associated_executor<T>::type get_associated_executor(const T& t)
{
  return associated_executor<T>::get(t);
}

// ����E��ȡ
template <typename T, typename E>
typename associated_executor<T, E>::type get_associated_executor(const T& t, const E& ex)
{
  return associated_executor<T, E>::get(t, ex);
}

// ����T��ȡ
template <typename T, typename E>
typename associated_executor<T, typename E::executor_type>::type get_associated_executor(const T& t, E& ctx)
{
  return associated_executor<T, typename E::executor_type>::get(t, ctx.get_executor());
}

template <typename T, typename E = system_executor>
using associated_executor_t = typename associated_executor<T, E>::type;

}  // namespace boost::asio

#endif