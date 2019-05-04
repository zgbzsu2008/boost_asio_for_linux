#ifndef BOOST_ASIO_WAIT_TRAITS_HPP
#define BOOST_ASIO_WAIT_TRAITS_HPP

namespace boost::asio {
template <typename Clock>
struct wait_traits
{
  static typename Clock::duration to_wait_duration(const typename Clock::duration& d) { return d; }

  static typename Clock::duration to_wait_duration(const typename Clock::time_point& t) {
    typename Clock::time_point now = Clock::now();
    if (now + (Clock::duration::max()) < t) {
      return Clock::duration::max();
    }
    if (now + (Clock::duration::min()) > t) {
      return Clock::duration::min();
    }
    return t - now;
  }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_WAIT_TRAITS_HPP
