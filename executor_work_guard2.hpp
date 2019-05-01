#ifndef BOOST_ASIO_EXECUTOR_WORK_GUARD2_HPP
#define BOOST_ASIO_EXECUTOR_WORK_GUARD2_HPP

#include "associated_executor2.hpp"

namespace boost::asio {

template <typename Executor>
class executor_work_guard
{
 public:
  using executor_type = Executor;
  executor_type get_executor() const { return executor_; }

  explicit executor_work_guard(const executor_type& e) : executor_(e), owns_(true) { executor_.on_work_started(); }

  executor_work_guard(const executor_work_guard& other) : executor_(other.executor_), owns_(other.owns_)
  {
    if (owns_) {
      executor_.on_work_started();
    }
  }

  executor_work_guard(executor_work_guard&& other)
      : executor_(std::forward<executor_type>(other.executor_)), owns_(other.owns_)
  {
    other.owns_ = false;
  }

  ~executor_work_guard()
  {
    if (owns_) {
      executor_.on_work_finished();
    }
  }

  bool owns_work() const { return owns_; }

  void reset()
  {
    if (owns_) {
      executor_.on_work_finished();
    }
  }

 private:
  executor_work_guard& operator=(const executor_work_guard&) = delete;
  executor_type executor_;
  bool owns_;
};

template <typename T>
inline auto make_work_guard(const T& t)
{
  if constexpr (detail::is_executor<T>::value) {
    return executor_work_guard<T>(t);
  } else if constexpr (has_type_executor_type<T>) {
    static_assert(std::is_convertible<T&, execution_context&>::value);
    static_assert(std::is_member_function_pointer<(decltype(&T::get_executor))>::value);
    return executor_work_guard<typename T::executor_type>(t.get_executor());
  } else {
    static_assert(false, "make_work_guard failed");
  }
}
}  // namespace boost::asio
#endif  // BOOST_ASIO_EXECUTOR_WORK_GUARD2_HPP