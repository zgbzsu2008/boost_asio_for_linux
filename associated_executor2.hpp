#ifndef BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP
#define BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP

#include "executor.hpp"
#include "has_type_member.hpp"
#include "is_executor2.hpp"
#include "system_executor.hpp"

namespace boost::asio {
HAS_TYPE_MEMBER(executor_type);
template <typename T, typename Executor = system_executor>
auto get_associated_executor(const T& t, const Executor& ex = Executor())
{
  if constexpr (has_type_executor_type<T>::value) {
    static_assert(std::is_convertible<T&, execution_context&>::value);
    static_assert(std::is_member_function_pointer<decltype(&T::get_executor)>::value);
    static_assert(detail::is_executor<typename T::executor_type>::value);
    return const_cast<T*>(&t)->get_executor();
  } else {
    static_assert(detail::is_executor<Executor>::value);
    return ex;
  }
}
}  // namespace boost::asio
#endif  // !BOOST_ASIO_ASSOCIATED_EXECUTOR_HPP