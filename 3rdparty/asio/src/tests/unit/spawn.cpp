//
// spawn.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2024 Casey Bodley (cbodley at redhat dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/spawn.hpp"

#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_CONTEXT_FIBER)

#include <memory>
#include <stdexcept>
#include "archetypes/async_ops.hpp"
#include "asio/any_completion_handler.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/deferred.hpp"
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"

void void_returning_coroutine(asio::yield_context)
{
}

int int_returning_coroutine(asio::yield_context)
{
  return 42;
}

void test_spawn_with_any_completion_handler()
{
  asio::io_context ctx;

  bool called = false;
  asio::spawn(ctx, void_returning_coroutine,
      asio::any_completion_handler<void(std::exception_ptr)>(
        [&](std::exception_ptr)
        {
          called = true;
        }));

  ASIO_CHECK(!called);

  ctx.run();

  ASIO_CHECK(called);

  int result = 0;
  asio::spawn(ctx, int_returning_coroutine,
      asio::any_completion_handler<void(std::exception_ptr, int)>(
        [&](std::exception_ptr, int i)
        {
          result = i;
        }));

  ASIO_CHECK(result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(result == 42);
}

void test_spawn_deferred()
{
  asio::io_context ctx;

  {
    bool called = false;
    auto fn = asio::spawn(ctx, void_returning_coroutine, asio::deferred);

    fn([&](std::exception_ptr)
        {
          called = true;
        });

    ASIO_CHECK(!called);

    ctx.poll();

    ASIO_CHECK(ctx.stopped());
    ASIO_CHECK(called);
  }
  {
    int result = 0;
    auto fn = asio::spawn(ctx, int_returning_coroutine, asio::deferred);

    fn([&](std::exception_ptr, int i)
        {
          result = i;
        });

    ASIO_CHECK(result == 0);

    ctx.restart();
    ctx.poll();

    ASIO_CHECK(ctx.stopped());
    ASIO_CHECK(result == 42);
  }
}

void sleeping_coroutine(asio::yield_context yield)
{
  asio::steady_timer timer(yield.get_executor(),
      asio::steady_timer::time_point::max());
  timer.async_wait(yield);
}

void test_spawn_cancel()
{
  asio::cancellation_signal sig;
  asio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  asio::spawn(ctx, sleeping_coroutine,
      asio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e)
        {
          result = e;
          called = true;
        }));

  ctx.poll();
  ASIO_CHECK(!ctx.stopped());

  ASIO_CHECK(!called);
  ASIO_CHECK(result == nullptr);

  sig.emit(asio::cancellation_type::all);

  ctx.poll();
  ASIO_CHECK(ctx.stopped());

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);
  try
  {
    std::rethrow_exception(result);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == asio::error::operation_aborted);
  }
  catch (...)
  {
    ASIO_ERROR("expected system_error");
  }
}

void throwing_coroutine(asio::yield_context)
{
  throw std::runtime_error("oops");
}

void test_spawn_exception()
{
  asio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  asio::spawn(ctx, throwing_coroutine,
      [&](std::exception_ptr e)
      {
        result = e;
        called = true;
      });

  ctx.poll();
  ASIO_CHECK(ctx.stopped());

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);
  try
  {
    std::rethrow_exception(result);
  }
  catch (const std::runtime_error&)
  {
    // ok
  }
  catch (...)
  {
    ASIO_ERROR("expected runtime_error");
  }
}

std::unique_ptr<int> factory_coroutine(asio::yield_context)
{
  return std::unique_ptr<int>(new int(42));
}

void test_spawn_return_move_only()
{
  asio::io_context ctx;

  std::unique_ptr<int> result;
  bool called = false;
  asio::spawn(ctx, factory_coroutine,
      [&](std::exception_ptr, std::unique_ptr<int> r)
      {
        result = std::move(r);
        called = true;
      });

  ctx.poll();
  ASIO_CHECK(ctx.stopped());

  ASIO_CHECK(called);
  ASIO_CHECK(result);
  ASIO_CHECK(*result == 42);
}

