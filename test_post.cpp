#include <iostream>
#include "epoll_reactor.hpp"
#include "executor_work_guard.hpp"
#include "io_context.hpp"
#include "post.hpp"
#include "service_registry_helpers.hpp"
#include "thread_group.hpp"

namespace asio_post {

using namespace boost::asio;

int main()
{
  io_context ioc;
  auto work = make_work_guard(ioc);
  detail::thread_group threads;
  threads.create_thread([&] { ioc.run(); }, 4);
  post(ioc, [&]() {
    detail::epoll_reactor* reactor = new detail::epoll_reactor(ioc);
    add_service<detail::epoll_reactor>(ioc, reactor);
    reactor->init_task();
  });

  std::this_thread::sleep_for(std::chrono::seconds(3));

  for (int i = 0;; ++i) {
    post(ioc, [=]() { std::cout << i << " pid = " << std::this_thread::get_id() << '\n'; });
  }

  ioc.run();
  return 0;
}
}  // namespace io_context_post