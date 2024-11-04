// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

    TeleVideo Model 910 / 910 Plus

    Hardware:
    6502 CPU
    6545 CRTC
    6551 ACIA

    IRQ = ACIA gated with flip-flop driven by CRTC VBlank (not wire-OR)
    NMI = AY-5-3600 keyboard char present

    Esc-V (with a capital V) brings up the self-test screen.

    TODO:
        - Reverse attribute handling on the self-test screen doesn't seem
          to match the picture shown in the Operator's Manual
        - Make 910 keyboard into a device since the similar Model 925 uses
          a 950-compatible serial keyboard instead (with a second ACIA)
        - Add printer port and remaining DIP switches

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/input_merger.h"
#include "machine/kb3600.h"
#include "machine/mos6551.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define ACIA_TAG    "acia"
#define CRTC_TAG    "crtc"
#define RS232_TAG   "rs232"
#define KBDC_TAG    "ay3600"

#define MASTER_CLOCK 13.608_MHz_XTAL

class tv910_state : public driver_device
{
public:
	tv910_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_crtc(*this, CRTC_TAG)
		, m_vram(*this, "vram")
		, m_chrrom(*this, "graphics")
		, m_ay3600(*this, KBDC_TAG)
		, m_kbdrom(*this, "keyboard")
		, m_kbspecial(*this, "keyb_special")
		, m_beep(*this, "bell")
		, m_dsw1(*this, "DSW1")
		, m_charset(*this, "CHARSET")
	{ }

	void tv910(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	uint8_t charset_r();
	uint8_t kbd_ascii_r();
	uint8_t kbd_flags_r();

	void vbl_ack_w(uint8_t data);
	void nmi_ack_w(uint8_t data);
	void control_w(uint8_t data);

	void vbl_w(int state);

	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	void ay3600_ako_w(int state);

	void tv910_mem(address_map &map) ATTR_COLD;

	required_device<m6502_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<r6545_1_device> m_crtc;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint8_t> m_chrrom;
	required_device<ay3600_device> m_ay3600;
	required_region_ptr<uint8_t> m_kbdrom;
	required_ioport m_kbspecial;
	required_device<beep_device> m_beep;
	required_ioport m_dsw1;
	required_ioport m_charset;

	uint16_t m_lastchar, m_strobe;
	uint8_t m_transchar;
	bool m_anykeydown;
	int m_repeatdelay;

	uint8_t m_control;
};

void tv910_state::tv910_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x47ff).ram().share("vram"); // VRAM
	map(0x8010, 0x801f).r(FUNC(tv910_state::charset_r));
	map(0x8020, 0x8020).rw(m_crtc, FUNC(r6545_1_device::status_r), FUNC(r6545_1_device::address_w));
	map(0x8021, 0x8021).rw(m_crtc, FUNC(r6545_1_device::register_r), FUNC(r6545_1_device::register_w));
	map(0x8030, 0x8033).rw(ACIA_TAG, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x8040, 0x804f).w(FUNC(tv910_state::vbl_ack_w));
	map(0x8050, 0x805f).w(FUNC(tv910_state::nmi_ack_w));
	map(0x8060, 0x806f).r(FUNC(tv910_state::kbd_ascii_r));
	map(0x8070, 0x807f).r(FUNC(tv910_state::kbd_flags_r));
	map(0x9000, 0x9000).w(FUNC(tv910_state::control_w));
	map(0x9001, 0x9001).portr("DSW1");
	map(0x9002, 0x9002).portr("DSW2");
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void tv910_state::control_w(uint8_t data)
{
	m_control = data;
	#if 0
	printf("%02x to control (%c%c%c%c%c)\n",
		data,
		(data & 0x10) ? 'U' : 'B',
		(data & 0x8) ? '6' : '5',
		(data & 0x4) ? 'X' : ' ',
		(data & 0x2) ? 'C' : 'c',
		(data & 1) ? 'B' : ' ');
	#endif

	m_beep->set_state(BIT(data, 0));
}

uint8_t tv910_state::charset_r()
{
	return m_charset->read();
}

void tv910_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6502_NMI_LINE, CLEAR_LINE);
	m_strobe = 0;
}

uint8_t tv910_state::kbd_ascii_r()
{
	return m_transchar;
}

