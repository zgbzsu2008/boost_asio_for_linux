#ifndef BOOST_ASIO_DETAIL_TIMER_QUEUE_SET_HPP
#define BOOST_ASIO_DETAIL_TIMER_QUEUE_SET_HPP

#include "timer_queue_base.hpp"

namespace boost::asio::detail {
class timer_queue_set
{
 public:
  timer_queue_set() : first_(0) {}

  void insert(timer_queue_base* q)
  {
    q->next_ = first_;
    first_ = q;
  }
  void erase(timer_queue_base* q)
  {
    if (first_) {
      if (q == first_) {
        first_ = q->next_;
        q->next_ = 0;
        return;
      }
      for (auto p = first_; p->next_; p = p->next_) {
        if (p->next_ == q) {
          p->next_ = q->next_;
          q->next_ = 0;
          return;
        }
      }
    }
  }

  bool all_empty() const
  {
    for (auto p = first_; p; p = p->next_) {
      if (!p->empty()) {
        return false;
      }
    }
    return true;
  }

  long wait_duration_msec(long max_duration) const
  {
    long min_duration = max_duration;
    for (auto p = first_; p; p = p->next_) {
      min_duration = p->wait_duration_msec(min_duration);
    }
    return min_duration;
  }

  long wait_duration_usec(long max_duration) const
  {
    long min_duration = max_duration;
    for (auto p = first_; p; p = p->next_) {
      min_duration = p->wait_duration_usec(min_duration);
    }
    return min_duration;
  }

  void get_ready_timers(op_queue<operation>& ops)
  {
    for (auto p = first_; p; p = p->next_) {
      p->get_ready_timers(ops);
    }
  }

  void get_all_timers(op_queue<operation>& ops)
  {
    for (auto p = first_; p; p = p->next_) {
      p->get_all_timers(ops);
    }
  }

 private:
  timer_queue_base* first_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_TIMER_QUEUE_SET_HPP
