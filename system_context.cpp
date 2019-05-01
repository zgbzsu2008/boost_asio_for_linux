#ifndef BOOST_ASIO_SYSTEM_CONTEXT_IPP
#define BOOST_ASIO_SYSTEM_CONTEXT_IPP

#include <system_error>
#include "system_context.hpp"
#include "system_executor.hpp"

namespace boost::asio {
struct system_context::thread_function
{
  detail::scheduler* scheduler_;
  void operator()()
  {
    std::error_code ec;
    scheduler_->run(ec);
  }
};

system_context::system_context() : scheduler_(use_service<detail::scheduler>(*this))
{
  scheduler_.work_started();

  thread_function f = {&scheduler_};
  std::size_t num_threads = std::thread::hardware_concurrency() * 2;
  threads_.create_thread(f, num_threads ? num_threads : 2);
}

system_context::~system_context()
{
  scheduler_.work_finished();
  scheduler_.stop();
  threads_.join();
}

system_context::executor_type system_context::get_executor() { return executor_type(); }

void system_context::stop() { scheduler_.stop(); }

bool system_context::stopped() const { return scheduler_.stopped(); }

void system_context::join()
{
  scheduler_.work_finished();
  threads_.join();
}
}  // namespace boost::asio
#endif  // !BOOST_ASIO_SYSTEM_CONTEXT_IPP