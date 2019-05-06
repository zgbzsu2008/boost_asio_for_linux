#ifndef BOOST_ASIO_ASYNC_RESULT_HPP
#define BOOST_ASIO_ASYNC_RESULT_HPP

#include <type_traits>
#include "config.hpp"
#include "noncopyable.hpp"

namespace boost::asio {
template <typename T, typename S>
class async_result : private detail::noncopyable
{
 public:
  using value_type = T;
  using result_type = void;

  explicit async_result(value_type& h) { (void)h; }
  result_type get() {}
};

// std::decay_t<T> handler_
template <typename T, typename S>
struct async_completion : private detail::noncopyable
{
  using handler_type = typename async_result<typename std::decay_t<T>, S>::value_type;

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  explicit async_completion(T& token)
      : handler_(static_cast<typename std::conditional_t<std::is_same_v<T, handler_type>, handler_type&, T&&>>(token)),
        result_(handler_)
  {}
#else
  explicit async_completion(typename std::decay_t<T>& token) : handler_(token), result(handler_) {}
  explicit async_completion(const typename std::decay_t<T>& token) : handler_(token), result(handler_) {}
#endif

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  typename std::conditional_t<std::is_same_v<T, handler_type>, handler_type&, handler_type> handler_;
#else
  handler_type handler_;
#endif

  async_result<typename std::decay_t<T>, S> result_;
};

namespace detail {
template <typename T, typename S>
struct async_result_helper : public async_result<typename std::decay_t<T>, S>
{};
}  // namespace detail
}  // namespace boost::asio

#endif  // !BOOST_ASIO_ASYNC_RESULT_HPP