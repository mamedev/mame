// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Amiga 1200 Keyboard

    391508-01 = Rev 0 is MC68HC05C4AFN
    391508-02 = Rev 1 is MC68HC05C12FN

***************************************************************************/

#include "emu.h"
#include "a1200.h"
#include "matrix.h"

#include "cpu/m6805/m68hc05.h"

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

MACHINE_CONFIG_START(a1200_kbd_device::device_add_mconfig)
	MCFG_DEVICE_ADD("mpu", M68HC705C8A, XTAL(3'000'000))
	MCFG_M68HC05_PORTB_R_CB(READ8(*this, a1200_kbd_device, mpu_portb_r));
	MCFG_M68HC05_PORTD_R_CB(IOPORT("MOD"));
	MCFG_M68HC05_PORTA_W_CB(WRITE8(*this, a1200_kbd_device, mpu_porta_w));
	MCFG_M68HC05_PORTB_W_CB(WRITE8(*this, a1200_kbd_device, mpu_portb_w));
	MCFG_M68HC05_PORTC_W_CB(WRITE8(*this, a1200_kbd_device, mpu_portc_w));
	MCFG_M68HC05_TCMP_CB(WRITELINE(*this, a1200_kbd_device, mpu_tcmp));
MACHINE_CONFIG_END

const tiny_rom_entry *a1200_kbd_device::device_rom_region() const
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

void a1200_kbd_device::device_reset()
{
}

} } } // namespace bus::amiga::keyboard
