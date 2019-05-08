#ifndef BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP
#define BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP

namespace boost::asio {

template <typename Function, typename... Args>
inline void asio_handler_invoke(Function& function, Args... /*args*/)
{
  function();
}

template <typename Function, typename... Args>
inline void asio_handler_invoke(const Function& function, Args... /*args*/)
{
  Function tmp(function);
  tmp();
}

}  // namespace boost::asio
#endif  // !BOOST_ASIO_HANDLER_INVOKE_HOOK_HPP
