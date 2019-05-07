#ifndef BOOST_ASIO_BASIC_WAITABLE_TIMER_HPP
#define BOOST_ASIO_BASIC_WAITABLE_TIMER_HPP

#if defined(BOOST_ASIO_HAS_MOVE)
#include <utility>
#endif  // defined(BOOST_ASIO_HAS_MOVE)

#include "async_result.hpp"
#include "basic_io_object.hpp"
#include "chrono_time_traits.hpp"
#include "detail_deadline_timer_service.hpp"
#include "waitable_timer_service.hpp"

namespace boost::asio {

template <typename Clock, typename WaitTraits = wait_traits<Clock>,
          typename Service = waitable_timer_service<Clock, WaitTraits>>
class basic_waitable_timer;

template <typename Clock, typename WaitTraits, typename Service>
class basic_waitable_timer
    : public basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>
{
 public:
  using executor_type = io_context::executor_type;
  using clock_type = Clock;
  using duration = typename clock_type::duration;
  using time_point = typename clock_type::time_point;
  using traits_type = WaitTraits;

  explicit basic_waitable_timer(io_context& ioc)
      : basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>(ioc)
  {}

  basic_waitable_timer(io_context& ioc, const time_point& expiry_time)
      : basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>(ioc)
  {
    std::error_code ec;
    this->get_service().expires_at(this->get_impl(), expiry_time, ec);
    if (ec) detail::throw_exception(ec);
  }

  basic_waitable_timer(io_context& ioc, const duration& expiry_time)
      : basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>(ioc)
  {
    std::error_code ec;
    this->get_service().expires_after(this->get_impl(), expiry_time, ec);
    if (ec) detail::throw_exception(ec);
  }

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  basic_waitable_timer(basic_waitable_timer&& other)
      : basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>(std::move(other))
  {}

  basic_waitable_timer& operator=(basic_waitable_timer&& other)
  {
    basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>::operator=(
        std::move(other));
    return *this;
  }
#endif  // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  ~basic_waitable_timer() {}

  executor_type get_executor()
  {
    return basic_io_object<detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>>::get_executor();
  }

  std::size_t cancel()
  {
    std::error_code ec;
    std::size_t s = this->get_service().cancle(this->get_impl(), ec);
    detail::throw_exception(ec);
    return s;
  }

  std::size_t cancel_one()
  {
    std::error_code ec;
    std::size_t s = this->get_service().cancle_one(this->get_impl(), ec);
    return s;
  }

  time_point expiry() const { return this->get_service().expiry(this->get_impl()); }

  std::size_t expires_at(const time_point& expiry_time) const
  {
    std::error_code ec;
    std::size_t s = this->get_service().expires_at(this->get_impl(), expiry_time, ec);
    if (ec) detail::throw_exception(ec);
    return s;
  }

  std::size_t expires_after(const duration& expiry_time)
  {
    std::error_code ec;
    std::size_t s = this->get_service().expires_after(this->get_impl(), expiry_time, ec);
    if (ec) detail::throw_exception(ec);
    return s;
  }

  void wait()
  {
    std::error_code ec;
    this->get_service().wait(this->get_impl(), ec);
    if(ec) detail::throw_exception(ec);
  }

  void wait(std::error_code& ec) { this->get_service().wait(this->get_impl(), ec); }

  template <typename WaitHandler>
  typename detail::async_result_helper<WaitHandler, void(std::error_code)>::result_type async_wait(
      WaitHandler&& handler)
  {
    async_completion<WaitHandler, void(std::error_code ec)> init(handler);
    this->get_service().async_wait(this->get_impl(), init.handler_);
    return init.result_.get();
  }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_BASIC_WAITABLE_TIMER_HPP
