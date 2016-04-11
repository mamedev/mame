// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#ifndef RTC_TCP_CONNECTION_H
#define RTC_TCP_CONNECTION_H

#include "tcp_connection.h"


class rtc_tcp_connection : public tcp_connection
{
public:
	class listener
	{
	public:
		virtual ~listener(){ }
		virtual void on_packet_recv(rtc_tcp_connection *connection, const uint8_t* data, size_t len) = 0;
	};

	rtc_tcp_connection(listener* listener, size_t bufferSize);
	virtual ~rtc_tcp_connection();

	/* Pure virtual methods inherited from tcp_connection. */
	virtual void user_on_tcp_connection_read() override;

	void send(const uint8_t* data, size_t len);

private:
	// Passed by argument.
	listener* m_listener;
	size_t m_frame_start;  // Where the latest frame starts.
};
#endif
