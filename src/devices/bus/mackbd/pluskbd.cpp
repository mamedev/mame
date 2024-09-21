// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    Apple M0110A keyboard with integrated keypad

    This keyboard emulates an M0120 keypad with an M0110 keyboard
    plugged in to it.  Keypad keys and arrow keys produce scan codes
    with the 0x79 prefix.  The keyboard simulates holding shift when
    pressing the = / * + keys on the keypad.

    This keyboard was only available in ANSI layout, no ISO layout
    variants were made.  International markets continued to receive the
    M0110/M0120 keyboard/keypad.  International variants of the M0110A
    were eventually produced, differing only in key cap labels.  All
    keys have the same shapes, sizes, positions and scan codes as they
    do on the U.S. version.

    Emulation based entirely on examining the MPU program and observing
    behaviour.  There may be additional hardware in the keyboard that is
    not emulated (e.g. a watchdog timer).

    +-----------+---------------+
    | Pin       | Keyboard      |
    +-----------+---------------+
    | P10 (27)  | row drive     |
    | P11 (28)  | row drive     |
    | P12 (29)  | row drive     |
    | P13 (30)  | row drive     |
    | P14 (31)  | row drive     |
    | P15 (32)  | row drive     |
    | P16 (33)  | row drive     |
    | P17 (34)  | row drive     |
    +-----------+---------------+
    | P20 (21)  | row drive     |
    | P21 (22)  | row drive     |
    | P22 (23)  | Shift         |
    | P23 (24)  | Caps Lock     |
    | P24 (35)  | Option        |
    | P25 (36)  | Command       |
    | P26 (37)  | host clock    |
    | P27 (38)  | host data     |
    +-----------+---------------+
    | DB0 (12)  | column read   |
    | DB1 (13)  | column read   |
    | DB2 (14)  | column read   |
    | DB3 (15)  | column read   |
    | DB4 (16)  | column read   |
    | DB5 (17)  | column read   |
    | DB6 (18)  | column read   |
    | DB6 (19)  | column read   |
    +-----------+---------------+

    +-----+-----------------------------------------------------------+
    |     | P10   P11   P12   P13   P14   P15   P16   P17   P20   P21 |
    +-----+-----------------------------------------------------------+
    | DB0 |        \    Left Right  Down  KP0   KP.   Ent             |
    | DB1 | KP*   KP/   KP=  Clear  Bsp    =     -     0     Z        |
    | DB2 | KP8   KP9   KP-    Up   KP1   KP2   KP3   KP+         KP7 |
    | DB3 |  P     [     ]     '    Rtn   KP4   KP5   KP6   Spc       |
    | DB4 |  D     F     G     H     J     K     L     ;     A     S  |
    | DB5 |  W     E     R     T     Y     U     I     O    Tab    Q  |
    | DB6 |  2     3     4     5     6     7     8     9     `     1  |
    | DB7 |  V     B     N     M     ,     .     /           X     C  |
    +-----+-----------------------------------------------------------+

    Known part numbers:
    * M0110A (U.S.)
    * M0110A F (French)
    * M0110A J (Japanese)

    The French version has icons rather than English text for Backspace,
    Tab, Caps Lock, Return, Shift, Option, Clear and Enter.

    The Japanese version has katakana labels on the key caps in addition
    to the Latin labels, ¥ (Yen) replacing \ (backslash), and カナ
    (kana) replacing Caps Lock.  It still has the ANSI "typewriter
    shift" arrangement for Latin characters and ASCII punctuation (it
    doesn't use the JIS "bit shift" arrangement).

***************************************************************************/

#include "emu.h"
#include "pluskbd.h"

#include "cpu/mcs48/mcs48.h"

#define LOG_MATRIX      (1U << 1)
#define LOG_COMM        (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_MATRIX | LOG_COMM)
//#define LOG_OUTPUT_STREAM std::cerr
#include "logmacro.h"

#define LOGMATRIX(...)      LOGMASKED(LOG_MATRIX, __VA_ARGS__)
#define LOGCOMM(...)        LOGMASKED(LOG_COMM, __VA_ARGS__)


namespace {

ROM_START(keyboard)
	ROM_REGION(0x0400, "mpu", 0)
	ROM_LOAD("341-0332-a.bin", 0x000000, 0x000400, CRC(6554f5b6) SHA1(a80404a122d74721cda13b285c412057c2c78bd7))
ROM_END


class keyboard_base : public device_t, public device_mac_keyboard_interface
{
public:
	ioport_value host_data_r()
	{
		return m_host_data_in ^ 0x01;
	}

protected:
	keyboard_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_mac_keyboard_interface(mconfig, *this)
		, m_mpu{ *this, "mpu" }
		, m_rows{ *this, "ROW%u", 0U }
	{
	}

	virtual tiny_rom_entry const *device_rom_region() const override
	{
		return ROM_NAME(keyboard);
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		I8048(config, m_mpu, 6_MHz_XTAL); // NEC 8048HC517 341-0332-A with ceramic resonator
		m_mpu->set_addrmap(AS_IO, &keyboard_base::mpu_io);
		m_mpu->p1_out_cb().set(FUNC(keyboard_base::p1_w));
		m_mpu->p2_in_cb().set_ioport("P2");
		m_mpu->p2_out_cb().set(FUNC(keyboard_base::p2_w));
		m_mpu->bus_in_cb().set(FUNC(keyboard_base::bus_r));
	}

	virtual void device_start() override
	{
		m_row_drive = 0x03ffU;
		m_host_clock_out = 1U;
		m_host_data_out = 1U;
		m_host_data_in = 0x01U;

		save_item(NAME(m_row_drive));
		save_item(NAME(m_host_clock_out));
		save_item(NAME(m_host_data_out));
		save_item(NAME(m_host_data_in));
	}

	virtual void data_w(int state) override
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(keyboard_base::update_host_data), this), state);
	}

private:
	void p1_w(u8 data)
	{
		m_row_drive = (m_row_drive & 0x0300) | u16(data);
	}

	void p2_w(u8 data)
	{
		m_row_drive = (m_row_drive & 0x00ff) | (u16(data & 0x03) << 8);

		if (BIT(data, 6) != m_host_clock_out)
		{
			if (BIT(data, 6))
				LOGCOMM("%s: host clock out 0 -> 1 data=%u\n", machine().describe_context(), (m_host_data_out && m_host_data_in) ? 1U : 0U);
			else
				LOGCOMM("%s: host clock out 1 -> 0\n", machine().describe_context());
			write_clock(m_host_clock_out = BIT(data, 6));
		}

		if (BIT(data, 7) != m_host_data_out)
		{
			LOGCOMM("%s: host data out %u -> %u\n", machine().describe_context(), m_host_data_out, BIT(data, 7));
			write_data(m_host_data_out = BIT(data, 7));
		}
	}

	u8 bus_r()
	{
		u8 result(0xffU);
		for (unsigned i = 0U; m_rows.size() > i; ++i)
		{
			if (!BIT(m_row_drive, i))
				result &= m_rows[i]->read();
		}
		LOGMATRIX("read matrix: row drive = %X, result = %X\n", m_row_drive, result);
		return result;
	}

	void mpu_io(address_map &map)
	{
		// MPU writes 0xff to these addresses before reading columns to ensure the bus pins aren't pulled down
		map(0x2a, 0x33).nopw();
	}

	TIMER_CALLBACK_MEMBER(update_host_data)
	{
		if (bool(param) != bool(m_host_data_in))
		{
			LOGCOMM("host data in %u -> %u\n", m_host_data_in, param ? 1U : 0U);
			m_host_data_in = param ? 0x01U : 0x00U;
		}
	}

	required_device<mcs48_cpu_device> m_mpu;
	required_ioport_array<10> m_rows;

	u16 m_row_drive         = 0x03ffU;      // current bit pattern driving rows (active low)
	u8  m_host_clock_out    = 1U;           // clock line drive to host (idle high)
	u8  m_host_data_out     = 1U;           // data line drive to host (idle high)
	u8  m_host_data_in      = 0x01U;        // data line drive from host (idle high)
};


INPUT_PORTS_START(keyboard_us)
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
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(keyboard_base, host_data_r)
INPUT_PORTS_END

INPUT_PORTS_START(keyboard_fr)
	PORT_INCLUDE(keyboard_us)

	PORT_MODIFY("ROW0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('z')   PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR(U'é')  PORT_CHAR('2')

	PORT_MODIFY("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('<')   PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'ˆ')  PORT_CHAR(U'¨')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('"')   PORT_CHAR('3')

	PORT_MODIFY("ROW2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$')   PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('\'')  PORT_CHAR('4')

	PORT_MODIFY("ROW3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ù')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('(')   PORT_CHAR('5')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR(',')   PORT_CHAR('?')

	PORT_MODIFY("ROW4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR(U'§')  PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(';')   PORT_CHAR('.')

	PORT_MODIFY("ROW5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR(U'è')  PORT_CHAR('7')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR(':')   PORT_CHAR('/')

	PORT_MODIFY("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))    PORT_NAME("Keypad ,")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(')')   PORT_CHAR(0xb0)     // °
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('!')   PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('=')   PORT_CHAR('+')

	PORT_MODIFY("ROW7")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR(U'à')  PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('m')   PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR(U'ç')  PORT_CHAR('9')

	PORT_MODIFY("ROW8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('w')   PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('q')   PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')   PORT_CHAR(U'£')

	PORT_MODIFY("ROW9")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('&')   PORT_CHAR('1')
INPUT_PORTS_END

INPUT_PORTS_START(keyboard_jp)
	PORT_INCLUDE(keyboard_us)

	PORT_MODIFY("ROW0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')   PORT_CHAR('P')      PORT_NAME(u8"P  \u30bb")                // セ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')      PORT_NAME(u8"D  \u30b7")                // シ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')   PORT_CHAR('W')      PORT_NAME(u8"W  \u30c6")                // テ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')   PORT_CHAR('@')      PORT_NAME(u8"2  @  \u30d5")             // フ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')   PORT_CHAR('V')      PORT_NAME(u8"V  \u30d2")                // ヒ

	PORT_MODIFY("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(U'¥')  PORT_CHAR('|')      PORT_NAME(u8"¥  |  \u30d8")             // ヘ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')   PORT_CHAR('{')      PORT_NAME(u8"[  {  \u309b  \u300d")     // ゛ 」
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')      PORT_NAME(u8"F  \u30cf")                // ハ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')   PORT_CHAR('E')      PORT_NAME(u8"E  \u30a4  \u30a3")        // イ ィ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')   PORT_CHAR('#')      PORT_NAME(u8"3  #  \u30a2  \u30a1")     // ア ァ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')   PORT_CHAR('B')      PORT_NAME(u8"B  \u30b3")                // コ

	PORT_MODIFY("ROW2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')   PORT_CHAR('}')      PORT_NAME(u8"]  }  \u30e0  \u30fc")     // ム ー
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')      PORT_NAME(u8"G  \u30ad")                // キ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')   PORT_CHAR('R')      PORT_NAME(u8"R  \u30b9")                // ス
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')   PORT_CHAR('$')      PORT_NAME(u8"4  $  \u30a6  \u30a5")     // ウ ゥ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')   PORT_CHAR('N')      PORT_NAME(u8"N  \u30df")                // ミ

	PORT_MODIFY("ROW3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')      PORT_NAME(u8"'  \"  \u30b1  \u30ed")    // ケ ロ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')      PORT_NAME(u8"H  \u30af")                // ク
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')   PORT_CHAR('T')      PORT_NAME(u8"T  \u30ab")                // カ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')   PORT_CHAR('%')      PORT_NAME(u8"5  %  \u30a8  \u30a7")     // エ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')   PORT_CHAR('M')      PORT_NAME(u8"M  \u30e2")                // モ

	PORT_MODIFY("ROW4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')      PORT_NAME(u8"J  \u30de")                // マ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')   PORT_CHAR('Y')      PORT_NAME(u8"Y  \u30f3")                // ン
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')   PORT_CHAR('^')      PORT_NAME(u8"6  ^  \u30aa  \u30a9")     // オ ォ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')   PORT_CHAR('<')      PORT_NAME(u8",  <  \u30cd  \u3001")     // ネ 、

	PORT_MODIFY("ROW5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')   PORT_CHAR('+')      PORT_NAME(u8"=  +  \u309c  \u300c")     // ゜ 「
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')      PORT_NAME(u8"K  \u30ce")                // ノ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')   PORT_CHAR('U')      PORT_NAME(u8"U  \u30ca")                // ナ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')   PORT_CHAR('&')      PORT_NAME(u8"7  &  \u30e4  \u30e3")     // ヤ ャ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR('>')      PORT_NAME(u8".  >  \u30eb  \u3002")     // ル 。

	PORT_MODIFY("ROW6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')   PORT_CHAR('_')      PORT_NAME(u8"-  _  \u30db")             // ホ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')      PORT_NAME(u8"L  \u30ea")                // リ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')   PORT_CHAR('I')      PORT_NAME(u8"I  \u30cb")                // ニ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')   PORT_CHAR('*')      PORT_NAME(u8"8  *  \u30e6  \u30e5")     // ユ ュ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')   PORT_CHAR('?')      PORT_NAME(u8"/  ?  \u30e1  \u30fb")     // メ ・

	PORT_MODIFY("ROW7")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')   PORT_CHAR(')')      PORT_NAME(u8"0  )  \u30ef  \u30f2")     // ワ ヲ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')      PORT_NAME(u8";  :  \u30ec")             // レ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')   PORT_CHAR('O')      PORT_NAME(u8"O  \u30e9")                // ラ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')   PORT_CHAR('(')      PORT_NAME(u8"9  (  \u30e8  \u30e7")     // ヨ ョ

	PORT_MODIFY("ROW8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')   PORT_CHAR('Z')      PORT_NAME(u8"Z  \u30c4  \u30c3")        // ツ ッ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')      PORT_NAME(u8"A  \u30c1")                // チ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')   PORT_CHAR('X')      PORT_NAME(u8"X  \u30b5")                // サ

	PORT_MODIFY("ROW9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')      PORT_NAME(u8"S  \u30c8")                // ト
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')   PORT_CHAR('Q')      PORT_NAME(u8"Q  \u30bf")                // タ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')   PORT_CHAR('!')      PORT_NAME(u8"1  !  \u30cc")             // ヌ
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')   PORT_CHAR('C')      PORT_NAME(u8"C  \u30bd")                // ソ

	PORT_MODIFY("P2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))   PORT_NAME(u8"\u30ab\u30ca (Kana)") PORT_TOGGLE // カナ
INPUT_PORTS_END


class m0110a_device : public keyboard_base
{
public:
	m0110a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: keyboard_base(mconfig, MACKBD_M0110A, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME(keyboard_us);
	}
};

class m0110a_f_device : public keyboard_base
{
public:
	m0110a_f_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: keyboard_base(mconfig, MACKBD_M0110A_F, tag, owner, clock)
	{
	}

	static auto parent_rom_device_type() { return &MACKBD_M0110A; }

protected:
	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME(keyboard_fr);
	}
};

class m0110a_j_device : public keyboard_base
{
public:
	m0110a_j_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: keyboard_base(mconfig, MACKBD_M0110A_J, tag, owner, clock)
	{
	}

	static auto parent_rom_device_type() { return &MACKBD_M0110A; }

protected:
	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME(keyboard_jp);
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MACKBD_M0110A  , device_mac_keyboard_interface, m0110a_device,   "mackbd_m0110a",   "Macintosh Plus Keyboard (U.S. - M0110A)")
DEFINE_DEVICE_TYPE_PRIVATE(MACKBD_M0110A_F, device_mac_keyboard_interface, m0110a_f_device, "mackbd_m0110a_f", "Macintosh Plus Keyboard (French - M0110A F)")
DEFINE_DEVICE_TYPE_PRIVATE(MACKBD_M0110A_J, device_mac_keyboard_interface, m0110a_j_device, "mackbd_m0110a_j", "Macintosh Plus Keyboard (Japanese - M0110A J)")
