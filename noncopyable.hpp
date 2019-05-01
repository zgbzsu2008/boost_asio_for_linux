#ifndef BOOST_ASIO_DETAIL_NONCOPYABLE_HPP
#define BOOST_ASIO_DETAIL_NONCOPYABLE_HPP

namespace boost::asio::detail {
class noncopyable
{
 protected:
  noncopyable() {}
  ~noncopyable() {}

 private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_NONCOPYABLE_HPP