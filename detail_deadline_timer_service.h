#ifndef BOOST_ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP
#define BOOST_ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP

#include "chrono_time_traits.hpp"
#include "epoll_reactor.hpp"
#include "io_context.hpp"
#include "service_registry_helpers.hpp"
#include "timer_queue.hpp"
#include "wait_traits.hpp"
#include "wait_handler.hpp"

namespace boost::asio::detail {
template <typename Clock>
class deadline_timer_service : public service_base<deadline_timer_service<Clock>>
{
 public:
  using time_point = typename Clock::time_point;
  using duration = typename Clock::duration;
  using timer_scheduler = epoll_reactor;

  struct impl_type : private noncopyable
  {
    time_point expiry;
    bool might_have_pending_waits;
    typename timer_queue<Clock>::per_timer_data timer_data;
  };

  deadline_timer_service(io_context& ioc)
      : service_base<deadline_timer_service<Clock>>(ioc), scheduler_(use_service<timer_scheduler>(ioc))
  {
    scheduler_.init_task();
    scheduler_.add_timer_queue(timer_queue_);
  }

  ~deadline_timer_service() { scheduler_.remove_timer_queue(timer_queue_); }

  void shutdown() {}

  void construct(impl_type& impl)
  {
    impl.expiry = time_point();
    impl.might_have_pending_waits = false;
  }

  void destroy(impl_type& impl)
  {
    std::error_code ec;
    this->cancle(impl, ec);
  }

  void move_construct(impl_type& impl, impl_type& other_impl)
  {
    scheduler_.move_timer(timer_queue_, impl.timer_data, other_impl.timer_data);
    impl.expiry = other_impl.expiry;
    other_impl.expiry = time_point();
    impl.might_have_pending_waits = other_impl.might_have_pending_waits;
    other_impl.might_have_pending_waits = false;
  }

  void move_assign(impl_type& impl, deadline_timer_service& other_service, impl_type& other_impl)
  {
    if (this != &other_service) {
      if (impl.might_have_pending_waits) {
        scheduler_.cancel_timer(timer_queue_, impl.timer_data);
      }
    }

    other_service.scheduler_.move_timer(other_service.timer_queue_, impl.timer_data, other_impl.timer_data);

    impl.expiry = other_impl.expiry;
    other_impl.expiry = time_point();

    impl.might_have_pending_waits = other_impl.might_have_pending_waits;
    other_impl.might_have_pending_waits = false;
  }

  std::size_t cancle(impl_type& impl, std::error_code& ec)
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

  std::size_t cancle_one(impl_type& impl, std::error_code& ec)
  {
    if (!impl.might_have_pending_waits) {
      ec = std::error_code();
      return 0;
    }
    std::size_t count = scheduler_.cancel_timer(timer_queue_, impl.timer_data, 1);
    if (count == 0) {
      impl.might_have_pending_waits = false;
    }
    ec = std::error_code();
    return count;
  }

  time_point expiry(const impl_type& impl) const { return impl.expiry; }

  time_point expires_at(const impl_type& impl) const { return impl.expiry; }

  duration expires_from_now(const impl_type& impl) const { return Clock::subtract(this->expiry(impl), Clock::now()); }

  std::size_t expires_at(impl_type& impl, const time_point& expiry_time, std::error_code& ec)
  {
    std::size_t count = this->cancle(impl, ec);
    impl.expiry = expiry_time;
    ec = std::error_code();
    return count;
  }

  std::size_t expires_after(impl_type& impl, const duration& expiry_time, std::error_code& ec)
  {
    return this->expires_at(impl, Clock::add(Clock::now(), expiry_time), ec);
  }

  std::size_t expires_from_now(impl_type& impl, const duration& expiry_time, std::error_code& ec)
  {
    return this->expires_at(impl, Clock::add(Clock::now(), expiry_time), ec);
  }

  void wait(impl_type& impl, std::error_code& ec)
  {
    time_point now = Clock::now();
    ec = std::error_code();
    while (Clock::less_than(now, impl.expiry) && !ec) {
      this->do_wait(Clock::to_chrono_duration(Clock::subtract(impl.expiry, now)), ec);
      now = Clock::now();
    }
  }

  template <typename Handler>
  void async_wait(impl_type& impl, Handler& handler)
  {
    using op = wait_handler<Handler>;
    typename op::ptr p = {std::addressof(handler), op::ptr::allocate(handler), 0};
    p.p = new (p.v) op(handler);

    impl.might_have_pending_waits = true;
    scheduler_.schedule_timer(timer_queue_, impl.expiry, impl.timer_data, p.p);
    p.v = p.p = 0;
  }

 private:
  void do_wait(const duration& timeout, std::error_code& ec)
  {
    std::this_thread::sleep_for(Clock::to_chrono_duration(timeout));
    ec = std::error_code();
  }

  timer_queue<Clock> timer_queue_;
  timer_scheduler& scheduler_;
};
};  // namespace boost::asio::detail
#endif
