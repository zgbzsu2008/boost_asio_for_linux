#ifndef BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP
#define BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP

#include <chrono>
#include <numeric>
#include "wait_traits.hpp"

namespace boost::asio::detail {

// timer_traits·¶Î§¼ì²â°æ
template <typename Clock, typename WaitTraits>
struct chrono_time_traits
{
  using clock_type = Clock;
  using duration = typename clock_type::duration;
  using time_point = typename clock_type::time_point;

  static time_point now() { return clock_type::now(); }

  static time_point add(const time_point& t, const duration& d)
  {
    const time_point epoch;
    if (t >= epoch) {
      if (time_point::max() - t < d) {
        return time_point::max();
      }
    } else {
      if (-(t - time_point::min()) > d) {
        return time_point::min();
      }
    }
    return t + d;
  }

  static duration subtract(const time_point& t1, const time_point t2)
  {
    const time_point epoch;
    if (t1 >= epoch) {
      if (t2 >= epoch) {
        return t1 - t2;
      } else if (t2 == time_point::min()) {
        return duration::max();
      } else if (time_point::max() - t1 < epoch - t2) {
        return duration::max();
      } else {
        return t1 - t2;
      }
    } else {
      if (t2 < epoch) {
        return t1 - t2;
      } else if (t1 == time_point::min()) {
        return duration::max();
      } else if (time_point::max() - t2 < epoch - t1) {
        return duration::min();
      } else {
        return -(t2 - t1);
      }
    }
  }

  static bool less_than(const time_point& t1, const time_point& t2) { return t1 < t2; }

  static std::chrono::nanoseconds to_chrono_duration(const duration& d) { return WaitTraits::to_wait_duration(d); }
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP