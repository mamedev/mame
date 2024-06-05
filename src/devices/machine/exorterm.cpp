// license:BSD-3-Clause
// copyright-holders: 68bit
/****************************************************************************

Motorola EXORterm 155 (M68SDS)

The terminfo database has an entry for this terminal named 'ex155'.

TODO
 - Keyboard layout. All the codes appear to have been implemented following
   the manual but have not been checked with hardware. The key repetition
   rate and delay need to be checked.
 - Video character ROM needs dumping. An apparently close Motorola published
   7x9 character set has been substituted for now, but three of the codes
   (0x1a, 0x1b, and 0x1c) do not match the characters listed in the manual.

****************************************************************************/

#include "emu.h"
#include "exorterm.h"

#include "speaker.h"

#include "exorterm155.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

exorterm155_device::exorterm155_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_irqs(*this, "irqs")
	, m_acia(*this, "acia")
	, m_brg(*this, "brg")
	, m_rs232_baud(*this, "RS232_BAUD")
	, m_disable_fac(*this, "DISABLE_FAC")
	, m_display_fac(*this, "DISPLAY_FAC")
	, m_pia_kbd(*this, "pia_kbd")
	, m_pia_cfg(*this, "pia_cfg")
	, m_pia_disp(*this, "pia_disp")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
	, m_sys_timer_clock(*this, "sys_timer_clock")
	, m_vram(*this, "videoram")
	, m_chargen(*this, "chargen")
	, m_beeper(*this, "beeper")
	, m_online_led(*this, "online_led")
	, m_auto_lf_led(*this, "auto_lf_led")
	, m_page_mode_led(*this, "page_mode_led")
	, m_insert_char_led(*this, "insert_char_led")
	, m_rs232_conn_txd_handler(*this)
	, m_rs232_conn_dtr_handler(*this)
	, m_rs232_conn_rts_handler(*this)
	  // Keyboard
	, m_kbd_scan_timer(nullptr)
	, m_kbd_repeat_timer(nullptr)
	, m_kbd_modifiers(*this, "KBD_MOD")
	, m_kbd_rows(*this, "KBD_ROW%u", 0)
	, m_kbd_next_row(0)
	, m_kbd_processing(0)
	, m_kbd_repeat_row(0)
	, m_kbd_repeat_column(0)
	, m_kbd_last_modifiers(0)
{
}

exorterm155_device::exorterm155_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: exorterm155_device(mconfig, EXORTERM155, tag, owner, clock)
{
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void exorterm155_device::mem_map(address_map &map)
{
	// 2 x 1Kx4 static RAM.
	map(0x0000, 0x03ff).ram(); // Scratch RAM

	map(0xa000, 0xa7ff).rom(); // U89  68K edit mode
	//map(0xa800, 0xafff).rom(); // U70 (reserved)
	//map(0xb000, 0xb7ff).rom(); // U59 (reserved)
	map(0xc000, 0xc7ff).rom(); // U49  Extended display
	map(0xc800, 0xcfff).rom(); // U104 Basic display #1
	map(0xd000, 0xd7ff).rom(); // U98  Basic display #2
	// Appears to mirror to ff00-ffff for the reset vectors.
	map(0xd800, 0xdfff).rom().mirror(0x2000); // U78  Terminal control

	// 4 x 1Kx4 static RAM.
	map(0xe000, 0xe7ff).ram().share(m_vram);

	map(0xef00, 0xef01).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xef04, 0xef07).rw(m_pia_kbd, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xef08, 0xef0b).rw(m_pia_cfg, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xef0c, 0xef0f).rw(m_pia_disp, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START( exorterm155 )

	PORT_START("DISABLE_FAC")
	PORT_DIPNAME(0x01, 0x01, "Disable FAC codes") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x01, "Off")

	PORT_START("DISPLAY_FAC")
	PORT_DIPNAME(0x01, 0x01, "Display FAC codes") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x01, "Off")

	PORT_START("DIP_SWITCHES_A")
	PORT_DIPNAME(0x01, 0x01, "Stop bits") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, "2")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPNAME(0x02, 0x02, "Duplex") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, "Half")
	PORT_DIPSETTING(0x02, "Full")
	PORT_DIPNAME(0x04, 0x04, "Parity") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, "Yes")
	PORT_DIPSETTING(0x04, "No")
	PORT_DIPNAME(0x08, 0x08, "Parity") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x08, "Even")
	PORT_DIPNAME(0x10, 0x00, "Word size") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, "8-bit")
	PORT_DIPSETTING(0x10, "7-bit")
	PORT_DIPNAME(0x20, 0x00, "Connection") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00, "Direct")
	PORT_DIPSETTING(0x20, "Modem")
	PORT_DIPNAME(0x40, 0x40, "Modem Type") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x00, "202")
	PORT_DIPSETTING(0x40, "103")
	PORT_DIPNAME(0x80, 0x80, "Turnaround") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00, "Code")
	PORT_DIPSETTING(0x80, "S-chan")

	PORT_START("DIP_SWITCHES_B")
	PORT_DIPNAME(0x01, 0x01, "Code") PORT_DIPLOCATION("SW2:9")
	PORT_DIPSETTING(0x00, "ETX")
	PORT_DIPSETTING(0x01, "EOT")
	PORT_DIPNAME(0x02, 0x02, "Spare 1") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x02, "Off")
	PORT_DIPNAME(0x04, 0x04, "Spare 2") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x04, "Off")
	PORT_DIPNAME(0x08, 0x08, "Spare 3") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x08, "Off")
	PORT_DIPNAME(0x10, 0x10, "Display special characters") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x10, "Off")
	PORT_DIPNAME(0x40, 0x40, "Transparent mode") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x40, "Off")
	PORT_DIPNAME(0x80, 0x80, "Video invert") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x00, "Invert")
	PORT_DIPSETTING(0x80, "Normal")

	PORT_START("RS232_BAUD")
	PORT_DIPNAME(0x1ff, 0x01, "RS232 Baud Rate")
	PORT_DIPSETTING(0x100, "110") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x080, "150") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x040, "300") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x020, "600") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x010, "1200") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x008, "1800") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x004, "2400") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x002, "4800") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")
	PORT_DIPSETTING(0x001, "9600") PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8,9")

	// Keyboard

	PORT_START("KBD_MOD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("All Caps")   PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("KBD_ROW0")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x00040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x00080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_NAME("Auto LF")
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)   PORT_NAME("Online")
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_NAME("Page Mode")
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD_ROW1")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')   PORT_CHAR('@')
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')   PORT_CHAR('^')
	PORT_BIT( 0x00040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')   PORT_CHAR('&')
	PORT_BIT( 0x00080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')   PORT_CHAR('*')
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')   PORT_CHAR('(')
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')   PORT_CHAR(')')
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')   PORT_CHAR('~')
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')   PORT_CHAR('+')
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("Break") PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)     PORT_NAME("Insert Character") PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Delete Character")  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_NAME("Set Tabs")

	PORT_START("KBD_ROW2")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Escape")    PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')   PORT_CHAR('Q')
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')   PORT_CHAR('W')
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')   PORT_CHAR('E')
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')   PORT_CHAR('R')
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')   PORT_CHAR('T')
	PORT_BIT( 0x00040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')   PORT_CHAR('Y')
	PORT_BIT( 0x00080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')   PORT_CHAR('U')
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')   PORT_CHAR('I')
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')   PORT_CHAR('O')
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')   PORT_CHAR('P')
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\')  PORT_CHAR('|')
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END)        PORT_NAME("Linefeed")
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Return")    PORT_CHAR(0x0dU)
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Forward Tab") PORT_CHAR('\t')
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_NAME("Back Tab")
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("Line Delete")

	PORT_START("KBD_ROW3")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')
	PORT_BIT( 0x00040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')
	PORT_BIT( 0x00080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')   PORT_CHAR('{')
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')   PORT_CHAR('}')
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)         PORT_NAME("Cursor Up") PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Cursor Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_NAME("Line Insert")

	PORT_START("KBD_ROW4")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)       PORT_NAME("Home / Clear")
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')   PORT_CHAR('Z')
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')   PORT_CHAR('X')
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')   PORT_CHAR('C')
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')   PORT_CHAR('V')
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')   PORT_CHAR('B')
	PORT_BIT( 0x00040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')   PORT_CHAR('N')
	PORT_BIT( 0x00080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')   PORT_CHAR('M')
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')   PORT_CHAR('<')
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR('>')
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')   PORT_CHAR('?')
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("Page Send / Line Send")
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Backspace") PORT_CHAR(0x08U)
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Cursor Left") PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("Cursor Right") PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("Page Erase / Line Erase")

INPUT_PORTS_END


// PA0 to PA6 - Keyboard data.
u8 exorterm155_device::pia_kbd_pa_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

u8 exorterm155_device::pia_kbd_pb_r()
{
	// PB7 is a 'keyboard attached' signal, low when a keyboard is attached.
	return 0x00;
}

void exorterm155_device::pia_kbd_pb_w(u8 data)
{
	m_online_led = BIT(data, 0) ? 0 : 1;
	m_auto_lf_led = BIT(data, 1) ? 0 : 1;
	m_page_mode_led = BIT(data, 2) ? 0 : 1;
	m_insert_char_led = BIT(data, 5) ? 0 : 1;
	// Bit 3 is AUX ON, and bit 4 is AUX EN, but it is not clear
	// if the firmware uses these?
}

void exorterm155_device::pia_cfg_cb2_w(int state)
{
	m_beeper->set_state(!state);
}

void exorterm155_device::pia_disp_pa_w(u8 data)
{
	m_cursor_addr = (m_cursor_addr & 0x700) | data;
}

u8 exorterm155_device::pia_disp_pb_r()
{
	u8 ret = 0;

	if (m_dsr)
		ret |= 0x20;

	return ret;
}

void exorterm155_device::pia_disp_pb_w(u8 data)
{
	m_cursor_addr = (m_cursor_addr & 0x0ff) | ((data & 0x7) << 8);

	m_inv_video = BIT(data, 3);
	m_special_char_disp = BIT(data, 4);

	m_rs232_conn_dtr_handler(BIT(data, 7));
}

//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

u32 exorterm155_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 sy = 0, ma = 0;
	u8 disable_fac = m_disable_fac->read() == 0;
	u8 display_fac = m_display_fac->read() == 0;
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	m_framecnt++;

	for (u8 y = 0; y < 24; y++)
	{
		for (u8 ra = 0; ra < 12; ra++)
		{
			u32 *p = &bitmap.pix(sy++);
			// FAC codes are cleared on horizontal sync.
			u8 underline1 = 0;
			u8 underline2 = 0;
			u8 non_display = 0;
			u8 invert = 0;
			u8 half_bright = 0;
			u8 blink = 0;

			for (u16 x = ma; x < ma + 80; x++)
			{
				u8 code = m_vram[x + 0x80];
				u16 gfx = 0;

				if (BIT(code, 7) && !disable_fac)
				{
					underline1 = BIT(code, 4);
					non_display = BIT(code, 3);
					invert = BIT(code, 2);
					half_bright = BIT(code, 1);
					blink = BIT(code, 0);
				}

				// Copying the circuit logic for loading from
				// the character generator which occurs when:
				//
				// 1. Special character display is enabled or
				// it is not a special character 0x00 to 0x1f.
				//
				// AND
				//
				// 2. Not 'non display'
				//
				// AND
				//
				// 3. Not 'blink' and blinked off.
				//
				// AND
				//
				// 4. Not FAC code or Display FAC enabled.
				//
				if ((m_special_char_disp || (code & 0xe0)) &&
					!non_display &&
					!(blink && (m_framecnt & 8)) &&
					(!BIT(code, 7) || display_fac))
				{
					offs_t address = ((code & 0x7f) << 4) | (ra & 0x0f);
					gfx = m_chargen[address];
				}

				if (underline1 && underline2 && ra == 10)
					gfx |= 0x1ff;

				u8 cursor_display = x + 0x80 == m_cursor_addr;
				cursor_display = cursor_display && (m_framecnt & 4);

				if (m_inv_video ^ invert ^ cursor_display)
					gfx ^= 0x1ff; // invert

				u32 font_color = palette[2 - half_bright];

				/* Display a scanline of a character */
				*p++ = (BIT(gfx, 7)) ? font_color : 0;
				*p++ = (BIT(gfx, 6)) ? font_color : 0;
				*p++ = (BIT(gfx, 5)) ? font_color : 0;
				*p++ = (BIT(gfx, 4)) ? font_color : 0;
				*p++ = (BIT(gfx, 3)) ? font_color : 0;
				*p++ = (BIT(gfx, 2)) ? font_color : 0;
				*p++ = (BIT(gfx, 1)) ? font_color : 0;
				*p++ = (BIT(gfx, 0)) ? font_color : 0;
				*p++ = (BIT(gfx, 8)) ? font_color : 0;

				// The circuit has logic to delay underlining
				// but not for the other attributes.
				underline2 = underline1;
			}
		}
		ma += 80;
	}

	return 0;
}

static const gfx_layout char_layout =
{
	8,12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END

//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void exorterm155_device::sys_timer_w(int state)
{
	// 3.4ms
	m_pia_disp->cb1_w(state);

	// Divide by 16, giving 27.5ms.
	if (state == 1) m_sys_timer_count++;
	m_pia_disp->ca1_w((m_sys_timer_count & 8) != 0);

	// The terminal firmware does not initialize properly if a key
	// interrupt is received too soon after reset, so a hold off period is
	// implemented and measured in terms of these timer interrupts.
	if (m_kbd_start_holdoff > 0)
	{
		if (m_kbd_start_holdoff == 1)
			kbd_start_processing();
		m_kbd_start_holdoff--;
	}
}

void exorterm155_device::write_f1_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 0))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f3_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 1))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f5_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 2))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f6_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 3))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f7_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 4))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f8_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 5))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f9_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 6))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f11_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 7))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::write_f13_clock(int state)
{
	if (BIT(m_rs232_baud->read(), 8))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void exorterm155_device::acia_txd_w(int state)
{
	m_rs232_conn_txd_handler(state);
}

void exorterm155_device::acia_rts_w(int state)
{
	m_rs232_conn_rts_handler(state);
}

void exorterm155_device::rs232_conn_dcd_w(int state)
{
	m_acia->write_dcd(state);
}

void exorterm155_device::rs232_conn_dsr_w(int state)
{
	// Input of Display PIA PB5, pulled high.
	m_dsr = state;
}

void exorterm155_device::rs232_conn_ri_w(int state)
{
	m_pia_disp->ca2_w(state);
}

void exorterm155_device::rs232_conn_cts_w(int state)
{
	m_acia->write_cts(state);
}

void exorterm155_device::rs232_conn_rxd_w(int state)
{
	m_acia->write_rxd(state);
}


// Keyboard

u8 const exorterm155_device::translation_table[][5][17] = {
	{   // Standard
		{ 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xff, 0xf7, 0xf8, 0xc7, 0xff },
		{  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '`',  '-',  '=', 0x8c, 0xd0, 0xd1, 0xdc },
		{ 0x1b,  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p', '\\', 0x0a, 0x0d, 0xda, 0xdb, 0xd7 },
		{  'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '[',  ']', 0xff, 0xc1, 0xc2, 0xd6 },
		{ 0xc0,  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/', 0xd9,  ' ', 0x08, 0xc3, 0xc4, 0xd4 }
	},
	{   // Shift
		{ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xff, 0xff, 0xff, 0xff, 0xff },
		{  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '~',  '_',  '+', 0xff, 0xd6, 0xd7, 0xff },
		{ 0x1b , 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '|', 0x0a, 0x0d, 0xdb, 0xff, 0xff },
		{  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',  '{',  '}', 0xff, 0xff, 0xff, 0xff },
		{ 0xd8,  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?', 0xdf,  ' ', 0x08, 0xff, 0xff, 0xd5 }
	},
	{   // Ctrl
		{ 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xff, 0xff, 0xff, 0xff, 0xff },
		{  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0', 0x00, 0x1f, 0x1e, 0xff, 0xff, 0xff, 0xff },
		{ 0x1b, 0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1c, 0x0a, 0x0d, 0x09, 0xff, 0xff },
		{ 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c,  ';', '\'', 0x1b, 0x1d, 0xff, 0xff, 0xff, 0xff },
		{ 0xff, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, ',',   '.', 0x1f, 0xff, 0x00, 0x08, 0xff, 0xff, 0xff }
	}
};

bool const exorterm155_device::caps_table[5][17] = {
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false },
	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, false, false, false }
};

void exorterm155_device::kbd_start_processing()
{
	attotime period = attotime::from_hz(2'400);
	m_kbd_processing = 1;
	m_kbd_scan_timer->adjust(period, 0, period);
}

void exorterm155_device::kbd_reset_state()
{
	std::fill(std::begin(m_kbd_states), std::end(m_kbd_states), ioport_value(0));
	m_kbd_next_row = 0;
}

void exorterm155_device::kbd_repeat_start(uint8_t row, uint8_t column)
{
	attotime delay = attotime::from_msec(750);
	attotime interval = attotime::from_msec(100);

	m_kbd_repeat_row = row;
	m_kbd_repeat_column = column;
	m_kbd_repeat_timer->adjust(delay, 0, interval);
}

void exorterm155_device::kbd_repeat_restart()
{
	attotime delay = attotime::from_msec(750);
	attotime interval = attotime::from_msec(100);

	if ((m_kbd_repeat_row != 0xff) || (m_kbd_repeat_column != 0xff))
		m_kbd_repeat_timer->adjust(delay, 0, interval);
}

void exorterm155_device::kbd_repeat_stop()
{
	m_kbd_repeat_row = 0xff;
	m_kbd_repeat_column = 0xff;
	m_kbd_repeat_timer->reset();
}

void exorterm155_device::kbd_send_translated(u8 code)
{
	unsigned row = (code >> 5) & 0x07;
	unsigned col = (code >> 0) & 0x1f;

	u16 modifiers = m_kbd_modifiers->read();
	bool shift = BIT(modifiers, 1) != ((modifiers & 4) && caps_table[row][col]);
	bool ctrl = BIT(modifiers, 0);

	unsigned map = ctrl ? 2 : shift ? 1 : 0;
	u8 result = translation_table[map][row][col];

	if (result == 0xff)
		return;

	m_term_data = result;
	// Triggers on the falling edge.
	m_pia_kbd->ca1_w(ASSERT_LINE);
	m_pia_kbd->ca1_w(CLEAR_LINE);
	m_pia_kbd->ca1_w(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(exorterm155_device::kbd_scan_row)
{
	u16 const modifiers(m_kbd_modifiers->read());
	if (modifiers != m_kbd_last_modifiers)
		kbd_repeat_restart();

	m_kbd_last_modifiers = modifiers;

	ioport_value &state(m_kbd_states[m_kbd_next_row]);
	ioport_value const keys(m_kbd_rows[m_kbd_next_row]->read());
	ioport_value const change(state ^ keys);

	ioport_value mask(1U);
	for (uint8_t column = 0U; m_kbd_processing && (state != keys); ++column, mask <<= 1)
	{
		if (change & mask)
		{
			state ^= mask;
			if (keys & mask)
			{
				kbd_send_translated((m_kbd_next_row << 5) | column);
				kbd_repeat_start(m_kbd_next_row, column);
			}
			else
			{
				if (m_kbd_repeat_row == m_kbd_next_row && m_kbd_repeat_column == column)
					kbd_repeat_stop();
			}
		}
	}

	m_kbd_next_row = (m_kbd_next_row + 1) % 5;
}

TIMER_CALLBACK_MEMBER(exorterm155_device::kbd_repeat)
{
	assert((m_kbd_repeat_row != 0xff) || (m_kbd_repeat_column != 0xff));
	kbd_send_translated((m_kbd_repeat_row << 5) | m_kbd_repeat_column);
}




void exorterm155_device::device_start()
{
	m_online_led.resolve();
	m_auto_lf_led.resolve();
	m_page_mode_led.resolve();
	m_insert_char_led.resolve();

	// register for save states
	save_item(NAME(m_term_data));
	save_item(NAME(m_framecnt));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_inv_video));
	save_item(NAME(m_special_char_disp));

	save_item(NAME(m_sys_timer_count));
	save_item(NAME(m_dsr));
	save_item(NAME(m_kbd_start_holdoff));

	// Keyboard
	m_kbd_scan_timer = timer_alloc(FUNC(exorterm155_device::kbd_scan_row), this);
	m_kbd_repeat_timer = timer_alloc(FUNC(exorterm155_device::kbd_repeat), this);
	kbd_reset_state();
	kbd_repeat_stop();

	save_item(NAME(m_kbd_states));
	save_item(NAME(m_kbd_next_row));
	save_item(NAME(m_kbd_processing));
	save_item(NAME(m_kbd_repeat_row));
	save_item(NAME(m_kbd_repeat_column));
	save_item(NAME(m_kbd_last_modifiers));

	m_dsr = 1;
}

void exorterm155_device::device_reset()
{
	m_brg->rsa_w(CLEAR_LINE);
	m_brg->rsb_w(ASSERT_LINE);

	m_online_led = 0;
	m_auto_lf_led = 0;
	m_page_mode_led = 0;
	m_insert_char_led = 0;

	m_framecnt = 0;
	m_cursor_addr = 0;
	m_inv_video = 0;
	m_special_char_disp = 0;
	m_sys_timer_count = 0;

	m_beeper->set_state(0);

	// Keyboard
	kbd_reset_state();
	kbd_repeat_stop();
	m_kbd_start_holdoff = 180;
	m_kbd_last_modifiers = 0;
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( exorterm155 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("51aw1018b25.bin", 0xa000, 0x0800, CRC(5f9352ba) SHA1(9fb52992bd3f3409fa8ec1002e2ebdf61318d260))
	ROM_LOAD("51aw1018b22.bin", 0xc000, 0x0800, CRC(b6bb9df0) SHA1(5c0bda69d94fd58c21f07c810525043cd22a9d7d))
	ROM_LOAD("51aw1018b27.bin", 0xc800, 0x0800, CRC(15f77b53) SHA1(1f73098ebf83d7255efcf2b509d12e559001ca5b))
	ROM_LOAD("51aw1018b26.bin", 0xd000, 0x0800, CRC(f5b204bc) SHA1(9815fd7d7ba617ebb3f55e595e4931d75942b6a9))
	ROM_LOAD("51aw1018b24.bin", 0xd800, 0x0800, CRC(18a0ed66) SHA1(5fa6e4b3c27969c1d7e27c79fc2993e4cc9262cf))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("chargen.rom", 0x0000, 0x0800, CRC(0087cbad) SHA1(a6d3dd0512db4c459944c60d728536755a570964) BAD_DUMP) // constructed from an apparently similar published Motorola character set

ROM_END

//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void exorterm155_device::device_add_mconfig(machine_config &config)
{
	M6800(config, m_maincpu, 16.6698_MHz_XTAL / 18);
	m_maincpu->set_addrmap(AS_PROGRAM, &exorterm155_device::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_size(80*9, 24*12);
	m_screen->set_visarea(0, 80*9-1, 0, 24*12-1);
	m_screen->set_screen_update(FUNC(exorterm155_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	config.set_default_layout(layout_exorterm155);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set(FUNC(exorterm155_device::acia_txd_w));
	m_acia->rts_handler().set(FUNC(exorterm155_device::acia_rts_w));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<1>().set(FUNC(exorterm155_device::write_f1_clock));
	m_brg->out_f<3>().set(FUNC(exorterm155_device::write_f3_clock));
	m_brg->out_f<5>().set(FUNC(exorterm155_device::write_f5_clock));
	m_brg->out_f<6>().set(FUNC(exorterm155_device::write_f6_clock));
	m_brg->out_f<7>().set(FUNC(exorterm155_device::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(exorterm155_device::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(exorterm155_device::write_f9_clock));
	m_brg->out_f<11>().set(FUNC(exorterm155_device::write_f11_clock));
	m_brg->out_f<13>().set(FUNC(exorterm155_device::write_f13_clock));

	PIA6821(config, m_pia_kbd);
	m_pia_kbd->readpa_handler().set(FUNC(exorterm155_device::pia_kbd_pa_r));
	m_pia_kbd->readpb_handler().set(FUNC(exorterm155_device::pia_kbd_pb_r));
	m_pia_kbd->writepb_handler().set(FUNC(exorterm155_device::pia_kbd_pb_w));
	m_pia_kbd->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_pia_kbd->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia_cfg);
	m_pia_cfg->readpa_handler().set_ioport("DIP_SWITCHES_A");
	m_pia_cfg->readpb_handler().set_ioport("DIP_SWITCHES_B");
	m_pia_cfg->cb2_handler().set(FUNC(exorterm155_device::pia_cfg_cb2_w));
	m_pia_cfg->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_pia_cfg->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia_disp);
	m_pia_disp->writepa_handler().set(FUNC(exorterm155_device::pia_disp_pa_w));
	m_pia_disp->readpb_handler().set(FUNC(exorterm155_device::pia_disp_pb_r));
	m_pia_disp->writepb_handler().set(FUNC(exorterm155_device::pia_disp_pb_w));
	m_pia_disp->ca2_w(0);
	m_pia_disp->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
	m_pia_disp->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<6>));

	// Derived from the horizontal blanking, 3.4ms.
	CLOCK(config, m_sys_timer_clock, 4706 / 16);
	m_sys_timer_clock->signal_handler().set(FUNC(exorterm155_device::sys_timer_w));

	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, 2000);
	m_beeper->add_route(ALL_OUTPUTS, "bell", 0.25);
}

ioport_constructor exorterm155_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(exorterm155);
}

const tiny_rom_entry *exorterm155_device::device_rom_region() const
{
	return ROM_NAME(exorterm155);
}

DEFINE_DEVICE_TYPE(EXORTERM155, exorterm155_device, "exorterm155_device", "EXORTERM155")
