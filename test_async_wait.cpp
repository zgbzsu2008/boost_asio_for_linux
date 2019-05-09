#include <iostream>
#include "steady_timer.hpp"
#include "thread_group.hpp"

namespace test_async_wait {

using namespace boost::asio;

void print(const std::error_code&) { std::cout << "hello,world!\n"; }

int main()
{
  io_context ioc;
  detail::thread_group threads;
  threads.create_thread([&]() { ioc.run(); }, 1);

  steady_timer t(ioc, std::chrono::seconds(5));
  t.async_wait(&print);
  ioc.run();

  return 0;
}

}  // namespace test_async_wait