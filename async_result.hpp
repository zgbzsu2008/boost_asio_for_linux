#ifndef BOOST_ASIO_ASYNC_RESULT_HPP
#define BOOST_ASIO_ASYNC_RESULT_HPP

#include <type_traits>

#include "noncopyable.hpp"

namespace boost::asio
{
template <typename T, typename S> class async_result : private detail::noncopyable
{
 public:
  using handler_type = T;
  using result_type = void;

  explicit async_result(handler_type& h) { (void)h; }
  result_type get() {}
};

// std::decay_t<T> handler_
template <typename T, typename S> struct async_completion : private detail::noncopyable {
  using handler_type = typename async_result<typename std::decay_t<T>, S>::value_type;
  explicit async_completion(handler_type&& token) : handler_(std::move(token)), result_(handler_) {}

  handler_type handler_;
  async_result<typename std::decay_t<T>, S> result_;
};

namespace detail
{
template <typename T, typename S> struct async_result_helper : public async_result<typename std::decay_t<T>, S> {
};
}  // namespace detail
}  // namespace boost::asio

#endif  // !BOOST_ASIO_ASYNC_RESULT_HPP