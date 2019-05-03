#ifndef BOOST_ASIO_DETAIL_SCHEDULER_CPP
#define BOOST_ASIO_DETAIL_SCHEDULER_CPP

#include <iostream>
#include <type_traits>

#include "epoll_reactor.hpp"
#include "execution_context.hpp"
#include "scheduler.hpp"
#include "scheduler_thread_info.hpp"
#include "service_registry_helpers.hpp"

namespace boost::asio::detail {
// 私有队列+task_operation_ push到公有队列
struct scheduler::task_cleanup
{
  ~task_cleanup()
  {
    if (this_thread_->private_outstanding_work > 0) {
      scheduler_->outstanding_work_ += this_thread_->private_outstanding_work;
    }
    this_thread_->private_outstanding_work = 0;

    lock_->lock();
    scheduler_->op_queue_.push(this_thread_->private_op_queue);
    scheduler_->task_interrupted_ = true;
    scheduler_->op_queue_.push(&scheduler_->task_operation_);
  }

  scheduler *scheduler_;
  mutex::scoped_lock *lock_;
  thread_info *this_thread_;
};

// 私有队列push到公有队列，修改outstanding_work_
struct scheduler::work_cleanup
{
  ~work_cleanup()
  {
    if (this_thread_->private_outstanding_work > 0) {
      scheduler_->outstanding_work_ += this_thread_->private_outstanding_work;
    }
    this_thread_->private_outstanding_work = 0;
    scheduler_->work_finished();

    if (!this_thread_->private_op_queue.empty()) {
      lock_->lock();
      scheduler_->op_queue_.push(this_thread_->private_op_queue);
    }
  }

