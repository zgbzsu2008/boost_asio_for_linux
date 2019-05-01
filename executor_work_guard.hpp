#ifndef BOOST_ASIO_EXECUTOR_WORK_GUARD_HPP
#define BOOST_ASIO_EXECUTOR_WORK_GUARD_HPP

#include "associated_executor.hpp"
#include "is_executor.hpp"

namespace boost::asio {

template <typename Executor>
class executor_work_guard
{
 public:
  using executor_type = Executor;
  executor_type get_executor() const { return executor_; }

  explicit executor_work_guard(const executor_type& e) : executor_(e), owns_(true) { executor_.on_work_started(); }

  executor_work_guard(const executor_work_guard& other) : executor_(other.executor_), owns_(other.owns_)
  {
    if (owns_) {
      executor_.on_work_started();
    }
  }

  executor_work_guard(executor_work_guard&& other)
      : executor_(std::forward<executor_type>(other.executor_)), owns_(other.owns_)
  {
    other.owns_ = false;
  }

  ~executor_work_guard()
  {
    if (owns_) {
      executor_.on_work_finished();
    }
  }

  bool owns_work() const { return owns_; }

  void reset()
  {
    if (owns_) {
      executor_.on_work_finished();
    }
  }

 private:
  executor_work_guard& operator=(const executor_work_guard&) = delete;
  executor_type executor_;
  bool owns_;
};

template <typename Executor>
inline executor_work_guard<Executor> make_work_guard(
    const Executor& ex, typename std::enable_if<detail::is_executor<Executor>::value>::type* = 0)
{
  return executor_work_guard<Executor>(ex);
}

template <typename Context>
inline executor_work_guard<typename Context::executor_type> make_work_guard(
    Context& ctx, typename std::enable_if<std::is_convertible<Context&, execution_context&>::value>::type* = 0)
{
  return executor_work_guard<typename Context::executor_type>(ctx.get_executor());
}
}  // namespace boost::asio
#endif  // BOOST_ASIO_EXECUTOR_WORK_GUARD_HPP