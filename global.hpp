#ifndef BOOST_ASIO_DETAIL_GLOBAL_HPP
#define BOOST_ASIO_DETAIL_GLOBAL_HPP

#include <mutex>

namespace boost::asio::detail
{
template <typename T> struct global_impl {
  ~global_impl();

  static void do_init() { instance_.ptr_ = new T; }
  static std::once_flag init_once_;
  static global_impl instance_;
  T* ptr_;
};

// ʵ����once_flag�൱��һ������ʹ�������̶߳���������ȴ���������һ���߳�ͬ�����С�
// ������߳��׳��쳣����ô�ӵȴ��е��߳���ѡ��һ����������������̡�

template <typename T> std::once_flag global_impl<T>::init_once_;

template <typename T> global_impl<T> global_impl<T>::instance_;

template <typename T> inline global_impl<T>::~global_impl() { delete ptr_; }

template <typename T> T& global()
{
  std::call_once(global_impl<T>::init_once_, &global_impl<T>::do_init);
  return *global_impl<T>::instance_.ptr_;
}
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_GLOBAL_HPP