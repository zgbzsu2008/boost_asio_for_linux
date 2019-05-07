#include <iostream>
#include "steady_timer.hpp"
#include <functional>

using namespace boost::asio;

void print(const std::error_code&) { std::cout << "hello,world!\n"; }

int main()
{
  std::function<void()> p;
  io_context ioc;
  steady_timer t(ioc, std::chrono::seconds(5));
  t.async_wait(&print);
  ioc.run();

  return 0;
}