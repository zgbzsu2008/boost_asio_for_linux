#ifndef BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP
#define BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP

#include <memory>

#include "associated_allocator.hpp"
#include "noncopyable.hpp"
#include "recycling_allocator.hpp"
#include "handler_alloc_hook.hpp"

namespace handler_alloc_helpers {
template <typename Handler>
inline void* allocate(std::size_t s, Handler& h)
{
#if !defined(BOOST_ASIO_HAS_HANDLER_HOOKS)
  return ::operator new(s);
#else
  using boost::asio::asio_handler_allocate;
  return asio_handler_allocate(s, std::addressof(h));
#endif
}

template <typename Handler>
inline void deallocate(void* p, std::size_t s, Handler& h)
{
#if !defined(BOOST_ASIO_HAS_HANDLER_HOOKS)
  ::operator delete(p);
#else
  using boost::asio::asio_handler_deallocate;
  asio_handler_deallocate(p, s, std::addressof(h));
#endif
}
}  // namespace handler_alloc_helpers

namespace boost::asio::detail {
template <typename Handler, typename T>
class hook_allocator
{
 public:
  using value_type = T;

  explicit hook_allocator(Handler& h) : handler_(h) {}

  template <typename U>
  hook_allocator(const hook_allocator<Handler, U>& a) : handler_(a.handler_)
  {}

  template <typename U>
  struct rebind
  {
    using other = hook_allocator<Handler, U>;
  };

  T* allocate(std::size_t n) { return static_cast<T*>(handler_alloc_helpers::allocate(sizeof(T) * n, handler_)); }

  void deallocate(T* p, std::size_t n) { handler_alloc_helpers::deallocate(p, sizeof(T) * n, handler_); }

  Handler handler_;
};

template <typename Handler>
class hook_allocator<Handler, void>
{
 public:
  using value_type = void;

  explicit hook_allocator(Handler& h) : handler_(h) {}

  template <typename U>
  hook_allocator(const hook_allocator<Handler, U>& a) : handler_(a.handler_)
  {}

  template <typename U>
  struct rebind
  {
    using other = hook_allocator<Handler, U>;
  };

  Handler handler_;
};

template <typename Handler, typename Alloc>
struct get_hook_allocator
{
  using type = Alloc;
  static type get(Handler&, const Alloc& a) { return a; }
};

// ƫ�ػ�std::allocator<T>
template <typename Handler, typename T>
struct get_hook_allocator<Handler, std::allocator<T>>
{
  using type = hook_allocator<Handler, T>;
  static type get(Handler& handler, const std::allocator<T>&) { return type(handler); }
};
}  // namespace boost::asio::detail

#define BOOST_ASIO_DEFINE_HANDLER_PTR(op)                                                                   \
  struct ptr                                                                                                \
  {                                                                                                         \
    Handler* h;                                                                                             \
    op* v;                                                                                                  \
    op* p;                                                                                                  \
    ~ptr() { reset(); }                                                                                     \
    static op* allocate(Handler& handler)                                                                   \
    {                                                                                                       \
      using alloc_type = associated_allocator_t<Handler>;                                                   \
      using hook_type = typename get_hook_allocator<Handler, alloc_type>::type;                             \
      using alloc_op = typename rebind_alloc<hook_type, op>::type;                                          \
      alloc_op a(get_hook_allocator<Handler, alloc_type>::get(handler, get_associated_allocator(handler))); \
      return a.allocate(1);                                                                                 \
    }                                                                                                       \
    void reset()                                                                                            \
    {                                                                                                       \
      if (p) {                                                                                              \
        p->~op();                                                                                           \
        p = 0;                                                                                              \
      }                                                                                                     \
      if (v) {                                                                                              \
        using alloc_type = associated_allocator_t<Handler>;                                                 \
        using hook_type = typename get_hook_allocator<Handler, alloc_type>::type;                           \
        using alloc_op = typename rebind_alloc<hook_type, op>::type;                                        \
        alloc_op a(get_hook_allocator<Handler, alloc_type>::get(*h, get_associated_allocator(*h)));         \
        a.deallocate(static_cast<op*>(v), 1);                                                               \
        v = 0;                                                                                              \
      }                                                                                                     \
    }                                                                                                       \
  };

#define BOOST_ASIO_DEFINE_HANDLER_ALLOCATOR_PTR(op)                       \
  struct ptr                                                              \
  {                                                                       \
    const Alloc* a;                                                       \
    void* v;                                                              \
    op* p;                                                                \
    ~ptr() { reset(); }                                                   \
                                                                          \
    static op* allocate(const Alloc& a)                                   \
    {                                                                     \
      using alloc_type = typename get_recycling_allocator<Alloc>::type;   \
      using alloc_op = typename rebind_alloc<alloc_type, op>::type;       \
      alloc_op a1(get_recycling_allocator<Alloc>::get(a));                \
      return a1.allocate(1);                                              \
    }                                                                     \
    void reset()                                                          \
    {                                                                     \
      if (p) {                                                            \
        p->~op();                                                         \
        p = 0;                                                            \
      }                                                                   \
      if (v) {                                                            \
        using alloc_type = typename get_recycling_allocator<Alloc>::type; \
        using alloc_op = typename rebind_alloc<alloc_type, op>::type;     \
        alloc_op a1(get_recycling_allocator<Alloc>::get(*a));             \
        a1.deallocate(static_cast<op*>(v), 1);                            \
        v = 0;                                                            \
      }                                                                   \
    }                                                                     \
  };

#endif  // BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP