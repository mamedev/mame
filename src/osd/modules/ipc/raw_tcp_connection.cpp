// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#include "raw_tcp_connection.h"

/* Instance methods. */

raw_tcp_connection::raw_tcp_connection(listener* listener, size_t bufferSize) :
	tcp_connection(bufferSize),
	m_listener(listener)
{
}

raw_tcp_connection::~raw_tcp_connection()
{
}

void raw_tcp_connection::user_on_tcp_connection_read()
{
	// We may receive multiple packets in the same TCP chunk. If one of them is
	// a DTLS Close Alert this would be closed (close() called) so we cannot call
	// our listeners anymore.
	if (is_closing())
		return;

	m_listener->on_data_recv(this, m_buffer, m_buffer_data_len);

	m_buffer_data_len = 0;
}
