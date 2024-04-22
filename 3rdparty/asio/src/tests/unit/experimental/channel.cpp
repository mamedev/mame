//
// experimental/channel.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/experimental/channel.hpp"

#include <utility>
#include "asio/any_completion_handler.hpp"
#include "asio/bind_executor.hpp"
#include "asio/bind_immediate_executor.hpp"
#include "asio/error.hpp"
#include "asio/io_context.hpp"
#include "asio/system_executor.hpp"
#include "../unit_test.hpp"

using namespace asio;
using namespace asio::experimental;

void unbuffered_channel_test()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  ASIO_CHECK(ch1.is_open());
  ASIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(asio::error::eof, "hello");

  ASIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(asio::error::eof, std::move(s1));

  ASIO_CHECK(!b2);
  ASIO_CHECK(!s1.empty());

  asio::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  bool b3 = ch1.try_send(asio::error::eof, std::move(s1));

  ASIO_CHECK(b3);
  ASIO_CHECK(s1.empty());

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");

  bool b4 = ch1.try_receive([](asio::error_code, std::string){});

  ASIO_CHECK(!b4);

  asio::error_code ec2 = asio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(asio::error::eof, std::move(s3),
      [&](asio::error_code ec)
      {
        ec2 = ec;
      });

  asio::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  ASIO_CHECK(b5);
  ASIO_CHECK(ec3 == asio::error::eof);
  ASIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  ASIO_CHECK(!ec2);
}

void buffered_channel_test()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  ASIO_CHECK(ch1.is_open());
  ASIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(asio::error::eof, "hello");

  ASIO_CHECK(b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(asio::error::eof, std::move(s1));

  ASIO_CHECK(!b2);
  ASIO_CHECK(!s1.empty());

  asio::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s2 == "hello");

  bool b4 = ch1.try_receive([](asio::error_code, std::string){});

  ASIO_CHECK(!b4);

  asio::error_code ec2 = asio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(asio::error::eof, std::move(s3),
      [&](asio::error_code ec)
      {
        ec2 = ec;
      });

  asio::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  ASIO_CHECK(b5);
  ASIO_CHECK(ec3 == asio::error::eof);
  ASIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  ASIO_CHECK(!ec2);

  bool b6 = ch1.try_send(asio::error_code(), "goodbye");

  ASIO_CHECK(b6);

  ch1.close();

  asio::error_code ec4;
  std::string s5;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec4 = ec;
        s5 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  ASIO_CHECK(!ec4);
  ASIO_CHECK(s5 == "goodbye");

  asio::error_code ec5;
  std::string s6;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec5 = ec;
        s6 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  ASIO_CHECK(ec5 == asio::experimental::channel_errc::channel_closed);
  ASIO_CHECK(s6.empty());
}

void buffered_error_channel_test()
{
  io_context ctx;

  channel<void(asio::error_code)> ch1(ctx, 1);

  ASIO_CHECK(ch1.is_open());
  ASIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(asio::error::eof);

  ASIO_CHECK(b1);

  bool b2 = ch1.try_send(asio::error::eof);

  ASIO_CHECK(!b2);

  asio::error_code ec1;
  ch1.async_receive(
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);

  bool b4 = ch1.try_receive([](asio::error_code){});

  ASIO_CHECK(!b4);

  asio::error_code ec2 = asio::error::would_block;
  ch1.async_send(asio::error::eof,
      [&](asio::error_code ec)
      {
        ec2 = ec;
      });

  asio::error_code ec3;
  bool b5 = ch1.try_receive(
      [&](asio::error_code ec)
      {
        ec3 = ec;
      });

  ASIO_CHECK(b5);
  ASIO_CHECK(ec3 == asio::error::eof);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(!ec2);
}

void unbuffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);
  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");
}

void unbuffered_immediate_receive()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(system_executor(),
        [&](asio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");

  ctx.run();

  ASIO_CHECK(!ec1);
}

void unbuffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](asio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);
  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx2.run();

  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");
}

void unbuffered_non_immediate_send()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s2),
      [&](asio::error_code ec)
      {
        ec2 = ec;
      });

  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "0123456789");
  ASIO_CHECK(!ec2);
}

void unbuffered_immediate_send()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s2),
      bind_immediate_executor(system_executor(),
        [&](asio::error_code ec)
        {
          ec2 = ec;
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(!ec2);

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "0123456789");
}

void unbuffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s2),
      bind_executor(ctx2,
        [&](asio::error_code ec)
        {
          ec2 = ec;
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "0123456789");
  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx2.run();

  ASIO_CHECK(!ec2);
}

void buffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](asio::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");
}

void buffered_immediate_receive()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(system_executor(),
        [&](asio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");

  ctx.restart();
  ctx.run();
}

void buffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](asio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx2.run();

  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");
}

void buffered_non_immediate_send()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      [&](asio::error_code ec)
      {
        ec1 = ec;
      });

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(!ec1);
}

void buffered_immediate_send()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      bind_immediate_executor(system_executor(),
        [&](asio::error_code ec)
        {
          ec1 = ec;
        }));

  ASIO_CHECK(!ec1);

  ctx.run();
}

void buffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(asio::error_code, std::string)> ch1(ctx, 1);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(asio::error::eof, std::move(s1),
      bind_executor(ctx2,
        [&](asio::error_code ec)
        {
          ec1 = ec;
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx2.run();

  ASIO_CHECK(!ec1);
}

void try_send_via_dispatch()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(asio::system_executor(),
        [&](asio::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.poll();

  ASIO_CHECK(ec1 == asio::error::would_block);

  std::string s2 = "0123456789";
  ch1.try_send_via_dispatch(asio::error::eof, std::move(s2));

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "0123456789");
  ASIO_CHECK(s2.empty());
}

void try_send_n_via_dispatch()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(asio::system_executor(),
        [&](asio::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);

  asio::error_code ec2 = asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(asio::system_executor(),
        [&](asio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);

  ctx.poll();

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(ec2 == asio::error::would_block);

  std::string s3 = "0123456789";
  ch1.try_send_n_via_dispatch(2, asio::error::eof, std::move(s3));

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "0123456789");
  ASIO_CHECK(ec2 == asio::error::eof);
  ASIO_CHECK(s2 == "0123456789");
  ASIO_CHECK(s3.empty());
}

struct multi_signature_handler
{
  std::string* s_;
  asio::error_code* ec_;

  void operator()(std::string s)
  {
    *s_ = s;
  }

  void operator()(asio::error_code ec)
  {
    *ec_ = ec;
  }
};

void implicit_error_signature_channel_test()
{
  io_context ctx;

  channel<void(std::string)> ch1(ctx);

  ASIO_CHECK(ch1.is_open());
  ASIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send("hello");

  ASIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(std::move(s1));

  ASIO_CHECK(!b2);
  ASIO_CHECK(!s1.empty());

  std::string s2;
  asio::error_code ec1 = asio::error::would_block;
  multi_signature_handler h1 = {&s2, &ec1};
  ch1.async_receive(h1);

  bool b3 = ch1.try_send(std::move(s1));

  ASIO_CHECK(b3);
  ASIO_CHECK(s1.empty());

  ctx.run();

  ASIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");
  ASIO_CHECK(ec1 == asio::error::would_block);

  std::string s3;
  asio::error_code ec2;
  multi_signature_handler h2 = {&s3, &ec2};
  bool b4 = ch1.try_receive(h2);

  ASIO_CHECK(!b4);

  std::string s4 = "zyxwvutsrqponmlkjihgfedcba";
  asio::error_code ec3;
  ch1.async_send(std::move(s4),
      [&](asio::error_code ec)
      {
        ec3 = ec;
      });

  std::string s5;
  asio::error_code ec4 = asio::error::would_block;
  multi_signature_handler h3 = {&s5, &ec4};
  bool b5 = ch1.try_receive(h3);

  ASIO_CHECK(b5);
  ASIO_CHECK(ec4 == asio::error::would_block);
  ASIO_CHECK(s5 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  ASIO_CHECK(!ec3);

  std::string s6;
  asio::error_code ec5 = asio::error::would_block;
  multi_signature_handler h4 = {&s6, &ec5};
  ch1.async_receive(h4);

  ch1.close();

  ctx.restart();
  ctx.run();

  ASIO_CHECK(s6.empty());
  ASIO_CHECK(ec5 == asio::experimental::channel_errc::channel_closed);
}

void channel_with_any_completion_handler_test()
{
  io_context ctx;

  channel<void(asio::error_code, std::string)> ch1(ctx);

  asio::error_code ec1 = asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      asio::any_completion_handler<
        void(asio::error_code, std::string)>(
          [&](asio::error_code ec, std::string s)
          {
            ec1 = ec;
            s1 = std::move(s);
          }));

  asio::error_code ec2 = asio::error::would_block;
  std::string s2 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(asio::error::eof, std::move(s2),
      asio::any_completion_handler<void(asio::error_code)>(
        [&](asio::error_code ec)
        {
          ec2 = ec;
        }));

  ASIO_CHECK(ec1 == asio::error::would_block);
  ASIO_CHECK(ec2 == asio::error::would_block);

  ctx.run();

  ASIO_CHECK(ec1 == asio::error::eof);
  ASIO_CHECK(s1 == "zyxwvutsrqponmlkjihgfedcba");
  ASIO_CHECK(!ec2);
}

ASIO_TEST_SUITE
(
  "experimental/channel",
  ASIO_TEST_CASE(unbuffered_channel_test)
  ASIO_TEST_CASE(buffered_channel_test)
  ASIO_TEST_CASE(buffered_error_channel_test)
  ASIO_TEST_CASE(unbuffered_non_immediate_receive)
  ASIO_TEST_CASE(unbuffered_immediate_receive)
  ASIO_TEST_CASE(unbuffered_executor_receive)
  ASIO_TEST_CASE(unbuffered_non_immediate_send)
  ASIO_TEST_CASE(unbuffered_immediate_send)
  ASIO_TEST_CASE(unbuffered_executor_send)
  ASIO_TEST_CASE(buffered_non_immediate_receive)
  ASIO_TEST_CASE(buffered_immediate_receive)
  ASIO_TEST_CASE(buffered_executor_receive)
  ASIO_TEST_CASE(buffered_non_immediate_send)
  ASIO_TEST_CASE(buffered_immediate_send)
  ASIO_TEST_CASE(buffered_executor_send)
  ASIO_TEST_CASE(try_send_via_dispatch)
  ASIO_TEST_CASE(try_send_n_via_dispatch)
  ASIO_TEST_CASE(implicit_error_signature_channel_test)
  ASIO_TEST_CASE(channel_with_any_completion_handler_test)
)
