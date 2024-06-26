// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macadb.cpp - handles various aspects of ADB on the Mac.

***************************************************************************/

#include "emu.h"
#include "macadb.h"

#define LOG_TALK_LISTEN     (1U << 1)
#define LOG_STATE           (1U << 2)
#define LOG_LINESTATE       (1U << 3)
#define VERBOSE             (0)

#include "logmacro.h"

// ADB states
static constexpr int32_t ADB_STATE_NEW_COMMAND  = 0;
static constexpr int32_t ADB_STATE_XFER_EVEN    = 1;
static constexpr int32_t ADB_STATE_XFER_ODD     = 2;
static constexpr int32_t ADB_STATE_IDLE         = 3;
static constexpr int32_t ADB_STATE_NOTINIT      = 4;

// ADB commands
static constexpr int ADB_CMD_RESET              = 0;
static constexpr int ADB_CMD_FLUSH              = 1;

// use 1 MHz base to get microseconds (hack: *2 to get old wrong HC05 numbers to all realign to reality)
static constexpr int adb_timebase = 2000000;

static constexpr int adb_short = 85;
static constexpr int adb_long = 157;
static constexpr int adb_srq = 300;     // too long annoys the M50753 PMUs but Egret/Cuda/PIC are fine with it

// ADB line states
enum
{
	// receive states
	LST_IDLE = 0,
	LST_ATTENTION,
	LST_BIT0,
	LST_BIT1,
	LST_BIT2,
	LST_BIT3,
	LST_BIT4,
	LST_BIT5,
	LST_BIT6,
	LST_BIT7,
	LST_TSTOP,
	LST_WAITT1T,
	LST_RCVSTARTBIT,
	LST_SRQNODATA,

	// send states
	LST_TSTOPSTART,
	LST_TSTOPSTARTa,
	LST_STARTBIT,
	LST_SENDBIT0,
	LST_SENDBIT0a,
	LST_SENDBIT1,
	LST_SENDBIT1a,
	LST_SENDBIT2,
	LST_SENDBIT2a,
	LST_SENDBIT3,
	LST_SENDBIT3a,
	LST_SENDBIT4,
	LST_SENDBIT4a,
	LST_SENDBIT5,
	LST_SENDBIT5a,
	LST_SENDBIT6,
	LST_SENDBIT6a,
	LST_SENDBIT7,
	LST_SENDBIT7a,
	LST_SENDSTOP,
	LST_SENDSTOPa
};

// device type definition
DEFINE_DEVICE_TYPE(MACADB, macadb_device, "macadb", "Mac ADB HLE")

static INPUT_PORTS_START( macadb )
	PORT_START("MOUSE0") /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("MOUSE1") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE2") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	/* This handles most of the Apple Extended ADB keyboard.  F12 defaults to the RESET/POWER key, but real F12 is avaiable to be mapped too. */
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)    /* extra key on ISO : */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)    /* keyboard Enter : */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")     PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command / Open Apple") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option / Solid Apple") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down Arrow") PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up Arrow") PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */

	/* keypad */
	PORT_START("KEY4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x40
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   // 0x41
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x42
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)          PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // 0x43
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x44
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) // 0x45
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x46
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))     // 0x47
	PORT_BIT(0x0700, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x48, 49, 4a
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // 0x4b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // 0x4c
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4d
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // 0x4e
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4f

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x50
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD)        PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) // 0x51
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) // 0x52
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) // 0x53
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) // 0x54
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) // 0x55
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) // 0x56
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) // 0x57
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) // 0x58
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) // 0x59
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x5a
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) // 0x5b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) // 0x5c
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset / Power")           PORT_CODE(KEYCODE_F12)          // 0x5d (converted to 0x7f7f)
	PORT_BIT(0xc000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))  // 60
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)          PORT_CHAR(UCHAR_MAMEKEY(F6))  // 61
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)          PORT_CHAR(UCHAR_MAMEKEY(F7))  // 62
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))  // 63
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)          PORT_CHAR(UCHAR_MAMEKEY(F8))  // 64
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)          PORT_CHAR(UCHAR_MAMEKEY(F9))  // 65
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)         PORT_CHAR(UCHAR_MAMEKEY(F11)) // 67
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13)         PORT_CHAR(UCHAR_MAMEKEY(F13)) // 69
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14)         PORT_CHAR(UCHAR_MAMEKEY(F14)) // 6b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)         PORT_CHAR(UCHAR_MAMEKEY(F10)) // 6d
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)         PORT_CHAR(UCHAR_MAMEKEY(F12)) // 6f

	PORT_START("KEY7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15)         PORT_CHAR(UCHAR_MAMEKEY(F15))     // 71
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Insert/Help") PORT_CHAR(UCHAR_MAMEKEY(INSERT))  // 72
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)        PORT_CHAR(UCHAR_MAMEKEY(HOME))    // 73
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)        PORT_CHAR(UCHAR_MAMEKEY(PGUP))    // 74
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)         PORT_CHAR(UCHAR_MAMEKEY(DEL)) // 75
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))  // 76
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)         PORT_CHAR(UCHAR_MAMEKEY(END)) // 77
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))  // 78
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)        PORT_CHAR(UCHAR_MAMEKEY(PGDN)) // 79
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))  // 7a
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)    // 7b-7f are unused
INPUT_PORTS_END

