#ifndef BOOST_ASIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP
#define BOOST_ASIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP

#include <memory>
#include "execution_context.hpp"
#include "executor_op.hpp"
#include "executor_work_guard.hpp"
#include "handler_invoke_helpers.hpp"
#include "mutex.hpp"
#include "op_queue.hpp"
#include "scheduler_operation.hpp"

namespace boost::asio::detail {

class strand_executor_service : public execution_context_service_base<strand_executor_service>
{
 public:
  class strand_impl
  {
   public:
    ~strand_impl()
    {
      // 链表中移除this
      if (service_->impl_list_ == this) {
        service_->impl_list_ = next_;
      }
      if (prev_) {
        prev_->next_ = next_;
      }
      if (next_) {
        next_->prev_ = prev_;
      }
    }

   private:
    friend class strand_executor_service;
    detail::mutex* mutex_;
    bool locked_;  // 判断锁是否被持有
    bool shutdown_;
    op_queue<scheduler_operation> waiting_queue_;
    op_queue<scheduler_operation> ready_queue_;
    strand_impl* next_;
    strand_impl* prev_;
    strand_executor_service* service_;
  };

  using impl_type = std::shared_ptr<strand_impl>;

  explicit strand_executor_service(execution_context& ctx)
      : execution_context_service_base<strand_executor_service>(ctx), mutex_(), salt_(0), impl_list_(0)
  {}

  void shutdown()
  {
    op_queue<scheduler_operation> ops;
    mutex::scoped_lock lock(mutex_);
    while (strand_impl* impl = impl_list_) {
      impl->mutex_->lock();
      impl->shutdown_ = true;
      ops.push(impl->waiting_queue_);
      ops.push(impl->ready_queue_);
      impl->mutex_->unlock();
      impl = impl->next_;
    }
  }

  impl_type create_impl()
  {
    impl_type new_impl(new strand_impl);
    new_impl->locked_ = false;
    new_impl->shutdown_ = false;
    mutex::scoped_lock lock(mutex_);

    // TEA 算法
    std::size_t salt = salt_++;
    std::size_t mutex_index = reinterpret_cast<std::size_t>(new_impl.get());
    mutex_index += (reinterpret_cast<std::size_t>(new_impl.get()) >> 3);
    mutex_index ^= salt + 0x9e3779b9 + (mutex_index << 6) + (mutex_index >> 2);
    mutex_index = mutex_index % num_mutexes;
    if (!mutexes_[mutex_index].get()) {
      mutexes_[mutex_index].reset(new mutex);
    }
    new_impl->mutex_ = mutexes_[mutex_index].get();

    // 链表中添加this
    new_impl->next_ = impl_list_;
    new_impl->prev_ = 0;
    if (impl_list_) {
      impl_list_->prev_ = new_impl.get();
    }
    impl_list_ = new_impl.get();
    new_impl->service_ = this;

    return new_impl;
  }

  template <typename Executor, typename Function, typename Alloc>
  static void dispatch(const impl_type& impl, Executor& ex, Function&& func, const Alloc& a)
  {
    using func_type = typename std::decay_t<Function>;
    if (call_stack<strand_impl>::contains(impl.get())) {
      func_type tmp(std::forward<Function>(func));
      fenced_block b(fenced_block::full);
      boost_asio_handler_invoke_helpers::invoke(tmp, tmp);
      return;
    }
    using op = executor_op<func_type, Alloc>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::forward<Function>(func), a);

    bool first = enqueue(impl, p.p);
    p.v = p.p = 0;
    if (first) {
      ex.dispatch(invoker<Executor>(impl, ex), a);
    }
  }

  template <typename Executor, typename Function, typename Alloc>
  static void post(const impl_type& impl, Executor& ex, Function&& func, const Alloc& a)
  {
    using func_type = typename std::decay_t<Function>;
    using op = executor_op<func_type, Alloc>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::forward<Function>(func), a);

    bool first = enqueue(impl, p.p);
    p.v = p.p = 0;
    if (first) {
      ex.post(invoker<Executor>(impl, ex), a);
    }
  }

  template <typename Executor, typename Function, typename Alloc>
  static void defer(const impl_type& impl, Executor& ex, Function&& func, const Alloc& a)
  {
    using func_type = typename std::decay_t<Function>;
    using op = executor_op<func_type, Alloc>;
    typename op::ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    p.p = new (p.v) op(std::forward<Function>(func), a);

    bool first = enqueue(impl, p.p);
    p.v = p.p = 0;
    if (first) {
      ex.defer(invoker<Executor>(impl, ex), a);
    }
  }

  static bool running_in_this_thread(const impl_type& impl) { return !!call_stack<strand_impl>::contains(impl.get()); }

 private:
  friend class strand_impl;
  template <typename Executor>
  class invoker
  {
   public:
    invoker(const impl_type& impl, Executor& ex) : impl_(impl), work_(ex) {}
    invoker(const invoker& other) : impl_(other.impl), work_(other.work_) {}
#if defined(BOOST_ASIO_HAS_MOVE)
    invoker(invoker&& other)
        : impl_(std::forward<impl_type>(other.impl_)), work_(std::forward<executor_work_guard<Executor>>(other.impl_))
    {}
#endif
    struct on_invoker_exit
    {
      invoker* this_;
      ~on_invoker_exit()
      {
        this_->impl_->mutex_->lock();
        this_->impl_->ready_queue_.push(this_->impl_->waiting_queue_);
        bool more_handlers = this_->impl_->locked_ = !this_->impl_->ready_queue_.empty();
        this_->impl_->mutex_->unlock();

        if (more_handlers) {
          Executor ex(this_->work_.get_executor());
          recycling_allocator<void> alloc;
          ex.post(std::move(*this_), alloc);
        }
      }
    };

    void operator()()
    {
      call_stack<strand_impl>::contains ctx(impl_.get());
      on_invoker_exit on_exit = {this};
      (void)on_exit;
      
      std::error_code ec;
      while (scheduler_operation* o = impl_->ready_queue_.front()) {
        impl_->ready_queue_.pop();
        o->complete(impl_.get(), ec, 0);
      }
    }

   private:
    impl_type impl_;
    executor_work_guard<Executor> work_;
  };

  static bool enqueue(const impl_type& impl, scheduler_operation* op)
  {
    impl->mutex_->lock();
    if (impl->shutdown_) {
      impl->mutex_->unlock();
      op->destroy();
      return false;
    } else if (impl->locked_) {  // 锁已被持有，等待中...
      impl->waiting_queue_.push(op);
      impl->mutex_->unlock();
      return false;
    } else {
      impl->locked_ = true;
      impl->mutex_->unlock();
      impl->ready_queue_.push(op);
      return true;
    }
  }

  detail::mutex mutex_;

  static constexpr int num_mutexes = 193;
  std::shared_ptr<mutex> mutexes_[num_mutexes];

  std::size_t salt_;
  strand_impl* impl_list_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP
