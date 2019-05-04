#ifndef BOOST_ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP
#define BOOST_ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP

#include "epoll_reactor.hpp"
#include "io_context.hpp"
#include "service_registry_helpers.hpp"
#include "time_traits.hpp"
#include "timer_queue.hpp"
#include "wait_traits.hpp"

namespace boost::asio::detail {
template <typename T>
class deadline_timer_service : public service_base<deadline_timer_service<T>>
{
 public:
  using time_point = typename T::time_point;
  using duration = typename T::duration;
  using timer_scheduler = epoll_reactor;

  struct implementation_type : private noncopyable
  {
    time_point expiry;
    bool might_have_pending_waits;
    typename timer_queue<T>::ptr_timer_data timer_data;
  };

  deadline_timer_service(io_context& ioc)
      : service_base<deadline_timer_service<T>>(ioc), scheduler_(use_service<timer_scheduler>(ioc))
  {
    scheduler_.init_task();
    scheduler_.add_timer_queue(timer_queue_);
  }

  ~deadline_timer_service() { scheduler_.remove_timer_queue(timer_queue_); }

  void shutdown() {}

  void construct(implementation_type& impl)
  {
    impl.expiry = time_point();
    impl.might_have_pending_waits = false;
  }

  void destroy(implementation_type& impl)
  {
    std::error_code ec;
    this->cancle(impl, ec);
  }

  std::size_t cancle(implementation_type& impl, std::error_code& ec)
  {
    if (!impl.might_have_pending_waits) {
      ec = std::error_code();
      return 0;
    }
    std::size_t count = scheduler_.cancel_timer(timer_queue_, impl.timer_data);
    impl.might_have_pending_waits = false;
    ec = std::error_code();
    return count;
  }

  void move_construct(implementation_type& impl, implementation_type& other_impl)
  {
    scheduler_.move_timer(timer_queue_, impl.timer_data, other_impl.timer_data);
    impl.expiry = other_impl.expiry;
    other_impl.expiry = time_point();
    impl.might_have_pending_waits = other_impl.might_have_pending_waits;
    other_impl.might_have_pending_waits = false;
  }

  time_point expiry(const implementation_type& impl) const { return impl.expiry; }

  time_point expires_at(const implementation_type& impl) const { return impl.expiry; }

  duration expires_from_now(const implementation_type& impl) const
  {
    return T::subtract(this->expiry(impl), T::now());
  }

  std::size_t expiry_at(implementation_type& impl, const time_point& expiry_time, std::error_code& ec)
  {
    std::size_t count = this->cancle(impl, ec);
    impl.expiry = expiry_time;
    ec = std::error_code();
    return count;
  }

  std::size_t expires_after(implementation_type& impl, const duration& expiry_time, std::error_code& ec)
  {
    return this->expiry_at(impl, T::add(T::now(), expiry_time), ec);
  }

  std::size_t expires_from_now(implementation_type& impl, const duration& expiry_time, std::error_code& ec)
  {
    return this->expiry_at(impl, T::add(T::now(), expiry_time), ec);
  }

  void wait(implementation_type& impl, std::error_code& ec)
  {
    time_point now = T::now();
    ec=std::error_code();
    while (T::less_than(now, impl.expiry) && !ec) {

    }
  }

 private:
  template <typename Duration>
  void do_wait(const Duration& timeout, std::error_code& ec)
  {
    std::this_thread::sleep_for(std::chrono::seconds(timeout.total_seconds()) +
                                std::chrono::microseconds(timeout.total_microseconds()));
    ec = std::error_code();
  }

  timer_queue<T> timer_queue_;
  timer_scheduler& scheduler_;
};
};  // namespace boost::asio::detail
#endif