macadb_device::macadb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MACADB, tag, owner, clock),
		m_mouse0(*this, "MOUSE0"),
		m_mouse1(*this, "MOUSE1"),
		m_mouse2(*this, "MOUSE2"),
		m_keys(*this, "KEY%u", 0),
		write_via_clock(*this),
		write_via_data(*this),
		write_adb_data(*this),
		write_adb_irq(*this),
		m_bIsMCUMode(true),
		m_last_kbd{0, 0},
		m_last_mouse{0, 0}
{
}

ioport_constructor macadb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(macadb);
}

void macadb_device::device_start()
{
	this->m_adb_timer = timer_alloc(FUNC(macadb_device::mac_adb_tick), this);
	this->m_adb_timer->adjust(attotime::never);

	std::fill(std::begin(m_adb_buffer), std::end(m_adb_buffer), 0);
	m_adb_listenreg = 0;
	m_adb_listenaddr = 0;
	m_adb_stream_ptr = 0;
	m_adb_linein = 0;
	std::fill(std::begin(m_adb_keybuf), std::end(m_adb_keybuf), 0);

	save_item(NAME(m_last_adb_time));
	save_item(NAME(m_key_matrix));
	save_item(NAME(m_adb_waiting_cmd));
	save_item(NAME(m_adb_datasize));
	save_item(NAME(m_adb_buffer));
	save_item(NAME(m_adb_command));
	save_item(NAME(m_adb_send));
	save_item(NAME(m_adb_timer_ticks));
	save_item(NAME(m_adb_extclock));
	save_item(NAME(m_adb_direction));
	save_item(NAME(m_adb_listenreg));
	save_item(NAME(m_adb_listenaddr));
	save_item(NAME(m_adb_last_talk));
	save_item(NAME(m_adb_srq_switch));
	save_item(NAME(m_adb_stream_ptr));
	save_item(NAME(m_adb_linestate));
	save_item(NAME(m_adb_srqflag));
	save_item(NAME(m_adb_keybuf));
	save_item(NAME(m_adb_keybuf_start));
	save_item(NAME(m_adb_keybuf_end));
	save_item(NAME(m_adb_mouseaddr));
	save_item(NAME(m_adb_lastmousex));
	save_item(NAME(m_adb_lastmousey));
	save_item(NAME(m_adb_lastbutton));
	save_item(NAME(m_adb_mouse_initialized));
	save_item(NAME(m_adb_keybaddr));
	save_item(NAME(m_adb_keybinitialized));
	save_item(NAME(m_adb_currentkeys));
	save_item(NAME(m_adb_modifiers));
	save_item(NAME(m_adb_linein));
	save_item(NAME(m_last_kbd));
	save_item(NAME(m_last_mouse));
	save_item(NAME(m_mouse_handler));
	save_item(NAME(m_keyboard_handler));
}

void macadb_device::adb_data_w(int state)
{
	if (m_adb_timer_ticks > 0)
	{
		m_adb_command <<= 1;
		if (state)
		{
			m_adb_command |= 1;
		}
		else
		{
			m_adb_command &= ~1;
		}
	}
}

/* *************************************************************************
 * High-level ADB primitives used by all lower-level implementations
 * *************************************************************************/

static char const *const adb_statenames[4] = { "NEW", "EVEN", "ODD", "IDLE" };

