// license:BSD-3-Clause
// copyright-holders:smf
#include "terminal.h"

serial_terminal_device::serial_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: generic_terminal_device(mconfig, SERIAL_TERMINAL, "Serial Terminal", tag, owner, clock, "serial_terminal", __FILE__),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_rs232_txbaud(*this, "RS232_TXBAUD"),
	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_startbits(*this, "RS232_STARTBITS"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

static INPUT_PORTS_START(serial_terminal)
	PORT_INCLUDE(generic_terminal)

	MCFG_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", serial_terminal_device, update_serial)
	MCFG_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", serial_terminal_device, update_serial)
	MCFG_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_terminal_device, update_serial)
	MCFG_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_terminal_device, update_serial)
	MCFG_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_terminal_device, update_serial)
	MCFG_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_terminal_device, update_serial)
INPUT_PORTS_END

ioport_constructor serial_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_terminal);
}

void serial_terminal_device::device_start()
{
	generic_terminal_device::device_start();
}

WRITE_LINE_MEMBER(serial_terminal_device::update_serial)
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

void serial_terminal_device::device_reset()
{
	generic_terminal_device::device_reset();

	update_serial(0);
}

void serial_terminal_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void serial_terminal_device::send_key(UINT8 code)
{
	if (is_transmit_register_empty())
	{
		transmit_register_setup(code);
		return;
	}

	m_key_valid = true;
	m_curr_key = code;
}

void serial_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void serial_terminal_device::tra_complete()
{
	if (m_key_valid)
	{
		transmit_register_setup(m_curr_key);
		m_key_valid = false;
	}
}

void serial_terminal_device::rcv_complete()
{
	receive_register_extract();
	term_write(get_received_char());
}

const device_type SERIAL_TERMINAL = &device_creator<serial_terminal_device>;
