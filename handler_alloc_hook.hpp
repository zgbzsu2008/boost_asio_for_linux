#ifndef BOOST_ASIO_DETAIL_HANDLER_ALLOC_HOOK_HPP
#define BOOST_ASIO_DETAIL_HANDLER_ALLOC_HOOK_HPP

#include "thread_context.hpp"
#include "config.hpp"

namespace boost::asio {

inline void* asio_handler_allocate(std::size_t size, ...)
{
#if defined(BOOST_ASIO_SMALL_BLOCK_RECYCLING)
  return detail::thread_info_base::allocate(detail::thread_context::thread_call_stack::top(), size);
#else
  return ::operator new(size);
#endif
}

inline void asio_handler_deallocate(void* pointer, std::size_t size, ...)
{
#if defined(BOOST_ASIO_SMALL_BLOCK_RECYCLING)
  detail::thread_info_base::deallocate(detail::thread_context::thread_call_stack::top(), pointer, size);
#else
  ::operator delete(pointer, size);
#endif
}
}  // namespace boost::asio
#endif  // !BOOST_ASIO_DETAIL_HANDLER_ALLOC_HOOK_HPP
