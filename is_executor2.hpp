#ifndef BOOST_ASIO_DETAIL_IS_EXECUTOR2_HPP
#define BOOST_ASIO_DETAIL_IS_EXECUTOR2_HPP

#include <type_traits>

namespace boost::asio::detail {
template <typename T>
struct is_executor_class
    : public std::integral_constant<
          bool,
          std::is_member_function_pointer<decltype(&T::context)>::value &&
              std::is_member_function_pointer<decltype(&T::on_work_started)>::value &&
              std::is_member_function_pointer<decltype(&T::on_work_finished)>::value &&
              std::is_member_function_pointer<decltype(&T::template dispatch<void(), std::allocator<void>>)>::value &&
              std::is_member_function_pointer<decltype(&T::template post<void(), std::allocator<void>>)>::value &&
              std::is_member_function_pointer<decltype(&T::template defer<void(), std::allocator<void>>)>::value>
{};

template <typename T>
struct is_executor : public std::conditional<std::is_class<T>::value, is_executor_class<T>, std::false_type>::type
{};
};      // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_IS_EXECUTOR_HPP
