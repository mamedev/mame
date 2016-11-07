// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "asio.h"
#include <string>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;

  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit server(const std::string& address, const std::string& port,
      const std::string& doc_root);

  /// Run the server's io_context loop.
  void run();

private:
  /// Perform an asynchronous accept operation.
  void do_accept();

  /// Wait for a request to stop the server.
  void do_await_stop();

  /// The io_context used to perform asynchronous operations.
  asio::io_context m_io_context;

  /// The signal_set is used to register for process termination notifications.
  asio::signal_set m_signals;

  /// Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor m_acceptor;

  /// The connection manager which owns all live connections.
  connection_manager m_connection_manager;

  /// The handler for all incoming requests.
  request_handler m_request_handler;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
