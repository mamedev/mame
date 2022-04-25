// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*********************************************************************

    a9m0330.cpp
    Apple model A9M0330 ADB keyboard for the Apple IIgs
    by R. Belmont, with hints from pluskbd.cpp by Vas Crabb

    Port 1: Row selects 0-7
    Port 2: Bit 0: row select bit 8
            Bit 1: row select bit 9
            Bit 2: Shift
            Bit 3: Caps Lock
            Bit 4: Option
            Bit 5: Command
            Bit 6: Reset/Power? Control?
            Bit 7: ADB out
    Bus: Matrix read input
    T1: ADB in

*********************************************************************/

#include "emu.h"
#include "a9m0330.h"

DEFINE_DEVICE_TYPE(ADB_A9M0330, a9m0330_device, "a9m0330", "Apple IIgs ADB Keyboard (A9M0330)");

ROM_START(a9m0330)
	ROM_REGION(0x400, "mcu", 0)
	// from early-production ROM 00 Woz Limited Edition IIgs.  keyboard "Part Number 658-4081  825-1301-A"
	// ROM is marked "NEC Japan  8626XD 341-0232A  543" so 26th week of 1986
	ROM_LOAD("341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76))
	// from later non-Woz ROM 01.  keyboard "Model A9M0330"
	// ROM is marked "NEC Japan 8806HD  8048HC610  341-0124-A  (c) APPLE 87" so 6th week of 1988
	ROM_LOAD("341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013))
ROM_END

// matrix is believed to be the same as the Mac Plus keyboard at the moment
static INPUT_PORTS_START(a9m0330)
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))   PORT_NAME("Keypad *")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))      PORT_NAME("Keypad 8")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')   PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')   PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')   PORT_CHAR('@')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')   PORT_CHAR('V')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\')  PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))  PORT_NAME("Keypad /")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))      PORT_NAME("Keypad 9")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')   PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')   PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')   PORT_CHAR('B')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) PORT_NAME("Keypad =")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))  PORT_NAME("Keypad -")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')   PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')   PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')   PORT_CHAR('N')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))    PORT_NAME("Keypad Clear")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')   PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')   PORT_CHAR('M')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                      PORT_NAME("Backspace")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))      PORT_NAME("Keypad 1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x0d)                      PORT_NAME("Return")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')   PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')   PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')   PORT_CHAR('<')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))      PORT_NAME("Keypad 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')   PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))      PORT_NAME("Keypad 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))      PORT_NAME("Keypad 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')   PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')   PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR('>')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))    PORT_NAME("Keypad .")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))      PORT_NAME("Keypad 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))      PORT_NAME("Keypad 5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')   PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')   PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')   PORT_CHAR('?')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))  PORT_NAME("Keypad Enter")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')   PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))   PORT_NAME("Keypad +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))      PORT_NAME("Keypad 6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')   PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')   PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')   PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                      PORT_NAME("Tab")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')   PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')   PORT_CHAR('X')

	PORT_START("ROW9")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))      PORT_NAME("Keypad 7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')   PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')   PORT_CHAR('C')

	PORT_START("P2")
	PORT_BIT(0x43, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)           PORT_NAME("Shift")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)                         PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps Lock") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)   PORT_CODE(KEYCODE_RALT)   PORT_CHAR(UCHAR_SHIFT_2)           PORT_NAME("Option")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                                                            PORT_NAME("Command")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    input_ports - device-specific input ports
-------------------------------------------------*/
ioport_constructor a9m0330_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a9m0330);
}

/*-------------------------------------------------
    device_add_mconfig - device-specific
    machine configurations
-------------------------------------------------*/
void a9m0330_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 6_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &a9m0330_device::program_map);
	m_mcu->bus_in_cb().set(FUNC(a9m0330_device::bus_r));
	m_mcu->p1_out_cb().set(FUNC(a9m0330_device::p1_w));
	m_mcu->p2_in_cb().set_ioport("P2");
	m_mcu->p2_out_cb().set(FUNC(a9m0330_device::p2_w));
	m_mcu->t0_in_cb().set(FUNC(a9m0330_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(a9m0330_device::t1_r));
}

/*-------------------------------------------------
    rom_region - device-specific ROM region
-------------------------------------------------*/
const tiny_rom_entry *a9m0330_device::device_rom_region() const
{
	return ROM_NAME(a9m0330);
}

/***************************************************************************
    DEVICE IMPLEMENTATION
***************************************************************************/

a9m0330_device::a9m0330_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	adb_device(mconfig, ADB_A9M0330, tag, owner, clock),
	adb_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_mcu(*this, "mcu")
	, m_rows{ *this, "ROW%u", 0U }
{
}

void a9m0330_device::device_start()
{
	adb_device::device_start();

	save_item(NAME(m_adb_state));
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_our_last_adb_state));
}

void a9m0330_device::device_reset()
{
	adb_device::device_reset();

	m_our_last_adb_state = 1;
	m_adb_state = 1;
	m_kbd_row = 0x3ff;
}

void a9m0330_device::adb_w(int state)
{
	m_adb_state = state;
}

void a9m0330_device::program_map(address_map &map)
{
	map(0x0000, 0x03ff).rom().region("mcu", 0);
}

u8 a9m0330_device::bus_r()
{
	u8 result = 0xffU;

	for (unsigned int i = 0U; m_rows.size() > i; i++)
	{
		if (!BIT(m_kbd_row, i))
		{
			result &= m_rows[i]->read();
		}
	}

	return result;
}

void a9m0330_device::p1_w(u8 data)
{
	m_kbd_row &= 0x300;
	m_kbd_row |= data;
}

void a9m0330_device::p2_w(u8 data)
{
	m_kbd_row &= 0xff;
	m_kbd_row |= (data & 0x03) << 8;

	// ADB drive is through an inverting transistor
	int adb_state = (data>>7) ^ 1;
	if (adb_state != m_our_last_adb_state)
	{
		m_adb_cb(adb_state);
		m_our_last_adb_state = adb_state;
	}
}

DECLARE_READ_LINE_MEMBER(a9m0330_device::t0_r)
{
	// appears to be a config jumper of some sort
	return 0;
}

DECLARE_READ_LINE_MEMBER(a9m0330_device::t1_r)
{
	return m_adb_state & 1;
}
