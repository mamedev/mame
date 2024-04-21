//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER4_SERVER_HPP
#define HTTP_SERVER4_SERVER_HPP

#include <asio.hpp>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include "request_parser.hpp"

namespace http {
namespace server4 {

struct request;
struct reply;

/// The top-level coroutine of the HTTP server.
class server : asio::coroutine
{
public:
  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit server(asio::io_context& io_context,
      const std::string& address, const std::string& port,
      std::function<void(const request&, reply&)> request_handler);

  /// Perform work associated with the server.
  void operator()(
      std::error_code ec = std::error_code(),
      std::size_t length = 0);

private:
  typedef asio::ip::tcp tcp;

  /// The user-supplied handler for all incoming requests.
  std::function<void(const request&, reply&)> request_handler_;

  /// Acceptor used to listen for incoming connections.
  std::shared_ptr<tcp::acceptor> acceptor_;

  /// The current connection from a client.
  std::shared_ptr<tcp::socket> socket_;

  /// Buffer for incoming data.
  std::shared_ptr<std::array<char, 8192>> buffer_;

  /// The incoming request.
  std::shared_ptr<request> request_;

  /// Whether the request is valid or not.
  request_parser::result_type parse_result_;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  std::shared_ptr<reply> reply_;
};

} // namespace server4
} // namespace http

#endif // HTTP_SERVER4_SERVER_HPP
