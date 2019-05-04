#ifndef BOOST_ASIO_STEADY_TIMER_HPP
#define BOOST_ASIO_STEADY_TIMER_HPP

#include <chrono>
#include "basic_waitable_timer.hpp"

namespace boost::asio {
typedef basic_waitable_timer<std::chrono::steady_clock> steady_timer;
}

#endif  // !BOOST_ASIO_STEADY_TIMER_HPP