  scheduler *scheduler_;
  mutex::scoped_lock *lock_;
  thread_info *this_thread_;
};

scheduler::scheduler(execution_context &ctx, int concurrency_hint)
    : execution_context_service_base<scheduler>(ctx),
      one_thread_(concurrency_hint == 1),
      mutex_(concurrency_hint > 1),
      task_(0),
      task_interrupted_(false),
      outstanding_work_(0),
      stopped_(false),
      shutdown_(false),
      concurrency_hint_(concurrency_hint)
{}

void scheduler::shutdown()
{
  mutex::scoped_lock lock(mutex_);
  shutdown_ = true;
  lock.unlock();

  while (!op_queue_.empty()) {
    operation *o = op_queue_.front();
    op_queue_.pop();
    if (o != &task_operation_) {
      o->destroy();
    }
  }
  task_ = 0;
}

void scheduler::init_task()
{
  mutex::scoped_lock lock(mutex_);
  if (!shutdown_ && !task_) {
    task_ = &use_service<epoll_reactor>(this->context());
    op_queue_.push(&task_operation_);
    wake_one_thread_and_unlock(lock);
  }
}

std::size_t scheduler::run(std::error_code &ec)
{
  ec = std::error_code();
  if (outstanding_work_ == 0) {
    stop();
    return 0;
  }

  thread_info this_thread;
  this_thread.private_outstanding_work = 0;
  thread_call_stack::context ctx(this, this_thread);

  mutex::scoped_lock lock(mutex_);
  std::size_t n = 0;
  for (; do_run_one(lock, this_thread, ec); lock.lock()) {
    if (n != (std::numeric_limits<std::size_t>::max)()) {
      ++n;
    }
  }
  return n;
}

std::size_t scheduler::run_one(std::error_code &ec)
{
  ec = std::error_code();
  if (outstanding_work_ == 0) {
    stop();
    return 0;
  };

  thread_info this_thread;
  this_thread.private_outstanding_work = 0;
  thread_call_stack::context ctx(this, this_thread);

  mutex::scoped_lock lock(mutex_);
  return do_run_one(lock, this_thread, ec);
}

std::size_t scheduler::wait_one(long usec, std::error_code &ec)
{
  ec = std::error_code();
  if (outstanding_work_ == 0) {
    stop();
    return 0;
  };

  thread_info this_thread;
  this_thread.private_outstanding_work = 0;
  thread_call_stack::context ctx(this, this_thread);

  mutex::scoped_lock lock(mutex_);
  return do_wait_one(lock, this_thread, usec, ec);
}

std::size_t scheduler::poll(std::error_code &ec)
{
  ec = std::error_code();
  if (outstanding_work_ == 0) {
    stop();
    return 0;
  };

  thread_info this_thread;
  this_thread.private_outstanding_work = 0;
  thread_call_stack::context ctx(this, this_thread);

  mutex::scoped_lock lock(mutex_);
  if (one_thread_) {  // 单线程嵌套poll thread_call_stack中取出任务
    if (auto outer_info = static_cast<thread_info *>(ctx.next_by_key())) {
      op_queue_.push(outer_info->private_op_queue);
    }
  }

  std::size_t n = 0;
  for (; do_poll_one(lock, this_thread, ec); lock.lock()) {
    if (n != (std::numeric_limits<std::size_t>::max)()) {
      ++n;
    }
  }
  return n;
}

std::size_t scheduler::poll_one(std::error_code &ec)
{
  ec = std::error_code();
  if (outstanding_work_ == 0) {
    stop();
    return 0;
  };

  thread_info this_thread;
  this_thread.private_outstanding_work = 0;
  thread_call_stack::context ctx(this, this_thread);

  mutex::scoped_lock lock(mutex_);
  if (one_thread_) {
    if (auto outer_info = static_cast<thread_info *>(ctx.next_by_key())) {
      op_queue_.push(outer_info->private_op_queue);
    }
  }

  return do_poll_one(lock, this_thread, ec);
}

void scheduler::stop()
{
  mutex::scoped_lock lock(mutex_);
  this->stop_all_threads(lock);
}

bool scheduler::stopped() const
{
  mutex::scoped_lock lock(mutex_);
  return stopped_;
}

void scheduler::restart()
{
  mutex::scoped_lock lock(mutex_);
  stopped_ = false;
}

void scheduler::compensating_work_started()
{
  thread_info_base *this_thread = thread_call_stack::contains(this);
  ++(static_cast<thread_info *>(this_thread)->private_outstanding_work);
}

void scheduler::do_dispatch(operation *op)
{
  work_started();
  mutex::scoped_lock lock(mutex_);
  op_queue_.push(op);
  wake_one_thread_and_unlock(lock);
}

void scheduler::post_immediate_completion(operation *op, bool is_continuation)
{
  if (one_thread_ || is_continuation) {
    if (thread_info_base *this_thread = thread_call_stack::contains(this)) {
      ++(static_cast<thread_info *>(this_thread)->private_outstanding_work);
      (static_cast<thread_info *>(this_thread))->private_op_queue.push(op);
      return;
    }
  }

  work_started();
  mutex::scoped_lock lock(mutex_);
  op_queue_.push(op);
  wake_one_thread_and_unlock(lock);
}

void scheduler::post_deferred_completion(operation *op)
{
  if (one_thread_) {
    if (thread_info_base *this_thread = thread_call_stack::contains(this)) {
      // ++(static_cast<thread_info*>(this_thread)->private_outstanding_work);
      (static_cast<thread_info *>(this_thread))->private_op_queue.push(op);
      return;
    }
  }

  // work_started();
  mutex::scoped_lock lock(mutex_);
  op_queue_.push(op);
  wake_one_thread_and_unlock(lock);
}

void scheduler::post_deferred_completions(op_queue<operation> &ops)
{
  if (one_thread_) {
    if (thread_info_base *this_thread = thread_call_stack::contains(this)) {
      ops.push(static_cast<thread_info *>(this_thread)->private_op_queue);
      return;
    }
  }

  mutex::scoped_lock lock(mutex_);
  op_queue_.push(ops);
  wake_one_thread_and_unlock(lock);
}

void scheduler::abandon_operations(op_queue<operation> &ops)
{
  op_queue<scheduler::operation> ops2;
  ops2.push(ops);
}

std::size_t scheduler::do_run_one(mutex::scoped_lock &lock, thread_info &this_thread, const std::error_code &ec)
{
  while (!stopped_) {
    if (!op_queue_.empty()) {
      std::cout << "scheduler::do_run_one(): working... pid= " << std::this_thread::get_id() << '\n';
      operation *o = op_queue_.front();
      op_queue_.pop();
      bool more_handlers = (!op_queue_.empty());
      if (o == &task_operation_) {  // task op
        task_interrupted_ = more_handlers;
        if (more_handlers && !one_thread_) {
          wakeup_event_.unlock_and_signal_one(lock);
        } else {
          lock.unlock();
        }
        task_cleanup on_exit = {this, &lock, &this_thread};
        (void)on_exit;

        task_->run(more_handlers ? 0 : -1, this_thread.private_op_queue);
      } else {
        if (more_handlers && !one_thread_) {
          wakeup_event_.unlock_and_signal_one(lock);
        } else {
          lock.unlock();
        }
        work_cleanup on_exit = {this, &lock, &this_thread};
        (void)on_exit;

        std::size_t task_result = o->task_result_;
        o->complete(this, ec, task_result);
        return 1;
      }
    } else {
      std::cout << "scheduler::do_run_one(): waiting pid= " << std::this_thread::get_id() << '\n';
      wakeup_event_.clear(lock);
      wakeup_event_.wait(lock);
    }
  }

  return 0;
}

std::size_t scheduler::do_wait_one(mutex::scoped_lock &lock, thread_info &this_thread, long usec,
                                   const std::error_code &ec)
{
  if (stopped_) {
    return 0;
  }

  // 1，
  operation *o = op_queue_.front();
  if (o == 0) {
    wakeup_event_.clear(lock);
    wakeup_event_.wait_for_usec(lock, usec);
    usec = 0;
    o = op_queue_.front();
  }

  // 2，
  if (o == &task_operation_) {
    op_queue_.pop();
    bool more_handlers = (!op_queue_.empty());
    task_interrupted_ = more_handlers;
    if (more_handlers && !one_thread_) {
      wakeup_event_.unlock_and_signal_one(lock);
    } else {
      lock.unlock();
    }
    {
      task_cleanup on_exit = {this, &lock, &this_thread};
      (void)on_exit;
      task_->run(more_handlers ? 0 : usec, this_thread.private_op_queue);
    }

    o = op_queue_.front();
    if (o == &task_operation_) {
      if (!one_thread_) {
        wakeup_event_.maybe_unlock_and_signal_one(lock);
      }
      return 0;
    }
  }

  // 3，
  if (o == 0) {
    return 0;
  }

  // 4，
  op_queue_.pop();
  bool more_handlers = (!op_queue_.empty());
  std::size_t task_result = o->task_result_;
  if (more_handlers && !one_thread_) {
    wakeup_event_.unlock_and_signal_one(lock);
  } else {
    lock.unlock();
  }

  work_cleanup on_exit = {this, &lock, &this_thread};
  (void)on_exit;

  o->complete(this, ec, task_result);
  return 1;
}

std::size_t scheduler::do_poll_one(mutex::scoped_lock &lock, thread_info &this_thread, const std::error_code &ec)
{
  if (stopped_) {
    return 0;
  }

  // 1，
  operation *o = op_queue_.front();
  if (o == &task_operation_) {
    op_queue_.pop();
    lock.unlock();
    {
      task_cleanup c = {this, &lock, &this_thread};
      (void)c;
    }
    task_->run(0, this_thread.private_op_queue);

    o = op_queue_.front();
    if (o == &task_operation_) {
      wakeup_event_.maybe_unlock_and_signal_one(lock);
      return 0;
    }
  }

  // 2，
  if (o == 0) {
    return 0;
  }

  // 3，
  op_queue_.pop();
  bool more_handlers = (!op_queue_.empty());
  std::size_t task_result = o->task_result_;

  if (more_handlers && !one_thread_) {
    wake_one_thread_and_unlock(lock);
  } else {
    lock.unlock();
  }

  work_cleanup on_exit = {this, &lock, &this_thread};
  (void)on_exit;

  o->complete(this, ec, task_result);

  return 1;
}

void scheduler::stop_all_threads(mutex::scoped_lock &lock)
{
  stopped_ = true;
  wakeup_event_.signal_all(lock);
  if (!task_interrupted_ && task_) {
    task_interrupted_ = true;
    task_->interrupt();
  }
}

void scheduler::wake_one_thread_and_unlock(mutex::scoped_lock &lock)
{
  if (!wakeup_event_.maybe_unlock_and_signal_one(lock)) {
    if (!task_interrupted_ && task_) {
      task_interrupted_ = true;
      task_->interrupt();
      std::cout << "scheduler::wake_one_thread_and_unlock(): interrupt\n";
    }
    lock.unlock();
  }
}

}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_SCHEDULER_CPP