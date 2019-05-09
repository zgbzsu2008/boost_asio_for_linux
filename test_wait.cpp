#include <iostream>
#include "steady_timer.hpp"

namespace test_wait {

using namespace boost::asio;

int main()
{
  io_context ioc;
  detail::thread_group threads;
  threads.create_thread([&]() { ioc.run(); }, 1);

  steady_timer t(ioc, std::chrono::seconds(5));
  t.wait();
  std::cout << "hello,world!\n";

  return 0;
}
}  // namespace test_wait