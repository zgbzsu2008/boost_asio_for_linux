#include <iostream>
#include "steady_timer.hpp"
#include "thread_group.hpp"

using namespace boost::asio;
using std::placeholders::_1;

class printer
{
 public:
  printer(io_context& ioc) : timer_(ioc, std::chrono::seconds(1)), count_(0)
  {
    timer_.async_wait(std::bind(&printer::print, this));
  }

  ~printer() { std::cout << "final count is " << count_ << '\n'; }

  void print()
  {
    if (count_ < 100) {
      std::cout << count_ << '\n';
      ++count_;
      timer_.expires_at(timer_.expiry() + std::chrono::seconds(1));
      timer_.async_wait(std::bind(&printer::print, this));
    }
  }

 private:
  steady_timer timer_;
  int count_;
};

int main()
{
  io_context ioc;
  detail::thread_group threads;
  threads.create_thread([&]() { ioc.run(); }, 1);

  printer p(ioc);
  ioc.run();

  return 0;
}