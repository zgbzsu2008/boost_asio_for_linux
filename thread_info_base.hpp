#ifndef BOOST_ASIO_DETAIL_THREAD_INFO_BASE_HPP
#define BOOST_ASIO_DETAIL_THREAD_INFO_BASE_HPP

#include <limits>
#include <memory>
#include "noncopyable.hpp"

namespace boost::asio::detail {
class thread_info_base : private noncopyable
{
 public:
  struct default_tag
  {
    enum
    {
      mem_index = 0
    };
  };

  struct awaitee_tag
  {
    enum
    {
      mem_index = 1
    };
  };

  thread_info_base()
  {
    for (int i = 0; i < max_mem_index; i++) {
      reusable_memory_[i] = 0;
    }
  }

  ~thread_info_base()
  {
    for (int i = 0; i < max_mem_index; ++i) {
      auto& ptr = reusable_memory_[i];
      if (ptr) {
        ::operator delete(ptr);
        ptr = 0;
      }
    }
  }

  static void* allocate(thread_info_base* this_thread, std::size_t size)
  {
    return allocate(default_tag(), this_thread, size);
  }

  static void deallocate(thread_info_base* this_thread, void* pointer, std::size_t size)
  {
    deallocate(default_tag(), this_thread, pointer, size);
  }

  template <typename Purpose>
  static void* allocate(Purpose, thread_info_base* this_thread, std::size_t size)
  {
    std::size_t chunks = (size + chunk_size - 1) / chunk_size;
    if (this_thread && this_thread->reusable_memory_[Purpose::mem_index]) {
      void* pointer = this_thread->reusable_memory_[Purpose::mem_index];
      this_thread->reusable_memory_[Purpose::mem_index] = 0;
      unsigned char* const mem = static_cast<unsigned char*>(pointer);
      if (static_cast<std::size_t>(mem[0]) >= chunks) {
        mem[size] = mem[0];
        return pointer;
      }
      ::operator delete(pointer);
    }

    void* const pointer = ::operator new((chunk_size * chunks) + 1);
    unsigned char* const mem = static_cast<unsigned char*>(pointer);
    if (chunks <= std::numeric_limits<unsigned char>::max()) {
      mem[size] = static_cast<unsigned char>(chunks);
    } else {
      mem[size] = 0;
    }
    return pointer;
  }

  template <typename Purpose>
  static void deallocate(Purpose, thread_info_base* this_thread, void* pointer, std::size_t size)
  {
    if (size <= chunk_size * std::numeric_limits<unsigned char>::max()) {
      if (this_thread && !this_thread->reusable_memory_[Purpose::mem_index]) {
        unsigned char* const mem = static_cast<unsigned char*>(pointer);
        mem[0] = mem[size];
        this_thread->reusable_memory_[Purpose::mem_index] = pointer;
        return;
      }
    }
    ::operator delete(pointer);
  }

 private:
  enum
  {
    chunk_size = 4
  };
  enum
  {
    max_mem_index = 2
  };
  void* reusable_memory_[max_mem_index];
};
}  // namespace boost::asio::detail

#endif