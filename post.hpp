#ifndef BOOST_ASIO_POST_HPP
#define BOOST_ASIO_POST_HPP

#include "associated_allocator.hpp"
#include "associated_executor.hpp"
#include "async_result.hpp"
#include "execution_context.hpp"
#include "work_dispatcher.hpp"

namespace boost::asio {
template <typename T>
typename detail::async_result_helper<T, void()>::result_type post(T&& token)
{
  using handler = typename detail::async_result_helper<T, void()>::value_type;
  async_completion<T, void()> init(std::forward<T>(token));  // std::decay_t<T>

  typename associated_executor<handler>::type ex(get_associated_executor(init.handler_));       // system_executor
  typename associated_allocator<handler>::type alloc(get_associated_allocator(init.handler_));  // std::allocator<void>

  ex.post(init.handler_, alloc);
  return init.result_.get();
}

template <typename T, typename E>
typename detail::async_result_helper<T, void()>::result_type post(
    E& ex, T&& token, typename std::enable_if<detail::is_executor<E>::value>::type* =0)
{
  using handler = typename detail::async_result_helper<T, void()>::value_type;
  async_completion<T, void()> init(std::forward<T>(token));
  typename associated_allocator<handler>::type alloc(get_associated_allocator(init.handler_));

  ex.post(detail::work_dispatcher<handler>(init.handler_), alloc);
  return init.result_.get();
}

template <typename T, typename E>
typename detail::async_result_helper<T, void()>::result_type post(
    E& ctx, T&& token, typename std::enable_if<std::is_convertible<E&, execution_context&>::value>::type* =0)
{
  using handler = typename detail::async_result_helper<T, void()>::value_type;
  async_completion<T, void()> init(std::forward<T>(token));
  typename associated_allocator<handler>::type alloc(get_associated_allocator(init.handler_));

  ctx.get_executor().post(detail::work_dispatcher<handler>(init.handler_), alloc);
  return init.result_.get();
}
}  // namespace boost::asio
#endif  // !BOOST_ASIO_POST_HPP