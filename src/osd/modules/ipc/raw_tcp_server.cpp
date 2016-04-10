// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#include "raw_tcp_server.h"
#include <string>

#define MAX_TCP_CONNECTIONS_PER_SERVER 10

/* Instance methods. */

raw_tcp_server::raw_tcp_server(uv_loop_t* loop, const std::string &ip, uint16_t port, int backlog, listener* listener, raw_tcp_connection::listener* connListener) :
	tcp_server(loop, ip, port, backlog),
	m_listener(listener),
	m_conn_listener(connListener)
{
}

void raw_tcp_server::user_on_tcp_connection_alloc(tcp_connection** connection)
{
	// Allocate a new raw_tcp_connection for the raw_tcp_server to handle it.
	*connection = new raw_tcp_connection(m_conn_listener, 65536);
}

void raw_tcp_server::user_on_new_tcp_connection(tcp_connection* connection)
{
	// Allow just MAX_TCP_CONNECTIONS_PER_SERVER.
	if (get_num_connections() > MAX_TCP_CONNECTIONS_PER_SERVER)
		connection->close();
}

void raw_tcp_server::user_on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer)
{
	// Notify the listener.
	// NOTE: Don't do it if closing (since at this point the listener is already freed).
	// At the end, this is just called if the connection was remotely closed.
	if (!is_closing())
		m_listener->on_raw_tcp_connection_closed(this, static_cast<raw_tcp_connection*>(connection), is_closed_by_peer);
}

void raw_tcp_server::user_on_tcp_server_closed()
{
}
