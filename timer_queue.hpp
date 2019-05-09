#ifndef BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP
#define BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP

#include <limits>
#include <vector>
#include "error_code.hpp"
#include "timer_queue_base.hpp"
#include "wait_op.hpp"

// 定时器队列
namespace boost::asio::detail {
template <typename T>
class timer_queue : public timer_queue_base
{
 public:
  using time_point = typename T::time_point;  // 时间点
  using duration = typename T::duration;      // 时间间隔

  // 定时器数据
  class per_timer_data
  {
   public:
    per_timer_data() : heap_index_(std::numeric_limits<std::size_t>::max()), next_(0), prev_(0) {}

   private:
    friend class timer_queue;

    op_queue<wait_op> op_queue_;  // 定时器操作队列wait_op
    std::size_t heap_index_;      // 定时器最小堆中的id
    per_timer_data* next_;
    per_timer_data* prev_;
  };

  timer_queue() : timers_(), heap_() {}

  // 定时器添加到队列中
  bool enqueue_timer(const time_point& time, per_timer_data& timer, wait_op* op)
  {
    if ((timer.prev_ == 0) && (&timer != timers_)) {
      if (this->is_positive_infinity(time)) {
        // 对于永不过期的计时器，不需要任何堆index
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
      } else {
        // 定时器添加到数组尾部
        timer.heap_index_ = heap_.size();
        heap_entry entry = {time, &timer};
        heap_.push_back(entry);
        up_heap(heap_.size() - 1); // 最小堆操作up
      }

      // 添加到链表中, 链表头部
      timer.next_ = timers_;
      timer.prev_ = 0;
      if (timers_) {
        timers_->prev_ = &timer;
      }
      timers_ = &timer;
    }
    // 定时器操作添加到op_queue队列中
    timer.op_queue_.push(op);
    return timer.heap_index_ == 0 && timer.op_queue_.front() == op;
  }

  // 定时器队列是否空
  virtual bool empty() const { return timers_ == 0; }

  // 等待时间间隔毫秒10^-3
  virtual long wait_duration_msec(long max_duration) const
  {
    if (heap_.empty()) {
      return max_duration;
    }
    return this->to_msec(T::to_chrono_duration(T::subtract(heap_[0].time_, T::now())), max_duration);
  }

  // 等待时间间隔微妙10^-6
  virtual long wait_duration_usec(long max_duration) const
  {
    if (heap_.empty()) {
      return max_duration;
    }
    return this->to_usec(T::to_chrono_duration(T::subtract(heap_[0].time_, T::now())), max_duration);
  }

  // 返回已经到期的定时器操作
  virtual void get_ready_timers(op_queue<operation>& ops)
  {
    if (!heap_.empty()) {
      const time_point now = T::now();
      while (!heap_.empty() && !T::less_than(now, heap_[0].time_)) { // 是否超时
        per_timer_data* timer = heap_[0].timer_;
        ops.push(timer->op_queue_);
        this->remove_timer(*timer); // 移除定时器
      }
    }
  }

  // 返回所有定时器
  virtual void get_all_timers(op_queue<operation>& ops)
  {
    while (timers_) {
      per_timer_data* timer = timers_;
      timers_ = timers_->next_;
      ops.push(timer->op_queue_);
      timer->next_ = 0;
      timer->prev_ = 0;
    }
    heap_.clear();
  }

  // 取消定时器
  std::size_t cancel_timer(per_timer_data& timer, op_queue<operation>& ops,
                           std::size_t max_cancelled = std::numeric_limits<std::size_t>::max())
  {
    std::size_t num_cancelled = 0;
    if (timer.prev_ != 0 || &timer == timers_) { // 定时器在队列中<是头部或者有前驱>
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

  // 移动定时器
  void move_timer(per_timer_data target, per_timer_data& source)
  {
    // 处理数组中的位置
    target.op_queue_.push(source.op_queue_);
    target.heap_index_ = source.heap_index_;
    source.heap_index_ = std::numeric_limits<std::size_t>::max();
    if (target.heap_index < heap_.size()) {
      heap_[target.heap_index_].timer_ = &target;
    }

    // 处理链表中的位置
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
  void up_heap(std::size_t index)
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
          (child + 1 == heap_.size() || T::less_than(heap_[child].time_, heap_[child + 1].time_)) ? child : child + 1;
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
    heap_[index1].timer_->heap_index_ = index1;
    heap_[index2].timer_->heap_index_ = index2;
  }

  // 定时器时间点是否最大值
  static bool is_positive_infinity(const time_point& time) { return time == time_point::max(); }

  // 时间间隔转换nans-->msec
  long to_msec(const duration& d, long max_duration) const
  {
    if (d.count() < 0) {
      return 0;
    }
    int64_t msec = d.count() / 1000000;
    if (msec == 0) {
      return 1;
    }
    if (msec > max_duration) {
      return max_duration;
    }
    return static_cast<long>(msec);
  }

  // 时间间隔转换nans-->usec
  long to_usec(const duration& d, long max_duration) const
  {
    if (d.count() < 0) {
      return 0;
    }
    int64_t usec = d.count() / 1000;
    if (usec == 0) {
      return 1;
    }
    if (usec > max_duration) {
      return max_duration;
    }
    return static_cast<long>(usec);
  }

  // 移除定时器
  void remove_timer(per_timer_data& timer)
  {
    std::size_t index = timer.heap_index_;
    if (!heap_.empty() && index < heap_.size()) {
      // 在数组尾部直接移除
      if (index == heap_.size() - 1) {
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
        heap_.pop_back();
      } else {
        // 跟数组尾部交换再移除
        swap_heap(index, heap_.size() - 1);
        timer.heap_index_ = std::numeric_limits<std::size_t>::max();
        heap_.pop_back();
        // 重新调整成最小堆
        if (index > 0 && T::less_than(heap_[index].time_, heap_[(index - 1) / 2].time_)) {
          up_heap(index);
        } else {
          down_heap(index);
        }
      }
    }

    // 在链表中移除定时器
    if (timers_ == &timer) {
      timers_ = timers_->next_;
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

  per_timer_data* timers_; // 定时器链表存储

  // 最小堆数据
  struct heap_entry
  {
    time_point time_;        // 定时器时间点
    per_timer_data* timer_;  // 定时器数据
  };

  std::vector<heap_entry> heap_;  // 用数组保存最小堆
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_TIMER_QUEUE_HPP
