#ifndef BOOST_ASIO_SYSTEM_CONTEXT_HPP
#define BOOST_ASIO_SYSTEM_CONTEXT_HPP

#include <system_error>

#include "scheduler.hpp"
#include "thread_group.hpp"
#include "execution_context.hpp"

namespace boost::asio {

class system_executor;
class system_context : public execution_context {
 public:
  system_context();
  ~system_context();

  using executor_type = system_executor;
  executor_type get_executor() const;

  void stop();
  bool stopped() const;

  void join();

 private:
  friend class system_executor;
  struct thread_function;

  detail::scheduler& scheduler_;
  detail::thread_group threads_;
};
}

#endif // BOOST_ASIO_SYSTEM_CONTEXT_HPP
