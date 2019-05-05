#ifndef BOOST_ASIO_DEADLINE_TIMER_SERVICE_HPP
#define BOOST_ASIO_DEADLINE_TIMER_SERVICE_HPP

#include "io_context.hpp"
#include "time_traits.hpp"

namespace boost::asio {
template <typename TimeType, typename TimeTraits = boost::asio::time_traits<TimeType> >
class deadline_timer_service
    : public boost::asio::detail::service_base<deadline_timer_service<TimeType, TimeTraits> >
{
 public:
  typedef TimeTraits traits_type;
  typedef typename traits_type::time_point time_point;
  typedef typename traits_type::duration duration;

 private:
  typedef detail::deadline_timer_service<traits_type> service_impl_type;

 public:
  typedef typename service_impl_type::impl_type impl_type;
  explicit deadline_timer_service(boost::asio::io_context& io_context)
      : boost::asio::detail::service_base<deadline_timer_service<TimeType, TimeTraits> >(io_context),
        service_impl_(io_context)
  {}

  void construct(impl_type& impl) { service_impl_.construct(impl); }
  void destroy(impl_type& impl) { service_impl_.destroy(impl); }

  std::size_t cancel(impl_type& impl, std::error_code& ec)
  {
    return service_impl_.cancel(impl, ec);
  }

  std::size_t cancel_one(impl_type& impl, std::error_code& ec)
  {
    return service_impl_.cancel_one(impl, ec);
  }

  time_point expires_at(const impl_type& impl) const { return service_impl_.expiry(impl); }

  std::size_t expires_at(impl_type& impl, const time_point& expiry_time, std::error_code& ec)
  {
    return service_impl_.expires_at(impl, expiry_time, ec);
  }

  duration expires_from_now(const impl_type& impl) const
  {
    return TimeTraits::subtract(service_impl_.expiry(impl), TimeTraits::now());
  }

  std::size_t expires_from_now(impl_type& impl, const duration& expiry_time,
                               std::error_code& ec)
  {
    return service_impl_.expires_after(impl, expiry_time, ec);
  }

  void wait(impl_type& impl, std::error_code& ec) { service_impl_.wait(impl, ec); }

  template <typename WaitHandler>
  typename detail::async_result_helper<T, void(std::error_code)>::result_type async_wait(
      impl_type& impl, BOOST_ASIO_MOVE_ARG(WaitHandler) handler)
  {
    async_completion<WaitHandler, void(std::error_code)> init(handler);

    service_impl_.async_wait(impl, init.completion_handler);

    return init.result.get();
  }

 private:
  void shutdown() { service_impl_.shutdown(); }

  service_impl_type service_impl_;
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_DEADLINE_TIMER_SERVICE_HPP
