//
// cancel_at.cpp
// ~~~~~~~~~~~~~
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
#include "asio/cancel_at.hpp"

#include <functional>
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"
#include "unit_test.hpp"

using namespace asio;
namespace bindns = std;
namespace chronons = asio::chrono;

void increment_on_cancel(int* count, const asio::error_code& error)
{
  if (error == asio::error::operation_aborted)
    ++(*count);
}

void cancel_at_function_object_test()
{
  io_context ioc;
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        cancel_at(now + chronons::seconds(10),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        cancel_at(now + chronons::seconds(10),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(10),
        cancel_at(now + chronons::milliseconds(100),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

void cancel_at_timer_function_object_test()
{
  io_context ioc;
  steady_timer cancellation_timer1(ioc);
  steady_timer cancellation_timer2(ioc);
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(10),
        cancel_at(cancellation_timer2, now + chronons::milliseconds(100),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

struct incrementer_token_v1
{
  explicit incrementer_token_v1(int* c) : count(c) {}
  int* count;
};

struct incrementer_handler_v1
{
  explicit incrementer_handler_v1(incrementer_token_v1 t) : count(t.count) {}

  void operator()(asio::error_code error)
  {
    increment_on_cancel(count, error);
  }

  int* count;
};

namespace asio {

template <>
class async_result<incrementer_token_v1, void(asio::error_code)>
{
public:
  typedef incrementer_handler_v1 completion_handler_type;
  typedef void return_type;
  explicit async_result(completion_handler_type&) {}
  return_type get() {}
};

} // namespace asio

void cancel_at_completion_token_v1_test()
{
  io_context ioc;
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        incrementer_token_v1(&count)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        cancel_at(now + chronons::seconds(10),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        cancel_at(now + chronons::seconds(10),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(10),
        cancel_at(now + chronons::milliseconds(100),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

void cancel_at_timer_completion_token_v1_test()
{
  io_context ioc;
  steady_timer cancellation_timer1(ioc);
  steady_timer cancellation_timer2(ioc);
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        incrementer_token_v1(&count)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(10),
        cancel_at(cancellation_timer2, now + chronons::milliseconds(100),
          incrementer_token_v1(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

struct incrementer_token_v2
{
  explicit incrementer_token_v2(int* c) : count(c) {}
  int* count;
};

namespace asio {

template <>
class async_result<incrementer_token_v2, void(asio::error_code)>
{
public:
  template <typename Initiation, typename... Args>
  static void initiate(Initiation initiation,
      incrementer_token_v2 token, Args&&... args)
  {
    initiation(
        bindns::bind(&increment_on_cancel,
          token.count, bindns::placeholders::_1),
        static_cast<Args&&>(args)...);
  }
};

} // namespace asio

void cancel_at_completion_token_v2_test()
{
  io_context ioc;
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        incrementer_token_v2(&count)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        incrementer_token_v2(&count)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(5),
        cancel_at(now + chronons::seconds(10),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::milliseconds(100),
        cancel_at(now + chronons::seconds(10),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(now + chronons::seconds(10),
        cancel_at(now + chronons::milliseconds(100),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

void cancel_at_timer_completion_token_v2_test()
{
  io_context ioc;
  steady_timer cancellation_timer1(ioc);
  steady_timer cancellation_timer2(ioc);
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        incrementer_token_v2(&count)));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        incrementer_token_v2(&count)));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::milliseconds(100));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(5),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::milliseconds(100),
        cancel_at(cancellation_timer2, now + chronons::seconds(10),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 2);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer1, now + chronons::seconds(10),
        cancel_at(cancellation_timer2, now + chronons::milliseconds(100),
          incrementer_token_v2(&count))));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 3);
}

void partial_cancel_at_test()
{
  io_context ioc;
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(cancel_at(now + chronons::seconds(5)))(
      incrementer_token_v2(&count));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(cancel_at(now + chronons::milliseconds(100)))(
      incrementer_token_v2(&count));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);
}

void partial_cancel_at_timer_test()
{
  io_context ioc;
  steady_timer cancellation_timer(ioc);
  int count = 0;

  steady_timer t(ioc, chronons::milliseconds(100));
  auto now = steady_timer::clock_type::now();
  t.async_wait(cancel_at(cancellation_timer, now + chronons::seconds(5)))(
      incrementer_token_v2(&count));

  ioc.run();

  ASIO_CHECK(count == 0);

  t.expires_after(chronons::seconds(5));
  now = steady_timer::clock_type::now();
  t.async_wait(
      cancel_at(cancellation_timer, now + chronons::milliseconds(100)))(
        incrementer_token_v2(&count));

  ioc.restart();
  ioc.run();

  ASIO_CHECK(count == 1);
}

ASIO_TEST_SUITE
(
  "cancel_at",
  ASIO_TEST_CASE(cancel_at_function_object_test)
  ASIO_TEST_CASE(cancel_at_timer_function_object_test)
  ASIO_TEST_CASE(cancel_at_completion_token_v1_test)
  ASIO_TEST_CASE(cancel_at_timer_completion_token_v1_test)
  ASIO_TEST_CASE(cancel_at_completion_token_v2_test)
  ASIO_TEST_CASE(cancel_at_timer_completion_token_v2_test)
  ASIO_TEST_CASE(partial_cancel_at_test)
  ASIO_TEST_CASE(partial_cancel_at_timer_test)
)
