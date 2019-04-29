#ifndef BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP
#define BOOST_ASIO_ASSOCIATED_ALLOCATOR_HPP

#include <memory>
#include <type_traits>

#include <boost/asio/detail/type_check.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T, typename E, typename = void>
struct associated_allocator_impl {
  using type = E;
  static type get(const T&, const E& e) { return e; }
};

// ƫ�ػ�����T::allocator_type������ӵ��get_allocator()
template <typename T, typename E>
struct associated_allocator_impl<T, E, typename type_check<typename T::allocator_type>::type> {
  using type = typename T::allocator_type;
  static type get(const T& t, const E&) { return t.get_allocator(); }
};

}  // namespace detail

// ����T/Alloc��ص��ڴ������
template <typename T, typename Alloc = std::allocator<void>>
struct associated_allocator {
  using type = typename detail::associated_allocator_impl<T, Alloc>::type;
  static type get(const T& t, const Alloc& a = Alloc()) {
    return detail::associated_allocator_impl<T, Alloc>::get(t, a);
  }
};

// ����T���߲���Alloc��get
template <typename T, typename Alloc>
inline typename associated_allocator<T, Alloc>::type get_associated_allocator(
    const T& t, const Alloc& a) {
  return associated_allocator<T, Alloc>::get(t, a);
}

// �ػ�std::allocator<T> 
template <typename T>
inline typename associated_allocator<T>::type get_associated_allocator(const T& t) {
  return associated_allocator<T>::get(t);
}

// ����T��get
template <typename T, typename Alloc>
inline typename associated_allocator<T, Alloc>::type get_associated_allocator(
    const T& t, const Alloc& a,
    typename detail::type_check<typename T::allocator_type>::type* = 0) {
  return associated_allocator<T, Alloc>::get(t, a);
}

template <typename T, typename Alloc = std::allocator<void>>
using associated_allocator_t = typename associated_allocator<T, Alloc>::type;

}  // namespace asio
}  // namespace boost

#endif