uint8_t tv910_state::kbd_flags_r()
{
	uint8_t rv = 0;
	ioport_value kbspecial = m_kbspecial->read();

	// D0: Keyboard strobe (AY-5-3600 AKO)
	if (m_anykeydown)
		rv |= 0x01;

	// D1: Printer DTR (pin 20)

	// D6: FUNC key (not ALPHA LOCK as indicated in memory map)
	if (BIT(kbspecial, 4))
		rv |= 0x40;

	// D7: ALPHA LOCK key (not FUNC as indicated in memory map)
	if (BIT(kbspecial, 0))
		rv |= 0x80;

	return rv;
}

int tv910_state::ay3600_shift_r()
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

int tv910_state::ay3600_control_r()
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

void tv910_state::ay3600_data_ready_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_lastchar = m_ay3600->b_r();
		m_transchar = m_kbdrom[m_lastchar];
		m_strobe = 1;

		m_maincpu->set_input_line(M6502_NMI_LINE, ASSERT_LINE);
		//printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);
	}
}

void tv910_state::ay3600_ako_w(int state)
{
	m_anykeydown = (state == ASSERT_LINE) ? true : false;

	if (m_anykeydown)
	{
		m_repeatdelay = 10;
	}
}

/* Input ports */

/*
Keyboard matrix (thanks to Al Kossow!)

   X0     X1     X2     X3     X4     X5     X6     X7
Y8                             BKTAB  FN19   FN18   FN17
Y7  B     3      E      F      [      RET    `      {
Y6  .     7      U      K      BRK    SPACE  BS     HOME
Y5  V     2      W      D      P      ENTER  0      DEL
Y4  ,     6      Y      J      '      /      9      DOWN
Y3  C     1      Q      S      O      ;      =      CLRSP
Y2  M     5      T      H             LEFT   8      UP
Y1  X     ESC    TAB    A      I      L      -      Z
Y0  N     4      R      G      LF     PRNT   \      RIGHT
*/

static INPUT_PORTS_START( tv910 )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")              PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")              PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	/// 001 - LF
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	/// 040 - BRK
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR(']')
	/// 100 - BACKTAB

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PRTSCR)    PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2190")         PORT_CODE(KEYCODE_LEFT) // ←
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")           PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F19)       PORT_CHAR(UCHAR_MAMEKEY(F19))

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace")        PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F18)       PORT_CHAR(UCHAR_MAMEKEY(F18))

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2192")         PORT_CODE(KEYCODE_RIGHT) // →
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2191")         PORT_CODE(KEYCODE_UP) // ↑
/// 008 - CLRSP
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2193")         PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)      // ↓  E0 47
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Func") PORT_CODE(KEYCODE_LALT)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Baud Rate" ) PORT_DIPLOCATION("S1:4,3,2,1")
	PORT_DIPSETTING(    0x1, "50" )
	PORT_DIPSETTING(    0x2, "75" )
	PORT_DIPSETTING(    0x3, "110" )
	PORT_DIPSETTING(    0x4, "135" )
	PORT_DIPSETTING(    0x5, "150" )
	PORT_DIPSETTING(    0x6, "300" )
	PORT_DIPSETTING(    0x7, "600" )
	PORT_DIPSETTING(    0x8, "1200" )
	PORT_DIPSETTING(    0x9, "1800" )
	PORT_DIPSETTING(    0xa, "2400" )
	PORT_DIPSETTING(    0xb, "3600" )
	PORT_DIPSETTING(    0xc, "4800" )
	PORT_DIPSETTING(    0xd, "7200" )
	PORT_DIPSETTING(    0x0, "9600" )
	//PORT_DIPSETTING(  0xe, "9600" )
	PORT_DIPSETTING(    0xf, "19200" )

	PORT_DIPNAME( 0x10, 0x00, "Word Length" ) PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING( 0x00, "8 data bits" )
	PORT_DIPSETTING( 0x10, "7 data bits" )

	PORT_DIPNAME( 0x20, 0x00, "Parity" ) PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING( 0x00, "No parity" )
	PORT_DIPSETTING( 0x20, "Send parity" )

	PORT_DIPNAME( 0x40, 0x00, "Parity Type" ) PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING( 0x00, "Odd" )
	PORT_DIPSETTING( 0x40, "Even" )

	PORT_DIPNAME( 0x80, 0x00, "Stop Bits" ) PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPSETTING( 0x80, "2" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "CR Code" ) PORT_DIPLOCATION("S1:10") // TCHAR0
	PORT_DIPSETTING( 0x00, "CR only" )
	PORT_DIPSETTING( 0x01, "CRLF" )

	PORT_DIPNAME( 0x02, 0x02, "Auto Wraparound at 80th Position" ) PORT_DIPLOCATION("S1:9") // TCHAR1
	PORT_DIPSETTING( 0x00, DEF_STR(No) )
	PORT_DIPSETTING( 0x02, DEF_STR(Yes) ) // required for self-test to work

	PORT_DIPNAME( 0x0c, 0x00, "Emulation" ) PORT_DIPLOCATION("S2:1,2")
	PORT_DIPSETTING(    0x0, "Standard 910" )
	PORT_DIPSETTING(    0x4, "ADM-3A/5" )
	PORT_DIPSETTING(    0x8, "ADDS 25" )
	PORT_DIPSETTING(    0xc, "Hazeltine 1410" )

	PORT_DIPNAME( 0x10, 0x00, "Refresh Rate" ) PORT_DIPLOCATION("S2:3")
	PORT_DIPSETTING( 0x00, "60 Hz" )
	PORT_DIPSETTING( 0x10, "50 Hz" )

	PORT_DIPNAME( 0x60, 0x00, "Cursor Type" ) PORT_DIPLOCATION("S2:4,5")
	PORT_DIPSETTING(    0x00, "Blinking block" )
	PORT_DIPSETTING(    0x40, "Blinking underline" )
	PORT_DIPSETTING(    0x20, "Steady block" )
	PORT_DIPSETTING(    0x60, "Steady underline" )

	PORT_DIPNAME( 0x80, 0x00, "Conversation Mode" ) PORT_DIPLOCATION("S2:6") // F/HDX
	PORT_DIPSETTING( 0x00, "Half duplex" )
	PORT_DIPSETTING( 0x80, "Full duplex" )

	PORT_DIPNAME( 0x100, 0x100, "Colors" ) PORT_DIPLOCATION("S2:7") // BOW/WOB
	PORT_DIPSETTING( 0x00, "Black characters on green screen" )
	PORT_DIPSETTING( 0x100, "Green characters on black screen" )

