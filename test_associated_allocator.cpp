#include <iostream>
#include <string>
#include "associated_allocator.hpp"
//#include "associated_allocator2.hpp"
#include "recycling_allocator.hpp"

namespace associated_allocator {

using namespace boost::asio;

struct no_alloc
{};

struct has_alloc
{
  using allocator_type = typename detail::recycling_allocator<double>;
  allocator_type get_allocator() const { return allocator_type(); }
};

inline int test_associated_allocator()
{
  auto a1 = get_associated_allocator(no_alloc());
  static_assert(std::is_same<decltype(a1), std::allocator<void>>::value);

  auto a2 = get_associated_allocator(has_alloc());
  static_assert(std::is_same<decltype(a2), has_alloc::allocator_type>::value);

  auto a3 = get_associated_allocator(no_alloc(), detail::recycling_allocator<float>());
  static_assert(std::is_same<decltype(a3), detail::recycling_allocator<float>>::value);

  auto a4 = get_associated_allocator(has_alloc(), std::allocator<int>());
  static_assert(std::is_same<decltype(a4), has_alloc::allocator_type>::value);

  return 0;
}

}  // namespace associated_allocator