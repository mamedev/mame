//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <array>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include "asio.hpp"

using asio::ip::tcp;

// Class to manage the memory to be used for allocating objects that are
// associated with an execution context (such as services and internal state of
// I/O objects). It contains a single block of memory from which objects are
// monotonically allocated (similar to std::pmr::monotonic_resource). If no
// more space is available it delegates allocation to the global heap.
class context_memory
{
public:
  explicit context_memory(std::size_t preallocated)
    : preallocated_(preallocated),
      next_allocation_(0),
      storage_(new unsigned char[preallocated_])
  {
  }

  ~context_memory()
  {
    delete[] storage_;
  }

  context_memory(const context_memory&) = delete;
  context_memory& operator=(const context_memory&) = delete;

  void* allocate(std::size_t size, std::size_t align)
  {
    // Since this program is single-threaded there is no need to perform any
    // synchronisation when modifying next_allocation_. Use an atomic or other
    // form of synchronisation when using an exeution context from multiple
    // threads.
    std::size_t space = size + align;
    if (next_allocation_ + space < preallocated_)
    {
      void* ptr = storage_ + next_allocation_;
      next_allocation_ += space;
      return std::align(align, size, ptr, space);
    }
    else
    {
      return ::operator new(size);
    }
  }

  void deallocate(void* ptr)
  {
    auto* ucptr = static_cast<unsigned char*>(ptr);
    if (std::less_equal<unsigned char*>{}(storage_, ucptr)
        && std::less<unsigned char*>{}(ucptr, storage_ + preallocated_))
    {
      // Nothing to do.
    }
    else
    {
      ::operator delete(ptr);
    }
  }

private:
  std::size_t preallocated_;
  std::size_t next_allocation_;
  unsigned char* storage_;
};

// The allocator to be associated with the execution context. This allocatoro
// only needs to satisfy the C++11 minimal allocator requirements.
template <typename T>
class context_allocator
{
public:
  using value_type = T;

  explicit context_allocator(context_memory& mem)
    : memory_(mem)
  {
  }

  template <typename U>
  context_allocator(const context_allocator<U>& other) noexcept
    : memory_(other.memory_)
  {
  }

  bool operator==(const context_allocator& other) const noexcept
  {
    return &memory_ == &other.memory_;
  }

  bool operator!=(const context_allocator& other) const noexcept
  {
    return &memory_ != &other.memory_;
  }

  T* allocate(std::size_t n) const
  {
    return static_cast<T*>(memory_.allocate(sizeof(T) * n, alignof(T)));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    return memory_.deallocate(p);
  }

private:
  template <typename> friend class context_allocator;

  // The underlying memory.
  context_memory& memory_;
};

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_memory
{
public:
  handler_memory()
    : in_use_(false)
  {
  }

  handler_memory(const handler_memory&) = delete;
  handler_memory& operator=(const handler_memory&) = delete;

  void* allocate(std::size_t size)
  {
    if (!in_use_ && size < sizeof(storage_))
    {
      in_use_ = true;
      return &storage_;
    }
    else
    {
      return ::operator new(size);
    }
  }

  void deallocate(void* pointer)
  {
    if (pointer == &storage_)
    {
      in_use_ = false;
    }
    else
    {
      ::operator delete(pointer);
    }
  }

private:
  // Storage space used for handler-based custom memory allocation.
  typename std::aligned_storage<1024>::type storage_;

  // Whether the handler-based custom allocation storage has been used.
  bool in_use_;
};

// The allocator to be associated with the handler objects. This allocator only
// needs to satisfy the C++11 minimal allocator requirements.
template <typename T>
class handler_allocator
{
public:
  using value_type = T;

  explicit handler_allocator(handler_memory& mem)
    : memory_(mem)
  {
  }

  template <typename U>
  handler_allocator(const handler_allocator<U>& other) noexcept
    : memory_(other.memory_)
  {
  }

  bool operator==(const handler_allocator& other) const noexcept
  {
    return &memory_ == &other.memory_;
  }

  bool operator!=(const handler_allocator& other) const noexcept
  {
    return &memory_ != &other.memory_;
  }

  T* allocate(std::size_t n) const
  {
    return static_cast<T*>(memory_.allocate(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    return memory_.deallocate(p);
  }

private:
  template <typename> friend class handler_allocator;

  // The underlying memory.
  handler_memory& memory_;
};

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_),
        asio::bind_allocator(
          handler_allocator<int>(handler_memory_),
          [this, self](std::error_code ec, std::size_t length)
          {
            if (!ec)
            {
              do_write(length);
            }
          }));
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(data_, length),
        asio::bind_allocator(
          handler_allocator<int>(handler_memory_),
          [this, self](std::error_code ec, std::size_t /*length*/)
          {
            if (!ec)
            {
              do_read();
            }
          }));
  }

  // The socket used to communicate with the client.
  tcp::socket socket_;

  // Buffer used to store data received from the client.
  std::array<char, 1024> data_;

  // The memory to use for handler-based custom memory allocation.
  handler_memory handler_memory_;
};

class server
{
public:
  server(asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: server <port>\n";
      return 1;
    }

    context_memory memory(4096);
    context_allocator<void> allocator(memory);
    asio::io_context io_context(std::allocator_arg, allocator);
    server s(io_context, std::atoi(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
