#ifndef BOOST_ASIO_WAITABLE_TIMER_SERVICE_HPP
#define BOOST_ASIO_WAITABLE_TIMER_SERVICE_HPP

#include "deadline_timer_service.hpp"
#include "io_context.hpp"
#include "wait_traits.hpp"

namespace boost::asio {

template <typename Clock, typename WaitTraits = wait_traits<Clock>>
class waitable_timer_service : public detail::service_base<waitable_timer_service<Clock, WaitTraits>>
{
 public:
  using clock_type = Clock;
  using duration = typename clock_type::duration;
  using time_point = typename clock_type::time_point;
  using traits_type = WaitTraits;

 private:
  using service_impl_type = detail::deadline_timer_service<detail::chrono_time_traits<Clock, WaitTraits>>;

 public:
  using impl_type = typename service_impl_type::impl_type;
 
  explicit waitable_timer_service(io_context& ioc)
      : detail::service_base<waitable_timer_service<Clock, WaitTraits>>(ioc), service_impl_(ioc)
  {}

  void construct(impl_type& impl) { service_impl_.construct(impl); }
  void destroy(impl_type& impl) { service_impl_.destroy(impl); }

  void move_construct(impl_type& impl, impl_type& other_impl) { service_impl_.move_construct(impl, other_impl); }

  void move_assign(impl_type& impl, waitable_timer_service& other_service, impl_type& other_impl)
  {
    service_impl_.move_assign(impl, other_service.service_impl_, other_impl);
  }

  std::size_t cancle(service_impl_type& impl, std::error_code& ec) { return service_impl_.cancle(impl, ec); }
  std::size_t cancle_one(service_impl_type& impl, std::error_code& ec) { return service_impl_.cancle_one(impl, ec); }

  time_point expiry(const impl_type& impl) const { return service_impl_.expiry(impl); }

  std::size_t expires_at(impl_type& impl, const time_point& expiry_time, std::error_code& ec)
  {
    return service_impl_.expires_at(impl, expiry_time, ec);
  }

  std::size_t expires_after(impl_type& impl, const duration& expiry_time, std::error_code& ec)
  {
    return service_impl_.expires_after(impl, expiry_time, ec);
  }

  void wait(impl_type& impl, std::error_code& ec) { service_impl_.wait(impl, ec); }

  template <typename WaitHandler>
  typename detail::async_result_helper<WaitHandler, void(std::error_code ec)>::result_type async_wait(
      impl_type& impl, WaitHandler&& handler)
  {
    async_completion<WaitHandler, void(std::error_code)> init(handler);
    service_impl_.async_wait(impl, init.handler_);
    return init.result.get();
  }

 private:
  void shutdown() { service_impl_.shutdown(); }

  service_impl_type service_impl_;
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_WAITABLE_TIMER_SERVICE_HPP