#ifndef BOOST_ASIO_TIME_TRAITS_HPP
#define BOOST_ASIO_TIME_TRAITS_HPP

#include <chrono>

namespace boost::asio {

// 时间特性
template <typename Clock>
struct time_traits
{
  using clock_type = Clock;                            // 时钟
  using time_point = typename clock_type::time_point;  // 时间点
  using duration = typename clock_type::duration;      // 时间间隔

  // 返回当前时间点
  static time_point now() { return clock_type::now(); }

  // 新时间点 = 旧时间点 + 时间间隔
  static time_point add(const time_point& t, const duration& d) { return t + d; }

  // 时间间隔 = 时间点1 - 时间点2
  static duration subtract(const time_point& t1, const time_point& t2) { return t1 - t2; }

  // 时间点t1 < 时间点t2
  static bool less_than(const time_point& t1, const time_point& t2) { return t1 < t2; }

  // 当前duration转换为std::chrono类的duration
  static std::chrono::nanoseconds to_chrono_duration(const duration& d) { return d; }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_TIME_TRAITS_HPP