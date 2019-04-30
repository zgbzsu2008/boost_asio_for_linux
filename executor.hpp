#ifndef BOOST_ASIO_EXECUTOR_HPP
#define BOOST_ASIO_EXECUTOR_HPP

#include <atomic>
#include <exception>
#include <memory>
#include <typeinfo>

#include "execution_context.hpp"
#include "handler_alloc_helpers.hpp"
#include "noncopyable.hpp"
#include "scheduler_operation.hpp"
#include "executor_op.hpp"

namespace boost::asio {
class bad_executor : std::exception
{
 public:
  virtual const char* what() const noexcept { return "bad_executor"; }
};

class executor
{
 public:
  executor() : impl_(0) {}
  executor(nullptr_t) : impl_(0) {}  // std::nullptr_t 的重载以接受空指针常量

  executor(const executor& other) : impl_(other.clone()) {}
  executor(executor&& other) : impl_(other.impl_) { other.impl_ = 0; }

  template <typename Executor> executor(Executor e);

  template <typename Executor, typename Alloc> executor(std::allocator_arg_t, const Alloc& a, Executor e);

  ~executor() { destroy(); }

  executor& operator=(const executor& other)
  {
    destroy();
    impl_ = other.clone();
    return *this;
  }

  executor& operator=(executor&& other)
  {
    destroy();
    impl_ = other.impl_;
    other.impl_ = 0;
    return *this;
  }

  executor& operator=(nullptr_t)
  {
    destroy();
    impl_ = 0;
    return *this;
  }

  template <typename Executor> executor& operator=(Executor e)
  {
    executor tmp(e);
    destroy();
    impl_ = tmp.impl_;
    tmp.impl_ = 0;
    return *this;
  }

  execution_context& context() const { return get_impl()->context(); }
  void on_work_started() const { get_impl()->on_work_started(); }
  void on_work_finished() const { get_impl()->on_work_finished(); }

  // @func type = void()
  template <typename Function, typename Alloc> void dispatch(Function&& func, const Alloc& a) const
  {
    get_impl()->dispatch(function(func, a));
  }

  template <typename Function, typename Alloc> void post(Function&& func, const Alloc& a) const
  {
    get_impl()->post(function(func, a));
  }

  template <typename Function, typename Alloc> void defer(Function&& func, const Alloc& a) const
  {
    get_impl()->defer(function(func, a));
  }

 private:
  using type_id_result_type = const std::type_info&;
  template <typename T> static type_id_result_type type_id() { return typeid(T); }

  class function;
  template <typename, typename> class impl;
  friend class system_executor;

  class impl_base
  {
   public:
    virtual impl_base* clone() const = 0;
    virtual void destroy() = 0;
    virtual execution_context& context() = 0;
    virtual void on_work_started() = 0;
    virtual void on_work_finished() = 0;
    virtual void dispatch(function func) = 0;
    virtual void post(function func) = 0;
    virtual void defer(function func) = 0;
    virtual type_id_result_type target_type() const = 0;
    virtual void* target() = 0;
    virtual const void* target() const = 0;
    virtual bool equals(const impl_base* e) const = 0;

   protected:
    impl_base(bool fast_dispatch) : fast_dispatch_(fast_dispatch) {}
    virtual ~impl_base() = default;

   private:
    friend class executor;
    const bool fast_dispatch_;
  };

  impl_base* get_impl() const
  {
    if (!impl_) {
      throw bad_executor();
    }
    return impl_;
  }

  impl_base* clone() const { return impl_ ? impl_->clone() : 0; }

  void destroy()
  {
    if (impl_) {
      impl_->destroy();
    }
  }

  impl_base* impl_;
};

template <typename Executor> inline executor::executor(Executor e) {}

template <typename Executor, typename Alloc> class executor::impl : public executor::impl_base
{
 public:
  using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<impl>;

  static impl_base* create(const Executor& e, Alloc a = Alloc())
  {
    raw_mem mem(a);
    impl* p = new (mem.ptr_) impl(e, a);
    mem.ptr_ = 0;
    return p;
  }

  impl(const Executor& e, const Alloc& a) : impl_base(false), ref_count_(1), executor_(e), alloc_(a) {}

  ~impl() {}

  impl_base* clone() const
  {
    ++ref_count_;
    return const_cast<impl_base*>(static_cast<const impl_base*>(this));
  }

  void destory()
  {
    if (--ref_count_ == 0) {
      alloc_type a(alloc_);
      impl* p = this;
      p->~impl();
      a.decallocate(p, 1);
    }
  }

  void on_work_started() { return executor_.on_work_started(); }

  void on_work_finished() { executor_.on_work_finished(); }

  execution_context& context() { return executor_.context(); }

  void dispatch(function&& f) { executor_.dispatch(f, alloc_); }

  void post(function&& f) { executor_.post(f, alloc_); }

  void defer(function&& f) { executor_.defer(f, alloc_); }

  type_id_result_type target_type() { return typeid(Executor); }

  void* target() { return &executor_; }

  bool equals(const impl_base* e) const
  {
    if (this == e) {
      return true;
    }
    if (target_type() != e->target_type()) {
      return false;
    }
    return executor_ = *static_cast<const Executor*>(e->target());
  }

 private:
  mutable std::atomic<long> ref_count_;
  Executor executor_;
  Alloc alloc_;

  struct raw_mem : public noncopyable
  {
    alloc_type alloc_;
    impl* ptr_;
    explicit raw_mem(const Alloc& a) : alloc_(a), ptr_(alloc_.allocate(1)) {}

    ~raw_mem()
    {
      if (ptr_) {
        alloc_.deallocate(ptr_, 1);
      }
    }
  };
};

class executor::function
{
 public:
  template <typename F, typename Alloc> explicit function(F& func, const Alloc& a)
  {
    /*using op = detail::executor_op<F, Alloc>;
    using ptr_type = detail::ptr<op, Alloc>;
    ptr_type ptr;
    typename ptr p = {std::addressof(a), op::ptr::allocate(a), 0};
    op_ = new (p.v) op(func, a);
    p.v = 0;*/
  }

  function(function&& other) : op_(other.op_) { other.op_ = 0; }

  ~function()
  {
    if (op_) {
      op_->destroy();
    }
  }

  void operator()()
  {
    if (op_) {
      detail::scheduler_operation* op = op_;
      op_ = 0;
      op->complete(this, std::error_code(), 0);
    }
  }

 private:
  detail::scheduler_operation* op_;
};

/*
// 静态函数方法实现虚函数调用
class executor::function
{
public:
  template <typename F, typename Alloc>
  explicit function(const F& f, const Alloc&) : impl_(new impl<F>(f))
  {}

  void operator()()
  {
    impl_->invoke_(impl_.get());
  }

private:
  struct impl_base
  {
    void(*invoke_)(impl_base*);
  };

  template <typename F>
  struct impl : impl_base
  {
    impl(const F& f) : function_(f)
    {
      invoke_ = &function::invoke<F>;
    }
    F function_;
  };

  template <typename F>
  static void invoke(impl_base* i)
  {
    static_cast<impl<F>*>(i)->function_();
  }

  std::shared_ptr<impl_base> impl_;
};
*/

}  // namespace boost::asio

#endif  // !BOOST_ASIO_EXECUTOR_HPP