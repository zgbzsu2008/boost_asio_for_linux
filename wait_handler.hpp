#ifndef BOOST_ASIO_DETAIL_WAIT_HANDLER_HPP
#define BOOST_ASIO_DETAIL_WAIT_HANDLER_HPP

#include <functional>
#include "handler_alloc_helpers.hpp"
#include "wait_op.hpp"
#include "handler_work.hpp"
#include "fenced_block.hpp"

namespace boost::asio::detail {
template <typename Handler>
class wait_handler : public wait_op
{
 public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(wait_handler);

  wait_handler(Handler&& h) : wait_op(&wait_handler::do_complete), handler_(std::forward<Handler>(h)) {
    handler_work<Handler>::start(handler_);
  }

  static void do_complete(void* owner, operation* base, const std::error_code, std::size_t)
  {
    wait_handler* h(static_cast<wait_handler*>(base));
    ptr p = {std::addressof(h->handler_), h, h};
    handler_work<Handler> w(h->handler_);
    std::function<void(std::error_code)> hander = std::bind(h->handler_, h->ec_);
    p.reset();

    if (owner) {
      fenced_block b(fenced_block::half);
      w.complate(hander, hander);
    }
  }

 private:
  Handler handler_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_WAIT_HANDLER_HPP
