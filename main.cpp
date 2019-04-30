#include <iostream>
#include <string>
#include "associated_executor.hpp"
#include "io_context.hpp"
#include "service_registry_helpers.hpp"

namespace associated_executor {
using namespace boost::asio;

struct no_executor
{};

inline int associated_executor_test()
{
  auto e1 = get_associated_executor(no_executor());
  static_assert(std::is_same<decltype(e1), system_executor>::value);

  auto e2 = get_associated_executor(system_context());
  static_assert(std::is_same<decltype(e2), system_executor>::value);

  auto e3 = get_associated_executor(io_context());
  static_assert(std::is_same<decltype(e3), io_context::executor_type>::value);

  auto e4 = get_associated_executor(no_executor(), system_executor());
  static_assert(std::is_same<decltype(e4), system_executor>::value);

  auto e5 = get_associated_executor(io_context(), system_executor());
  static_assert(std::is_same<decltype(e5), io_context::executor_type>::value);

  auto e6 = get_associated_executor(no_executor(), io_context());
  static_assert(std::is_same<decltype(e6), io_context::executor_type>::value);

  // auto e7 = get_associated_executor(system_executor(), no_executor());

  return 0;
}
}  // namespace associated_executor

int main() { return associated_executor::associated_executor_test(); }