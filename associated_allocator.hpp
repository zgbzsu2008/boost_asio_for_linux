#ifndef BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP
#define BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP

#include <memory>
#include <type_traits>

namespace boost::asio
{
namespace detail
{
template <typename> struct associated_allocator_check {
  typedef void type;
};

template <typename T, typename E, typename = void> struct associated_allocator_impl {
  typedef E type;

  static type get(const T&, const E& e) { return e; }
};

template <typename T, typename E>
struct associated_allocator_impl<T, E, typename associated_allocator_check<typename T::allocator_type>::type> {
  typedef typename T::allocator_type type;

  static type get(const T& t, const E&) { return t.get_allocator(); }
};
}  // namespace detail

template <typename T, typename Allocator = std::allocator<void>> struct associated_allocator {
  typedef typename detail::associated_allocator_impl<T, Allocator>::type type;

  static type get(const T& t, const Allocator& a = Allocator())
  {
    return detail::associated_allocator_impl<T, Allocator>::get(t, a);
  }
};

template <typename T> inline typename associated_allocator<T>::type get_associated_allocator(const T& t)
{
  return associated_allocator<T>::get(t);
}

template <typename T, typename Allocator>
inline typename associated_allocator<T, Allocator>::type get_associated_allocator(const T& t, const Allocator& a)
{
  return associated_allocator<T, Allocator>::get(t, a);
}

template <typename T, typename Alloc = std::allocator<void>>
using associated_allocator_t = typename associated_allocator<T, Alloc>::type;

}  // namespace boost::asio

#endif  // BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP