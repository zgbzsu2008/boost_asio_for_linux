#ifndef BOOST_ASIO_IO_CONTEXT_HPP
#define BOOST_ASIO_IO_CONTEXT_HPP

#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <typeinfo>

#include "async_result.hpp"
#include "execution_context.hpp"
#include "noncopyable.hpp"
#include "fenced_block.hpp"
#include "executor_op.hpp"
#include "scheduler.hpp"
#include "throw_exception.hpp"

namespace boost::asio
{
namespace detail
{
#if defined(BOOST_ASIO_HAS_IOCP)
using io_context_impl = class win_iocp_io_context;
class win_iocp_overlapped_ptr;
#else
using io_context_impl = class scheduler;
#endif
}  // namespace detail

class io_context : public execution_context
{
 public:
  using impl_type = detail::io_context_impl;
#if defined(BOOST_ASIO_HAS_IOCP)
  friend class detail::win_iocp_overlapped_ptr;
#endif

  class executor_type;
  friend class executor_type;

  class service;
  class strand;

  io_context();

  explicit io_context(int concurrency_hint);

  ~io_context();

  executor_type get_executor();

  std::size_t run();

  template <typename Rep, typename Period> std::size_t run_for(const std::chrono::duration<Rep, Period>& rel_t)
  {
    return this->run_until(std::chrono::steady_clock::now() + rel_t);
  }

  template <typename Clock, typename Duration>
  std::size_t run_until(const std::chrono::time_point<Clock, Duration>& abs_t)
  {
    std::size_t n = 0;
    while (this->run_one_until(abs_t)) {
      if (n != std::numeric_limits<std::size_t>::max()) {
        ++n;
      }
    }
    return n;
  }

  std::size_t run_one();

  template <typename Rep, typename Period> std::size_t run_one_for(const std::chrono::duration<Rep, Period>& rel_t)
  {
    return this->run_one_until(std::chrono::steady_clock::now() + rel_t);
  }

  template <typename Clock, typename Duration>
  std::size_t run_one_until(const std::chrono::time_point<Clock, Duration>& abs_t)
  {
    typename Clock::time_point now = Clock::now();
    while (now < abs_t) {
      typename Clock::duration rel_t = abs_t - now();
      if (rel_t > std::chrono::seconds(1)) {
        rel_t = std::chrono::seconds(1);
      }
      std::error_code ec;
      auto ms = std::chrono::duration_cast<std::chrono::microseconds>(rel_t).count();
      std::size_t n = impl_.wait_one(static_cast<long>(ms), ec);
      detail::throw_exception(ec);
      if (n || impl_.stop()) return n;

      now = Clock::now();
    }
    return 0;
  }

  std::size_t poll();

  std::size_t poll_one();

  void stop();

  bool stopped() const;

  void restart();

  void reset();

 private:
  template <typename Service> friend Service& use_service(io_context& ioc);

  impl_type& add_impl(impl_type* impl);

  impl_type& impl_;
};

class io_context::executor_type
{
 public:
  io_context& context() const { return io_context_; }
  void on_work_started() const;
  void on_work_finished() const;

  template <typename Function, typename Alloc> void dispatch(Function&& func, const Alloc& a) const
  {
    using func_type = typename std::decay_t<Function>;

    if (running_in_this_thread()) {
      func_type tmp = std::move(func);
      detail::fenced_block b(detail::fenced_block::full);
      tmp();
      return;
    }

    /*using op = detail::executor_op<func_type, Alloc, detail::operation>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::move(func), a);
    io_context_.impl_.post_immediate_completion(p.p, false);
    p.v = p.p = 0;*/
  }
  template <typename Function, typename Alloc> void post(Function&& func, const Alloc& a) const
  {
    /*using func_type = typename std::decay_t<Function>;
    using op = detail::executor_op<func_type, Alloc, detail::operation>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::move(func), a);
    io_context_.impl_.post_immediate_completion(p.p, false);
    p.v = p.p = 0;*/
  }
  template <typename Function, typename Alloc> void defer(Function&& func, const Alloc& a) const
  {
    //using func_type = typename std::decay_t<Function>;
    //using op = detail::executor_op<func_type, Alloc, detail::operation>;
    //typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    //p.p = new (p.v) op(std::move(func), a);
    //io_context_.impl_.post_immediate_completion(p.p, true);
    //p.v = p.p = 0;
  }

  bool running_in_this_thread() const;

  friend bool operator==(const executor_type& a, const executor_type& b) { return &a.io_context_ == &b.io_context_; }

  friend bool operator!=(const executor_type& a, const executor_type& b) { return &a.io_context_ != &b.io_context_; }

 private:
  friend class io_context;
  explicit executor_type(io_context& ioc) : io_context_(ioc) {}
  io_context& io_context_;
};

class io_context::service : public execution_context::service
{
 public:
  io_context& get_io_context() { return static_cast<boost::asio::io_context&>(context()); }

 private:
  virtual void shutdown();

 protected:
  service(io_context& owner);
  virtual ~service();
};

namespace detail
{
template <typename Type> class service_base : public boost::asio::io_context::service
{
 public:
  static service_id<Type> id;
  service_base(io_context& io) : boost::asio::io_context::service(io) {}
};

template <typename Type> service_id<Type> service_base<Type>::id;

}  // namespace detail
}  // namespace boost::asio

#endif  // !BOOST_ASIO_IO_CONTEXT_HPP