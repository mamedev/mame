//
// inline_executor.cpp
// ~~~~~~~~~~~~~~~~~~~
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

// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include "asio/inline_executor.hpp"

#include <functional>
#include "asio/any_completion_executor.hpp"
#include "asio/dispatch.hpp"
#include "unit_test.hpp"

using namespace asio;

namespace bindns = std;

void increment(asio::detail::atomic_count* count)
{
  ++(*count);
}

void inline_executor_query_test()
{
  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::blocking)
      == asio::execution::blocking.always);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::blocking.possibly)
      == asio::execution::blocking.always);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::outstanding_work)
      == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::outstanding_work.untracked)
      == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::relationship)
      == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::relationship.fork)
      == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::mapping)
      == asio::execution::mapping.thread);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::inline_exception_handling)
      == asio::execution::inline_exception_handling.propagate);

  ASIO_CHECK(
      asio::query(inline_executor(),
        asio::execution::inline_exception_handling.propagate)
      == asio::execution::inline_exception_handling.propagate);

  ASIO_CHECK(
      asio::query(
        asio::require(inline_executor(),
          asio::execution::inline_exception_handling.terminate),
        asio::execution::inline_exception_handling)
      == asio::execution::inline_exception_handling.terminate);

  ASIO_CHECK(
      asio::query(
        asio::require(inline_executor(),
          asio::execution::inline_exception_handling.terminate),
        asio::execution::inline_exception_handling.propagate)
      == asio::execution::inline_exception_handling.terminate);
}

void inline_executor_execute_test()
{
  asio::detail::atomic_count count(0);

  inline_executor().execute(bindns::bind(increment, &count));

  asio::require(inline_executor(),
      asio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  asio::prefer(inline_executor(),
      asio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  asio::any_completion_executor ex = inline_executor();

  ex.execute(bindns::bind(increment, &count));

  ASIO_CHECK(count == 4);
}

void inline_executor_dispatch_test()
{
  asio::detail::atomic_count count(0);

  asio::dispatch(inline_executor(),
      bindns::bind(increment, &count));

  asio::dispatch(
      asio::require(inline_executor(),
        asio::execution::inline_exception_handling.terminate),
      bindns::bind(increment, &count));

  ASIO_CHECK(count == 2);
}

void throw_exception()
{
  throw 42;
}

void inline_executor_exception_test()
{
#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught = false;

  try
  {
    inline_executor().execute(throw_exception);
  }
  catch (...)
  {
    caught = true;
  }

  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)
}

ASIO_TEST_SUITE
(
  "inline_executor",
  ASIO_TEST_CASE(inline_executor_query_test)
  ASIO_TEST_CASE(inline_executor_execute_test)
  ASIO_TEST_CASE(inline_executor_dispatch_test)
  ASIO_TEST_CASE(inline_executor_exception_test)
)
