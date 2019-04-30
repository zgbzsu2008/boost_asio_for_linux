#ifndef BOOST_ASIO_IMPL_IO_CONTEXT_IPP
#define BOOST_ASIO_IMPL_IO_CONTEXT_IPP

#include "io_context.hpp"
#include <iostream>
#include "error_code.hpp"
#include "service_registry_helpers.hpp"
#include "throw_exception.hpp"

#if defined(BOOST_ASIO_HAS_IOCP)
#include "win_iocp_io_context.hpp"
#else
#include "scheduler.hpp"
#endif

namespace boost::asio
{
io_context::io_context() : impl_(add_impl(new impl_type(*this, std::max(2U, std::thread::hardware_concurrency() * 2))))
{
  std::cout << "io_context(): concurrency_hint = " << impl_.concurrency_hint() << '\n';
}

io_context::io_context(int concurrency_hint) : impl_(add_impl(new impl_type(*this, std::max(1, concurrency_hint))))
{
  std::cout << "io_context(): concurrency_hint = " << impl_.concurrency_hint() << '\n';
}

io_context::impl_type& io_context::add_impl(io_context::impl_type* impl)
{
  std::unique_ptr<impl_type> unique_impl(impl);
  add_service<impl_type>(*this, unique_impl.get());
  return *unique_impl.release();
}

io_context::~io_context() {}

io_context::executor_type io_context::get_executor() { return executor_type(*this); }

std::size_t io_context::run()
{
  std::error_code ec;
  auto n = impl_.run(ec);
  if (ec) {
    detail::throw_exception(ec);
  }
  return n;
}

std::size_t io_context::run_one()
{
  std::error_code ec;
  auto n = impl_.run_one(ec);
  if (ec) {
    detail::throw_exception(ec);
  }
  return n;
}

std::size_t io_context::poll()
{
  std::error_code ec;
  auto n = impl_.poll(ec);
  if (ec) {
    detail::throw_exception(ec);
  }
  return n;
}

std::size_t io_context::poll_one()
{
  std::error_code ec;
  auto n = impl_.poll_one(ec);
  if (ec) {
    detail::throw_exception(ec);
  }
  return n;
}

void io_context::stop() { impl_.stop(); }

bool io_context::stopped() const { return impl_.stopped(); }

void io_context::restart() { impl_.restart(); }

void io_context::reset() { restart(); }

void io_context::executor_type::on_work_started() const { io_context_.impl_.work_started(); }

void io_context::executor_type::on_work_finished() const { io_context_.impl_.work_finished(); }

bool io_context::executor_type::running_in_this_thread() const { return io_context_.impl_.can_dispatch(); }

io_context::service::service(boost::asio::io_context& owner) : execution_context::service(owner) {}

io_context::service::~service() {}

void io_context::service::shutdown() {}

}  // namespace boost::asio

#endif  // BOOST_ASIO_IO_CONTEXT_IPP