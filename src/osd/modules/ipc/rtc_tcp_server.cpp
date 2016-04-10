// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#include "rtc_tcp_server.h"
#include <string>

#define MAX_TCP_CONNECTIONS_PER_SERVER 10

/* Instance methods. */

rtc_tcp_server::rtc_tcp_server(uv_loop_t* loop, const std::string &ip, uint16_t port, int backlog, listener* listener, rtc_tcp_connection::listener* connListener) :
	tcp_server(loop, ip, port, backlog),
	m_listener(listener),
	m_conn_listener(connListener)
{
}

void rtc_tcp_server::user_on_tcp_connection_alloc(tcp_connection** connection)
{
	// Allocate a new rtc_tcp_connection for the rtc_tcp_server to handle it.
	*connection = new rtc_tcp_connection(m_conn_listener, 65536);
}

void rtc_tcp_server::user_on_new_tcp_connection(tcp_connection* connection)
{
	// Allow just MAX_TCP_CONNECTIONS_PER_SERVER.
	if (get_num_connections() > MAX_TCP_CONNECTIONS_PER_SERVER)
		connection->close();
}

void rtc_tcp_server::user_on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer)
{
	// Notify the listener.
	// NOTE: Don't do it if closing (since at this point the listener is already freed).
	// At the end, this is just called if the connection was remotely closed.
	if (!is_closing())
		m_listener->on_rtc_tcp_connection_closed(this, static_cast<rtc_tcp_connection*>(connection), is_closed_by_peer);
}

void rtc_tcp_server::user_on_tcp_server_closed()
{
}
