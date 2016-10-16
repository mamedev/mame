// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    network.cpp

    Network output interface.

***************************************************************************/

#include "output_module.h"
#include "modules/osdmodule.h"

#include <thread>
#include <set>
#include "asio.h"

class output_client
{
public:
  virtual ~output_client() {}
  virtual void deliver(std::string &msg) = 0;
};

using output_client_ptr = std::shared_ptr<output_client>;
using client_set = std::set<output_client_ptr>;

class output_session : public output_client,
		public std::enable_shared_from_this<output_session>
			    
{
public:
  output_session(asio::ip::tcp::socket socket, client_set *clients) : 
	m_socket(std::move(socket)), 
	m_clients(clients)
  {
  }

  void start()
  {
	m_clients->insert(shared_from_this());
    do_read();
  }

private:
  void deliver(std::string &msg)
  {
	std::strncpy(m_data, msg.c_str(), max_length); 	
	do_write(msg.size());
  }

  void do_read()
  {
    auto self(shared_from_this());
    m_socket.async_read_some(asio::buffer(m_input_m_data, max_length),
        [this, self](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            do_read();
          }
          else
          {
            m_clients->erase(shared_from_this());
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    asio::async_write(m_socket, asio::buffer(m_data, length),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (ec)
          {
            m_clients->erase(shared_from_this());
          }
        });
  }

  asio::ip::tcp::socket m_socket;
  enum { max_length = 1024 };
  char m_data[max_length];
  char m_input_m_data[max_length];
  client_set *m_clients;
};

class output_network_server
{
public:
  output_network_server(asio::io_context& io_context, short port) : 
    m_acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {
    do_accept();
  }
  
  void deliver_to_all(std::string msg)
  {
	for (auto client: m_clients)
      client->deliver(msg);
  }

private:
  void do_accept()
  {
    m_acceptor.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<output_session>(std::move(socket),&m_clients)->start();
          }

          do_accept();
        });
  }
  
  asio::ip::tcp::acceptor m_acceptor;
  client_set m_clients;
};


class output_network : public osd_module, public output_module
{
public:
	output_network()
	: osd_module(OSD_OUTPUT_PROVIDER, "network"), 
	  output_module(), 
	  m_io_context(nullptr), m_server(nullptr)
	{
	}
	
	virtual ~output_network() 
	{
	}

	virtual int init(const osd_options &options) override 
	{
		m_working_thread = std::thread([](output_network* self) { self->process_output(); }, this);
		return 0;
	}

	virtual void exit() override 
	{
		m_io_context->stop();
		m_working_thread.join();
		delete m_server;
		delete m_io_context;
	}

	// output_module

	virtual void notify(const char *outname, INT32 value) override
	{
		static char buf[256];
		sprintf(buf, "%s = %d\n", ((outname==nullptr) ? "none" : outname), value);
		m_server->deliver_to_all(buf);
	}

	// implementation
	void process_output()
	{
		m_io_context = new asio::io_context();
		m_server = new output_network_server(*m_io_context, 8000);
		m_io_context->run();
	}

private:
	std::thread m_working_thread;
	asio::io_context *m_io_context;
	output_network_server *m_server;
};

MODULE_DEFINITION(OUTPUT_NETWORK, output_network)
