#include "associated_executor.hpp"
//#include "associated_executor2.hpp"
#include "io_context.hpp"
#include "service_registry_helpers.hpp"

namespace test_associated_executor {

using namespace boost::asio;

struct no_executor
{};

int main()
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

  return 0;
}
}  // namespace test_associated_executor