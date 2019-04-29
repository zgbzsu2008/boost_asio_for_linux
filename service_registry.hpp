#ifndef BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP
#define BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP

#include <functional>
#include <mutex>
#include <type_traits>
#include <typeinfo>

#include "execution_context.hpp"
#include "noncopyable.hpp"

namespace boost::asio::detail
{
template <typename T> class typeid_wrapper
{
};

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

  template <typename Service> Service& use_service()
  {
    execution_context::service::key key;
    init_key<Service>(key, 0);
    factory_type factory = service_registry::create<Service, execution_context>;
    return *static_cast<Service*>(do_use_service(key, factory, &owner_));
  }

  template <typename Service> Service& use_service(io_context& owner)
  {
    execution_context::service::key key;
    init_key<Service>(key, 0);
    factory_type factory = service_registry::create<Service, io_context>;
    return *static_cast<Service*>(do_use_service(key, factory, &owner));
  }

  template <typename Service> void add_service(Service* new_service)
  {
    execution_context::service::key key;
    init_key<Service>(key, 0);
    return do_add_service(key, new_service);
  }

  template <typename Service> bool has_service() const
  {
    execution_context::service::key key;
    init_key<Service>(key, 0);
    return do_has_service(key);
  }

 private:
  template <typename Service> static void init_key(execution_context::service::key& key, ...)
  {
    init_key_from_id(key, Service::id);
  }

  // Service::key_type 是 Service 基类或者自身
  template <typename Service>
  static void init_key(
      execution_context::service::key& key,
      typename std::enable_if<std::is_base_of<typename Service::key_type, Service>::value>::type*)
  {
    key.type_info_ = &typeid(typeid_wrapper<Service>);
    key.id_ = 0;
  }

  static void init_key_from_id(execution_context::service::key& key,
                               const execution_context::id& id)
  {
    key.type_info_ = 0;
    key.id_ = &id;
  }

  template <typename Service>
  static void init_key_from_id(execution_context::service::key& key,
                               const service_id<Service>& /*id*/)
  {
    key.type_info_ = &typeid(typeid_wrapper<Service>);
    key.id_ = 0;
  }

  static bool keys_match(const execution_context::service::key& key1,
                         const execution_context::service::key& key2)
  {
    if (key1.id_ && key2.id_) {
      if (key1.id_ == key2.id_) return true;  // 比较id_
    }
    if (key1.type_info_ && key2.type_info_) {
      if (*key1.type_info_ == *key2.type_info_) return true;  // 比较type_info_
    }
    return false;
  }

  template <typename Service, typename Owner> static execution_context::service* create(void* owner)
  {
    return new Service(*static_cast<Owner*>(owner));
  }

  static void destroy(execution_context::service* service) { delete service; }

  execution_context::service* do_use_service(const execution_context::service::key& key,
                                             factory_type factory, void* owner)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    execution_context::service* service = first_service_;
    while (service) {
      if (keys_match(service->key_, key)) {
        return service;  // 存在service
      }
      service = service->next_;
    }

    lock.unlock();  // 解锁new service可嵌套
    using T = execution_context::service;
    using F = std::function<void(T*)>;
    std::unique_ptr<T, F> new_service(factory(owner), [&](T* t) { destroy(t); });
    new_service->key_ = key;
    lock.lock();

    service = first_service_;
    while (service) {  // 再次检查, 如果解锁期间已添加service
      if (keys_match(service->key_, key)) {
        return service;
      }
      service = service->next_;
    }

    new_service->next_ = first_service_;
    first_service_ = new_service.release();
    return first_service_;
  }

  void do_add_service(const execution_context::service::key& key,
                      execution_context::service* new_service)
  {
    if (&owner_ != &new_service->context()) {
      throw invalid_service_owner();  // context不同
    }
    std::unique_lock<std::mutex> lock(mutex_);
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
    std::unique_lock<std::mutex> lock(mutex_);
    execution_context::service* service = first_service_;
    while (service) {  // 遍历链表根据key匹配则有service
      if (keys_match(service->key_, key)) {
        return true;
      }
      service = service->next_;
    }
    return false;
  }

  mutable std::mutex mutex_;
  execution_context& owner_;
  execution_context::service* first_service_;
};
}  // namespace boost::asio::detail

#endif  // !BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP
