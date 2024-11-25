//
// co_spawn.cpp
// ~~~~~~~~~~~~
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
#include "asio/co_spawn.hpp"

#include "unit_test.hpp"

#if defined(ASIO_HAS_CO_AWAIT)

#include <stdexcept>
#include "asio/any_completion_handler.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/io_context.hpp"

asio::awaitable<void> void_returning_coroutine()
{
  co_return;
}

asio::awaitable<int> int_returning_coroutine()
{
  co_return 42;
}

void test_co_spawn_with_any_completion_handler()
{
  asio::io_context ctx;

  bool called = false;
  asio::co_spawn(ctx, void_returning_coroutine(),
      asio::any_completion_handler<void(std::exception_ptr)>(
        [&](std::exception_ptr)
        {
          called = true;
        }));

  ASIO_CHECK(!called);

  ctx.run();

  ASIO_CHECK(called);

  int result = 0;
  asio::co_spawn(ctx, int_returning_coroutine(),
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

void test_co_spawn_immediate_cancel()
{
  asio::cancellation_signal sig;
  asio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  asio::co_spawn(ctx, void_returning_coroutine(),
      asio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e)
        {
          result = e;
          called = true;
        }));

  ASIO_CHECK(!called);
  ASIO_CHECK(result == nullptr);

  sig.emit(asio::cancellation_type::all);
  ctx.run();

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);

  result = nullptr;
  called = false;
  asio::co_spawn(ctx, int_returning_coroutine(),
      asio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e, int i)
        {
          ASIO_CHECK(i != 42);
          result = e;
          called = true;
        }));

  ASIO_CHECK(!called);
  ASIO_CHECK(result == nullptr);

  sig.emit(asio::cancellation_type::all);
  ctx.restart();
  ctx.run();

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);
}

ASIO_TEST_SUITE
(
  "co_spawn",
  ASIO_TEST_CASE(test_co_spawn_with_any_completion_handler)
  ASIO_TEST_CASE(test_co_spawn_immediate_cancel)
)

#else // defined(ASIO_HAS_CO_AWAIT)

ASIO_TEST_SUITE
(
  "co_spawn",
  ASIO_TEST_CASE(null_test)
)

#endif // defined(ASIO_HAS_CO_AWAIT)
