#ifndef BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP
#define BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP

#include <memory>
#include <type_traits>

namespace boost::asio
{
namespace detail
{
template <typename T, typename E, typename ...> struct associated_allocator_impl {
  using type = E;
  static type get(const T&, const E& e) { return e; }
};

template <typename T, typename E> struct associated_allocator_impl<T, E, typename T::allocator_type> {
  using type = typename T::allocator_type;
  static type get(const T& t, const E&) { return t.get_allocator(); }
};

}  // namespace detail

// 参数T/Alloc相关的内存分配器
template <typename T, typename Alloc = std::allocator<void>> struct associated_allocator {
  using type = typename detail::associated_allocator_impl<T, Alloc>::type;
  static type get(const T& t, const Alloc& a = Alloc())
  {
    return detail::associated_allocator_impl<T, Alloc>::get(t, a);
  }
};

// 参数T或者参数Alloc中get
template <typename T, typename Alloc, typename ...>
typename associated_allocator<T, Alloc>::type get_associated_allocator(const T& t, const Alloc& a)
{
  return associated_allocator<T, Alloc>::get(t, a);
}

// 参数T中get
template <typename T, typename Alloc, typename T::allocator_type>
inline typename associated_allocator<T, Alloc>::type get_associated_allocator(const T& t, const Alloc& a)
{
  return associated_allocator<T, Alloc>::get(t, a);
}

// 特化std::allocator<T>
template <typename T>
typename associated_allocator<T>::type get_associated_allocator(const T& t)
{
  return associated_allocator<T>::get(t);
}

template <typename T, typename Alloc = std::allocator<void>>
using associated_allocator_t = typename associated_allocator<T, Alloc>::type;

}  // namespace boost::asio

#endif  // BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP