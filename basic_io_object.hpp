#ifndef BOOST_ASIO_BASIC_IO_OBJECT_HPP
#define BOOST_ASIO_BASIC_IO_OBJECT_HPP

#include "io_context.hpp"
#include "noncopyable.hpp"
#include "service_registry_helpers.hpp"

namespace boost::asio {
namespace detail {
template <typename IoObjectService>
class service_has_move
{
 private:
  typedef IoObjectService service_type;
  typedef typename service_type::implementation_type implementation_type;
  template <typename T, typename U>
  static auto asio_service_has_move_eval(T* t, U* u) -> decltype(t->move_construct(*u, *u), char());
  static char (&asio_service_has_move_eval(...))[2];

 public:
  static const bool value =
      sizeof(asio_service_has_move_eval(static_cast<service_type*>(0), static_cast<implementation_type*>(0))) == 1;
};

}  // namespace detail

template <typename IoObjectService, bool Moveable = detail::service_has_move<IoObjectService>::value>
class basic_io_object : private detail::noncopyable
{
  using service_type = IoObjectService;
  using implementation_type = typename service_type::implementation_type;
  using executor_type = io_context::executor_type;

  executor_type get_executor() { return service_.get_io_context().get_executor(); }

 protected:
  explicit basic_io_object(io_context& ioc) : service_(use_service<IoObjectService>(io_context))
  {
    service_.construct(implementation_);
  }

  basic_io_object(basic_io_object&& other);
  basic_io_object& operator=(basic_io_object&& other);

  template <typename IoObjectService1>
  basic_io_object(IoObjectService1& other_service,
                  typename IoObjectService1::implementation_type& other_implementation);

  ~basic_io_object() { service_.destroy(implementation_); }

  service_type& get_service() { return service_; }
  const service_type& get_service() const { return service_; }
  implementation_type& get_implementation() { return implementation_; }
  const implementation_type& get_implementation() const { return implementation_; }

 private:
  IoObjectService* service_;
  implementation_type implementation_;
};

template <typename IoObjectService, true>
class basic_io_object : private detail::noncopyable
{
  using service_type = IoObjectService;
  using implementation_type = typename service_type::implementation_type;
  using executor_type = io_context::executor_type;

  executor_type get_executor() { return service_.get_io_context().get_executor(); }

 protected:
  explicit basic_io_object(io_context& ioc) : service_(use_service<IoObjectService>(io_context))
  {
    service_.construct(implementation_);
  }

  basic_io_object(basic_io_object&& other) : servcie_(&other.get_service())
  {
    service_.move_construct(implementation_, implementation_);
  }
  basic_io_object& operator=(basic_io_object&& other)
  {
    service_->move_assign(implementation_, *other.service_, other.implementation_);
    service_ = other.service_;
    return *this;
  }

  template <typename IoObjectService1>
  basic_io_object(IoObjectService1& other_service, typename IoObjectService1::implementation_type& other_implementation)
      : service_(use_service<IoObjectService>(other_service.get_io_context()))
  {
    service_->converting_move_construct(implementation_, other_service, other_implementation);
  }

  ~basic_io_object() { service_.destroy(implementation_); }

  service_type& get_service() { return service_; }
  const service_type& get_service() const { return service_; }
  implementation_type& get_implementation() { return implementation_; }
  const implementation_type& get_implementation() const { return implementation_; }

 private:
  IoObjectService* service_;
  implementation_type implementation_;
};
}  // namespace boost::asio
#endif  // !BOOST_ASIO_BASIC_IO_OBJECT_HPP
