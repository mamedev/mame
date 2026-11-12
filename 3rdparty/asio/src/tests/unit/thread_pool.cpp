//
// thread_pool.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/thread_pool.hpp"

#include <functional>
#include "asio/dispatch.hpp"
#include "asio/post.hpp"
#include "unit_test.hpp"

using namespace asio;
namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;
    asio::post(*pool, bindns::bind(decrement_to_zero, pool, count));

    // Handler execution cannot nest, so count value should remain unchanged.
    ASIO_CHECK(*count == before_value);
  }
}

void nested_decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    asio::dispatch(*pool,
        bindns::bind(nested_decrement_to_zero, pool, count));

    // Handler execution is nested, so count value should now be zero.
    ASIO_CHECK(*count == 0);
  }
}

void thread_pool_test()
{
  thread_pool pool(1);

  int count1 = 0;
  asio::post(pool, bindns::bind(increment, &count1));

  int count2 = 10;
  asio::post(pool, bindns::bind(decrement_to_zero, &pool, &count2));

  int count3 = 10;
  asio::post(pool, bindns::bind(nested_decrement_to_zero, &pool, &count3));

  pool.wait();

  ASIO_CHECK(count1 == 1);
  ASIO_CHECK(count2 == 0);
  ASIO_CHECK(count3 == 0);
}

class test_service : public asio::execution_context::service
{
public:
#if defined(ASIO_NO_TYPEID)
  static asio::execution_context::id id;
#endif // defined(ASIO_NO_TYPEID)

  typedef test_service key_type;

  test_service(asio::execution_context& ctx)
    : asio::execution_context::service(ctx)
  {
  }

private:
  virtual void shutdown() {}
};

#if defined(ASIO_NO_TYPEID)
asio::execution_context::id test_service::id;
#endif // defined(ASIO_NO_TYPEID)

class test_context_service : public asio::execution_context::service
{
public:
  static asio::execution_context::id id;

  test_context_service(asio::execution_context& c, int value = 0)
    : asio::execution_context::service(c),
      value_(value)
  {
  }

  int get_value() const
  {
    return value_;
  }

private:
  virtual void shutdown() {}

  int value_;
};

asio::execution_context::id test_context_service::id;

class test_context_service_maker :
  public asio::execution_context::service_maker
{
public:
  void make(asio::execution_context& ctx) const override
  {
    (void)asio::make_service<test_context_service>(ctx, 42);
  }
};

void thread_pool_service_test()
{
  asio::thread_pool pool1(1);
  asio::thread_pool pool2(1);
  asio::thread_pool pool3(1);

  // Implicit service registration.

  asio::use_service<test_service>(pool1);

  ASIO_CHECK(asio::has_service<test_service>(pool1));

  test_service* svc1 = new test_service(pool1);
  try
  {
    asio::add_service(pool1, svc1);
    ASIO_ERROR("add_service did not throw");
  }
  catch (asio::service_already_exists&)
  {
  }
  delete svc1;

  // Explicit service registration.

  test_service& svc2 = asio::make_service<test_service>(pool2);

  ASIO_CHECK(asio::has_service<test_service>(pool2));
  ASIO_CHECK(&asio::use_service<test_service>(pool2) == &svc2);

  test_service* svc3 = new test_service(pool2);
  try
  {
    asio::add_service(pool2, svc3);
    ASIO_ERROR("add_service did not throw");
  }
  catch (asio::service_already_exists&)
  {
  }
  delete svc3;

  // Explicit registration with invalid owner.

  test_service* svc4 = new test_service(pool2);
  try
  {
    asio::add_service(pool3, svc4);
    ASIO_ERROR("add_service did not throw");
  }
  catch (asio::invalid_service_owner&)
  {
  }
  delete svc4;

  ASIO_CHECK(!asio::has_service<test_service>(pool3));

  // Initial service registration.

  asio::thread_pool pool4{1, test_context_service_maker{}};

  ASIO_CHECK(asio::has_service<test_context_service>(pool4));
  ASIO_CHECK(asio::use_service<test_context_service>(pool4).get_value()
      == 42);
}

void thread_pool_executor_query_test()
{
  thread_pool pool(1);

  ASIO_CHECK(
      &asio::query(pool.executor(),
        asio::execution::context)
      == &pool);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::blocking)
      == asio::execution::blocking.possibly);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::blocking.possibly)
      == asio::execution::blocking.possibly);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::outstanding_work)
      == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::outstanding_work.untracked)
      == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::relationship)
      == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::relationship.fork)
      == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::mapping)
      == asio::execution::mapping.thread);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::inline_exception_handling)
      == asio::execution::inline_exception_handling.terminate);

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::allocator)
      == std::allocator<void>());

  ASIO_CHECK(
      asio::query(pool.executor(),
        asio::execution::occupancy)
      == 1);
}

void thread_pool_executor_execute_test()
{
  int count = 0;
  thread_pool pool(1);

  pool.executor().execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.never
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.never,
      asio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked,
      asio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  asio::require(pool.executor(),
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked,
      asio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  asio::prefer(
      asio::require(pool.executor(),
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked,
        asio::execution::relationship.continuation),
      asio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  asio::prefer(
      asio::require(pool.executor(),
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked,
        asio::execution::relationship.continuation),
      asio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  pool.wait();

  ASIO_CHECK(count == 10);
}

template <typename T>
class custom_allocator
{
public:
  using value_type = T;

  custom_allocator(int* live_count, int* total_count)
    : live_count_(live_count),
      total_count_(total_count)
  {
  }

  template <typename U>
  custom_allocator(const custom_allocator<U>& other) noexcept
    : live_count_(other.live_count_),
      total_count_(other.total_count_)
  {
  }

  bool operator==(const custom_allocator& other) const noexcept
  {
    return &live_count_ == &other.live_count_ &&
      &total_count_ == &other.total_count_;;
  }

  bool operator!=(const custom_allocator& other) const noexcept
  {
    return &live_count_ != &other.live_count_ ||
      &total_count_ != &other.total_count_;
  }

  T* allocate(std::size_t n) const
  {
    ++(*live_count_);
    ++(*total_count_);
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    --(*live_count_);
    ::operator delete(p);
  }

private:
  template <typename> friend class custom_allocator;

  int* live_count_;
  int* total_count_;
};

void thread_pool_allocator_test()
{
  int live_count;
  int total_count;

#if !defined(ASIO_NO_TS_EXECUTORS)
  {
    live_count = 0;
    total_count = 0;
    thread_pool pool1(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count));
    (void)pool1;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);
#endif // !defined(ASIO_NO_TS_EXECUTORS)

  {
    live_count = 0;
    total_count = 0;
    thread_pool pool2(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count), 1);
    (void)pool2;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);

  {
    live_count = 0;
    total_count = 0;
    thread_pool pool3(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count), 1,
        asio::config_from_string(""));
    (void)pool3;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);
}

ASIO_TEST_SUITE
(
  "thread_pool",
  ASIO_TEST_CASE(thread_pool_test)
  ASIO_TEST_CASE(thread_pool_service_test)
  ASIO_TEST_CASE(thread_pool_executor_query_test)
  ASIO_TEST_CASE(thread_pool_executor_execute_test)
  ASIO_TEST_CASE(thread_pool_allocator_test)
)
