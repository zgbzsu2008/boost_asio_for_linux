#ifndef BOOST_ASIO_ASSOCIATED_ALLOCATOR2_HPP
#define BOOST_ASIO_ASSOCIATED_ALLOCATOR2_HPP

#include <memory>
#include "has_type_member.hpp"

namespace boost::asio {
HAS_TYPE_MEMBER(allocator_type);
template <typename T, typename Alloc = std::allocator<void>>
auto get_associated_allocator(const T& t, const Alloc& a = Alloc())
{
  if constexpr (has_type_allocator_type<T>::value) {
    static_assert(std::is_member_function_pointer<decltype(&T::get_allocator)>::value);
    return t.get_allocator();
  } else {
    return a;
  }
}
}  // namespace boost::asio
#endif  // BOOST_ASIO_ASSOCIATED_ALLOCATOR2_HPP