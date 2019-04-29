#ifndef BOOST_ASIO_DETAIL_CALL_STACK_HPP
#define BOOST_ASIO_DETAIL_CALL_STACK_HPP

#include <memory>
#include <thread>

#include "noncopyable.hpp"

namespace boost::asio::detail
{
template <typename Key, typename Value = unsigned char> class call_stack
{
 public:
  class context : private noncopyable
  {
   public:
    explicit context(Key* k) : key_(k)
    {
      value_ = reinterpret_cast<unsigned char*>(this);
      next_ = call_stack<Key, Value>::top_;
      call_stack<Key, Value>::top = this;
    }

    // 创建时context加入链表栈
    context(Key* k, Value& v) : key_(k), value_(&v)
    {
      next_ = call_stack<Key, Value>::top_;
      call_stack<Key, Value>::top_ = this;
    }

    // 销毁时context移除链表栈
    ~context() { call_stack<Key, Value>::top_ = next_; }

    // 根据栈顶key_查找 在调用链表栈顶的下一个开始查找
    // 一般用于在递归函数调用，查看递归栈内的数据信息
    Value* next_by_key() const
    {
      for (context* elem = next_; elem != 0; elem = elem->next_) {
        if (elem->key_ == key_) {
          return elem->value_;
        }
      }
      return 0;
    }

   private:
    friend class call_stack<Key, Value>;
    Key* key_;
    Value* value_;
    context* next_;
  };  // ! class context

 public:
  friend class context;

  // 根据栈顶key_查找是否在调用链表栈中
  static Value* contains(Key* k)
  {
    for (context* elem = top_; elem != 0; elem = elem->next_) {
      if (elem->key_ == k) {
        return elem->value_;
      }
    }
    return 0;
  }

  // 调用链表栈顶值
  static Value* top() { return top_ ? top_->value_ : 0; }

 private:
  static thread_local context* top_;  // 每个线程都有其副本
};

template <typename Key, typename Value>
thread_local typename call_stack<Key, Value>::context* call_stack<Key, Value>::top_ = 0;

}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_CALL_STACK_HPP
