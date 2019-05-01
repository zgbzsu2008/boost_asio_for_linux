#ifndef BOOST_ASIO_DETAIL_SCHEDULER_HPP
#define BOOST_ASIO_DETAIL_SCHEDULER_HPP

#include <algorithm>
#include <atomic>
#include <memory>
#include "conditionally_enabled_event.hpp"
#include "conditionally_enabled_mutex.hpp"
#include "execution_context.hpp"
#include "op_queue.hpp"
#include "scheduler_operation.hpp"
#include "thread.hpp"
#include "thread_context.hpp"

namespace boost::asio::detail {
struct scheduler_thread_info;
class epoll_reactor;
class scheduler : public execution_context_service_base<scheduler>, public thread_context
{
 public:
  using operation = scheduler_operation;

  explicit scheduler(execution_context &ctx,
                     int concurrency_hint = std::max(2, int(2 * detail::thread::hardware_concurrency())));
  void shutdown();
  void init_task();

  std::size_t run(std::error_code &ec);
  std::size_t run_one(std::error_code &ec);
  std::size_t wait_one(long usec, std::error_code &ec);
  std::size_t poll(std::error_code &ec);
  std::size_t poll_one(std::error_code &ec);

  void stop();
  bool stopped() const;
  void restart();

  void work_started() { ++outstanding_work_; }
  void compensating_work_started();
  void work_finished()
  {
    if (--outstanding_work_ == 0) {
      stop();
    }
  }

  bool can_dispatch() { return thread_call_stack::contains(this) != 0; }
  void do_dispatch(operation *op);

  void post_immediate_completion(operation *op, bool is_continuation);
  void post_deferred_completion(operation *op);
  void post_deferred_completions(op_queue<operation> &ops);
  void abandon_operations(op_queue<operation> &ops);

  int concurrency_hint() const { return concurrency_hint_; }

 private:
  using mutex = conditionally_enabled_mutex;
  using event = conditionally_enabled_event;
  using thread_info = scheduler_thread_info;

  std::size_t do_run_one(mutex::scoped_lock &lock, thread_info &this_thread, const std::error_code &ec);
  std::size_t do_wait_one(mutex::scoped_lock &lock, thread_info &this_thread, long usec, const std::error_code &ec);
  std::size_t do_poll_one(mutex::scoped_lock &lock, thread_info &this_thread, const std::error_code &ec);

  void stop_all_threads(mutex::scoped_lock &lock);
  void wake_one_thread_and_unlock(mutex::scoped_lock &lock);

  struct task_cleanup;
  friend struct task_cleanup;

  struct work_cleanup;
  friend struct work_cleanup;

  const bool one_thread_;
  mutable mutex mutex_;
  event wakeup_event_;

  epoll_reactor *task_;

  struct task_operation : operation
  {
    task_operation() : operation(0) {}
  } task_operation_;

  bool task_interrupted_;
  std::atomic<std::size_t> outstanding_work_;
  op_queue<operation> op_queue_;
  bool stopped_;
  bool shutdown_;
  const int concurrency_hint_;
};
}  // namespace boost::asio::detail

#endif  //! BOOST_ASIO_DETAIL_SCHEDULER_HPP