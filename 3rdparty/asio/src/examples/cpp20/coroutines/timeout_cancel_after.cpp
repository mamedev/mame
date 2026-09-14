//
// timeout_cancel_after.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>

using namespace asio;
using ip::tcp;
using namespace std::literals::chrono_literals;

awaitable<void> echo(tcp::socket sock)
{
  char data[4196];
  for (;;)
  {
    auto n = co_await sock.async_read_some(buffer(data), cancel_after(10s));
    co_await async_write(sock, buffer(data, n), cancel_after(10s));
  }
}

awaitable<void> listen(tcp::acceptor& acceptor)
{
  for (;;)
  {
    co_spawn(
        acceptor.get_executor(),
        echo(co_await acceptor.async_accept()),
        detached);
  }
}

int main()
{
  io_context ctx;
  tcp::acceptor acceptor(ctx, {tcp::v4(), 54321});
  co_spawn(ctx, listen(acceptor), detached);
  ctx.run();
}
