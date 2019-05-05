#ifndef BOOST_ASIO_WAIT_TRAITS_HPP
#define BOOST_ASIO_WAIT_TRAITS_HPP

namespace boost::asio {

template <typename Clock>
struct wait_traits
{
  using clock_type = Clock;
  using duration = typename clock_type::duration;
  using time_point = typename clock_type::time_point;

  static duration to_wait_duration(const duration& d) { return d; }

  static duration to_wait_duration(const time_point& t)
  {
    time_point now = clock_type::now();
    if (now + (duration::max()) < t) {
      return duration::max();
    }
    if (now + duration::min() > t) {
      return duration::min();
    }
    return t - now;
  }
};

}  // namespace boost::asio
#endif  // !BOOST_ASIO_WAIT_TRAITS_HPP