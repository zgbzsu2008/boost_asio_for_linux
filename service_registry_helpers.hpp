#ifndef BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HELPERS_HPP
#define BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HELPERS_HPP

#include <iostream>
#include "execution_context.hpp"
#include "io_context.hpp"
#include "service_registry.hpp"

namespace boost::asio {
template <typename Service>
Service& use_service(execution_context& e)
{
  std::cout << "::user_service: " << typeid(Service).name() << '\n';
  return e.service_registry_->template use_service<Service>();
}

template <typename Service, typename... Args>
Service& make_service(execution_context& e, Args&&... args)
{
  std::cout << "::make_service: " << typeid(Service).name() << '\n';
  std::unique_ptr<Service> service(new Service(e, std::forward<Args>(args)...));
  e.service_registry_->template add_service<Service>(service.get());
  Service& result = *service.release();
  return result;
}

template <typename Service>
void add_service(execution_context& e, Service* new_service)
{
  std::cout << "::add_service: " << typeid(Service).name() << '\n';
  e.service_registry_->template add_service<Service>(new_service);
}

template <typename Service>
bool has_service(execution_context& e)
{
  return e.service_registry_->template has_service<Service>();
}

template <typename Service>
Service& use_service(io_context& ioc)
{
  std::cout << "::add_service: " << typeid(Service).name() << '\n';
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0));
  (void)static_cast<const execution_context::id*>(&Service::id);
  return ioc.service_registry_->template use_service<Service>(ioc);
}

template <>
inline detail::io_context_impl& use_service<detail::io_context_impl>(io_context& ioc)
{
  std::cout << "::use_service: " << typeid(detail::io_context_impl).name() << '\n';
  return ioc.impl_;
}
}  // namespace boost::asio
#endif  // !BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HELPERS_HPP
