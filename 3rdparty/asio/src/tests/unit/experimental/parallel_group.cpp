//
// experimental/parallel_group.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/experimental/parallel_group.hpp"

#include <array>
#include "asio/bind_cancellation_slot.hpp"
#include "asio/cancellation_signal.hpp"
#include "asio/deferred.hpp"
#include "asio/error.hpp"
#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"
#include "../unit_test.hpp"

struct wait_for_cancel_filter
{
  asio::steady_timer* timer_;
  asio::cancellation_type_t react_to_;

  void operator()(asio::cancellation_type_t type)
  {
    if (!!(type & react_to_))
      timer_->cancel();
  }
};

struct wait_for_cancel_initiation
{
  asio::cancellation_type_t react_to_;

  template <typename Handler>
  void operator()(Handler&& handler, asio::steady_timer* timer) const
  {
    auto slot = asio::get_associated_cancellation_slot(handler);
    timer->expires_after(asio::chrono::hours(1));
    if (slot.is_connected())
      slot.assign(wait_for_cancel_filter{timer, react_to_});

    timer->async_wait(
        asio::bind_cancellation_slot(asio::cancellation_slot(),
          static_cast<Handler&&>(handler)));
  }
};

template <typename CompletionToken = asio::deferred_t>
auto async_wait_for_cancel(asio::steady_timer& timer,
    asio::cancellation_type_t react_to,
    CompletionToken&& token = asio::deferred_t())
  -> decltype(
      asio::async_initiate<CompletionToken, void(asio::error_code)>(
        wait_for_cancel_initiation{react_to}, token, &timer))
{
  return asio::async_initiate<CompletionToken, void(asio::error_code)>(
      wait_for_cancel_initiation{react_to}, token, &timer);
}

void non_terminal_group_cancellation_test()
{
  asio::io_context ioc;
  asio::steady_timer t0(ioc);
  asio::steady_timer t1(ioc);
  asio::cancellation_signal cancel_signal;

  int called = 0;
  std::array<std::size_t, 2> order = {{}};
  asio::error_code ec0 = asio::error::would_block;
  asio::error_code ec1 = asio::error::would_block;

  asio::experimental::make_parallel_group(
      async_wait_for_cancel(t0, asio::cancellation_type::partial),
      async_wait_for_cancel(t1, asio::cancellation_type::terminal)
    ).async_wait(
        asio::experimental::wait_for_one(),
        asio::bind_cancellation_slot(
          cancel_signal.slot(),
          [&](std::array<std::size_t, 2> o,
            asio::error_code e0, asio::error_code e1)
          {
            ++called;
            order = o;
            ec0 = e0;
            ec1 = e1;
          }
        )
      );

  cancel_signal.emit(asio::cancellation_type::partial);

  ioc.run_for(asio::chrono::seconds(5));

  ASIO_CHECK(called == 1);
  ASIO_CHECK(order[0] == 0);
  ASIO_CHECK(order[1] == 1);
  ASIO_CHECK(ec0 == asio::error::operation_aborted);
  ASIO_CHECK(ec1 == asio::error::operation_aborted);
}

ASIO_TEST_SUITE
(
  "experimental/parallel_group",
  ASIO_TEST_CASE(non_terminal_group_cancellation_test)
)
