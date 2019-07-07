// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "keyboard.h"

namespace {
INPUT_PORTS_START(serial_keyboard)
	PORT_INCLUDE(generic_keyboard)

	PORT_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", serial_keyboard_device, update_serial)
	PORT_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_keyboard_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_keyboard_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_keyboard_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_keyboard_device, update_serial)
INPUT_PORTS_END
} // anonymous namespace

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: serial_keyboard_device(mconfig, SERIAL_KEYBOARD, tag, owner, clock)
{
}

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: generic_keyboard_device(mconfig, type, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_startbits(*this, "RS232_STARTBITS")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

ioport_constructor serial_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_keyboard);
}

WRITE_LINE_MEMBER( serial_keyboard_device::input_txd )
{
	device_buffered_serial_interface::rx_w(state);
}

WRITE_LINE_MEMBER( serial_keyboard_device::update_serial )
{
	reset();
}

void serial_keyboard_device::device_reset()
{
	generic_keyboard_device::device_reset();

	clear_fifo();

	int const startbits = convert_startbits(m_rs232_startbits->read());
	int const databits = convert_databits(m_rs232_databits->read());
	parity_t const parity = convert_parity(m_rs232_parity->read());
	stop_bits_t const stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int const txbaud = convert_baud(m_rs232_txbaud->read());
	set_tra_rate(txbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
	receive_register_reset();
	transmit_register_reset();
}

void serial_keyboard_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void serial_keyboard_device::send_key(uint8_t code)
{
	transmit_byte(code);
}

void serial_keyboard_device::received_byte(uint8_t byte)
{
}

DEFINE_DEVICE_TYPE(SERIAL_KEYBOARD, serial_keyboard_device, "serial_keyboard", "Serial Keyboard")
