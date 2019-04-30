#ifndef BOOST_ASIO_DETAIL_HAS_TYPE_MEMBER_HPP
#define BOOST_ASIO_DETAIL_HAS_TYPE_MEMBER_HPP

#include <type_traits>

namespace boost::asio::detail {
// template <typename... Ts> struct make_void { typedef void type; };
// template <typename... Ts> using void_t = typename make_void<Ts...>::type;

template <class, class = std::void_t<>> struct has_type_member : std::false_type {};
template <class T> struct has_type_member<T, std::void_t<typename T::type>> : std::true_type {};
}  // namespace boost::asio::detail

#define HAS_TYPE_MEMBER(member)                                                                  \
  template <typename T, typename = std::void_t<>> struct has_type_##member : std::false_type {}; \
  template <typename T> struct has_type_##member<T, std::void_t<typename T::member>> : std::true_type {};

#endif  // !BOOST_ASIO_DETAIL_HAS_TYPE_MEMBER_HPP
