// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Keyboard emulation for Ampex 210 and related terminals.

*******************************************************************************/

#include "emu.h"
#include "ampex210_kbd.h"

//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(AMPEX230_KEYBOARD, ampex230_keyboard_device, "ampex230_kbd", "Ampex 230 Keyboard")

ampex230_keyboard_device::ampex230_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AMPEX230_KEYBOARD, tag, owner, clock)
	, m_data_out_callback(*this)
	, m_clock_out_callback(*this)
	, m_mcu(*this, "mcu")
	, m_key_row(*this, "ROW%d", 0)
	, m_key_scan(0)
{
}

//**************************************************************************
//  INPUT MATRIX
//**************************************************************************

static INPUT_PORTS_START(ampex230_keyboard)
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 00")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 01")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 02")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 03")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 04")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 05")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 06")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 07")

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 08")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 09")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 0F")

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 11")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 13")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 14")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 15")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 16")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 17")

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 18")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 19")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 1F")

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 20")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 21")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 22")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 23")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 24")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 25")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 26")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 27")

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 28")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 29")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 2B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 2C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 2D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 2F")

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 30")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 31")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 32")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 33")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 34")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 35")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 36")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 37")

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 38")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 39")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 3F")

	PORT_START("ROW8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 40")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 41")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 42")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 43")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 44")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 45")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 46")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 47")

	PORT_START("ROW9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 48")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 49")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 4F")

	PORT_START("ROW10")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 50")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 51")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 52")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 53")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 56")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 57")

	PORT_START("ROW11")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 58")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 59")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 5F")

	PORT_START("ROW12")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 60")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 61")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 62")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 63")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 64")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 65")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 66")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 67")

	PORT_START("ROW13")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 68")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 69")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6A")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6B")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6C")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 6F")

	PORT_START("MODIFIERS")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Modifier 0")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Modifier 1")
INPUT_PORTS_END

ioport_constructor ampex230_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ampex230_keyboard);
}

//**************************************************************************
//  DEVICE INITIALIZATION AND STARTUP
//**************************************************************************

void ampex230_keyboard_device::device_resolve_objects()
{
	// resolve callbacks
	m_data_out_callback.resolve_safe();
	m_clock_out_callback.resolve_safe();
}

void ampex230_keyboard_device::device_start()
{
	save_item(NAME(m_key_scan));
}

//**************************************************************************
//  EXTERNAL CONTROL
//**************************************************************************

WRITE_LINE_MEMBER(ampex230_keyboard_device::strobe_w)
{
	m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? ASSERT_LINE : CLEAR_LINE);
}

//**************************************************************************
//  MCU I/O HANDLERS
//**************************************************************************

u8 ampex230_keyboard_device::bus_r()
{
	u8 result = 0xff;
	for (int j = 0; j < 14; j++)
		if (!BIT(m_key_scan, j))
			result &= m_key_row[j]->read();

	return result;
}

void ampex230_keyboard_device::p1_w(u8 data)
{
	m_key_scan = (m_key_scan & 0x00ff) | u16(data & 0x3f) << 8;

	m_clock_out_callback(BIT(data, 6));
	m_data_out_callback(BIT(data, 7));
}

void ampex230_keyboard_device::p2_w(u8 data)
{
	m_key_scan = (m_key_scan & 0x3f00) | data;
}

//**************************************************************************
//  CONFIGURATION AND ROM DEFINITION
//**************************************************************************

void ampex230_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 4.9152_MHz_XTAL); // TMP8048P-1251
	m_mcu->bus_in_cb().set(FUNC(ampex230_keyboard_device::bus_r));
	m_mcu->t0_in_cb().set_ioport("MODIFIERS");
	m_mcu->t1_in_cb().set_ioport("MODIFIERS").rshift(1);
	m_mcu->p1_out_cb().set(FUNC(ampex230_keyboard_device::p1_w));
	m_mcu->p2_out_cb().set(FUNC(ampex230_keyboard_device::p2_w));
}

ROM_START(ampex230_kbd)
	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("3515245-05.u1", 0x000, 0x400, CRC(f8133738) SHA1(46953d6b511ee37ca0aa2316af39c875b22ee2a7))
ROM_END

const tiny_rom_entry *ampex230_keyboard_device::device_rom_region() const
{
	return ROM_NAME(ampex230_kbd);
}