int macadb_device::adb_pollkbd(int update)
{
	int report, codes[2], result;

	codes[0] = codes[1] = 0xff; // key up
	report = result = 0;

	for (int i = 0; i < 8; i++)
	{
		int keybuf = m_keys[i]->read();

		// any changes in this row?
		if ((keybuf != m_key_matrix[i]) && (report < 2))
		{
			// check each column bit
			for (int j = 0; j < 16; j++)
			{
				if (((keybuf ^ m_key_matrix[i]) >> j) & 1)
				{
					// update m_key_matrix
					if (update)
					{
						m_key_matrix[i] = (m_key_matrix[i] & ~ (1 << j)) | (keybuf & (1 << j));
					}

					codes[report] = (i<<4)|j;

					// key up?
					if (!(keybuf & (1 << j)))
					{
						codes[report] |= 0x80;
					}

					// update modifier state
					if (update)
					{
						if (((i<<4)|j) == 0x39)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x20;
							}
							else
							{
								m_adb_modifiers |= 0x20;
							}
						}
						if (((i<<4)|j) == 0x36)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x8;
							}
							else
							{
								m_adb_modifiers |= 0x08;
							}
						}
						if (((i<<4)|j) == 0x38)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x4;
							}
							else
							{
								m_adb_modifiers |= 0x04;
							}
						}
						if (((i<<4)|j) == 0x3a)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x2;
							}
							else
							{
								m_adb_modifiers |= 0x02;
							}
						}
						if (((i<<4)|j) == 0x37)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x1;
							}
							else
							{
								m_adb_modifiers |= 0x01;
							}
						}
					}

					// we run out of keys we can track?
					report++;
					if (report == 2)
					{
						break;
					}
				}
			}

			// we run out of keys we can track?
			if (report == 2)
			{
				break;
			}
		}
	}

	// reset handling
	if (codes[0] == 0x5d)
	{
		codes[0] = codes[1] = 0x7f;
	}
	else if (codes[0] == 0xdd)
	{
		codes[0] = codes[1] = 0xff;
	}

	// figure out if there was a change
	if ((m_adb_currentkeys[0] != codes[0]) || (m_adb_currentkeys[1] != codes[1]))
	{
		result = 1;

		// if we want to update the current read, do so
		if (update)
		{
			if(m_adb_currentkeys[0] != codes[0]) {
				m_adb_keybuf[m_adb_keybuf_end] = codes[0];
				m_adb_keybuf_end = (m_adb_keybuf_end+1) % kADBKeyBufSize;
			}
			if(m_adb_currentkeys[1] != codes[1]) {
				m_adb_keybuf[m_adb_keybuf_end] = codes[1];
				m_adb_keybuf_end = (m_adb_keybuf_end+1) % kADBKeyBufSize;
			}
			m_adb_currentkeys[0] = codes[0];
			m_adb_currentkeys[1] = codes[1];
		}
	}

	return result;
}

int macadb_device::adb_pollmouse()
{
	int NewX, NewY, NewButton;

	if (!m_adb_mouse_initialized)
	{
		return 0;
	}

	NewButton = m_mouse0->read() & 0x01;
	NewX = m_mouse1->read();
	NewY = m_mouse2->read();

	if ((NewX != m_adb_lastmousex) || (NewY != m_adb_lastmousey) || (NewButton != m_adb_lastbutton))
	{
		return 1;
	}

	return 0;
}

void macadb_device::adb_accummouse( uint8_t *MouseX, uint8_t *MouseY )
{
	int MouseCountX = 0, MouseCountY = 0;
	int NewX, NewY;

	NewX = m_mouse1->read();
	NewY = m_mouse2->read();

//  printf("pollmouse: X %d Y %d\n", NewX, NewY);

	/* see if it moved in the x coord */
	if (NewX != m_adb_lastmousex)
	{
		int diff = NewX - m_adb_lastmousex;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		MouseCountX += diff;
		m_adb_lastmousex = NewX;
	}

	/* see if it moved in the y coord */
	if (NewY != m_adb_lastmousey)
	{
		int diff = NewY - m_adb_lastmousey;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		MouseCountY += diff;
		m_adb_lastmousey = NewY;
	}

	m_adb_lastbutton = m_mouse0->read() & 0x01;

	*MouseX = (uint8_t)MouseCountX;
	*MouseY = (uint8_t)MouseCountY;
}

void macadb_device::adb_talk()
{
	int addr, reg;

	addr = (m_adb_command>>4);
	reg = (m_adb_command & 3);

//  printf("Mac sent %x (cmd %d addr %d reg %d mr %d kr %d)\n", m_adb_command, (m_adb_command>>2)&3, addr, reg, m_adb_mouseaddr, m_adb_keybaddr);

	if (m_adb_waiting_cmd)
	{
		switch ((m_adb_command>>2)&3)
		{
			case 0:
			case 1:
				switch (reg)
				{
					case ADB_CMD_RESET:
						LOGMASKED(LOG_TALK_LISTEN, "ADB RESET: reg %x address %x\n", reg, addr);
						m_adb_direction = 0;
						m_adb_send = 0;
						break;

					case ADB_CMD_FLUSH:
						LOGMASKED(LOG_TALK_LISTEN, "ADB FLUSH: reg %x address %x\n", reg, addr);

						m_adb_direction = 0;
						m_adb_send = 0;
						break;

					default:    // reserved/unused
						break;
				}
				break;

			case 2: // listen
				m_adb_datasize = 0;
				if ((addr == m_adb_keybaddr) || (addr == m_adb_mouseaddr))
				{
					LOGMASKED(LOG_TALK_LISTEN, "ADB LISTEN: reg %x address %x\n", reg, addr);
					m_adb_direction = 1;    // input from Mac
					m_adb_command = 0;
					m_adb_listenreg = reg;
					m_adb_listenaddr = addr;
					m_adb_stream_ptr = 0;
					memset(m_adb_buffer, 0, sizeof(m_adb_buffer));
				}
				else
				{
					LOGMASKED(LOG_TALK_LISTEN, "ADB LISTEN to unknown device %d, timing out\n", addr);
					m_adb_direction = 0;
				}
				break;

			case 3: // talk
				LOGMASKED(LOG_TALK_LISTEN, "ADB TALK: reg %x address %x (K %x M %x)\n", reg, addr, m_adb_keybaddr, m_adb_mouseaddr);

				// keep track of what device the Mac last TALKed to
				m_adb_last_talk = addr;

				m_adb_direction = 0;    // output to Mac
				if (addr == m_adb_mouseaddr)
				{
					uint8_t mouseX, mouseY;

					LOGMASKED(LOG_TALK_LISTEN, "Talking to mouse, register %x\n", reg);

					switch (reg)
					{
						// read mouse
						case 0:
							if (m_adb_srq_switch)
							{
								m_adb_srq_switch = 0;
								mouseX = mouseY = 0;
							}
							else
							{
								this->adb_accummouse(&mouseX, &mouseY);
							}
							//printf("X %x Y %x\n", mouseX, mouseY);
							m_adb_buffer[0] = (m_adb_lastbutton & 0x01) ? 0x00 : 0x80;
							m_adb_buffer[0] |= mouseY & 0x7f;
							m_adb_buffer[1] = (mouseX & 0x7f) | 0x80;

							if ((m_adb_buffer[0] != m_last_mouse[0]) || (m_adb_buffer[1] != m_last_mouse[1]))
							{
								m_adb_datasize = 2;
								m_last_mouse[0] = m_adb_buffer[0];
								m_last_mouse[1] = m_adb_buffer[1];
							}
							else
							{
								m_adb_datasize = 0;

								if ((adb_pollkbd(0)) && (m_keyboard_handler & 0x20))
								{
									LOGMASKED(LOG_TALK_LISTEN, "Keyboard requesting service\n");
									m_adb_srqflag = true;
								}
							}
							break;

						// get ID/handler
						case 3:
							m_adb_buffer[0] = m_mouse_handler;
							m_adb_buffer[1] = 0x01; // handler 1
							m_adb_datasize = 2;

							m_adb_mouse_initialized = 1;
							break;

						default:
							break;
					}
				}
				else if (addr == m_adb_keybaddr)
				{
					LOGMASKED(LOG_TALK_LISTEN, "Talking to keyboard, register %x\n", reg);

					switch (reg)
					{
						// read keyboard
						case 0:
							if (m_adb_srq_switch)
							{
								m_adb_srq_switch = 0;
							}
							else
							{
								adb_pollkbd(1);
							}

							if (m_adb_keybuf_start == m_adb_keybuf_end)
							{
//                              printf("%s: buffer empty\n", __func__);
								m_adb_buffer[0] = 0xff;
								m_adb_buffer[1] = 0xff;
							}
							else
							{
								m_adb_buffer[1] = m_adb_keybuf[m_adb_keybuf_start];
								m_adb_keybuf_start = (m_adb_keybuf_start+1) % kADBKeyBufSize;
								if(m_adb_keybuf_start != m_adb_keybuf_end)
								{
									m_adb_buffer[0] = m_adb_keybuf[m_adb_keybuf_start];
									m_adb_keybuf_start = (m_adb_keybuf_start+1) % kADBKeyBufSize;
								}
								else
								{
									m_adb_buffer[0] = 0xff;
								}
							}

							if ((m_adb_buffer[0] != m_last_kbd[0]) || (m_adb_buffer[1] != m_last_kbd[1]))
							{
								m_adb_datasize = 2;
								m_last_kbd[0] = m_adb_buffer[0];
								m_last_kbd[1] = m_adb_buffer[1];
							}
							else
							{
								m_adb_datasize = 0;

								if ((adb_pollmouse()) && (m_mouse_handler & 0x20))
								{
									LOGMASKED(LOG_TALK_LISTEN, "Mouse requesting service\n");
									m_adb_srqflag = true;
								}
							}
							break;

						// read modifier keys
						case 2:
							{
								this->adb_pollkbd(1);
								m_adb_buffer[0] = m_adb_modifiers;
								m_adb_buffer[1] = 0xff;
								m_adb_datasize = 2;
							}
							break;

						// get ID/handler
						case 3:
							m_adb_buffer[0] = m_keyboard_handler;
							m_adb_buffer[1] = 0x01; // handler 1
							m_adb_datasize = 2;

							m_adb_keybinitialized = 1;
							break;

						default:
							break;
					}
				}
				else
				{
					LOGMASKED(LOG_TALK_LISTEN, "ADB: talking to unconnected device %d (K %d M %d)\n", addr, m_adb_keybaddr, m_adb_mouseaddr);
					m_adb_buffer[0] = m_adb_buffer[1] = 0;
					m_adb_datasize = 0;

					if ((adb_pollmouse()) && (m_mouse_handler & 0x20))
					{
						LOGMASKED(LOG_TALK_LISTEN, "Mouse requesting service\n");
						m_adb_srqflag = true;
					}
				}
				break;
		}

		m_adb_waiting_cmd = 0;
	}
	else
	{
		LOGMASKED(LOG_TALK_LISTEN, "Got LISTEN data %02x %02x for device %x reg %x\n", m_adb_command, m_adb_buffer[1], m_adb_listenaddr, m_adb_listenreg);
		m_adb_direction = 0;

		if (m_adb_listenaddr == m_adb_mouseaddr)
		{
			if (m_adb_listenreg == 3)
			{
				switch (m_adb_buffer[1])
				{
					case 0x00:  // unconditional set handler & address to value
						LOGMASKED(LOG_TALK_LISTEN, "MOUSE: moving to address & setting handler bits to %02x\n", m_adb_command);
						m_mouse_handler = m_adb_command & 0x7f;
						m_adb_mouseaddr = m_adb_command & 0x0f;
						break;

					case 0xfe:  // unconditional address change
						LOGMASKED(LOG_TALK_LISTEN, "MOUSE: moving to address %x\n", m_adb_command);
						m_adb_mouseaddr = m_adb_command & 0x0f;
						m_mouse_handler &= 0xf0;
						m_mouse_handler |= m_adb_mouseaddr;
						m_adb_mouse_initialized = 1;
						break;
				}
			}
		}
		else if (m_adb_listenaddr == m_adb_keybaddr)
		{
			if (m_adb_listenreg == 3)
			{
				switch (m_adb_buffer[1])
				{
				case 0x00: // unconditional set handler & address to value
					LOGMASKED(LOG_TALK_LISTEN, "KEYBOARD: moving to address & setting handler bits to %02x\n", m_adb_command);
					m_keyboard_handler = m_adb_command & 0x7f;
					m_adb_keybaddr = m_adb_command & 0x0f;
					break;

				case 0xfe: // unconditional address change
					LOGMASKED(LOG_TALK_LISTEN, "KEYBOARD: moving to address %x\n", m_adb_command);
					m_adb_keybaddr = m_adb_command & 0x0f;
					m_keyboard_handler &= 0xf0;
					m_keyboard_handler |= m_adb_keybaddr;
					break;
				}
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(macadb_device::mac_adb_tick)
{
	if (m_bIsMCUMode)
	{
		switch (m_adb_linestate)
		{
			case LST_SRQNODATA:
				set_adb_line(ASSERT_LINE);
				m_adb_linestate = LST_IDLE;
				break;

			case LST_TSTOPSTART:
				LOGMASKED(LOG_LINESTATE, "Send: TStopStart begin\n");
				set_adb_line(ASSERT_LINE);
				m_adb_timer->adjust(attotime::from_ticks(adb_short, adb_timebase));
				m_adb_linestate++;
				break;

			case LST_TSTOPSTARTa:
				LOGMASKED(LOG_LINESTATE, "Send: TStopStart end\n");
				set_adb_line(CLEAR_LINE);
				m_adb_timer->adjust(attotime::from_ticks(adb_short, adb_timebase));
				m_adb_linestate++;
				break;

			case LST_STARTBIT:
				LOGMASKED(LOG_LINESTATE, "Send: Start bit\n");
				set_adb_line(ASSERT_LINE);
				m_adb_timer->adjust(attotime::from_ticks(adb_long, adb_timebase));
				m_adb_linestate++;
				break;

			case LST_SENDBIT0:
			case LST_SENDBIT1:
			case LST_SENDBIT2:
			case LST_SENDBIT3:
			case LST_SENDBIT4:
			case LST_SENDBIT5:
			case LST_SENDBIT6:
			case LST_SENDBIT7:
				set_adb_line(CLEAR_LINE);
				if (m_adb_buffer[m_adb_stream_ptr] & 0x80)
				{
					LOGMASKED(LOG_LINESTATE, "Send: 1\n");
					m_adb_timer->adjust(attotime::from_ticks(adb_short, adb_timebase));
				}
				else
				{
					LOGMASKED(LOG_LINESTATE, "Send: 0\n");
					m_adb_timer->adjust(attotime::from_ticks(adb_long, adb_timebase));
				}
				m_adb_linestate++;
				break;

			case LST_SENDBIT0a:
			case LST_SENDBIT1a:
			case LST_SENDBIT2a:
			case LST_SENDBIT3a:
			case LST_SENDBIT4a:
			case LST_SENDBIT5a:
			case LST_SENDBIT6a:
				set_adb_line(ASSERT_LINE);
				if (m_adb_buffer[m_adb_stream_ptr] & 0x80)
				{
					m_adb_timer->adjust(attotime::from_ticks(adb_long, adb_timebase));
				}
				else
				{
					m_adb_timer->adjust(attotime::from_ticks(adb_short, adb_timebase));
				}
				m_adb_buffer[m_adb_stream_ptr] <<= 1;
				m_adb_linestate++;
				break;

			case LST_SENDBIT7a:
				set_adb_line(ASSERT_LINE);
				if (m_adb_buffer[m_adb_stream_ptr] & 0x80)
				{
//                    printf("  ");
					m_adb_timer->adjust(attotime::from_ticks(adb_long, adb_timebase));
				}
				else
				{
//                    printf("  ");
					m_adb_timer->adjust(attotime::from_ticks(adb_short, adb_timebase));
				}

				m_adb_stream_ptr++;
				if (m_adb_stream_ptr == m_adb_datasize)
				{
					m_adb_linestate++;
				}
				else
				{
					m_adb_linestate = LST_SENDBIT0;
				}
				break;

			case LST_SENDSTOP:
				LOGMASKED(LOG_LINESTATE, "Send: Stop bit begin\n");
				set_adb_line(CLEAR_LINE);
				m_adb_timer->adjust(attotime::from_ticks((adb_short*2), adb_timebase));
				m_adb_linestate++;
				break;

			case LST_SENDSTOPa:
				LOGMASKED(LOG_LINESTATE, "Send: Stop bit end\n");
				set_adb_line(ASSERT_LINE);
				m_adb_timer->adjust(attotime::never);
				m_adb_linestate = LST_IDLE;
				break;
		}
	}
	else
	{
		// for input to Mac, the VIA reads on the *other* clock edge, so update this here
		if (!m_adb_direction)
		{
			write_via_data((m_adb_send & 0x80)>>7);
			m_adb_send <<= 1;
		}

		// do one clock transition on CB1 to advance the VIA shifter
		//printf("ADB transition (%d)\n", m_adb_timer_ticks);
		if (m_adb_direction)
		{
			write_via_clock(m_adb_extclock ^ 1);
			write_via_clock(m_adb_extclock);
		}
		else
		{
			write_via_clock(m_adb_extclock);
			write_via_clock(m_adb_extclock ^ 1);
		}

		m_adb_timer_ticks--;
		if (!m_adb_timer_ticks)
		{
			m_adb_timer->adjust(attotime::never);

			if ((m_adb_direction) && (!m_bIsMCUMode))
			{
				adb_talk();
				if((m_adb_last_talk == 2) && m_adb_datasize) {
					m_adb_timer_ticks = 8;
					m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				}
			}

			if (!(m_adb_direction) && !(m_bIsMCUMode))
			{
			//  write_via_clock(m_adb_extclock);
				write_via_clock(m_adb_extclock ^ 1);
			}
		}
		else
		{
			m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
		}
	}
}

void macadb_device::mac_adb_newaction(int state)
{
	if (state != m_adb_state)
	{
		LOGMASKED(LOG_STATE, "New ADB state: %s\n", adb_statenames[state]);

		m_adb_state = state;
		m_adb_timer_ticks = 8;

		switch (state)
		{
			case ADB_STATE_NEW_COMMAND:
				m_adb_command = m_adb_send = 0;
				m_adb_direction = 1;    // Mac is shifting us a command
				m_adb_waiting_cmd = 1;  // we're going to get a command
				write_adb_irq(CLEAR_LINE);
				m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				break;

			case ADB_STATE_XFER_EVEN:
			case ADB_STATE_XFER_ODD:
				//printf("EVEN/ODD: adb datasize %d\n", m_adb_datasize);
				if (m_adb_datasize > 0)
				{
					int i;

					// is something trying to send to the Mac?
					if (m_adb_direction == 0)
					{
						// set up the byte
						m_adb_send = m_adb_buffer[0];
						//printf("ADB sending %02x\n", m_adb_send);
						m_adb_datasize--;

						// move down the rest of the buffer, if any
						for (i = 0; i < m_adb_datasize; i++)
						{
							m_adb_buffer[i] = m_adb_buffer[i+1];
						}
					}

				}
				else
				{
					m_adb_send = 0;
					write_adb_irq(ASSERT_LINE);
				}

				m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				break;

			case ADB_STATE_IDLE:
				write_adb_irq(CLEAR_LINE);
				break;
		}
	}
}

void macadb_device::adb_vblank()
{
	if (m_adb_state == ADB_STATE_IDLE)
	{
		if (this->adb_pollmouse())
		{
			// if the mouse was the last TALK, we can just send the new data
			// otherwise we need to pull SRQ
			if (m_adb_last_talk == m_adb_mouseaddr)
			{
				// repeat last TALK to get updated data
				m_adb_waiting_cmd = 1;
				this->adb_talk();

				m_adb_timer_ticks = 8;
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			else
			{
				write_adb_irq(ASSERT_LINE);
				m_adb_command = m_adb_send = 0;
				m_adb_timer_ticks = 1;  // one tick should be sufficient to make it see the IRQ
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				m_adb_srq_switch = 1;
			}
		}
		else if (this->adb_pollkbd(0))
		{
			if (m_adb_last_talk == m_adb_keybaddr)
			{
				// repeat last TALK to get updated data
				m_adb_waiting_cmd = 1;
				this->adb_talk();

				m_adb_timer_ticks = 8;
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			else
			{
				write_adb_irq(ASSERT_LINE);
				m_adb_command = m_adb_send = 0;
				m_adb_timer_ticks = 1;  // one tick should be sufficient to make it see  the IRQ
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				m_adb_srq_switch = 1;
			}
		}
	}
}

void macadb_device::device_reset()
{
	int i;

	m_adb_srq_switch = 0;
	write_adb_irq(CLEAR_LINE);      // no interrupt
	m_adb_timer_ticks = 0;
	m_adb_command = 0;
	m_adb_extclock = 0;
	m_adb_send = 0;
	m_adb_waiting_cmd = 0;
	m_adb_state = 0;
	m_adb_srqflag = false;
	m_adb_state = ADB_STATE_NOTINIT;
	m_adb_direction = 0;
	m_adb_datasize = 0;
	m_adb_last_talk = -1;

	m_adb_linestate = 0;

	// mouse
	m_adb_mouseaddr = 3;
	m_mouse_handler = 0x23;
	m_adb_lastmousex = m_adb_lastmousey = m_adb_lastbutton = 0;
	m_adb_mouse_initialized = 0;

	// keyboard
	m_adb_keybaddr = 2;
	m_keyboard_handler = 0x22;
	m_adb_keybinitialized = 0;
	m_adb_currentkeys[0] = m_adb_currentkeys[1] = 0xff;
	m_adb_modifiers = 0xff;
	for (i=0; i<9; i++)
	{
		m_key_matrix[i] = 0;
	}
	m_adb_keybuf_start = 0;
	m_adb_keybuf_end = 0;

	m_last_adb_time = 0;
}

void macadb_device::adb_linechange_w(int state)
{
/*  static char const *const states[] =
    {
        "idle",
        "attention",
        "bit0",
        "bit1",
        "bit2",
        "bit3",
        "bit4",
        "bit5",
        "bit6",
        "bit7",
        "tstop",
        "waitt1t",
        "rcvstartbit",
        "srqnodata"
    };*/

	if (state == m_adb_linein)
	{
		return;
	}
	m_adb_linein = state;

	int dtime = (int)(machine().time().as_ticks(adb_timebase) - m_last_adb_time);
	m_last_adb_time = machine().time().as_ticks(adb_timebase);

/*  if (m_adb_linestate <= 12)
    {
        printf("linechange: %d -> %d, time %d (state %d = %s)\n", state^1, state, dtime, m_adb_linestate, states[m_adb_linestate]);
    }
    else
    {
        printf("linechange: %d -> %d, time %d (state %d)\n", state^1, state, dtime, m_adb_linestate);
    }*/

	if ((m_adb_direction) && (m_adb_linestate == LST_TSTOP))
	{
		if (m_adb_stream_ptr & 1)   // odd byte, can't end here
		{
//            printf("critical linechange: odd, cont\n");
			m_adb_linestate = LST_BIT0;
		}
		else
		{
			if (dtime < 90)
			{
//                printf("critical linechange: even, and it's another bit\n");
				m_adb_linestate = LST_BIT0;
			}
		}
	}

	switch (m_adb_linestate)
	{
		case LST_IDLE:
			if ((state) && (dtime >= 4500))     // reset
			{
				//printf("ADB RESET\n");
			}
			else if ((state) && (dtime >= 1200))    // attention
			{
				//printf("ADB ATTENTION\n");
				m_adb_waiting_cmd = 1;
				m_adb_direction = 0;
				m_adb_linestate++;
			}
			break;

		case LST_ATTENTION:
			if ((!state) && (dtime >= 90))     // Tsync
			{
				//printf("ADB Tsync\n");
				m_adb_command = 0;
				m_adb_linestate++;
			}
			break;

		case LST_BIT0:
		case LST_BIT1:
		case LST_BIT2:
		case LST_BIT3:
		case LST_BIT4:
		case LST_BIT5:
		case LST_BIT6:
		case LST_BIT7:
			if (!state)
			{
				if (dtime >= 90)    // "1" bit
				{
					m_adb_command |= 1;
				}
				LOGMASKED(LOG_LINESTATE, "ADB bit %d (dtime = %d)\n", m_adb_command & 1, dtime);

				if (m_adb_linestate != LST_BIT7)
				{
					m_adb_command <<= 1;
				}
				else
				{
					if (m_adb_direction)
					{
						LOGMASKED(LOG_LINESTATE, "listen byte[%d] = %02x\n", m_adb_stream_ptr, m_adb_command);
						m_adb_buffer[m_adb_stream_ptr++] = m_adb_command;
						m_adb_command = 0;
					}
				}

				m_adb_linestate++;
			}
			break;

		case LST_TSTOP:
			if (state)
			{
				LOGMASKED(LOG_LINESTATE, "ADB TSTOP, command byte %02x\n", m_adb_command);

				if (m_adb_direction)
				{
					m_adb_command = m_adb_buffer[0];
				}

				m_adb_srqflag = false;
				adb_talk();

				if (m_adb_srqflag)
				{
					set_adb_line(CLEAR_LINE);
					m_adb_linestate = LST_SRQNODATA;
					m_adb_timer->adjust(attotime::from_ticks(adb_srq, adb_timebase)); // SRQ time
				}
				else
				{
					set_adb_line(ASSERT_LINE);

					if (m_adb_datasize > 0)
					{
						LOGMASKED(LOG_TALK_LISTEN, "Device has %d bytes of data:\n", m_adb_datasize);
						for (int i = 0; i < m_adb_datasize; i++)
						{
							LOGMASKED(LOG_TALK_LISTEN, "  %02x", m_adb_buffer[i]);
						}
						LOGMASKED(LOG_TALK_LISTEN, "\n");
						m_adb_linestate = LST_TSTOPSTART; // T1t
						m_adb_timer->adjust(attotime::from_ticks(324 / 4, adb_timebase));
						m_adb_stream_ptr = 0;
					}
					else if (m_adb_direction)   // if direction is set, we LISTENed to a valid device
					{
						m_adb_linestate = LST_WAITT1T;
					}
					else    // no valid device targetted, time out
					{
						if (m_adb_srqflag)
						{
							m_adb_linestate = LST_SRQNODATA;
							m_adb_timer->adjust(attotime::from_ticks(adb_srq, adb_timebase));   // SRQ time
						}
						else
						{
							m_adb_linestate = LST_IDLE;
						}
					}
				}
			}
			break;

		case LST_WAITT1T:
			if ((!state) && (dtime >= 300))     // T1t
			{
				LOGMASKED(LOG_LINESTATE, "ADB T1t\n");
				m_adb_linestate++;
			}
			break;

		case LST_RCVSTARTBIT:
			if ((!state) && (dtime >= 90))       // start
			{
				LOGMASKED(LOG_LINESTATE, "ADB start\n");
				m_adb_linestate = LST_BIT0;
				m_adb_command = 0;
			}
			break;
	}
}
