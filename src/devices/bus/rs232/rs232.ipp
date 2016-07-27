// license:BSD-3-Clause
// copyright-holders:smf,Vas Crabb
#ifndef MAME_BUS_RS232_IPP
#define MAME_BUS_RS232_IPP

#include "rs232.h"


template <UINT32 FIFO_LENGTH>
WRITE_LINE_MEMBER( buffered_rs232_device<FIFO_LENGTH>::input_txd )
{
	device_serial_interface::rx_w(state);
}


template <UINT32 FIFO_LENGTH>
buffered_rs232_device<FIFO_LENGTH>::buffered_rs232_device(
		const machine_config &mconfig,
		device_type type,
		const char *name,
		const char *tag,
		device_t *owner,
		UINT32 clock,
		const char *shortname,
		const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_head(0U)
	, m_tail(0U)
	, m_empty(1U)
{
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::device_start()
{
	device_serial_interface::register_save_state(machine().save(), this);

	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_tail));
	save_item(NAME(m_empty));
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::tra_complete()
{
	assert(!m_empty || (m_head == m_tail));
	assert(m_head < ARRAY_LENGTH(m_fifo));
	assert(m_tail < ARRAY_LENGTH(m_fifo));

	if (!m_empty)
	{
		transmit_register_setup(m_fifo[m_head]);
		m_head = (m_head + 1U) % FIFO_LENGTH;
		m_empty = (m_head == m_tail) ? 1U : 0U;
	}
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::rcv_complete()
{
	receive_register_extract();
	received_byte(get_received_char());
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::clear_fifo()
{
	m_head = m_tail = 0U;
	m_empty = 1U;
}


template <UINT32 FIFO_LENGTH>
void buffered_rs232_device<FIFO_LENGTH>::transmit_byte(UINT8 byte)
{
	assert(!m_empty || (m_head == m_tail));
	assert(m_head < ARRAY_LENGTH(m_fifo));
	assert(m_tail < ARRAY_LENGTH(m_fifo));

	if (m_empty && is_transmit_register_empty())
	{
		transmit_register_setup(byte);
	}
	else if (m_empty || (m_head != m_tail))
	{
		m_fifo[m_tail] = byte;
		m_tail = (m_tail + 1U) & FIFO_LENGTH;
		m_empty = 0U;
	}
	else
	{
		logerror("FIFO overrun (byte = 0x%02x)", byte);
	}
}

#endif // MAME_BUS_RS232_IPP
