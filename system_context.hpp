#ifndef BOOST_ASIO_SYSTEM_CONTEXT_HPP
#define BOOST_ASIO_SYSTEM_CONTEXT_HPP

#include "execution_context.hpp"
#include "scheduler.hpp"
#include "thread_group.hpp"

namespace boost::asio {
class system_executor;
class system_context : public execution_context
{
 public:
  system_context();
  ~system_context();

  using executor_type = system_executor;
  executor_type get_executor();

  void stop();
  bool stopped() const;
  void join();

 private:
  friend class system_executor;
  struct thread_function;

  detail::scheduler& scheduler_;
  detail::thread_group threads_;
};
}  // namespace boost::asio
#endif  // BOOST_ASIO_SYSTEM_CONTEXT_HPP
