#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstddef>
#include <iostream>

#include "epoll_reactor.hpp"
#include "error_code.hpp"
#include "execution_context.hpp"
#include "scheduler.hpp"
#include "service_registry_helpers.hpp"

namespace boost::asio::detail {
epoll_reactor::epoll_reactor(execution_context& ctx)
    : execution_context_service_base(ctx),
      scheduler_(use_service<scheduler>(ctx)),
      interrupter_(),
      epoll_fd_(do_epoll_create()),
      timer_fd_(do_timerfd_create()),
      shutdown_(false),
      mutex_(scheduler_.concurrency_hint() == 1),
      registered_descriptors_mutex_(mutex_.enabled())
{
  epoll_event ev = {0, {0}};
  ev.events = EPOLLIN | EPOLLERR | EPOLLET;
  ev.data.ptr = &interrupter_;
  ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, interrupter_.fd(), &ev);
  interrupter_.interrupt();

  ev = {0, {0}};
  ev.events = EPOLLIN | EPOLLERR;
  ev.data.ptr = &timer_fd_;
  ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, timer_fd_, &ev);
}

epoll_reactor::~epoll_reactor()
{
  if (epoll_fd_ != -1) {
    ::close(epoll_fd_);
  }
  if (timer_fd_ != -1) {
    ::close(timer_fd_);
  }
}

void epoll_reactor::shutdown()
{
  mutex::scoped_lock lock(mutex_);
  shutdown_ = true;
  lock.unlock();
  op_queue<operation> ops;

  while (auto state = registered_descriptors_.first()) {
    for (int i = 0; i < max_ops; ++i) {
      ops.push(state->op_queue_[i]);
    }
    state->shutdown_ = true;
    registered_descriptors_.free(state);
  }
}

void epoll_reactor::init_task() { scheduler_.init_task(); }

int epoll_reactor::register_descriptor(socket_type descriptor, ptr_descriptor_data& descriptor_data)
{
  descriptor_data = allocate_descriptor_state();
  {
    mutex::scoped_lock lock(descriptor_data->mutex_);
    descriptor_data->reactor_ = this;
    descriptor_data->descriptor_ = descriptor;
    descriptor_data->shutdown_ = false;
    for (int i = 0; i < max_ops; ++i) {
      descriptor_data->try_speculative_[i] = true;
    }
  }
  epoll_event ev = {0, {0}};
  ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLPRI | EPOLLET;
  descriptor_data->registered_events_ = ev.events;
  ev.data.ptr = descriptor_data;
  int result = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, descriptor, &ev);
  if (result != 0) {
    if (errno == EPERM) {
      descriptor_data->registered_events_ = 0;
      return 0;
    }
    return errno;
  }
  return 0;
}

int epoll_reactor::register_internal_descriptor(int op_type, socket_type descriptor,
                                                ptr_descriptor_data& descriptor_data, reactor_op* op)
{
  descriptor_data = allocate_descriptor_state();
  {
    mutex::scoped_lock lock(descriptor_data->mutex_);
    descriptor_data->reactor_ = this;
    descriptor_data->descriptor_ = descriptor;
    descriptor_data->shutdown_ = false;
    descriptor_data->op_queue_[op_type].push(op);
    for (int i = 0; i < max_ops; ++i) {
      descriptor_data->try_speculative_[i] = true;
    }
  }
  epoll_event ev = {0, {0}};
  ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLPRI | EPOLLET;
  descriptor_data->registered_events_ = ev.events;
  ev.data.ptr = descriptor_data;
  int result = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, descriptor, &ev);
  if (result != 0) {
    return errno;
  }
  return 0;
}

void epoll_reactor::move_descriptor(socket_type descriptor, ptr_descriptor_data& target_descriptor_data,
                                    ptr_descriptor_data& source_descriptor_data)
{
  target_descriptor_data = source_descriptor_data;
  source_descriptor_data = 0;
}

void epoll_reactor::post_immediate_completion(reactor_op* op, bool is_continuation)
{
  scheduler_.post_immediate_completion(op, is_continuation);
}

void epoll_reactor::start_op(int op_type, socket_type descriptor, ptr_descriptor_data& descriptor_data, reactor_op* op,
                             bool is_continuation, bool allow_speculative)
{}

void epoll_reactor::cancel_ops(socket_type, ptr_descriptor_data& descriptor_data)
{
  if (!descriptor_data) {
    return;
  }
  mutex::scoped_lock lock(descriptor_data->mutex_);

  op_queue<operation> ops;
  for (int i = 0; i < max_ops; ++i) {
    while (!descriptor_data->op_queue_[i].empty()) {
      reactor_op* op = descriptor_data->op_queue_[i].front();
      op->ec_ = detail::error_code::operation_aborted;
      descriptor_data->op_queue_[i].pop();
      ops.push(op);
    }
  }
  lock.unlock();
  scheduler_.post_deferred_completions(ops);
}

void epoll_reactor::deregister_descriptor(socket_type descriptor, ptr_descriptor_data& descriptor_data, bool closing)
{
  if (!descriptor_data) {
    return;
  }

  mutex::scoped_lock lock(descriptor_data->mutex_);
  if (!descriptor_data->shutdown_) {
    if (closing) {
      //
    } else if (descriptor_data->registered_events_ != 0) {
      epoll_event ev = {0, {0}};
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, descriptor, &ev);
    }

    op_queue<operation> ops;
    for (int i = 0; i < max_ops; ++i) {
      while (!descriptor_data->op_queue_[i].empty()) {
        reactor_op* op = descriptor_data->op_queue_[i].front();
        op->ec_ = detail::error_code::operation_aborted;
        descriptor_data->op_queue_[i].pop();
        ops.push(op);
      }
    }

    descriptor_data->descriptor_ = -1;
    descriptor_data->shutdown_ = true;

    lock.unlock();
    scheduler_.post_deferred_completions(ops);
  } else {
    descriptor_data = 0;
  }
}

