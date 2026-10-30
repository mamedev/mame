//
// inline_or_executor.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/inline_or_executor.hpp"

#include <functional>
#include <sstream>
#include "asio/executor.hpp"
#include "asio/io_context.hpp"
#include "asio/dispatch.hpp"
#include "asio/post.hpp"
#include "unit_test.hpp"

using namespace asio;
namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void inline_or_executor_conversion_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);

  // Converting constructors.

  inline_or_executor<executor> s2(s1);
  inline_or_executor<executor> s3 =
    inline_or_executor<io_context::executor_type>(s1);

  // Converting assignment.

  s3 = s1;
  s3 = inline_or_executor<io_context::executor_type>(s1);
}

void inline_or_executor_query_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);

  ASIO_CHECK(
      &asio::query(s1, asio::execution::context)
        == &ioc);

  ASIO_CHECK(
      asio::query(s1, asio::execution::blocking)
        == asio::execution::blocking.always);

  ASIO_CHECK(
      asio::query(s1, asio::execution::blocking.always)
        == asio::execution::blocking.always);

  ASIO_CHECK(
      asio::query(s1, asio::execution::outstanding_work)
        == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(s1, asio::execution::outstanding_work.untracked)
        == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(s1, asio::execution::relationship)
        == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(s1, asio::execution::relationship.fork)
        == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(s1, asio::execution::mapping)
        == asio::execution::mapping.thread);

  ASIO_CHECK(
      asio::query(s1, asio::execution::allocator)
        == std::allocator<void>());
}

void inline_or_executor_execute_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);
  int count = 0;

  s1.execute(bindns::bind(increment, &count));

  // Handler is run inline.
  ASIO_CHECK(count == 1);

  count = 0;
  asio::require(s1, asio::execution::blocking.possibly).execute(
      bindns::bind(increment, &count));

  // Handler is run inline.
  ASIO_CHECK(count == 1);

  count = 0;
  asio::require(s1, asio::execution::blocking.never).execute(
      bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  ASIO_CHECK(!ioc.stopped());

  asio::require(s1,
      asio::execution::blocking.never,
      asio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  asio::require(s1,
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  asio::require(s1,
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked,
      asio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  asio::require(s1,
      asio::execution::blocking.never,
      asio::execution::outstanding_work.untracked,
      asio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  asio::prefer(
      asio::require(s1,
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked,
        asio::execution::relationship.continuation),
      asio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  asio::prefer(
      asio::require(s1,
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked,
        asio::execution::relationship.continuation),
      asio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);
}

ASIO_TEST_SUITE
(
  "inline_or_executor",
  ASIO_COMPILE_TEST_CASE(inline_or_executor_conversion_test)
  ASIO_TEST_CASE(inline_or_executor_query_test)
  ASIO_TEST_CASE(inline_or_executor_execute_test)
)
