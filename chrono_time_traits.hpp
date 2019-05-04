#ifndef BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP
#define BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP

#include <chrono>
#include <numeric>
#include "wait_traits.hpp"

namespace boost::asio::detail {

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
        t1 - t2;
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

  static int64_t to_seconds(const duration& d) { return d.count() / 1000000000; }

  static int64_t to_microseconds(const duration& d) { return d.count() / 1000000; }

  static int64_t to_milliseconds(const duration& d) { return d.count() / 1000; }

  /*
  class posix_time_duration {
   public:
    explicit posix_time_duration(const duration& d) : d_(d) {}

    int64_t ticks() const { return d_.count(); }
    int64_t total_seconds() const { return duration_cast<1, 1000>(); }
    int64_t total_milliseconds() const { return duration_cast<1, 1000>(); }
    int64_t total_microseconds() const { return duration_cast<1, 100000>(); }

   private:
    template <int64_t Num, int64_t Den>
    int64_t duration_cast() const {
      const int64_t num1 = period::num / std::gcd(period::num, Num);
      const int64_t num2 = Num / std::gcd(period::num, Num);

      const int64_t den1 = period::den / std::gcd(period::den, Den);
      const int64_t den2 = Den / std::gcd(period::den, Den);

      const num = num1 * den2;
      const den = num2 * den1;

      if (num == 1 && den == 1) {
        return ticks();
      } else if (num != 1 && den == 1) {
        return ticks() * num;
      } else if (num == 1 && period::den != 1) {
        return ticks() / den;
      } else {
        return ticks() * num / den;
      }
    }
    duration d_;
  };

  static boost::posix_time::time_duration to_posix_duration(const duration& d) {
    return posix_time_duration(WaitTraits::to_wait_duration(d));
  }
  */
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_CHRONO_TIME_TRAITS_HPP
