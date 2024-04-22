//
// posix_chat_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "asio.hpp"
#include "chat_message.hpp"

#if defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)

using asio::ip::tcp;
namespace posix = asio::posix;

class posix_chat_client
{
public:
  posix_chat_client(asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : socket_(io_context),
      input_(io_context, ::dup(STDIN_FILENO)),
      output_(io_context, ::dup(STDOUT_FILENO)),
      input_buffer_(chat_message::max_body_length)
  {
    do_connect(endpoints);
  }

private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, tcp::endpoint)
        {
          if (!ec)
          {
            do_read_header();
            do_read_input();
          }
        });
  }

  void do_read_header()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            close();
          }
        });
  }

  void do_read_body()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_write_output();
          }
          else
          {
            close();
          }
        });
  }

  void do_write_output()
  {
    // Write out the message we just received, terminated by a newline.
    static char eol[] = { '\n' };
    std::array<asio::const_buffer, 2> buffers = {{
      asio::buffer(read_msg_.body(), read_msg_.body_length()),
      asio::buffer(eol) }};
    asio::async_write(output_, buffers,
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read_header();
          }
          else
          {
            close();
          }
        });
  }

  void do_read_input()
  {
    // Read a line of input entered by the user.
    asio::async_read_until(input_, input_buffer_, '\n',
        [this](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            // Write the message (minus the newline) to the server.
            write_msg_.body_length(length - 1);
            input_buffer_.sgetn(write_msg_.body(), length - 1);
            input_buffer_.consume(1); // Remove newline from input.
            write_msg_.encode_header();
            do_write_message();
          }
          else if (ec == asio::error::not_found)
          {
            // Didn't get a newline. Send whatever we have.
            write_msg_.body_length(input_buffer_.size());
            input_buffer_.sgetn(write_msg_.body(), input_buffer_.size());
            write_msg_.encode_header();
            do_write_message();
          }
          else
          {
            close();
          }
        });
  }

  void do_write_message()
  {
    asio::async_write(socket_,
        asio::buffer(write_msg_.data(), write_msg_.length()),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read_input();
          }
          else
          {
            close();
          }
        });
  }

  void close()
  {
    // Cancel all outstanding asynchronous operations.
    socket_.close();
    input_.close();
    output_.close();
  }

private:
  tcp::socket socket_;
  posix::stream_descriptor input_;
  posix::stream_descriptor output_;
  chat_message read_msg_;
  chat_message write_msg_;
  asio::streambuf input_buffer_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: posix_chat_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);

    posix_chat_client c(io_context, endpoints);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
int main() {}
#endif // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
