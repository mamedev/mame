// license:BSD-3-Clause
// copyright-holders:smf
#include "keyboard.h"

namespace {
INPUT_PORTS_START(serial_keyboard)
	PORT_INCLUDE(generic_keyboard)

	MCFG_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", serial_keyboard_device, update_serial)
	MCFG_RS232_STARTBITS("RS232_STARTBITS", RS232_STARTBITS_1, "Start Bits", serial_keyboard_device, update_serial)
	MCFG_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", serial_keyboard_device, update_serial)
	MCFG_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", serial_keyboard_device, update_serial)
	MCFG_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", serial_keyboard_device, update_serial)
INPUT_PORTS_END
} // anonymous namespace

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: serial_keyboard_device(mconfig, SERIAL_KEYBOARD, "Serial Keyboard", tag, owner, clock, "serial_keyboard", __FILE__)
{
}

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: generic_keyboard_device(mconfig, type, name, tag, owner, clock, shortname, source)
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

void serial_keyboard_device::device_start()
{
	generic_keyboard_device::device_start();
	device_buffered_serial_interface::register_save_state(machine().save(), this);
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

void serial_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// give both bases a chance to handle it
	device_buffered_serial_interface::device_timer(timer, id, param, ptr);
	generic_keyboard_device::device_timer(timer, id, param, ptr);
}

void serial_keyboard_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void serial_keyboard_device::send_key(UINT8 code)
{
	transmit_byte(code);
}

void serial_keyboard_device::received_byte(UINT8 byte)
{
}

const device_type SERIAL_KEYBOARD = &device_creator<serial_keyboard_device>;
