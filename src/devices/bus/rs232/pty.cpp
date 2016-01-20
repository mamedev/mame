// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
#include <stdio.h>
#include "pty.h"

static const int TIMER_POLL = 1;

pseudo_terminal_device::pseudo_terminal_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PSEUDO_TERMINAL, "Pseudo terminal", tag, owner, clock, "pseudo_terminal", __FILE__),
		device_serial_interface(mconfig, *this),
		device_rs232_port_interface(mconfig, *this),
		device_pty_interface(mconfig, *this),
		m_rs232_txbaud(*this, "RS232_TXBAUD"),
		m_rs232_rxbaud(*this, "RS232_RXBAUD"),
		m_rs232_startbits(*this, "RS232_STARTBITS"),
		m_rs232_databits(*this, "RS232_DATABITS"),
		m_rs232_parity(*this, "RS232_PARITY"),
		m_rs232_stopbits(*this, "RS232_STOPBITS"),
		m_input_count(0),
		m_input_index(0),
		m_timer_poll(nullptr)
{
}

WRITE_LINE_MEMBER(pseudo_terminal_device::update_serial)
{
		int startbits = convert_startbits(m_rs232_startbits->read());
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
}

static INPUT_PORTS_START(pseudo_terminal)
		MCFG_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", pseudo_terminal_device, update_serial)
		MCFG_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", pseudo_terminal_device, update_serial)
		MCFG_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", pseudo_terminal_device, update_serial)
		MCFG_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", pseudo_terminal_device, update_serial)
		MCFG_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", pseudo_terminal_device, update_serial)
		MCFG_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", pseudo_terminal_device, update_serial)
INPUT_PORTS_END

ioport_constructor pseudo_terminal_device::device_input_ports() const
{
		return INPUT_PORTS_NAME(pseudo_terminal);
}

void pseudo_terminal_device::device_start()
{
		m_timer_poll = timer_alloc(TIMER_POLL);

		open();
}

void pseudo_terminal_device::device_stop()
{
		close();
}

void pseudo_terminal_device::device_reset()
{
		update_serial(0);
		queue();
}

void pseudo_terminal_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
		switch (id)
		{
		case TIMER_POLL:
				queue();
				break;

		default:
				device_serial_interface::device_timer(timer, id, param, ptr);
		}
}

void pseudo_terminal_device::tra_callback()
{
		output_rxd(transmit_register_get_data_bit());
}

void pseudo_terminal_device::tra_complete()
{
		queue();
}

void pseudo_terminal_device::rcv_complete()
{
		receive_register_extract();
		write(get_received_char());
}

void pseudo_terminal_device::queue(void)
{
		if (is_transmit_register_empty())
		{
				if (m_input_index == m_input_count)
				{
					m_input_index = 0;
					int tmp = read(m_input_buffer , sizeof(m_input_buffer));
					if (tmp > 0) {
						m_input_count = tmp;
					} else {
						m_input_count = 0;
					}
				}

				if (m_input_count != 0)
				{
						transmit_register_setup(m_input_buffer[ m_input_index++ ]);

						m_timer_poll->adjust(attotime::never);
				}
				else
				{
						int txbaud = convert_baud(m_rs232_txbaud->read());
						m_timer_poll->adjust(attotime::from_hz(txbaud));
				}
		}
}

const device_type PSEUDO_TERMINAL = &device_creator<pseudo_terminal_device>;
