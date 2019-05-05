#ifndef BOOST_ASIO_SYSTEM_TIMER_HPP
#define BOOST_ASIO_SYSTEM_TIMER_HPP

#include <chrono>
#include "basic_waitable_timer.hpp"

namespace boost::asio {
using steady_timer = basic_waitable_timer<std::chrono::system_clock>;
}

#endif // !BOOST_ASIO_SYSTEM_TIMER_HPP
