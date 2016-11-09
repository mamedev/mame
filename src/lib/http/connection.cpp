// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#include "connection.hpp"
#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http {
namespace server {

connection::connection(asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler)
  : m_socket(std::move(socket)),
    m_connection_manager(manager),
    m_request_handler(handler)
{
}

void connection::start()
{
  do_read();
}

void connection::stop()
{
  m_socket.close();
}

void connection::do_read()
{
  auto self(shared_from_this());
  m_socket.async_read_some(asio::buffer(m_buffer),
      [this, self](std::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
          request_parser::result_type result;
          std::tie(result, std::ignore) = m_request_parser.parse(
              m_request, m_buffer.data(), m_buffer.data() + bytes_transferred);

          if (result == request_parser::good)
          {
            m_request_handler.handle_request(m_request, m_reply);
            do_write();
          }
          else if (result == request_parser::bad)
          {
            m_reply = reply::stock_reply(reply::bad_request);
            do_write();
          }
          else
          {
            do_read();
          }
        }
        else if (ec != asio::error::operation_aborted)
        {
          m_connection_manager.stop(shared_from_this());
        }
      });
}

void connection::do_write()
{
  auto self(shared_from_this());
  asio::async_write(m_socket, m_reply.to_buffers(),
      [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          // Initiate graceful connection closure.
          asio::error_code ignored_ec;
          m_socket.shutdown(asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }

        if (ec != asio::error::operation_aborted)
        {
          m_connection_manager.stop(shared_from_this());
        }
      });
}

} // namespace server
} // namespace http
