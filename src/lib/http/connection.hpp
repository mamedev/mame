// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <array>
#include <memory>
#include "asio.h"
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http {
namespace server {

class connection_manager;

/// Represents a single connection from a client.
class connection
  : public std::enable_shared_from_this<connection>
{
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;

  /// Construct a connection with the given socket.
  explicit connection(asio::ip::tcp::socket socket,
      connection_manager& manager, request_handler& handler);

  /// Start the first asynchronous operation for the connection.
  void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();

private:
  /// Perform an asynchronous read operation.
  void do_read();

  /// Perform an asynchronous write operation.
  void do_write();

  /// Socket for the connection.
  asio::ip::tcp::socket m_socket;

  /// The manager for this connection.
  connection_manager& m_connection_manager;

  /// The handler used to process the incoming request.
  request_handler& m_request_handler;

  /// Buffer for incoming data.
  std::array<char, 8192> m_buffer;

  /// The incoming request.
  request m_request;

  /// The parser for the incoming request.
  request_parser m_request_parser;

  /// The reply to be sent back to the client.
  reply m_reply;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
