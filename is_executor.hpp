#ifndef BOOST_ASIO_DETAIL_IS_EXECUTOR_HPP
#define BOOST_ASIO_DETAIL_IS_EXECUTOR_HPP

#include <functional>
#include <type_traits>

namespace boost::asio::detail {

template <typename T> struct is_executor_impl
{
  static constexpr bool get()
  {
    if constexpr (std::is_class<T>::value) {
      if constexpr (std::is_member_function_pointer<decltype(&T::context)>::value &&
                    std::is_member_function_pointer<decltype(&T::on_work_started)>::value &&
                    std::is_member_function_pointer<decltype(&T::on_work_finished)>::value &&
                    std::is_member_function_pointer<decltype(&T::dispatch<void(), void()>)>::value &&
                    std::is_member_function_pointer<decltype(&T::post<void(), void()>)>::value &&
                    std::is_member_function_pointer<decltype(&T::defer<void(), void()>)>::value) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
};

template <typename T> struct is_executor : std::integral_constant<bool, is_executor_impl<T>::get()>
{};

};      // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_IS_EXECUTOR_HPP