#if 0
	PORT_DIPNAME( 0x200, 0x200, "Data Set Ready" )
	PORT_DIPSETTING( 0x00, "DSR connected" )
	PORT_DIPSETTING( 0x200, "DSR disconnected" )
#endif

	PORT_START("CHARSET") // actually a pair of jumpers: E4-E5 (bit 1), E6-E7 (bit 0)
	PORT_DIPNAME( 0x03, 0x00, "Character Set" )
	PORT_DIPSETTING( 0x00, "English" )
	PORT_DIPSETTING( 0x01, "German" )
	PORT_DIPSETTING( 0x02, "French" )
	PORT_DIPSETTING( 0x03, "Spanish" )
INPUT_PORTS_END

void tv910_state::machine_start()
{
	// DCD needs to be driven somehow, or else the terminal will complain
	// CTS also needs to be driven to prevent hanging caused by buffer overflow
	auto *acia = subdevice<mos6551_device>(ACIA_TAG);
	auto *rs232 = subdevice<rs232_port_device>(RS232_TAG);
	if (rs232->get_card_device() == nullptr)
	{
		acia->write_dcd(0);
		acia->write_cts(0);
	}

	// DSR is tied to GND
	acia->write_dsr(0);
}

void tv910_state::machine_reset()
{
	control_w(0);
}

MC6845_ON_UPDATE_ADDR_CHANGED( tv910_state::crtc_update_addr )
{
}

void tv910_state::vbl_w(int state)
{
	// this is ACKed by vbl_ack_w, state going 0 here doesn't ack the IRQ
	if (state)
		m_mainirq->in_w<0>(1);
}

void tv910_state::vbl_ack_w(uint8_t data)
{
	m_mainirq->in_w<0>(0);
}

