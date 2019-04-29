#ifndef BOOST_ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP
#define BOOST_ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP

#include <memory>

#include "thread_context.hpp"
#include "thread_info_base.hpp"

namespace boost::asio::detail
{
template <typename T> class recycling_allocator
{
 public:
  using value_type = T;

  template <typename U> struct rebind {
    using other = recycling_allocator<U>;
  };

  recycling_allocator(){};
  template <typename U> recycling_allocator(const recycling_allocator<U>&) {}

  T* allocate(std::size_t n)
  {
    auto top = thread_context::thread_call_stack::top();
    void* p = thread_info_base::allocate(top, sizeof(T) * n);
    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t n)
  {
    auto top = thread_context::thread_call_stack::top();
    thread_info_base::deallocate(top, p, sizeof(T) * n);
  }
};

template <> class recycling_allocator<void>
{
 public:
  using value_type = void;

  template <typename U> struct rebind {
    using other = recycling_allocator<U>;
  };

  recycling_allocator(){};
  template <typename U> recycling_allocator(const recycling_allocator<U>&) {}
};

template <typename Alloc> struct get_recycling_allocator {
  using type = Alloc;
  static type get(const Alloc& a) { return a; };
};

// ÌØ»¯std::allocator<T>
template <typename T> struct get_recycling_allocator<std::allocator<T>> {
  using type = std::allocator<T>;
  static type get(const std::allocator<T>&) { return type(); }
};

template <typename Alloc, typename T> struct rebind_alloc {
  using type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
  // type = Alloc<T>
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP
