#ifndef BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP
#define BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP

#include <pthread.h>
#include <signal.h>
#include <csignal>
#include "noncopyable.hpp"

namespace boost::asio::detail {
class signal_blocker : private noncopyable
{
 public:
  signal_blocker() : blocked_(false) { this->block(); }
  ~signal_blocker() { this->unblock(); }

  void block()
  {
    if (!blocked_) {
      sigset_t new_mask;
      sigfillset(&new_mask);
      blocked_ = (::pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask_) == 0);  // ±£¥Ê
    }
  }

  void unblock()
  {
    if (blocked_) {
      blocked_ = (::pthread_sigmask(SIG_BLOCK, &old_mask_, 0) != 0);  // ÷ÿ÷√
    }
  }

 private:
  bool blocked_;
  sigset_t old_mask_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP