// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#ifndef RAW_TCP_SERVER_H
#define RAW_TCP_SERVER_H

#include "tcp_server.h"
#include "tcp_connection.h"
#include "raw_tcp_connection.h"
#include <uv.h>

class raw_tcp_server : public tcp_server
{
public:
	class listener
	{
	public:
		virtual ~listener() { }
		virtual void on_raw_tcp_connection_closed(raw_tcp_server* tcpServer, raw_tcp_connection* connection, bool is_closed_by_peer) = 0;
	};

	raw_tcp_server(uv_loop_t* loop, const std::string &ip, uint16_t port, int backlog, listener* listener, raw_tcp_connection::listener* connListener);

	/* Pure virtual methods inherited from TCPServer. */
	virtual void user_on_tcp_connection_alloc(tcp_connection** connection) override;
	virtual void user_on_new_tcp_connection(tcp_connection* connection) override;
	virtual void user_on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer) override;
	virtual void user_on_tcp_server_closed() override;

private:
	// Passed by argument.
	listener* m_listener;
	raw_tcp_connection::listener* m_conn_listener;
};

#endif
