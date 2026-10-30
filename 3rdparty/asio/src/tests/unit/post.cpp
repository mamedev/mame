//
// post.cpp
// ~~~~~~~~
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
#include "asio/post.hpp"

#include "asio/bind_executor.hpp"
#include "asio/io_context.hpp"
#include "unit_test.hpp"

using namespace asio;
namespace bindns = std;
using bindns::placeholders::_1;
using bindns::placeholders::_2;

class move_only_result
{
public:
  explicit move_only_result(int value)
    : value_(value)
  {
  }

  move_only_result(move_only_result&& other)
    : value_(other.value_)
  {
  }

  int value() const
  {
    return value_;
  }

private:
  int value_;
};

static int function_count = 0;

void void_function()
{
  ++function_count;
}

struct void_function_object
{
  void_function_object() = default;
  void_function_object(void_function_object&&) = default;

  void operator()() &&
  {
    ++function_count;
  }
};

void void_handler(int* count)
{
  ++(*count);
}

move_only_result move_only_result_function()
{
  ++function_count;
  return move_only_result(42);
}

struct move_only_result_function_object
{
  move_only_result_function_object() = default;
  move_only_result_function_object(
      move_only_result_function_object&&) = default;

  move_only_result operator()() &&
  {
    ++function_count;
    return move_only_result(42);
  }
};

void move_only_result_handler(
    move_only_result result_in, int* count, int* result_out)
{
  ++(*count);
  *result_out = result_in.value();
}

void post_no_args_test()
{
  io_context ctx(1);

  int handler_count = 0;
  asio::post(asio::deferred)(
      asio::bind_executor(ctx,
        bindns::bind(void_handler, &handler_count)));

  ASIO_CHECK(handler_count == 0);

  ctx.run();

  ASIO_CHECK(handler_count == 1);
}

void post_function_test()
{
  io_context ctx(1);

  function_count = 0;
  int handler_count = 0;
  post(void_function, ctx.get_executor(),
      bindns::bind(void_handler, &handler_count));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);

  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);

  function_count = 0;
  handler_count = 0;
  post(void_function, ctx, bindns::bind(void_handler, &handler_count));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);

  function_count = 0;
  handler_count = 0;
  post(void_function_object(), ctx.get_executor(),
      bindns::bind(void_handler, &handler_count));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);

  function_count = 0;
  handler_count = 0;
  post(void_function_object(), ctx,
      bindns::bind(void_handler, &handler_count));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);

  function_count = 0;
  handler_count = 0;
  int handler_result = 0;
  post(move_only_result_function, ctx.get_executor(),
      bindns::bind(move_only_result_handler, _1,
        &handler_count, &handler_result));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);
  ASIO_CHECK(handler_result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);
  ASIO_CHECK(handler_result == 42);

  function_count = 0;
  handler_count = 0;
  handler_result = 0;
  post(move_only_result_function, ctx,
      bindns::bind(move_only_result_handler, _1,
        &handler_count, &handler_result));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);
  ASIO_CHECK(handler_result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);
  ASIO_CHECK(handler_result == 42);

  function_count = 0;
  handler_count = 0;
  handler_result = 0;
  post(move_only_result_function_object(), ctx.get_executor(),
      bindns::bind(move_only_result_handler, _1,
        &handler_count, &handler_result));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);
  ASIO_CHECK(handler_result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);
  ASIO_CHECK(handler_result == 42);

  function_count = 0;
  handler_count = 0;
  handler_result = 0;
  post(move_only_result_function_object(), ctx,
      bindns::bind(move_only_result_handler, _1,
        &handler_count, &handler_result));

  ASIO_CHECK(function_count == 0);
  ASIO_CHECK(handler_count == 0);
  ASIO_CHECK(handler_result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(function_count == 1);
  ASIO_CHECK(handler_count == 1);
  ASIO_CHECK(handler_result == 42);
}

ASIO_TEST_SUITE
(
  "post",
  ASIO_TEST_CASE(post_no_args_test)
  ASIO_TEST_CASE(post_function_test)
)
