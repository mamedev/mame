//
// any_completion_handler.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/any_completion_handler.hpp"

#include "unit_test.hpp"

#include <functional>
#include "asio/bind_allocator.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/bind_executor.hpp"
#include "asio/bind_immediate_executor.hpp"
#include "asio/error.hpp"
#include "asio/thread_pool.hpp"

namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void any_completion_handler_construction_test()
{
  int count = 0;
  asio::nullptr_t null_ptr = asio::nullptr_t();

  asio::any_completion_handler<void()> h1;

  ASIO_CHECK(!h1);
  ASIO_CHECK(h1 == null_ptr);

  asio::any_completion_handler<void()> h2(null_ptr);

  ASIO_CHECK(!h2);
  ASIO_CHECK(h2 == null_ptr);

  asio::any_completion_handler<void()> h3(
      bindns::bind(&increment, &count));

  ASIO_CHECK(!!h3);
  ASIO_CHECK(h3 != null_ptr);

  asio::any_completion_handler<void()> h4(std::move(h1));

  ASIO_CHECK(!h4);
  ASIO_CHECK(h4 == null_ptr);
  ASIO_CHECK(!h1);
  ASIO_CHECK(h1 == null_ptr);

  asio::any_completion_handler<void()> h5(std::move(h3));

  ASIO_CHECK(!!h5);
  ASIO_CHECK(h5 != null_ptr);
  ASIO_CHECK(!h3);
  ASIO_CHECK(h3 == null_ptr);

  asio::any_completion_handler<void()> h6(std::move(h5));

  ASIO_CHECK(!!h6);
  ASIO_CHECK(h6 != null_ptr);
  ASIO_CHECK(!h5);
  ASIO_CHECK(h5 == null_ptr);
}

void any_completion_handler_assignment_test()
{
  int count = 0;
  asio::nullptr_t null_ptr = asio::nullptr_t();

  asio::any_completion_handler<void()> h1;

  asio::any_completion_handler<void()> h2;
  h2 = null_ptr;

  ASIO_CHECK(!h2);

  asio::any_completion_handler<void()> h3;
  h3 = bindns::bind(&increment, &count);

  ASIO_CHECK(!!h3);

  asio::any_completion_handler<void()> h4;
  h4 = std::move(h1);

  ASIO_CHECK(!h4);
  ASIO_CHECK(!h1);

  h4 = std::move(h3);

  ASIO_CHECK(!!h4);
  ASIO_CHECK(!h3);
}

template <typename T>
class handler_allocator
{
public:
  using value_type = T;

  explicit handler_allocator(int* count)
    : count_(count)
  {
  }

  template <typename U>
  handler_allocator(const handler_allocator<U>& other) noexcept
    : count_(other.count_)
  {
  }

  bool operator==(const handler_allocator& other) const noexcept
  {
    return &count_ == &other.count_;
  }

  bool operator!=(const handler_allocator& other) const noexcept
  {
    return &count_ != &other.count_;
  }

  T* allocate(std::size_t n) const
  {
    ++(*count_);
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    ::operator delete(p);
  }

private:
  template <typename> friend class handler_allocator;

  int* count_;
};

class cancel_handler
{
public:
  explicit cancel_handler(int* count)
    : count_(count)
  {
  }

  void operator()(asio::cancellation_type_t)
  {
    ++(*count_);
  }

private:
  int* count_;
};

void any_completion_handler_associator_test()
{
  typedef asio::any_completion_handler<void()> handler_type;

  int count = 0;
  int alloc_count = 0;
  int cancel_count = 0;
  asio::thread_pool pool(1);
  asio::cancellation_signal sig;

  asio::any_completion_handler<void()> h1(
      asio::bind_allocator(handler_allocator<char>(&alloc_count),
        asio::bind_cancellation_slot(sig.slot(),
          asio::bind_executor(pool.get_executor(),
            asio::bind_immediate_executor(asio::system_executor(),
              bindns::bind(&increment, &count))))));

  ASIO_CHECK(alloc_count == 1);

  ASIO_REBIND_ALLOC(asio::associated_allocator<handler_type>::type,
      char) alloc1(asio::get_associated_allocator(h1));
  alloc1.deallocate(alloc1.allocate(1), 1);

  ASIO_CHECK(alloc_count == 2);

  asio::associated_cancellation_slot<handler_type>::type slot1
    = asio::get_associated_cancellation_slot(h1);

  ASIO_CHECK(slot1.is_connected());

  slot1.emplace<cancel_handler>(&cancel_count);

  ASIO_CHECK(cancel_count == 0);

  sig.emit(asio::cancellation_type::terminal);

  ASIO_CHECK(cancel_count == 1);

  asio::associated_executor<handler_type>::type ex1
    = asio::get_associated_executor(h1);

  ASIO_CHECK(ex1 == pool.get_executor());

  asio::associated_immediate_executor<
    handler_type, asio::thread_pool::executor_type>::type ex2
      = asio::get_associated_immediate_executor(h1, pool.get_executor());

  ASIO_CHECK(ex2 == asio::system_executor());
}

void increment_with_error(asio::error_code ec,
    asio::error_code* out_ec, int* count)
{
  *out_ec = ec;
  ++(*count);
}

void any_completion_handler_invocation_test()
{
  int count = 0;
  asio::error_code ec;

  asio::any_completion_handler<void()> h1(
      bindns::bind(&increment, &count));

  ASIO_CHECK(count == 0);

  std::move(h1)();

  ASIO_CHECK(count == 1);

  asio::any_completion_handler<void(asio::error_code)> h2(
      bindns::bind(&increment_with_error,
        bindns::placeholders::_1, &ec, &count));

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 1);

  std::move(h2)(asio::error::eof);

  ASIO_CHECK(ec == asio::error::eof);
  ASIO_CHECK(count == 2);
}

ASIO_TEST_SUITE
(
  "any_completion_handler",
  ASIO_TEST_CASE(any_completion_handler_construction_test)
  ASIO_TEST_CASE(any_completion_handler_assignment_test)
  ASIO_TEST_CASE(any_completion_handler_associator_test)
  ASIO_TEST_CASE(any_completion_handler_invocation_test)
)
