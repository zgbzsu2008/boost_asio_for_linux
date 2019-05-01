#ifndef BOOST_ASIO_DETAIL_SERVICE_REGISTRY2_HPP
#define BOOST_ASIO_DETAIL_SERVICE_REGISTRY2_HPP

#include <functional>
#include <type_traits>
#include <typeinfo>
#include "execution_context.hpp"
#include "has_type_member.hpp"
#include "mutex.hpp"
#include "noncopyable.hpp"

namespace boost::asio::detail {
HAS_TYPE_MEMBER(key_type);
HAS_TYPE_MEMBER(id);

template <typename T>
class typeid_wrapper
{};

class service_registry : private noncopyable
{
 public:
  using factory_type = std::function<execution_context::service*(void*)>;

  service_registry(execution_context& owner) : owner_(owner), first_service_(0) {}

  void shutdown_services()
  {
    execution_context::service* service = first_service_;
    while (service) {
      service->shutdown();
      service = service->next_;
    }
  }

  void destroy_services()
  {
    while (first_service_) {
      execution_context::service* next_service = first_service_->next_;
      destroy(first_service_);
      first_service_ = next_service;
    }
  }

  template <typename Service>
  Service& use_service()
  {
    execution_context::service::key key;
    init_key<Service>(key);
    factory_type factory = service_registry::create<Service, execution_context>;
    return *static_cast<Service*>(do_use_service(key, factory, &owner_));
  }

  template <typename Service>
  Service& use_service(io_context& owner)
  {
    execution_context::service::key key;
    init_key<Service>(key);
    factory_type factory = service_registry::create<Service, io_context>;
    return *static_cast<Service*>(do_use_service(key, factory, &owner));
  }

  template <typename Service>
  void add_service(Service* new_service)
  {
    execution_context::service::key key;
    init_key<Service>(key);
    return do_add_service(key, new_service);
  }

  template <typename Service>
  bool has_service() const
  {
    execution_context::service::key key;
    init_key<Service>(key);
    return do_has_service(key);
  }

 private:
  template <typename Service>
  static void init_key(execution_context::service::key& key)
  {
    if constexpr (has_type_key_type<Service>::value) {
      static_assert(std::is_convertible<typename Service::key_type&, Service&>::value);
      key.type_info_ = &typeid(typeid_wrapper<Service>);
      key.id_ = 0;
    } else if constexpr (has_type_id<Service>::value) {
      if constexpr (std::is_same<typename Service::id&, execution_context::id&>::value) {
        key.type_info_ = 0;
        key.id_ = &execution_context::id{};
      } else if constexpr (std::is_same<typename Service::id&, service_id<Service>&>::value) {
        key.type_info_ = &typeid(typeid_wrapper<Service>);
        key.id_ = 0;
      } else {
        static_assert(false, "service registery::init_key service id not executor_context::id or service_id<service>");
      }
    } else {
      static_assert(false, "service registery::init_key service has not type id");
    }
  }

  static bool keys_match(const execution_context::service::key& key1, const execution_context::service::key& key2)
  {
    if (key1.id_ && key2.id_) {
      if (key1.id_ == key2.id_) return true;  // 比较id_
    }
    if (key1.type_info_ && key2.type_info_) {
      if (*key1.type_info_ == *key2.type_info_) return true;  // 比较type_info_
    }
    return false;
  }

  template <typename Service, typename Owner>
  static execution_context::service* create(void* owner)
  {
    return new Service(*static_cast<Owner*>(owner));
  }

  static void destroy(execution_context::service* service) { delete service; }

  execution_context::service* do_use_service(const execution_context::service::key& key, factory_type factory,
                                             void* owner)
  {
    detail::mutex::scoped_lock lock(mutex_);
    execution_context::service* service = first_service_;
    while (service) {
      if (keys_match(service->key_, key)) {
        return service;  // 存在service
      }
      service = service->next_;
    }

    lock.unlock();  // 解锁 do_use_service 可嵌套 new service
    using T = execution_context::service;
    using F = std::function<void(T*)>;
    std::unique_ptr<T, F> new_service(factory(owner), [&](T* t) { destroy(t); });  // 自动回收
    new_service->key_ = key;
    lock.lock();

    service = first_service_;
    while (service) {  // 解锁期间已添加service
      if (keys_match(service->key_, key)) {
        return service;
      }
      service = service->next_;
    }

    new_service->next_ = first_service_;
    first_service_ = new_service.release();  // 取消回收
    return first_service_;
  }

  void do_add_service(const execution_context::service::key& key, execution_context::service* new_service)
  {
    if (&owner_ != &new_service->context()) {
      throw invalid_service_owner();  // context不同
    }

    detail::mutex::scoped_lock lock(mutex_);
    execution_context::service* service = first_service_;
    while (service) {
      if (keys_match(service->key_, key)) {
        throw service_already_exists();  // service存在
      }
      service = service->next_;
    }
    new_service->key_ = key;
    new_service->next_ = first_service_;
    first_service_ = new_service;
  }

  bool do_has_service(const execution_context::service::key& key) const
  {
    detail::mutex::scoped_lock lock(mutex_);
    execution_context::service* service = first_service_;
    while (service) {  // 遍历链表根据key匹配判断service是否存在
      if (keys_match(service->key_, key)) {
        return true;
      }
      service = service->next_;
    }
    return false;
  }

  mutable detail::mutex mutex_;
  execution_context& owner_;
  execution_context::service* first_service_;
};
}  // namespace boost::asio::detail
#endif  // !BOOST_ASIO_DETAIL_SERVICE_REGISTRY2_HPP
