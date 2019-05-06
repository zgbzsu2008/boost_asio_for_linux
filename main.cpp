#include <iostream>
#include "steady_timer.hpp"

using namespace boost::asio;

void print(const std::error_code&) { std::cout << "hello,world!\n"; }

int main()
{
  io_context ioc;
  steady_timer t(ioc, std::chrono::seconds(5));
  t.async_wait(&print);
  ioc.run();

  return 0;
}