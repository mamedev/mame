//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <signal.h>
#include <utility>
#include "connection.hpp"

namespace http {
namespace server2 {

server::server(const std::string& address, const std::string& port,
    const std::string& doc_root, std::size_t io_context_pool_size)
  : io_context_pool_(io_context_pool_size),
    signals_(io_context_pool_.get_io_context()),
    acceptor_(io_context_pool_.get_io_context()),
    request_handler_(doc_root)
{
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

  do_await_stop();

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(acceptor_.get_executor());
  asio::ip::tcp::endpoint endpoint =
    *resolver.resolve(address, port).begin();
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  do_accept();
}

void server::run()
{
  io_context_pool_.run();
}

void server::do_accept()
{
  acceptor_.async_accept(io_context_pool_.get_io_context(),
      [this](std::error_code ec, asio::ip::tcp::socket socket)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
        {
          return;
        }

        if (!ec)
        {
          std::make_shared<connection>(
              std::move(socket), request_handler_)->start();
        }

        do_accept();
      });
}

void server::do_await_stop()
{
  signals_.async_wait(
      [this](std::error_code /*ec*/, int /*signo*/)
      {
        io_context_pool_.stop();
      });
}

} // namespace server2
} // namespace http
