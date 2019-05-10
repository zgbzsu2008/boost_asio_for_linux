#ifndef BOOST_ASIO_BIND_EXECUTOR_HPP
#define BOOST_ASIO_BIND_EXECUTOR_HPP

#include "associated_allocator.hpp"
#include "associated_executor.hpp"
#include "async_result.hpp"
#include "execution_context.hpp"
#include "uses_executor.hpp"
#include "is_executor.hpp"

namespace boost::asio::detail {

template <typename Function, typename Executor, bool UsesExecutor>
class executor_binder_base;

template <typename Function, typename Executor, bool UsesExecutor>
class executor_binder_base<Function, Executor, true> : protected Executor
{
 protected:
   template <typename E, typename U>
  executor_binder_base(E&& e, U&& u) : executor_(std::forward(e)), target_(executor_arg_t(), executor_, std::forward(u))
  {}

  Executor executor_;
  Function target_;
};

template <typename Function, typename Executor, bool UsesExecutor>
class executor_binder_base<Function, Executor, false>
{
 protected:
  template <typename E, typename U>
  executor_binder_base(E&& e, U&& u) : executor_(std::forward(e)), target_(executor_arg_t(), executor_, std::forward(u))
  {}

  Executor executor_;
  Function target_;
};

template <typename Function, typename Executor>
class executor_binder;


template <typename R, typename ... Args, typename Executor>
class executor_binder<R(Args...), Executor>
{
 public:
  using target_type = std::function<R(Args...)>;
  using executor_type = Executor;

  using result_type = R;

   target_type& get()  { return this->target_; }
  const target_type& get() const  { return this->target_; }

  executor_type get_executor() const  { return this->executor_; }

  template <typename... Args>
  auto operator()(Args&&... args)
  {
   return this->target_(std::forward<Args>(args...));
  }

  template <typename... Args>
  auto operator()(Args&&... args) const
  {
    return this->target_(std::forward<Args>(args...));
  }
};

template <typename Executor, typename Function>
inline executor_binder<typename std::decay_t<Function>, Executor> bind_executor(
    const Executor& ctx, Function&& func, typename std::enable_if_t<is_executor<Executor>::value>* = 0)
{
  return executor_binder<typename std::decay_t<Function>, Executor>(executor_arg_t(), ex, std::forward(t));
}

template <typename Context, typename Function>
inline executor_binder<typename std::decay_t<Function>, typename Context::executor_type> bind_executor(
    Context& ctx, Function&& func, typename std::enable_if_t<std::is_convertible_v<Context&, execution_context&>>* = 0)
{
  return executor_binder<typename std::decay_t<Function>, typename Context::executor_type>(
      executor_arg_t(), ctx.get_executor(), std::forward(t));
}

// uses_executor特化
template <typename T, typename Executor>
struct uses_executor<executor_binder<T, Executor>, Executor> : std::true_type
{};

template <typename T, typename Executor, typename S>
class async_result<executor_binder<T, Executor>, S>
{
 public:
  using handler_type = executor_binder<typename async_result<T, S>::handler_type, Executor>;
  using return_type = typename async_result<T, S>::return_type;

  explicit async_result(executor_binder<T, Executor>& b) : target_(b.get()) {}

  return_type get() { return target_.get(); }

 private:
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  async_result<T, S> target_;
};

// associated_allocator特化
template <typename T, typename Executor, typename Alloc>
struct associated_allocator<executor_binder<T, Executor>, Alloc>
{
  using type = typename associated_allocator<T, Alloc>::type;

  static type get(const executor_binder<T, Alloc>& b, const Alloc& a = Alloc())
  {
    return associated_allocator<T, Alloc>::get(b.get(), a);
  }
};

// 特化associated_executor
template <typename T, typename Executor, typename Executor1>
struct associated_executor<executor_binder<T, Executor>, Executor1>
{
  using type = Executor;

  static type get(const executor_binder<T, Alloc>& b, const Executor1& = Executor1()) { return b.get_executor(); }
};
}  // namespace boost::asio::detail
#endif
