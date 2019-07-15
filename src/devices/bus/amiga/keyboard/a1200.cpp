// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Amiga 1200 Keyboard

    391508-01 = Rev 0 is MC68HC05C4AFN
    391508-02 = Rev 1 is MC68HC05C12FN

    The ROMs from the mask-programmed MPUs used by Commodore are not
    dumped.  The program here is for the UVEPROM MC68HC705C8A.  It will
    not work on the mask-programmed MPUs used by Commodore, due to
    differences in the onboard watchdog hardware.

    If /IRQ is tied low, this program supports a Num Lock mode mapping
    the numeric keypad over the main keyboard for compact keyboards that
    lack a physical numeric keypad.  To enable it, hold Ctrl and press
    Caps Lock; to disable it, press Caps Lock.  The Caps Lock LED will
    be lit and blink off every 22 seconds or so while Num Lock mode is
    active.  The following keys are remapped while Num Lock is active:
    * 04  4       ->  5A  Keypad (
    * 05  5       ->  5B  Keypad )
    * 06  6       ->  5C  Keypad /
    * 07  7       ->  3D  Keypad 7
    * 08  8       ->  3E  Keypad 8
    * 09  9       ->  3F  Keypad 9
    * 0A  0       ->  5D  Keypad *
    * 16  U       ->  2D  Keypad 4
    * 17  I       ->  2E  Keypad 5
    * 18  O       ->  2F  Keypad 6
    * 19  P       ->  4A  Keypad -
    * 26  J       ->  1D  Keypad 1
    * 27  K       ->  1E  Keypad 2
    * 28  L       ->  1F  Keypad 3
    * 29  ;       ->  5E  Keypad +
    * 37  M       ->  0F  Keypad 0
    * 39  .       ->  3C  Keypad .
    * 44  Return  ->  43  Keypad Enter

    Switching between full size and compact modes is currently
    implemented as a configuration option, but it should be split off
    as a separate device without the numeric keypad.

***************************************************************************/

#include "emu.h"
#include "a1200.h"
#include "matrix.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_NS(A1200_KBD, bus::amiga::keyboard, a1200_kbd_device, "a1200kbd_rb", "Amiga 1200 Keyboard Rev B")



namespace bus { namespace amiga { namespace keyboard {

namespace {

INPUT_PORTS_START(a1200_mod)
	PORT_START("MOD")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)        PORT_CHAR(UCHAR_MAMEKEY(LWIN))       PORT_NAME("Left Amiga")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(LALT))       PORT_NAME("Left Alt")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)             PORT_NAME("Left Shift")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))   PORT_NAME("Ctrl")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)        PORT_CHAR(UCHAR_MAMEKEY(RWIN))       PORT_NAME("Right Amiga")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_SHIFT_2)             PORT_NAME("Right Alt")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))     PORT_NAME("Right Shift")
INPUT_PORTS_END

INPUT_PORTS_START(a1200_us_keyboard)
	PORT_INCLUDE(matrix_us)
	PORT_INCLUDE(a1200_mod)

	// FIXME: split compact mode into a separate device without the numeric keypad
	PORT_START("IRQ")
	PORT_CONFNAME(0x01, 0x01, "Layout") PORT_CHANGED_MEMBER(DEVICE_SELF, a1200_kbd_device, layout_changed, 0)
	PORT_CONFSETTING(0x01, "Full Size")
	PORT_CONFSETTING(0x00, "Compact")
INPUT_PORTS_END


ROM_START(a1200kbd_revB)
	ROM_REGION(0x2000, "mpu", 0)
	ROM_LOAD("dfa_rev_b_a1200_hc705.bin", 0x0000, 0x2000, CRC(2a77eec4) SHA1(301ec6a69404457d912c89e3fc54095eda9f0e93))
