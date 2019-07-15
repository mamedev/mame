// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#include "emu.h"
#include "ie15.h"

ie15_terminal_device::ie15_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ie15_device(mconfig, SERIAL_TERMINAL_IE15, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_rxbaud(*this, "RS232_RXBAUD")
	, m_rs232_startbits(*this, "RS232_STARTBITS")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

static INPUT_PORTS_START(ie15_terminal)
	PORT_INCLUDE(ie15)

	PORT_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", ie15_terminal_device, update_serial)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", ie15_terminal_device, update_serial)
	PORT_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", ie15_terminal_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", ie15_terminal_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", ie15_terminal_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", ie15_terminal_device, update_serial)
INPUT_PORTS_END

ioport_constructor ie15_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ie15_terminal);
}

WRITE_LINE_MEMBER(ie15_terminal_device::update_serial)
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

void ie15_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void ie15_terminal_device::tra_complete()
{
	ie15_device::tra_complete();
}

void ie15_terminal_device::rcv_complete()
{
	receive_register_extract();
	term_write(get_received_char());
}

void ie15_terminal_device::device_start()
{
	ie15_device::device_start();
}

void ie15_terminal_device::device_reset()
{
	update_serial(0);
	ie15_device::device_reset();
}

void ie15_terminal_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	ie15_device::device_timer(timer, id, param, ptr);
}

DEFINE_DEVICE_TYPE(SERIAL_TERMINAL_IE15, ie15_terminal_device, "ie15_terminal", "IE15 Terminal")
