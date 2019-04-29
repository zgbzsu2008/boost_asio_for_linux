#ifndef BOOST_ASIO_DETAIL_EPOLL_REACTOR_HPP
#define BOOST_ASIO_DETAIL_EPOLL_REACTOR_HPP

#include "conditionally_enabled_mutex.hpp"
#include "execution_context.hpp"
#include "object_pool.hpp"
#include "op_queue.hpp"
#include "reactor_op.hpp"
#include "scheduler_operation.hpp"
#include "select_interrupter.hpp"

namespace boost::asio::detail
{
class epoll_reactor : public execution_context_service_base<epoll_reactor>
{
 private:
  using mutex = conditionally_enabled_mutex;

 public:
  enum op_types { read_op = 0, write_op = 1, connect_op = 1, except_op = 2, max_ops = 3 };

  class descriptor_state : operation
  {
    friend class epoll_reactor;
    friend class object_pool_access;

    descriptor_state* next_;
    descriptor_state* prev_;

    mutable mutex mutex_;
    epoll_reactor* reactor_;
    int descriptor_;
    uint32_t registered_events_;
    op_queue<reactor_op> op_queue_[max_ops];
    bool try_speculative_[max_ops];
    bool shutdown_;

    descriptor_state(bool locking);
    void set_ready_events(uint32_t events) { task_result_ = events; }
    void add_ready_events(uint32_t events) { task_result_ |= events; }
    operation* perform_io(uint32_t events);

    static void do_complete(void* owner, operation* base, const std::error_code& ec, std::size_t bytes_transferred);
  };
  using per_descriptor_data = descriptor_state*;
  using socket_type = int;

  epoll_reactor(execution_context& ctx);
  ~epoll_reactor();

  void shutdown();

  void init_task();

  int register_descriptor(socket_type descriptor, per_descriptor_data& descriptor_data);

  int register_internal_descriptor(int op_type, socket_type descriptor, per_descriptor_data& descriptor_data,
                                   reactor_op* op);

  void move_descriptor(socket_type descriptor, per_descriptor_data& target_descriptor_data,
                       per_descriptor_data& source_descriptor_data);

  void post_immediate_completion(reactor_op* op, bool is_continuation);

  void start_op(int op_type, socket_type descriptor, per_descriptor_data& descriptor_data, reactor_op* op,
                bool is_continuation, bool allow_speculative);

  void cancel_ops(socket_type descriptor, per_descriptor_data& descriptor_data);

  void deregister_descriptor(socket_type descriptor, per_descriptor_data& descriptor_data, bool closing);

  void deregister_internal_descriptor(socket_type descriptor, per_descriptor_data& descriptor_data);

  void cleanup_descriptor_data(per_descriptor_data& descriptor_data);

  void run(long usec, op_queue<operation>& ops);

  void interrupt();

 private:
  static int do_epoll_create();
  static int do_timerfd_create();

  per_descriptor_data allocate_descriptor_state();
  void free_descriptor_state(descriptor_state* s);

  detail::scheduler& scheduler_;
  detail::select_interrupter interrupter_;

  int epoll_fd_;
  int timer_fd_;
  bool shutdown_;
  mutable mutex mutex_;
  mutable mutex registered_descriptors_mutex_;
  object_pool<descriptor_state> registered_descriptors_;
  struct perform_io_cleanup_on_block_exit;
  friend struct perform_io_cleanup_on_block_exit;
};
using reactor = epoll_reactor;
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_EPOLL_REACTOR_HPP