MC6845_UPDATE_ROW( tv910_state::crtc_update_row )
{
	uint32_t  *p = &bitmap.pix(y);
	uint16_t  chr_base = (ra & 7) | m_charset->read() << 10;
	uint8_t   chr = m_vram[0x7ff];
	uint8_t   att = (chr & 0xf0) == 0x90 ? chr & 0x0f : 0;
	bool      bow = BIT(m_dsw1->read(), 8);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t offset = ( ma + i ) & 0x7ff;
		uint8_t chr = m_vram[ offset ];
		bool att_blk = (chr & 0xf0) == 0x90;
		bool half_int = BIT(chr, 7) && !att_blk;
		if (att_blk)
			att = chr & 0x0f;

		uint8_t data = m_chrrom[chr_base | (chr & 0x7f) << 3];
		rgb_t fg = rgb_t::green();
		rgb_t bg = rgb_t::black();
		if (half_int)
			fg = rgb_t(fg.r() / 2, fg.g() / 2, fg.b() / 2);

		if (ra == 9)
			data = BIT(att, 3) ? 0xff : 0;
		else if (ra == 0 || att_blk || BIT(att, 0) || (BIT(att, 1) && BIT(m_control, 1)))
			data = 0;

		if (i == cursor_x && (!BIT(m_control, 4) || ra == 9))
			data ^= 0xff;
		if (BIT(att, 2))
			data ^= 0xff;
		if (bow)
			data ^= 0xff;

		*p++ = BIT(data, 7) ? bg : fg;
		*p++ = BIT(data, 6) ? bg : fg;
		*p++ = BIT(data, 5) ? bg : fg;
		*p++ = BIT(data, 4) ? bg : fg;
		*p++ = BIT(data, 3) ? bg : fg;
		*p++ = BIT(data, 2) ? bg : fg;
		*p++ = BIT(data, 1) ? bg : fg;
		*p++ = BIT(data, 0) ? bg : fg;
	}
}

void tv910_state::tv910(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &tv910_state::tv910_mem);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK, 840, 0, 640, 270, 0, 240);
	screen.set_screen_update(CRTC_TAG, FUNC(r6545_1_device::screen_update));

	R6545_1(config, m_crtc, MASTER_CLOCK/8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(tv910_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(tv910_state::crtc_update_addr));
	m_crtc->out_vsync_callback().set(FUNC(tv910_state::vbl_w));

	AY3600(config, m_ay3600, 0);
	m_ay3600->x0().set_ioport("X0");
	m_ay3600->x1().set_ioport("X1");
	m_ay3600->x2().set_ioport("X2");
	m_ay3600->x3().set_ioport("X3");
	m_ay3600->x4().set_ioport("X4");
	m_ay3600->x5().set_ioport("X5");
	m_ay3600->x6().set_ioport("X6");
	m_ay3600->x7().set_ioport("X7");
	m_ay3600->x8().set_ioport("X8");
	m_ay3600->shift().set(FUNC(tv910_state::ay3600_shift_r));
	m_ay3600->control().set(FUNC(tv910_state::ay3600_control_r));
	m_ay3600->data_ready().set(FUNC(tv910_state::ay3600_data_ready_w));
	m_ay3600->ako().set(FUNC(tv910_state::ay3600_ako_w));

	mos6551_device &acia(MOS6551(config, ACIA_TAG, 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	acia.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(ACIA_TAG, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(ACIA_TAG, FUNC(mos6551_device::write_dcd));
	rs232.cts_handler().set(ACIA_TAG, FUNC(mos6551_device::write_cts));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, MASTER_CLOCK / 8400); // 1620 Hz (Row 10 signal)
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( tv910 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "1800000-020e_a38_9182.bin", 0x000000, 0x001000, CRC(ae71dd7f) SHA1(a12da9329e28a4a8e3c902f795059251311d2856) )

	ROM_REGION(0x2000, "graphics", 0)
	ROM_LOAD( "1800000-016a_a17_85ae.bin", 0x000000, 0x001000, CRC(835445b7) SHA1(dde94fb6531dadce48e19bf551f45f61bedf905b) )

	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD( "1800000-019b_bell_a2_43d6.bin", 0x000000, 0x000800, CRC(de954a77) SHA1(c4f7c19799c15d12d89f08dc31064fc6be9befb0) )
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY              FULLNAME               FLAGS
COMP( 1981, tv910, 0,      0,      tv910,   tv910, tv910_state, empty_init, "TeleVideo Systems", "TeleVideo Model 910", 0 )
