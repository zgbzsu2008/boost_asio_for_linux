#ifndef BOOST_ASIO_DETAIL_ERROR_CODE_HPP
#define BOOST_ASIO_DETAIL_ERROR_CODE_HPP

#include <system_error>

namespace boost::asio::detail
{
enum class error_code {
  success = 0,
  operation_aborted,
};

inline std::error_code make_error_code(error_code code)
{
  return {
      static_cast<int>(code),
      std::generic_category(),
  };
}
}  // namespace boost::asio::detail

namespace std
{
template <> struct is_error_code_enum<boost::asio::detail::error_code> : true_type {
};
}  // namespace std

#endif  //! BOOST_ASIO_DETAIL_ERROR_CODE