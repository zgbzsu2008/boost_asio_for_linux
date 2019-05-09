#ifndef BOOST_ASIO_TIME_TRAITS_HPP
#define BOOST_ASIO_TIME_TRAITS_HPP

#include <chrono>

namespace boost::asio {

// ʱ������
template <typename Clock>
struct time_traits
{
  using clock_type = Clock;                            // ʱ��
  using time_point = typename clock_type::time_point;  // ʱ���
  using duration = typename clock_type::duration;      // ʱ����

  // ���ص�ǰʱ���
  static time_point now() { return clock_type::now(); }

  // ��ʱ��� = ��ʱ��� + ʱ����
  static time_point add(const time_point& t, const duration& d) { return t + d; }

  // ʱ���� = ʱ���1 - ʱ���2
  static duration subtract(const time_point& t1, const time_point& t2) { return t1 - t2; }

  // ʱ���t1 < ʱ���t2
  static bool less_than(const time_point& t1, const time_point& t2) { return t1 < t2; }

  // ��ǰdurationת��Ϊstd::chrono���duration
  static std::chrono::nanoseconds to_chrono_duration(const duration& d) { return d; }
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_TIME_TRAITS_HPP