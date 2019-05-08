#include <iostream>
#include "steady_timer.hpp"
#include "thread_group.hpp"

using namespace boost::asio;

void print(const std::error_code&) { std::cout << "hello,world!\n"; }

int main()
{
  std::function<void()> p;
  io_context ioc;
  detail::thread_group threads;
  threads.create_thread([&]() { ioc.run(); }, 1);

  steady_timer t(ioc, std::chrono::seconds(5));
  t.async_wait(&print);
  ioc.run();

  return 0;
}