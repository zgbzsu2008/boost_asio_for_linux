#ifndef BOOST_ASIO_TIME_TRAITS_HPP
#define BOOST_ASIO_TIME_TRAITS_HPP

#include <chrono>

namespace boost::asio {

template <typename T = std::chrono::system_clock::time_point>
struct time_traits
{
  using time_type = std::chrono::system_clock::time_point;
  using duration_type = std::chrono::microseconds;
  static time_type now() { return std::chrono::system_clock::now(); }

  static time_type add(const time_type& t, const duration_type& d) { return t + d; }

  static duration_type subtract(const time_type& t1, const time_type& t2)
  {
    return std::chrono::duration_cast<duration_type>(t1 - t2);
  }

  static bool less_than(const time_type& t1, const time_type& t2) { return t1 < t2; }

  static std::chrono::milliseconds to_msec_duration(const duration_type& d)
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d);
  }

  static std::chrono::microseconds to_usec_duration(const duration_type& d) { return d; }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_TIME_TRAITS_HPP