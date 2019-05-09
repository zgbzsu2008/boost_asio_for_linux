#ifndef BOOST_ASIO_BASIC_IO_OBJECT_HPP
#define BOOST_ASIO_BASIC_IO_OBJECT_HPP

#include "config.hpp"
#include "io_context.hpp"
#include "service_registry_helpers.hpp"

namespace boost::asio {

#if defined(BOOST_ASIO_HAS_MOVE)
namespace detail {

template <typename IoObjectService>
class service_has_move
{
 private:
  using service_type = IoObjectService;
  using impl_type = typename service_type::impl_type;

  template <typename T, typename U>
  static auto asio_service_has_move_eval(T* t, U* u) -> decltype(t->move_construct(*u, *u), char());
  static char (&asio_service_has_move_eval(...))[2];

 public:
  static const bool value =
      sizeof(asio_service_has_move_eval(static_cast<service_type*>(0), static_cast<impl_type*>(0))) == 1;
};
}  // namespace detail
#endif  // !BOOST_ASIO_HAS_MOVE

// basic_io_object 成员 service_type, impl_type
// service_type 服务处理数据 impl_type
#if !defined(BOOST_ASIO_HAS_MOVE)
template <typename IoObjectService>
#else
template <typename IoObjectService, bool Movable = detail::service_has_move<IoObjectService>::value>
#endif
class basic_io_object
{
 public:
  using service_type = IoObjectService;
  using impl_type = typename service_type::impl_type;
  using executor_type = io_context::executor_type;

  executor_type get_executor() { return service_.get_io_context().get_executor(); }

 protected:
  explicit basic_io_object(io_context& ioc) : service_(use_service<IoObjectService>(ioc)) { service_.construct(impl_); }

  ~basic_io_object() { service_.destroy(impl_); }

  service_type& get_service() { return service_; }
  const service_type& get_service() const { return service_; }
  impl_type& get_impl() { return impl_; }
  const impl_type& get_impl() const { return impl_; }

 private:
  basic_io_object(const basic_io_object&) = delete;
  basic_io_object& operator=(const basic_io_object&) = delete;

  service_type& service_;
  impl_type impl_;
};

#if defined(BOOST_ASIO_HAS_MOVE)
template <typename IoObjectService>
class basic_io_object<IoObjectService, true>
{
 public:
  using service_type = IoObjectService;
  using impl_type = typename service_type::impl_type;
  using executor_type = io_context::executor_type;

  executor_type get_executor() { return service_.get_io_context().get_executor(); }

 protected:
  explicit basic_io_object(io_context& ioc) : service_(use_service<IoObjectService>(ioc)) { service_.construct(impl_); }

  basic_io_object(basic_io_object&& other) : service_(&other.get_service())
  {
    service_.move_construct(impl_, other.impl_);
  }

  basic_io_object& operator=(basic_io_object&& other)
  {
    service_.move_assign(impl_, other.service_, other.impl_);
    service_ = other.service_;
    return *this;
  }

  template <typename IoObjectService1>
  basic_io_object(IoObjectService1& other_service, typename IoObjectService1::impl_type& other_impl)
      : service_(use_service<IoObjectService>(other_service.get_io_context()))
  {
    service_->converting_move_construct(impl_, other_service, other_impl);
  }

  ~basic_io_object() { service_.destroy(impl_); }

  service_type& get_service() { return service_; }
  const service_type& get_service() const { return service_; }
  impl_type& get_impl() { return impl_; }
  const impl_type& get_impl() const { return impl_; }

 private:
  basic_io_object(const basic_io_object&) = delete;
  basic_io_object& operator=(const basic_io_object&) = delete;

  service_type& service_;
  impl_type impl_;
};
#endif  // defined(BOOST_ASIO_HAS_MOVE)
}  // namespace boost::asio
#endif  // !BOOST_ASIO_BASIC_IO_OBJECT_HPP