void epoll_reactor::deregister_internal_descriptor(socket_type descriptor, ptr_descriptor_data& descriptor_data)
{
  if (!descriptor_data) {
    return;
  }

  mutex::scoped_lock lock(descriptor_data->mutex_);
  if (!descriptor_data->shutdown_) {
    epoll_event ev = {0, {0}};
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, descriptor, &ev);

    op_queue<operation> ops;
    for (int i = 0; i < max_ops; ++i) {
      ops.push(descriptor_data->op_queue_[i]);
    }
    descriptor_data->descriptor_ = -1;
    descriptor_data->shutdown_ = true;

    lock.unlock();
    scheduler_.post_deferred_completions(ops);
  } else {
    descriptor_data = 0;
  }
}

void epoll_reactor::cleanup_descriptor_data(ptr_descriptor_data& descriptor_data)
{
  if (descriptor_data) {
    free_descriptor_state(descriptor_data);
    descriptor_data = 0;
  }
}

void epoll_reactor::epoll_reactor::interrupt()
{
  epoll_event ev = {0, {0}};
  ev.events = EPOLLIN | EPOLLERR | EPOLLET;
  ev.data.ptr = &interrupter_;
  ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, interrupter_.fd(), &ev);
}

int epoll_reactor::do_epoll_create()
{
  int fd = ::epoll_create1(EPOLL_CLOEXEC);
  if (fd < 0) {
    std::error_code ec(errno, std::generic_category());
    throw ec;
  }
  return fd;
}

int epoll_reactor::do_timerfd_create()
{
  int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
  if (fd < 0) {
    std::error_code ec(errno, std::generic_category());
    throw ec;
  }
  return fd;
}

epoll_reactor::ptr_descriptor_data epoll_reactor::allocate_descriptor_state()
{
  mutex::scoped_lock lock(registered_descriptors_mutex_);
  return registered_descriptors_.alloc(scheduler_.concurrency_hint() > 1);
}

void epoll_reactor::free_descriptor_state(descriptor_state* s)
{
  mutex::scoped_lock lock(registered_descriptors_mutex_);
  registered_descriptors_.free(s);
}

void epoll_reactor::run(long usec, op_queue<operation>& ops)
{
  int timeout;
  if (usec == 0) {
    timeout = 0;
  } else {
    timeout = static_cast<int>((usec < 0) ? -1 : ((usec - 1) / 1000 + 1));
  }

  epoll_event events[128];
  bool check_timers = false;
  int num_events = ::epoll_wait(epoll_fd_, events, 128, timeout);
  for (int i = 0; i < num_events; i++) {
    void* ptr = events[i].data.ptr;
    if (ptr == &interrupter_) {
      check_timers = true;
    } else if (ptr == &timer_fd_) {
      check_timers = true;
    } else {
      auto descriptor_data = static_cast<descriptor_state*>(ptr);
      if (!ops.is_enqueued(descriptor_data)) {  // 是否在队列中 比如read->write
        descriptor_data->set_ready_events(events[i].events);
        ops.push(descriptor_data);
      } else {
        descriptor_data->add_ready_events(events[i].events);
      }
    }
  }

  std::cout << "\tepoll_reactor::run(): " << num_events << " events happend\n";

  if (check_timers) {
  }
}

epoll_reactor::descriptor_state::descriptor_state(bool locking)
    : operation(&epoll_reactor::descriptor_state::do_complete), mutex_(locking)
{}

void epoll_reactor::descriptor_state::do_complete(void* owner, operation* base, const std::error_code& ec,
                                                  std::size_t bytes_transferred)
{
  if (owner) {
    auto descriptor_data = static_cast<descriptor_state*>(base);
    uint32_t events = static_cast<uint32_t>(bytes_transferred);
    if (auto op = descriptor_data->perform_io(events)) {
      op->complete(owner, ec, 0);
    }
  }
}

struct epoll_reactor::perform_io_cleanup_on_block_exit
{
  explicit perform_io_cleanup_on_block_exit(epoll_reactor* r) : reactor_(r), first_op_(0) {}
  ~perform_io_cleanup_on_block_exit()
  {
    if (first_op_) {
      if (!ops_.empty()) {
        reactor_->scheduler_.post_deferred_completions(ops_);
      } else {
        reactor_->scheduler_.compensating_work_started();
      }
    }
  }

  epoll_reactor* reactor_;
  op_queue<operation> ops_;
  operation* first_op_;
};

operation* epoll_reactor::descriptor_state::perform_io(uint32_t events)
{
  mutex_.lock();
  perform_io_cleanup_on_block_exit io_cleanup(reactor_);
  std::unique_lock<mutex> lock(mutex_, std::adopt_lock);  // mutex locked
  static const int flag[max_ops] = {EPOLLIN, EPOLLOUT, EPOLLPRI};
  for (int j = max_ops - 1; j >= 0; --j) {
    if (events & (flag[j] | EPOLLERR | EPOLLHUP)) {
      try_speculative_[j] = true;
      while (!op_queue_[j].empty()) {
        reactor_op* op = op_queue_[j].front();
        if (auto status = op->perform()) {
          op_queue_[j].pop();
          io_cleanup.ops_.push(op);
          if (status == reactor_op::done_and_exhausted) {
            try_speculative_[j] = false;
            break;
          }
        } else {
          break;
        }
      }
    }
  }

  if (!io_cleanup.ops_.empty()) {
    io_cleanup.first_op_ = io_cleanup.ops_.front();
    io_cleanup.ops_.pop();
  }
  return io_cleanup.first_op_;
}
}  // namespace boost::asio::detail