ROM_END

} // anonymous namespace



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a1200_kbd_device::a1200_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, A1200_KBD, tag, owner, clock)
	, device_amiga_keyboard_interface(mconfig, *this)
	, m_rows(*this, "ROW%u", 0)
	, m_mpu(*this, "mpu")
	, m_row_drive(0xffff)
	, m_host_kdat(true)
	, m_mpu_kdat(true)
{
}

WRITE_LINE_MEMBER(a1200_kbd_device::kdat_w)
{
	if (bool(state) != m_host_kdat)
	{
		LOG("host DATA %u -> %u\n", m_host_kdat ? 1 : 0, state ? 1 : 0);
		m_host_kdat = bool(state);
		if (m_mpu_kdat)
			m_mpu->set_input_line(M68HC05_TCAP_LINE, m_host_kdat ? 1 : 0);
	}
}

INPUT_CHANGED_MEMBER(a1200_kbd_device::layout_changed)
{
	m_mpu->set_input_line(M68HC05_IRQ_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

READ8_MEMBER(a1200_kbd_device::mpu_portb_r)
{
	u8 result(m_host_kdat ? 0xff : 0xfe);
	for (unsigned row = 0; m_rows.size() > row; ++row)
	{
		if (!BIT(m_row_drive, row))
			result &= m_rows[row]->read();
	}
	return result;
}

WRITE8_MEMBER(a1200_kbd_device::mpu_porta_w)
{
	m_row_drive = (m_row_drive & 0xff00) | u16(u8(data | ~mem_mask));
}

WRITE8_MEMBER(a1200_kbd_device::mpu_portb_w)
{
	u8 const kdat(BIT(data, 0) | BIT(~mem_mask, 0));
	m_host->kdat_w(kdat ? 1 : 0);
	m_host->kclk_w(BIT(data, 1));

	if (bool(kdat) != m_mpu_kdat)
	{
		LOG("keyboard DATA %u -> %u\n", m_mpu_kdat ? 1 : 0, kdat);
		m_mpu_kdat = bool(kdat);
		if (m_host_kdat)
			m_mpu->set_input_line(M68HC05_TCAP_LINE, kdat);
	}
}

WRITE8_MEMBER(a1200_kbd_device::mpu_portc_w)
{
	m_row_drive = (m_row_drive & 0x80ff) | (u16(u8(data | ~mem_mask) & 0x7f) << 8);
	machine().output().set_value("led_kbd_caps", BIT(~data, 7));
}

WRITE_LINE_MEMBER(a1200_kbd_device::mpu_tcmp)
{
	m_host->krst_w(state);
}

void a1200_kbd_device::device_add_mconfig(machine_config &config)
{
	M68HC705C8A(config, m_mpu, XTAL(3'000'000));
	m_mpu->port_r<1>().set(FUNC(a1200_kbd_device::mpu_portb_r));
	m_mpu->port_r<3>().set_ioport("MOD");
	m_mpu->port_w<0>().set(FUNC(a1200_kbd_device::mpu_porta_w));
	m_mpu->port_w<1>().set(FUNC(a1200_kbd_device::mpu_portb_w));
	m_mpu->port_w<2>().set(FUNC(a1200_kbd_device::mpu_portc_w));
	m_mpu->tcmp().set(FUNC(a1200_kbd_device::mpu_tcmp));
}

tiny_rom_entry const *a1200_kbd_device::device_rom_region() const
{
	return ROM_NAME(a1200kbd_revB);
}

ioport_constructor a1200_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a1200_us_keyboard);
}

void a1200_kbd_device::device_start()
{
	save_item(NAME(m_row_drive));
	save_item(NAME(m_host_kdat));
	save_item(NAME(m_mpu_kdat));

	m_row_drive = 0xffff;
	m_host_kdat = true;
	m_mpu_kdat = true;
}

void a1200_kbd_device::device_reset_after_children()
{
	m_mpu->set_input_line(M68HC05_IRQ_LINE, BIT(ioport("IRQ")->read(), 0) ? CLEAR_LINE : ASSERT_LINE);
}

} } } // namespace bus::amiga::keyboard
