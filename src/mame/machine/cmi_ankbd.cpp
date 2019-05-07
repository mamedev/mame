// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight Intelligent Alphanumeric Keyboard

    This ASCII serial keyboard was developed for Fairlight's Qasar System.
    The output signals are driven at RS232 levels, though the connector is
    custom.

***************************************************************************/

#include "emu.h"
#include "cmi_ankbd.h"

#include "cpu/m6800/m6800.h"
#include "machine/clock.h"
#include "machine/input_merger.h"

DEFINE_DEVICE_TYPE(CMI_ALPHANUMERIC_KEYBOARD, cmi_alphanumeric_keyboard_device, "cmi_ankbd", "Fairlight Intelligent Alphanumeric Keyboard")

cmi_alphanumeric_keyboard_device::cmi_alphanumeric_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CMI_ALPHANUMERIC_KEYBOARD, tag, owner, clock)
	, m_txd_handler(*this)
	, m_rts_handler(*this)
	, m_kbdcpu(*this, "kbdcpu")
	, m_pia(*this, "pia")
	, m_row_ports(*this, "ROW%u", 0)
{
}

void cmi_alphanumeric_keyboard_device::device_resolve_objects()
{
	m_txd_handler.resolve_safe();
	m_rts_handler.resolve_safe();
}

void cmi_alphanumeric_keyboard_device::device_start()
{
}

READ8_MEMBER( cmi_alphanumeric_keyboard_device::col_r )
{
	int row = m_pia->b_output() ^ 0xff;

	switch (row)
	{
		case 0x01: return m_row_ports[0]->read();
		case 0x02: return m_row_ports[1]->read();
		case 0x04: return m_row_ports[2]->read();
		case 0x08: return m_row_ports[3]->read();
		case 0x10: return m_row_ports[4]->read();
		case 0x20: return m_row_ports[5]->read();
		case 0x40: return m_row_ports[6]->read();
		case 0x80: return m_row_ports[7]->read();
		default:   return 0xff;
	}
}

WRITE_LINE_MEMBER( cmi_alphanumeric_keyboard_device::rxd_w )
{
	m_pia->cb2_w(state);
}

WRITE_LINE_MEMBER( cmi_alphanumeric_keyboard_device::cts_w )
{
	m_pia->ca1_w(state);
}

WRITE_LINE_MEMBER( cmi_alphanumeric_keyboard_device::txd_w )
{
	m_txd_handler(state);
}

WRITE_LINE_MEMBER( cmi_alphanumeric_keyboard_device::rts_w )
{
	m_rts_handler(state);
}

void cmi_alphanumeric_keyboard_device::alphakeys_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
	map(0x4000, 0x7fff).portr("OPTIONS");
	map(0x8000, 0xbfff).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xc3ff).rom().mirror(0x3c00);
}

static INPUT_PORTS_START( cmi_alphanumeric_keyboard )
	/* Alphanumeric keyboard */
	PORT_START("OPTIONS")
	PORT_DIPNAME( 0x07, 0x00, "Speed (baud)" )
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600"  )
	PORT_DIPSETTING(    0x05, "300"  )
	PORT_DIPSETTING(    0x06, "150"  )
	PORT_DIPSETTING(    0x07, "110"  )

	PORT_DIPNAME( 0x30, 0x20, "Parity" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPSETTING(    0x10, "None, bit 7 is 0" )
	PORT_DIPSETTING(    0x20, "Odd" )
	PORT_DIPSETTING(    0x30, "None, bit 7 is 1" )

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)            PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)         PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)            PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                PORT_CHAR('9')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)            PORT_NAME("Right")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)               PORT_NAME("Up")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)        PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                PORT_CHAR('0')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)              PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Set")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Add")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)            PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)           PORT_NAME("LShift")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)             PORT_NAME("Down")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Clear")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("WTF")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                PORT_CHAR('L')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)           PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Home")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)        PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                PORT_CHAR('K')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)            PORT_NAME("Return (a)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)            PORT_NAME("Return (b)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                PORT_CHAR('I')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Sub")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)            PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)           PORT_NAME("RShift")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)             PORT_CHAR('.')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)             PORT_NAME("Left")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                PORT_CHAR('O')
INPUT_PORTS_END

ioport_constructor cmi_alphanumeric_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cmi_alphanumeric_keyboard);
}

void cmi_alphanumeric_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_kbdcpu, 3.84_MHz_XTAL);
	m_kbdcpu->set_addrmap(AS_PROGRAM, &cmi_alphanumeric_keyboard_device::alphakeys_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_kbdcpu, M6802_IRQ_LINE);

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(cmi_alphanumeric_keyboard_device::col_r));
	m_pia->ca2_handler().set(FUNC(cmi_alphanumeric_keyboard_device::rts_w));
	m_pia->cb2_handler().set(FUNC(cmi_alphanumeric_keyboard_device::txd_w));
	m_pia->irqa_handler().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_pia->irqb_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	clock_device &pia_clock(CLOCK(config, "pia_clock", 3.84_MHz_XTAL / 4 / 100)); // E clock divided by MC14518
	pia_clock.signal_handler().set(m_pia, FUNC(pia6821_device::cb1_w));
}

ROM_START( cmi_ankbd )
	// This dump has been trimmed to size from within a roughly 2x-bigger file. The actual size is known based
	// on the format apparently used by the dumping device.
	ROM_REGION( 0x10000, "kbdcpu", 0 )
	ROM_LOAD( "cmikeys4.bin", 0xc000, 0x400, CRC(b214fbe9) SHA1(8c404f58ba3e5a50aa42f761e966c74374e96cc9) )
ROM_END

const tiny_rom_entry *cmi_alphanumeric_keyboard_device::device_rom_region() const
{
	return ROM_NAME(cmi_ankbd);
}
