#ifndef BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP
#define BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP

namespace boost::asio {

template <typename Function>
inline void asio_handler_invoke(Function& function, ...) {
  function();
}

template <typename Function>
inline void asio_handler_invoke(const Function& function, ...)
{
  Function tmp(function);
  tmp();
}

}
#endif // !BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP
