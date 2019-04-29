#include "execution_context.hpp"
#include "service_registry.hpp"

namespace boost::asio
{
execution_context::execution_context() : service_registry_(new detail::service_registry(*this)) {}

execution_context::~execution_context()
{
  shutdown();
  destroy();
  delete service_registry_;
}

void execution_context::shutdown() { service_registry_->shutdown_services(); }

void execution_context::destroy() { service_registry_->destroy_services(); }

template <typename Service> Service& use_service(execution_context& e)
{
  return e.service_registry_->template use_service<Service>();
}

template <typename Service, typename... Args> Service& make_service(execution_context& e, Args&&... args)
{
  std::unique_ptr<Service> service(new Service(e, std::forward<Args>(args)...));
  e.service_registry_->template add_service<Service>(service.get());
  Service& result = *service.release();
  return result;
}

template <typename Service> void add_service(execution_context& e, Service* new_service)
{
  e.service_registry_->template add_service<Service>(new_service);
}

template <typename Service> bool has_service(execution_context& e)
{
  return e.service_registry_->template has_service<Service>();
}

}  // namespace boost::asio
