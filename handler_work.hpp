#ifndef BOOST_ASIO_DETAIL_HANDLER_WORK_HP
#define BOOST_ASIO_DETAIL_HANDLER_WORK_HPP

#include "associated_executor.hpp"
#include "noncopyable.hpp"
#include "system_executor.hpp"
#include "handler_invoke_helpers.hpp"

namespace boost::asio::detail {

template <typename Handler, typename Executor = typename associated_executor<Handler>::type>
class handler_work : public noncopyable
{
 public:
  explicit handler_work(Handler& handler) : executor_(associated_executor<Handler>::get(handler)) {}

  ~handler_work() { executor_.on_work_finished(); }

  static void start(Handler& handler)
  {
    Executor ex(associated_executor<Handler>::get(handler));
    ex.on_work_started();
  }
  
  template <typename Function>
  void complate(Function&& function, Handler& handler) {
    executor_.dispatch(std::forward<Function>(function), associated_executor<Handler>::get(handler));
  }
 private:
  typename associated_executor<Handler>::type executor_;
};

template <typename Handler>
class handler_work<Handler, system_executor> : public noncopyable {
 public:
  explicit handler_work(Handler&) {}
  ~handler_work() {}

  static void start(Handler&) {}
 
  template <typename Function>
  void complate(Function&& function, Handler& handler)
  {
    boost_asio_handler_invoke_helpers::invoke(function, handler);
  }
};

}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_HANDLER_WORK_HPP
