#include <iostream>
#include "steady_timer.hpp"
#include "thread_group.hpp"

namespace test_bind_print {

using namespace boost::asio;
using std::placeholders::_1;

void print(const std::error_code&, steady_timer* t, int* count)
{
  if (*count < 5) {
    std::cout << *count << '\n';
    ++(*count);
    t->expires_at(t->expiry() + std::chrono::seconds(1));
    t->async_wait(std::bind(print, _1, t, count));
  }
}

int main()
{
  io_context ioc;
  detail::thread_group threads;
  threads.create_thread([&]() { ioc.run(); }, 1);

  int count = 0;
  steady_timer t(ioc, std::chrono::seconds(1));
  t.async_wait(std::bind(print, _1, &t, &count));
  ioc.run();

  std::cout << "final count is " << count << '\n';

  return 0;
}
}  // namespace test_bind_print