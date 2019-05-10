#ifndef BOOST_ASIO_USES_EXECUTOR_HPP
#define BOOST_ASIO_USES_EXECUTOR_HPP

#include <type_traits>

namespace boost::asio {

struct executor_arg_t
{
  constexpr executor_arg_t() {}
};

template <typename T, typename Executor>
struct uses_executor : std::false_type
{};
}  // namespace boost::asio

#endif  // !BOOST_ASIO_USES_EXECUTOR_HPP
