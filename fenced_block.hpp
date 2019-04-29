#ifndef BOOST_ASIO_DETAIL_FENCED_BLOCK_HPP
#define BOOST_ASIO_DETAIL_FENCED_BLOCK_HPP

#include <atomic>

#include "noncopyable.hpp"

namespace boost::asio::detail
{
class fenced_block : private noncopyable
{
 public:
  enum half_t { half };
  enum full_t { full };

  explicit fenced_block(half_t) {}

  explicit fenced_block(full_t) { std::atomic_thread_fence(std::memory_order_acquire); }

  ~fenced_block() { std::atomic_thread_fence(std::memory_order_release); }
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_FENCED_BLOCK_HPP