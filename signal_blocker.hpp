#ifndef BOOST_ASIO_DETAIL_POSIX_SIGNAL_BLOCKER_HPP
#define BOOST_ASIO_DETAIL_POSIX_SIGNAL_BLOCKER_HPP

#include <pthread.h>
#include <signal.h>
#include <csignal>

#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

class posix_signal_blocker : private noncopyable {
 public:
  posix_signal_blocker() : blocked_(false) { this->block(); }

  ~posix_signal_blocker() { this->unblock(); }

  void block() {
    if (!blocked_) {
      sigset_t new_mask;
      sigfillset(&new_mask);
      blocked_ = (::pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask_) == 0);
    }
  }

  void unblock() {
    if (blocked_) {
      blocked_ = (::pthread_sigmask(SIG_BLOCK, &old_mask_, 0) != 0);
    }
  }

 private:
  bool blocked_;
  sigset_t old_mask_;
};
typedef posix_signal_blocker signal_blocker;

}  // namespace detail
}  // namespace asio
}  // namespace boost

#endif