int coroutine_using_async_ops_0(asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    async_op_0(yield);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    async_op_ec_0(true, yield);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    async_op_ec_0(false, yield);
    ASIO_CHECK(false);
  }
  catch (asio::system_error& e)
  {
    ASIO_CHECK(e.code() == asio::error::operation_aborted);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    async_op_ex_0(true, yield);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    async_op_ex_0(false, yield);
    ASIO_CHECK(false);
  }
  catch (std::exception& e)
  {
    ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  return 123;
}

int coroutine_using_async_ops_1(asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i = async_op_1(yield);
    ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ec_1(true, yield);
    ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ec_1(false, yield);
    ASIO_CHECK(false);
    (void)i;
  }
  catch (asio::system_error& e)
  {
    ASIO_CHECK(e.code() == asio::error::operation_aborted);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ex_1(true, yield);
    ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ex_1(false, yield);
    ASIO_CHECK(false);
    (void)i;
  }
  catch (std::exception& e)
  {
    ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  return 234;
}

int coroutine_using_async_ops_2(asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_2(yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_ec_2(true, yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double> t = async_op_ec_2(false, yield);
    ASIO_CHECK(false);
    (void)t;
  }
  catch (asio::system_error& e)
  {
    ASIO_CHECK(e.code() == asio::error::operation_aborted);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_ex_2(true, yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double> t = async_op_ex_2(false, yield);
    ASIO_CHECK(false);
    (void)t;
  }
  catch (std::exception& e)
  {
    ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  return 345;
}

int coroutine_using_async_ops_3(asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_3(yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
    ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_ec_3(true, yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
    ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double, char> t = async_op_ec_3(false, yield);
    ASIO_CHECK(false);
    (void)t;
  }
  catch (asio::system_error& e)
  {
    ASIO_CHECK(e.code() == asio::error::operation_aborted);
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_ex_3(true, yield);
    ASIO_CHECK(i == 42);
    ASIO_CHECK(d == 2.0);
    ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double, char> t = async_op_ex_3(false, yield);
    ASIO_CHECK(false);
    (void)t;
  }
  catch (std::exception& e)
  {
    ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    ASIO_CHECK(false);
  }

  return 456;
}

void test_spawn_async_ops()
{
  asio::io_context ctx;

  bool called = false;
  asio::spawn(ctx, coroutine_using_async_ops_0,
      [&](std::exception_ptr, int i)
      {
        ASIO_CHECK(i == 123);
        called = true;
      });

  ASIO_CHECK(!called);

  ctx.run();

  ASIO_CHECK(called);

  called = false;
  asio::spawn(ctx, coroutine_using_async_ops_1,
      [&](std::exception_ptr, int i)
      {
        ASIO_CHECK(i == 234);
        called = true;
      });

  ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(called);

  called = false;
  asio::spawn(ctx, coroutine_using_async_ops_2,
      [&](std::exception_ptr, int i)
      {
        ASIO_CHECK(i == 345);
        called = true;
      });

  ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(called);

  called = false;
  asio::spawn(ctx, coroutine_using_async_ops_3,
      [&](std::exception_ptr, int i)
      {
        ASIO_CHECK(i == 456);
        called = true;
      });

  ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(called);
}

ASIO_TEST_SUITE
(
  "spawn",
  ASIO_TEST_CASE(test_spawn_with_any_completion_handler)
  ASIO_TEST_CASE(test_spawn_deferred)
  ASIO_TEST_CASE(test_spawn_cancel)
  ASIO_TEST_CASE(test_spawn_exception)
  ASIO_TEST_CASE(test_spawn_return_move_only)
  ASIO_TEST_CASE(test_spawn_async_ops)
)

#else // defined(ASIO_HAS_BOOST_CONTEXT_FIBER)

ASIO_TEST_SUITE
(
  "spawn",
  ASIO_TEST_CASE(null_test)
)

#endif // defined(ASIO_HAS_BOOST_CONTEXT_FIBER)
