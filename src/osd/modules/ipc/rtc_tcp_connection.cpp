// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic
#include "emu.h"
#include "rtc_tcp_connection.h"

/* Instance methods. */

rtc_tcp_connection::rtc_tcp_connection(listener* listener, size_t bufferSize) :
	tcp_connection(bufferSize),
	m_listener(listener),
	m_frame_start(0)
{
}

rtc_tcp_connection::~rtc_tcp_connection()
{
}

inline uint16_t get2bytes(const uint8_t* data, size_t i)
{
	return (uint16_t)(data[i + 1]) | ((uint16_t)(data[i])) << 8;
}

inline void set2bytes(uint8_t* data, size_t i, uint16_t value)
{
	data[i + 1] = (uint8_t)(value);
	data[i] = (uint8_t)(value >> 8);
}


void rtc_tcp_connection::user_on_tcp_connection_read()
{
	/*
	 * Framing RFC 4571
	 *
	 *     0                   1                   2                   3
	 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *     ---------------------------------------------------------------
	 *     |             LENGTH            |  STUN / DTLS / RTP / RTCP   |
	 *     ---------------------------------------------------------------
	 *
	 * A 16-bit unsigned integer LENGTH field, coded in network byte order
	 * (big-endian), begins the frame.  If LENGTH is non-zero, an RTP or
	 * RTCP packet follows the LENGTH field.  The value coded in the LENGTH
	 * field MUST equal the number of octets in the RTP or RTCP packet.
	 * Zero is a valid value for LENGTH, and it codes the null packet.
	 */

	// Be ready to parse more than a single frame in a single TCP chunk.
	while (true)
	{
		// We may receive multiple packets in the same TCP chunk. If one of them is
		// a DTLS Close Alert this would be closed (Close() called) so we cannot call
		// our listeners anymore.
		if (is_closing())
			return;

		size_t data_len = m_buffer_data_len - m_frame_start;
		size_t packet_len = 0;

		if (data_len >= 2)
			packet_len = (size_t)get2bytes(m_buffer + m_frame_start, 0);

		// We have packet_len bytes.
		if ((data_len >= 2) && data_len >= 2 + packet_len)
		{
			const uint8_t* packet = m_buffer + m_frame_start + 2;

			// Notify the listener.
			if (packet_len != 0)
			{
				m_listener->on_packet_recv(this, packet, packet_len);
			}

			// If there is no more space available in the buffer and that is because
			// the latest parsed frame filled it, then empty the full buffer.
			if ((m_frame_start + 2 + packet_len) == m_buffer_size)
			{
				osd_printf_error("no more space in the buffer, emptying the buffer data");

				m_frame_start = 0;
				m_buffer_data_len = 0;
			}
			// If there is still space in the buffer, set the beginning of the next
			// frame to the next position after the parsed frame.
			else
			{
				m_frame_start += 2 + packet_len;
			}

			// If there is more data in the buffer after the parsed frame then
			// parse again. Otherwise break here and wait for more data.
			if (m_buffer_data_len > m_frame_start)
			{
				// osd_printf_error("there is more data after the parsed frame, continue parsing");

				continue;
			}
			else
			{
				break;
			}
		}
		else // Incomplete packet.
		{
			// Check if the buffer is full.
			if (m_buffer_data_len == m_buffer_size)
			{
				// First case: the incomplete frame does not begin at position 0 of
				// the buffer, so move the frame to the position 0.
				if (m_frame_start != 0)
				{
					// osd_printf_error("no more space in the buffer, moving parsed bytes to the beginning of the buffer and wait for more data");

					std::memmove(m_buffer, m_buffer + m_frame_start, m_buffer_size - m_frame_start);
					m_buffer_data_len = m_buffer_size - m_frame_start;
					m_frame_start = 0;
				}
				// Second case: the incomplete frame begins at position 0 of the buffer.
				// The frame is too big, so close the connection.
				else
				{
					osd_printf_error("no more space in the buffer for the unfinished frame being parsed, closing the connection");

					// Close the socket.
					close();
				}
			}
			// The buffer is not full.
			else
			{
				osd_printf_verbose("frame not finished yet, waiting for more data");
			}

			// Exit the parsing loop.
			break;
		}
	}
}

void rtc_tcp_connection::send(const uint8_t* data, size_t len)
{
	// Write according to Framing RFC 4571.

	uint8_t frame_len[2];

	set2bytes(frame_len, 0, len);

	write(frame_len, 2, data, len);
}
