#ifndef BOOST_ASIO_EXECUTION_CONTEXT_HPP
#define BOOST_ASIO_EXECUTION_CONTEXT_HPP

#include <iostream>
#include <memory>
#include <system_error>

#include "noncopyable.hpp"

namespace boost::asio
{
class execution_context;
class io_context;

template <typename Service> Service& use_service(io_context& ioc);
template <typename Service> Service& use_service(execution_context& e);
template <typename Service, typename... Args> Service& make_service(execution_context& e, Args&&... args);
template <typename Service> void add_service(execution_context& e, Service* new_service);
template <typename Service> bool has_service(execution_context& e);

namespace detail
{
class service_registry;
}

class execution_context : private detail::noncopyable
{
 public:
  class id;
  class service;

  execution_context();
  virtual ~execution_context();

  void shutdown();
  void destroy();

  template <typename Service> friend Service& use_service(io_context& ioc);
  template <typename Service> friend Service& use_service(execution_context& e);
  template <typename Service, typename... Args> friend Service& make_service(execution_context& e, Args&&... args);
  template <typename Service> friend void add_service(execution_context& e, Service* new_service);
  template <typename Service> friend bool has_service(execution_context& e);

 private:
  boost::asio::detail::service_registry* service_registry_;
};

class execution_context::id : private detail::noncopyable
{
 public:
  id() {}
};

class execution_context::service : private detail::noncopyable
{
 public:
  execution_context& context() { return owner_; }

 protected:
  service(execution_context& owner) : owner_(owner), next_(0) {}
  virtual ~service() {}

 private:
  virtual void shutdown() = 0;
  friend class detail::service_registry;

  struct key {
    key() : type_info_(0), id_(0) {}
    const std::type_info* type_info_;
    const execution_context::id* id_;
  } key_;

  execution_context& owner_;
  service* next_;
};

class service_already_exists : public std::logic_error
{
 public:
  service_already_exists() : std::logic_error("Service already exists.") {}
};

class invalid_service_owner : public std::logic_error
{
 public:
  invalid_service_owner() : std::logic_error("Invalid service owner.") {}
};

namespace detail
{
template <typename Type> class service_id : public execution_context::id
{
 public:
  service_id() {}
};

template <typename Type> class execution_context_service_base : public execution_context::service
{
 public:
  static service_id<Type> id;
  execution_context_service_base(execution_context& e) : execution_context::service(e) {}
};

template <typename Type> service_id<Type> execution_context_service_base<Type>::id;
}  // namespace detail
}  // namespace boost::asio

#endif  // !BOOST_ASIO_EXECUTION_CONTEXT_HPP