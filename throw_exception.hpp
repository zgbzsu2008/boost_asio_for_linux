#ifndef BOOST_ASIO_DETAIL_THROW_EXCEPTION_HPP
#define BOOST_ASIO_DETAIL_THROW_EXCEPTION_HPP

#include <system_error>

namespace boost::asio::detail
{
template <typename Exception> void throw_exception(const Exception& e) { throw e; }
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_THROW_EXCEPTION_HPP