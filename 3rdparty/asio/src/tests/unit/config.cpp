//
// config.cpp
// ~~~~~~~~~~
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
#include "asio/config.hpp"

#include "asio/io_context.hpp"
#include <cstdlib>
#include "unit_test.hpp"

void config_from_string_test()
{
  asio::io_context ctx1(
      asio::config_from_string(
        "scheduler.concurrency_hint=123\n"
        " scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "reactor.registration_locking= 0 # comment\n"
        "reactor.io_locking=1"));

  asio::config cfg1(ctx1);
  ASIO_CHECK(cfg1.get("scheduler", "concurrency_hint", 0) == 123);
  ASIO_CHECK(cfg1.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg1.get("reactor", "registration_locking", true) == false);
  ASIO_CHECK(cfg1.get("reactor", "io_locking", false) == true);

  asio::io_context ctx2(
      asio::config_from_string(
        "prefix.scheduler.concurrency_hint=456\n"
        " prefix.scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "prefix.reactor.registration_locking= 0 # comment\n"
        "prefix.reactor.io_locking=1",
        "prefix"));

  asio::config cfg2(ctx2);
  ASIO_CHECK(cfg2.get("scheduler", "concurrency_hint", 0) == 456);
  ASIO_CHECK(cfg2.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg2.get("reactor", "registration_locking", true) == false);
  ASIO_CHECK(cfg2.get("reactor", "io_locking", false) == true);
}

void config_from_concurrency_hint_test()
{
  asio::io_context ctx0;

  asio::config cfg0(ctx0);
  ASIO_CHECK(cfg0.get("scheduler", "concurrency_hint", 0) == -1);
  ASIO_CHECK(cfg0.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg0.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg0.get("reactor", "io_locking", false) == true);

  asio::io_context ctx1(0);

  asio::config cfg1(ctx1);
  ASIO_CHECK(cfg1.get("scheduler", "concurrency_hint", 0) == 0);
  ASIO_CHECK(cfg1.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg1.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg1.get("reactor", "io_locking", false) == true);

  asio::io_context ctx2(1);

  asio::config cfg2(ctx2);
  ASIO_CHECK(cfg2.get("scheduler", "concurrency_hint", 0) == 1);
  ASIO_CHECK(cfg2.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg2.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg2.get("reactor", "io_locking", false) == true);

  asio::io_context ctx3(42);

  asio::config cfg3(ctx3);
  ASIO_CHECK(cfg3.get("scheduler", "concurrency_hint", 0) == 42);
  ASIO_CHECK(cfg3.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg3.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg3.get("reactor", "io_locking", false) == true);

  asio::io_context ctx4(ASIO_CONCURRENCY_HINT_UNSAFE);

  asio::config cfg4(ctx4);
  ASIO_CHECK(cfg4.get("scheduler", "concurrency_hint", 0) == 1);
  ASIO_CHECK(cfg4.get("scheduler", "locking", false) == false);
  ASIO_CHECK(cfg4.get("reactor", "registration_locking", true) == false);
  ASIO_CHECK(cfg4.get("reactor", "io_locking", false) == false);

  asio::io_context ctx5(ASIO_CONCURRENCY_HINT_UNSAFE_IO);

  asio::config cfg5(ctx5);
  ASIO_CHECK(cfg5.get("scheduler", "concurrency_hint", 0) == 1);
  ASIO_CHECK(cfg5.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg5.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg5.get("reactor", "io_locking", false) == false);

  asio::io_context ctx6(ASIO_CONCURRENCY_HINT_SAFE);

  asio::config cfg6(ctx6);
  ASIO_CHECK(cfg6.get("scheduler", "concurrency_hint", 0) == -1);
  ASIO_CHECK(cfg6.get("scheduler", "locking", false) == true);
  ASIO_CHECK(cfg6.get("reactor", "registration_locking", true) == true);
  ASIO_CHECK(cfg6.get("reactor", "io_locking", false) == true);
}

ASIO_TEST_SUITE
(
  "config",
  ASIO_TEST_CASE(config_from_string_test)
  ASIO_TEST_CASE(config_from_concurrency_hint_test)
)
