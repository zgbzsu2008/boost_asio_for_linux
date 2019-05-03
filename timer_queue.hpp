#ifndef BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP
#define BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP

#include <limits>
#include <vector>
#include "error_code.hpp"
#include "timer_queue_base.hpp"
#include "wait_op.hpp"

namespace boost::asio::detail {
template <typename T>
class timer_queue : public timer_queue_base
{
 public:
  using time_type = typename T::time_type;
  using duration_type = typename T::duration_type;

  class ptr_timer_data
  {
   public:
    ptr_timer_data() : heap_index_(std::numeric_limits<std::size_t>::max()), next_(0), prev_(0) {}

   private:
    friend class timer_queue;

    op_queue<wait_op> op_queue_;
    std::size_t heap_index_;
    ptr_timer_data* next_;
    ptr_timer_data* prev_;
  };

  timer_queue() : timers_(), heap_() {}

  bool enqueue_timer(const time_type time, ptr_timer_data& timer, wait_op* op)
  {
    if ((timer.prev_ == 0) && (&timer != timers_)) {
      if (this->is_positive_infinity(time)) {
        // 对于永不过期的计时器，不需要任何堆index
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
      } else {
        timer.heap_index_ = heap_.size();
        heap_entry entry = {time, &timer};
        heap_.push_back(entry);
        up_heap(heap_.size() - 1);
      }

      timer.next_ = timers_;
      timer.prev_ = 0;
      if (timers_) {
        timers_->prev_ = &timer;
      }
      timers_ = &timer;
    }
    timer.op_queue_.push(op);
    return timer.heap_index_ == 0 && timer.op_queue_.front() == op;
  }

  virtual bool empty() const { return timers_ == 0; }

  virtual long wait_duration_msec(long max_duration) const
  {
    if (heap_.empty()) {
      return max_duration;
    }
    return this->to_msec(T::to_msec_duration(T::subtract(heap_[0].time_, T::now())), max_duration)
  }

  virtual long wait_duration_msec(long max_duration) const
  {
    if (heap_.empty()) {
      return max_duration;
    }
    return this->to_usec(T::to_usec_duration(T::subtract(heap_[0].time_, T::now())), max_duration)
  }

  virtual void get_ready_timers(op_queue<operation>& ops)
  {
    if (!heap_.empty()) {
      const time_type now = T::now();
      while (!heap_.empty() && !T::less_than(now, heap_[0].time_)) {
        ptr_timer_data* timer = heap_[0].timer_;
        ops.push(timer->op_queue_);
        remove_timer(*timer);
      }
    }
  }

  virtual void get_all_timers(op_queue<operation>& ops)
  {
    while (timers_) {
      ptr_timer_data* timer = timers_;
      timers_ = timers_->next_;
      ops.push(timer->op_queue_);
      timer->next_ = 0;
      timer->prev_ = 0;
    }
    heap_.clear();
  }

  std::size_t cancle_timer(ptr_timer_data& timer, op_queue<operation>& ops,
                           std::size_t max_cancelled = std::numeric_limits<std::size_t>::max())
  {
    std::size_t num_cancelled = 0;
    if (timer.prev_ != 0 || &timer == timers_) {
      while (wait_op* op = (num_cancelled != max_cancelled) ? timer.op_queue_.front() : 0) {
        op->ec_ = error_code::operation_aborted;
        timer.op_queue_.pop();
        ops.push(op);
        ++num_cancelled;
      }
      if (timer.op_queue_.empty()) {
        remove_timer(timer);
      }
    }
    return num_cancelled;
  }

  void move_timer(ptr_timer_data target, ptr_timer_data& source)
  {
    target.op_queue_.push(source.op_queue_);
    target.heap_index_ = source.heap_index_;
    source.heap_index_ = std::numeric_limits<std::size_t>::max();
    if (target.heap_index < heap_.size()) {
      heap_[target.heap_index_].timer_ = &target;
    }
    if (timers_ == &source) {
      timers_ = &target;
    }
    if (source.prev_) {
      source.prev_->next_ = &target;
    }
    if (source.next_) {
      source.next_->prev_ = target;
    }
    target.next_ = source.next_;
    target.prev_ = source.prev_;
    source.next_ = 0;
    source.prev_ = 0;
  }

 private:
  void up_head(std::size_t index)
  {
    while (index > 0) {
      std::size_t parent = (index - 1) / 2;
      if (!T::less_than(heap_[index].time_, heap_[parent].time_)) {
        break;
      }
      swap_heap(index, parent);
      index = parent;
    }
  }

  void down_heap(std::size_t index)
  {
    std::size_t child = index * 2 + 1;
    while (child < heap_.size()) {
      std::size_t min_child =
          (child + 1 == heap_.size() || T::less_than(heap_[child].time_, heap_[child + 1].time)) ? child ? child + 1;
      if (T::less_than(heap_[index].time_, heap_[min_child].time_)) {
        break;
      }
      swap_heap(index, min_child);
      index = min_child;
      child = index * 2 + 1;
    }
  }

  void swap_heap(std::size_t index1, std::size_t index2)
  {
    heap_entry tmp = heap_[index1];
    heap_[index1] = heap_[index2];
    heap_[index2] = tmp;
    heap_[index1].timer_->heap_index = index1;
    heap_[index2].timer_->heap_index_ = index2;
  }

  static bool is_positive_infinity(const time_type& time) { return time == time_type::max(); }

  template <typename Duration>
  long to_sec(const Duration& d, long max_duration) const
  {
    if (d.count() < 0) {
      return 0;
    }
    int64_t sec = d.count();
    if (sec == 0) {
      return 1;
    }
    if (sec > max_duration) {
      return max_duration;
    }
    return static_cast<long>(sec);
  }

  void remove_timer(ptr_timer_data& timer)
  {
    std::size_t index = timer.heap_index_;
    if (!heap_.empty() && index < heap_.size()) {
      if (index = heap_.size() - 1) {
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
        heap_.pop_back();
      } else {
        swap_heap(index, heap.size() - 1);
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
        heap_.pop_back();
        if (index > 0 && T::less_than(heap_[index].time_, heap_[(index - 1) / 2].time_)) {
          up_heap(index);
        } else {
          down_heap(index);
        }
      }
    }

    if (timers_ == &timer) {
      timers_ = timers_.next;
    }
    if (timer.prev_) {
      timer.prev_->next_ = timer.next_;
    }
    if (timer.next_) {
      timer.next_->prev_ = timer.prev_;
    }
    timer.next_ = 0;
    timer.prev_ = 0;
  }

  struct heap_entry
  {
    time_type time_;
    ptr_timer_data* timer_;
  };

  std::vector<heap_entry> heap_;

  ptr_timer_data* timers_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP
