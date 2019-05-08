#ifndef BOOST_ASIO_DETAIL_HANDLER_INVOKE_HELPERS_HPP
#define BOOST_ASIO_DETAIL_HANDLER_INVOKE_HELPERS_HPP

#include <memory>
#include "handler_invoke_hook.hpp"
#include "config.hpp"

namespace boost_asio_handler_invoke_helpers {

template <typename Function, typename Context>
inline void invoke(Function& function, Context& context) {
#if !defined(BOOST_ASIO_HAS_HANDLER_HOOKS)
  Function tmp(function);
  tmp();
#else
  using boost::asio::asio_handler_invoke;
  asio_handler_invoke(function, std::addressof(context));
#endif
}

template <typename Function, typename Context>
inline void invoke(const Function& function, Context& context)
{
#if !defined(BOOST_ASIO_HAS_HANDLER_HOOKS)
  Function tmp(function);
  tmp();
#else
  using boost::asio::asio_handler_invoke;
  asio_handler_invoke(function, std::addressof(context));
#endif
}

}  // namespace boost_asio_handler_invoke_helpers
#endif
