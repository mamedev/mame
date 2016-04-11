// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#ifndef RAW_TCP_CONNECTION_H
#define RAW_TCP_CONNECTION_H

#include "tcp_connection.h"


class raw_tcp_connection : public tcp_connection
{
public:
	class listener
	{
	public:
		virtual ~listener(){ }
		virtual void on_data_recv(raw_tcp_connection *connection, const uint8_t* data, size_t len) = 0;
	};

	raw_tcp_connection(listener* listener, size_t bufferSize);
	virtual ~raw_tcp_connection();

	/* Pure virtual methods inherited from tcp_connection. */
	virtual void user_on_tcp_connection_read() override;

private:
	// Passed by argument.
	listener* m_listener;
};
#endif
