// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    network.cpp

    Network output interface.

*******************************************************************c********/

#include "output_module.h"
#include "modules/osdmodule.h"
#include "modules/ipc/raw_tcp_server.h"
#include "modules/ipc/raw_tcp_connection.h"
#include <uv.h>

#include <mutex>
#include <thread>

class output_network_server :
	public raw_tcp_server::listener,
	public raw_tcp_connection::listener
{

public:
	output_network_server(uv_loop_t* loop) { m_tcp_server = new raw_tcp_server(loop, "0.0.0.0", 8000, 256, this, this); }

	virtual ~output_network_server() { delete m_tcp_server; }

	void terminate_all() { m_tcp_server->terminate(); }
	void send_to_all(const uint8_t* data, size_t len) { m_tcp_server->send_to_all(data,len); }

	/* Pure virtual methods inherited from raw_tcp_server::listener. */
	virtual void on_raw_tcp_connection_closed(raw_tcp_server* tcpServer, raw_tcp_connection* connection, bool is_closed_by_peer) override { }
	/* Pure virtual methods inherited from raw_tcp_connection::listener. */
	virtual void on_data_recv(raw_tcp_connection *connection, const uint8_t* data, size_t len) override { }
private:
	raw_tcp_server* m_tcp_server;
};

class output_network : public osd_module, public output_module
{
public:
	output_network()
	: osd_module(OSD_OUTPUT_PROVIDER, "network"), output_module(), m_loop(nullptr), m_server(nullptr)
{
	}
	virtual ~output_network() {
	}

	virtual int init(const osd_options &options) override { 
		m_loop = new uv_loop_t;
		int err = uv_loop_init(m_loop);
		if (err) {
			return 1;
		}
		m_working_thread = std::thread([](output_network* self) { self->process_output(); }, this);
		return 0; 
	}

	virtual void exit() override { 
		m_server->terminate_all();
		m_working_thread.join();
		uv_loop_close(m_loop);
		delete m_loop;
		delete m_server;
	}

	// output_module

	virtual void notify(const char *outname, INT32 value) override { m_server->send_to_all((const uint8_t*)outname, strlen(outname)); }

	// implementation
	void process_output()
	{
		m_server = new output_network_server(m_loop);
		uv_run(m_loop, UV_RUN_DEFAULT);
	}
	
private:
	std::thread m_working_thread;
	uv_loop_t* m_loop;
	output_network_server *m_server;
};

MODULE_DEFINITION(OUTPUT_NETWORK, output_network)
