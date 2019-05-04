#ifndef BOOST_ASIO_TIME_TRAITS_HPP
#define BOOST_ASIO_TIME_TRAITS_HPP

#include <chrono>

namespace boost::asio {

template <typename T = std::chrono::system_clock>
struct time_traits;

template <>
struct time_traits<std::chrono::system_clock>
{
  using clock_type = std::chrono::system_clock;
  using time_point = clock_type::time_point;
  using duration = clock_type::duration;

  static time_point now() { return std::chrono::system_clock::now(); }

  static time_point add(const time_point& t, const duration& d) { return t + d; }

  static duration subtract(const time_point& t1, const time_point& t2) { return t1 - t2; }

  static bool less_than(const time_point& t1, const time_point& t2) { return t1 < t2; }

  template <typename Rep, typename Period>
  static std::chrono::duration<Rep, Period> to_chrono_duration(const duration& d)
  {
    return d;
  }
  // static posix_time_duration to_posix_duration(const duration& d) { return d; }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_TIME_TRAITS_HPP