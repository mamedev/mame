// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
#include "emu.h"
#include "pty.h"

#include <cstdio>


pseudo_terminal_device::pseudo_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSEUDO_TERMINAL, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	device_pty_interface(mconfig, *this),
	m_rs232_txbaud(*this, "RS232_TXBAUD"),
	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS"),
	m_flow(*this, "FLOW_CONTROL"),
	m_input_count(0),
	m_input_index(0),
	m_timer_poll(nullptr),
	m_rts(0),
	m_dtr(0),
	m_xoff(0)
{
}

void pseudo_terminal_device::update_serial(int state)
{
	int startbits = 1;
	int databits = convert_databits(m_rs232_databits->read());
	parity_t parity = convert_parity(m_rs232_parity->read());
	stop_bits_t stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int txbaud = convert_baud(m_rs232_txbaud->read());
	set_tra_rate(txbaud);

	int rxbaud = convert_baud(m_rs232_rxbaud->read());
	set_rcv_rate(rxbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

	m_xoff = 0;
}

static INPUT_PORTS_START(pseudo_terminal)
	PORT_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", pseudo_terminal_device, update_serial)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", pseudo_terminal_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", pseudo_terminal_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", pseudo_terminal_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", pseudo_terminal_device, update_serial)

	PORT_START("FLOW_CONTROL")
	PORT_CONFNAME(0x07, 0x00, "Flow Control")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, "RTS")
	PORT_CONFSETTING(   0x02, "DTR")
	PORT_CONFSETTING(   0x04, "XON/XOFF")
INPUT_PORTS_END

ioport_constructor pseudo_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pseudo_terminal);
}

void pseudo_terminal_device::device_start()
{
	m_timer_poll = timer_alloc(FUNC(pseudo_terminal_device::update_queue), this);

	open();
}

void pseudo_terminal_device::device_stop()
{
	close();
}

void pseudo_terminal_device::device_reset()
{
	update_serial(0);
	update_queue(0);
}

void pseudo_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void pseudo_terminal_device::tra_complete()
{
	update_queue(0);
}

void pseudo_terminal_device::rcv_complete()
{
	receive_register_extract();

	uint8_t const data = get_received_char();
	if (m_flow->read() != 4)
	{
		write(data);
	}
	else
	{
		switch (data)
		{
		case 0x13: // XOFF
			m_xoff = 1;
			return;

		case 0x11: // XON
			m_xoff = 0;
			return;

		default:
			break;
		}
		write(data);
	}
}

TIMER_CALLBACK_MEMBER(pseudo_terminal_device::update_queue)
{
	if (is_transmit_register_empty())
	{
		if (m_input_index == m_input_count)
		{
			m_input_index = 0;
			int const tmp = read(m_input_buffer, sizeof(m_input_buffer));
			if (tmp > 0)
				m_input_count = tmp;
			else
				m_input_count = 0;
		}

		if (m_input_count != 0)
		{
			uint8_t const fc = m_flow->read();

			if (fc == 0 || (fc == 1 && m_rts == 0) || (fc == 2 && m_dtr == 0) || (fc == 4 && m_xoff == 0))
			{
				transmit_register_setup(m_input_buffer[m_input_index++]);
				m_timer_poll->adjust(attotime::never);
				return;
			}
		}

		int const txbaud = convert_baud(m_rs232_txbaud->read());
		m_timer_poll->adjust(attotime::from_hz(txbaud));
	}
}

DEFINE_DEVICE_TYPE(PSEUDO_TERMINAL, pseudo_terminal_device, "pseudo_terminal", "Pseudo terminal")
