#ifndef BOOST_ASIO_STRAND_HPP
#define BOOST_ASIO_STRAND_HPP

#include "strand_executor_service.hpp"

namespace boost::asio {

template <typename Executor>
class strand
{
 public:
  using executor_type = Executor;

  strand() : executor_(), impl_(use_service<detail::strand_executor_service>(executor_.context()).create_impl()) {}

  explicit strand(const Executor& e)
      : executor_(e), impl_(use_service<detail::strand_executor_service>(executor_.context()).create_impl())
  {}

  strand(const strand& other) : executor_(other.executor_), impl_(other.impl_) {}

  template <class OtherExecutor>
  strand(const strand<OtherExecutor>& other) : executor_(other.executor_), impl_(other.impl_)
  {}

  strand& operator=(const strand& other)
  {
    executor_ = other.executor_;
    impl_ = other.impl_;
    return *this;
  }

  template <class OtherExecutor>
  strand& operator=(const strand<OtherExecutor>& other)
  {
    executor_ = other.executor_;
    impl_ = other.impl_;
    return *this;
  }
#if defined(BOOST_ASIO_HAS_MOVE)
  strand(strand&& other)
      : executor_(std::forward<Executor>(other.executor_)), impl_(std::forward<impl_type>(other.impl_))
  {}

  template <class OtherExecutor>
  strand(strand<OtherExecutor>&& other)
      : executor_(std::forward<Executor>(other.executor_)), impl_(std::forward<impl_type>(other.impl_))
  {}

  strand& operator=(strand&& other)
  {
    executor_ = std::forward<Executor>(other.executor_);
    impl_ = std::forward<impl_type>(other.impl_);
    return *this;
  }

  template <class OtherExecutor>
  strand& operator=(strand<OtherExecutor>&& other)
  {
    executor_ = std::forward<OtherExecutor>(other.executor_);
    impl_ = std::forward<impl_type>(other.impl_);
    return *this;
  }
#endif

  ~strand() {}

  executor_type get_executor() const { return executor_; }

  execution_context& context() const { return executor_.context(); }

  void on_work_started() const { executor_.on_work_started(); }

  void on_work_finished() const { executor_.on_work_finished(); }

  template <typename Function, typename Alloc>
  void dispatch(Function&& func, const Alloc& a)
  {
    detail::strand_executor_service::dispatch(impl_, executor_, std::forward<Function>(func), a);
  }

  template <typename Function, typename Alloc>
  void post(Function&& func, const Alloc& a)
  {
    detail::strand_executor_service::post(impl_, executor_, std::forward<Function>(func), a);
  }

  template <typename Function, typename Alloc>
  void defer(Function&& func, const Alloc& a)
  {
    detail::strand_executor_service::defer(impl_, executor_, std::forward<Function>(func), a);
  }

  bool running_in_this_thrand() const { return detail::strand_executor_service::running_in_this_thread(impl_); }

  friend bool operator==(const strand& a, const strand& b) { return a.impl_ == b.impl_; }

  friend bool operator!=(const strand& a, const strand& b) { return a.impl_ != b.impl_; }

 private:
  Executor executor_;
  using impl_type = detail::strand_executor_service::impl_type;
  impl_type impl_;
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_STRAND_HPP