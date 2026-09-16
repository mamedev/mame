// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard device
/*
    TODO: represent the real layout/scancodes for this keyboard.  The
    current HLE just sends ASCII and hopes, with an additional 0xc0
    "last key up" code.  We're also inheriting typematic behaviour from
    the serial keyboard which may not be desirable.
*/

#include "emu.h"
#include "ngen_kb.h"

ngen_keyboard_device::ngen_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: serial_keyboard_device(mconfig, NGEN_KEYBOARD, tag, owner, 0)
	, m_keys_down(0U)
	, m_last_reset(0U)
{
}


void ngen_keyboard_device::write(uint8_t data)
{
	// To be figured out
	// Code 0x92 is sent on startup, perhaps resets the keyboard MCU
	// Codes 0xAx and 0xBx appear to control the keyboard LEDs, lower nibbles controlling the state of the LEDs
	// When setting an error code via the LEDs, 0xB0 then 0xAE is sent (presumably for error code 0xE0),
	// so that means that 0xAx controls the Overtype, Lock, F1 and F2 LEDs, and 0xBx controls the F3, F8, F9 and F10 LEDs.
	logerror("KB: received character %02x\n",data);
	switch(data)
	{
	case 0x92U:  // reset(?)
		m_last_reset = 0x01U;
		break;
	}
}

static INPUT_PORTS_START( ngen_keyboard )
	PORT_INCLUDE(generic_keyboard)

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_19200, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_BAUD_19200, "19200") // TODO: Based on the RAM refresh timer (~78kHz) to be 19530Hz

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_2, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")
INPUT_PORTS_END


ioport_constructor ngen_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ngen_keyboard);
}

void ngen_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();

	save_item(NAME(m_keys_down));
	save_item(NAME(m_last_reset));

	set_rcv_rate(19200);
}

void ngen_keyboard_device::device_reset()
{
	serial_keyboard_device::device_reset();

	m_keys_down = uint8_t(~0U);
	m_last_reset = 0U;
}

void ngen_keyboard_device::rcv_complete()
{
	receive_register_extract();
	write(get_received_char());
}

void ngen_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	serial_keyboard_device::key_make(row, column);
	m_keys_down = uint8_t((row << 4) | column);
	m_last_reset = 0U;
}

void ngen_keyboard_device::key_break(uint8_t row, uint8_t column)
{
	serial_keyboard_device::key_break(row, column);
	if (m_keys_down == uint8_t((row << 4) | column))
	{
		m_keys_down = uint8_t(~0U);
		send_key(0xc0U);
	}
}

DEFINE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device, "ngen_kb", "NGEN Keyboard")
