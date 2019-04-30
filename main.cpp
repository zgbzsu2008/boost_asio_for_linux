#include <iostream>
#include <string>
#include <type_traits>
#include "associated_allocator2.hpp"

struct B {
};

struct A {
  using allocator_type = A;
  allocator_type get_allocator() const { return allocator_type(); }
};

/*
template <typename T, typename Allocator = std::allocator<int>, typename...>
Allocator get_associated_allocator(const T& t, const Allocator& a = Allocator(), ...)
{
  return a;
}

template <typename T, typename Allocator>
typename std::enable_if<std::is_class_v<typename T::type>, typename T::type>::type get_associated_allocator(
    const T& t, const Allocator& a)
{
  static_assert(std::is_member_function_pointer<decltype(&T::get_type)>::value);
  return t.get_type();
}
*/

int main()
{
  auto a = boost::asio::get_associated_allocator(A(), std::allocator<int>());
  (void)a;
  return 0;
}