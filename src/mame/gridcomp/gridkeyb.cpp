// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Sergey Svishchev
/***************************************************************************

    GRiD Compass keyboard

***************************************************************************/

#include "emu.h"

#include "gridkeyb.h"

/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_START( grid_keyboard )
	PORT_START("GRIDKBD_MOD")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Code")       PORT_CODE(KEYCODE_LALT)     PORT_CODE(KEYCODE_RALT)                 PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("GRIDKBD_COL0")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)       PORT_CHAR('\t')
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)       PORT_NAME("Escape") PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0xfCU, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GRIDKBD_COL1")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)     PORT_NAME("Return") PORT_CHAR(0x0dU)
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)      PORT_NAME("LeftArrow")
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("GRIDKBD_COL2")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("GRIDKBD_COL3")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("GRIDKBD_COL4")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("GRIDKBD_COL5")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("GRIDKBD_COL6")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("GRIDKBD_COL7")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(0x08U)
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)      PORT_NAME("DownArrow")
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)     PORT_NAME("RightArrow")
	PORT_BIT( 0x10U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)        PORT_NAME("UpArrow")
	PORT_BIT( 0x20U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x80U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')')
INPUT_PORTS_END



/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DEFINE_DEVICE_TYPE(GRID_KEYBOARD, grid_keyboard_device, "grid_keyboard", "GRiD Compass Keyboard")

ROM_START(grid_keyboard)
	ROM_REGION(0x0400, "mcu", 0)
	ROM_DEFAULT_BIOS("1101")

	// The 1101 keyboard firmware is used by all Compass systems.
	ROM_SYSTEM_BIOS(0, "1101", "GRiD 1101")
	ROMX_LOAD("300067-01.bin", 0x0000, 0x0400, CRC(48cb0b57) SHA1(3163a4209093c4cd9498db5d89ac9bb7c2ca2a6c), ROM_BIOS(0))

	// The 1137 firmware slightly differs and appears to be a rebuild with a newer compiler.
	ROM_SYSTEM_BIOS(1, "1137", "GRiD 1137")
	ROMX_LOAD("300001-02.bin", 0x0000, 0x0400, CRC(cebbccdc) SHA1(67ff291457ae5c97d4c3e6b3b96aa8cb8378552c), ROM_BIOS(1))
ROM_END

grid_keyboard_device::grid_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GRID_KEYBOARD, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_columns(*this, "GRIDKBD_COL%u", 0U)
	, m_modifiers(*this, "GRIDKBD_MOD")
	, m_irq_cb(*this)
	, m_dma_cb(*this)
	, m_nmi_cb(*this)
{
}

ioport_constructor grid_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(grid_keyboard);
}

void grid_keyboard_device::device_start()
{
}

void grid_keyboard_device::device_add_mconfig(machine_config &config)
{
	i8741a_device &mcu(I8741A(config, m_mcu, XTAL(5'000'000)));
	mcu.p1_in_cb().set(FUNC(grid_keyboard_device::p1_r));
	mcu.p2_in_cb().set_constant(0xff);
	mcu.p2_out_cb().set(FUNC(grid_keyboard_device::p2_w));
	mcu.t0_in_cb().set(FUNC(grid_keyboard_device::t0_r));
	mcu.t1_in_cb().set(FUNC(grid_keyboard_device::t1_r));
}

const tiny_rom_entry *grid_keyboard_device::device_rom_region() const
{
	return ROM_NAME(grid_keyboard);
}

u8 grid_keyboard_device::read(offs_t offset)
{
	return m_mcu->upi41_master_r(offset & 1);
}

void grid_keyboard_device::write(offs_t offset, u8 data)
{
	m_mcu->upi41_master_w(offset & 1, data);
}

void grid_keyboard_device::p2_w(u8 data)
{
	m_irq_cb(BIT(data, 3));
	m_dma_cb(BIT(data, 4));
	m_nmi_cb(BIT(data, 6));
}

u8 grid_keyboard_device::p1_r()
{
	unsigned const column = m_mcu->p2_r() & 0x07;
	u8 data = ~u8(m_columns[column]->read());

	// Ctrl is wired to P1.0 while the firmware selects decoder output 2.
	if (column == 2 && BIT(m_modifiers->read(), 0))
		data &= ~0x01;

	return data;
}

int grid_keyboard_device::t0_r()
{
	return !BIT(m_modifiers->read(), 1); // Shift, active low
}

int grid_keyboard_device::t1_r()
{
	return !BIT(m_modifiers->read(), 3); // Code, active low
}
