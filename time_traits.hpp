#ifndef BOOST_ASIO_TIME_TRAITS_HPP
#define BOOST_ASIO_TIME_TRAITS_HPP

#include <chrono>

namespace boost::asio {

template <typename Clock>
struct time_traits
{
  using clock_type = Clock;
  using time_point = typename clock_type::time_point;
  using duration = typename clock_type::duration;

  static time_point now() { return clock_type::now(); }

  static time_point add(const time_point& t, const duration& d) { return t + d; }

  static duration subtract(const time_point& t1, const time_point& t2) { return t1 - t2; }

  static bool less_than(const time_point& t1, const time_point& t2) { return t1 < t2; }

  static std::chrono::nanoseconds to_chrono_duration(const duration& d) { return d; }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_TIME_TRAITS_HPP