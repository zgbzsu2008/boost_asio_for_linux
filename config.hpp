#include <iostream>

#if !defined(BOOST_ASIO_HAS_TIMERFD)
#define BOOST_ASIO_HAS_TIMERFD
#endif

#if !defined(BOOST_ASIO_HAS_MOVE)
#define BOOST_ASIO_HAS_MOVE
#endif

#if !defined(BOOST_ASIO_HAS_HANDLER_HOOKS)
#define BOOST_ASIO_HAS_HANDLER_HOOKS
#endif

#if !defined(BOOST_ASIO_SMALL_BLOCK_RECYCLING)
#define BOOST_ASIO_SMALL_BLOCK_RECYCLING
#endif