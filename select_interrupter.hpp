#ifndef BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP
#define BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP

#include <errno.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <system_error>

namespace boost::asio::detail
{
class select_interrupter
{
 public:
  select_interrupter() { open_descriptors(); }
  ~select_interrupter() { close_descriptors(); }

  void recreate()
  {
    close_descriptors();
    fd_ = -1;
    open_descriptors();
  }

  void interrupt()
  {
    uint64_t n = 1;
    ssize_t result = ::write(fd_, &n, sizeof(uint64_t));
    (void)result;
  }

  bool reset()
  {
    for (;;) {
      uint64_t n(0);
      errno = 0;
      ssize_t result = ::read(fd_, &n, sizeof(uint64_t));
      if (result < 0 && errno == EINTR) {
        continue;
      }
      return result > 0;
    }
  }

  int fd() const { return fd_; }

 private:
  void open_descriptors()
  {
    fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (fd_ < 0) {
      throw std::error_code{errno, std::generic_category()};
    }
  }

  void close_descriptors()
  {
    if (fd_ != -1) {
      ::close(fd_);
    }
  }

  int fd_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP
