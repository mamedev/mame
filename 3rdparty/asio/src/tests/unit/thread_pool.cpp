//
// thread_pool.cpp
// ~~~~~~~~~~~~~~~
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

ASIO_TEST_SUITE
(
  "thread_pool",
  ASIO_TEST_CASE(thread_pool_test)
  ASIO_TEST_CASE(thread_pool_service_test)
  ASIO_TEST_CASE(thread_pool_executor_query_test)
  ASIO_TEST_CASE(thread_pool_executor_execute_test)
)
