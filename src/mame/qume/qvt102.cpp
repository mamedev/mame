// license:BSD-3-Clause
// copyright-holders:Dirk Best
/****************************************************************************

    Qume QVT-102/QVT-102A video terminal

    Hardware:
    - Motorola 6800 CPU
    - Hitachi HD46505SP (Motorola 6845-compatible) CRTC
    - Hitachi HD46850 (Motorola 6850-compatible) ACIA
    - M58725P-15 (6116-compatible) (2k x 8bit RAM)
    - Zilog Z8430 CTC
    - 16.6698MHz Crystal
    - 2x TC5514-APL + 3V battery, functioning as NVRAM

    Keyboard: D8748D, 6.000MHz Crystal, Beeper

    For the QVT102, the 'Setup' function is entered by typing F11. The 'left'
    and 'right' arrow keys then move between entries on a line, and the 'up'
    and 'down' arrow keys move between lines. The space bar cycles through
    options for an entry. Shift-S saves the values to NVRAM; F11 exits without
    saving; Shift-D resets to the default values; and Shift-R restores from
    the saved values.

    TODO:
    - Support QVT-102A differences (bidirectional aux, different keyboard)
    - Key click sounds weird

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "sound/spkrdev.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class qvt102_state : public driver_device
{
public:
	qvt102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_kbdmcu(*this, "kbdmcu")
		, m_keys_p1(*this, "P1_%u", 0U)
		, m_keys_p2(*this, "P2_%u", 0U)
		, m_keys_special(*this, "SPECIAL")
		, m_jumper(*this, "JUMPER")
		, m_acia(*this, "acia")
		, m_host(*this, "host")
		, m_aux(*this, "aux")
		, m_ctc(*this, "ctc")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
		, m_vram(*this, "videoram")
		, m_char_rom(*this, "chargen")
		, m_latch(0)
		, m_kbd_data(0)
		, m_kbd_bus(0xff), m_kbd_p1(0xff), m_kbd_p2(0xff)
	{ }

	void qvt102(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	required_device<m6800_cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<i8748_device> m_kbdmcu;
	required_ioport_array<8> m_keys_p1;
	required_ioport_array<8> m_keys_p2;
	required_ioport m_keys_special;
	required_ioport m_jumper;
	required_device<acia6850_device> m_acia;
	required_device<rs232_port_device> m_host;
	required_device<rs232_port_device> m_aux;
	required_device<z80ctc_device> m_ctc;
	required_device<hd6845s_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<u8> m_char_rom;

	void mem_map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);

	uint8_t vsync_ack_r();
	void vsync_w(int state);
	uint8_t kbd_r();
	void latch_w(uint8_t data);

	uint8_t ctc_r();
	void ctc_w(uint8_t data);

	void acia_txd_w(int state);
	void acia_rts_w(int state);
	void dsr_w(int state);
	void host_rxd_w(int state);
	void host_dcd_w(int state);

	uint8_t m_latch;
	int m_kbd_data;

	// keyboard mcu
	uint8_t mcu_bus_r();
	void mcu_bus_w(uint8_t data);
	int mcu_t0_r();
	int mcu_t1_r();
	void mcu_p1_w(uint8_t data);
	void mcu_p2_w(uint8_t data);

	uint8_t m_kbd_bus, m_kbd_p1, m_kbd_p2;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void qvt102_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("nvram");
	map(0x2800, 0x2800).rw(FUNC(qvt102_state::ctc_r), FUNC(qvt102_state::ctc_w));
	map(0x3000, 0x3000).r(FUNC(qvt102_state::vsync_ack_r));
	map(0x3800, 0x3800).w(FUNC(qvt102_state::latch_w));
	map(0x4000, 0x47ff).ram().share("videoram").mirror(0x3800);
	map(0x8000, 0x8000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x9800, 0x9801).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa000, 0xa000).r(FUNC(qvt102_state::kbd_r));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( qvt102 )
	PORT_START("P1_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')

	PORT_START("P1_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')

	PORT_START("P1_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')

	PORT_START("P1_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')

	PORT_START("P1_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')

	PORT_START("P1_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')

	PORT_START("P1_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')

	PORT_START("P1_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')

	PORT_START("P2_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                                  PORT_NAME("PF4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                                  PORT_NAME("PF2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(10)                      PORT_NAME("Line Feed")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))       PORT_NAME("Setup")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))    PORT_NAME("Break")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))    PORT_NAME("Print")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME("\xe2\x86\x93") // ↓
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME("\xe2\x86\x90") // ←
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('~')  PORT_CHAR('`')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   PORT_NAME("Keypad ,")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  PORT_NAME("Keypad .")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                       PORT_NAME("Return") // ↵
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))   PORT_NAME("No Scroll")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                        PORT_NAME("Back Space")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                                   PORT_NAME("PF3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                                   PORT_NAME("PF1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME("\xe2\x86\x91") // ↑
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))      PORT_NAME("Clear / Home")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME("\xe2\x86\x92") // →
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SPECIAL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1) // ⇧
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("JUMPER")
	PORT_DIPNAME(0x01, 0x00, "EIA DSR (W1)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x02, "EIA DCD (W2)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPNAME(0x04, 0x04, "EIA DTR (W3)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x00, "EIA RTS (W4)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x30, 0x00, "Character Set")
	PORT_DIPSETTING(   0x00, "US (W17)")
	PORT_DIPSETTING(   0x10, "GM (W18)")
	PORT_DIPSETTING(   0x20, "UK (W19)")
	PORT_DIPNAME(0x40, 0x00, "Attribute Code Intensity")
	PORT_DIPSETTING(   0x00, "Half (W21)")
	PORT_DIPSETTING(   0x40, "Full (W22)")
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD
//**************************************************************************

// 7------- unused
// -654---- row select/special keys
// ----3--- unknown
// -----21- unused
// -------0 speaker

uint8_t qvt102_state::mcu_bus_r()
{
	return m_kbd_bus;
}

void qvt102_state::mcu_bus_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 0));
	m_kbd_bus = data;
}

int qvt102_state::mcu_t0_r()
{
	int state = 1;

	// The keyboard firmware also scans for a key at bit 3, and it appears
	// to be a modifier key and is passed to the host in bit 3 of the
	// second code, but the terminal firmware appears to ignore it. The
	// schematics show no sign of a connection.

	if (BIT(m_kbd_bus, 4) == 0) state &= BIT(m_keys_special->read(), 0);
	if (BIT(m_kbd_bus, 5) == 0) state &= BIT(m_keys_special->read(), 1);
	if (BIT(m_kbd_bus, 6) == 0) state &= BIT(m_keys_special->read(), 2);

	return state;
}

int qvt102_state::mcu_t1_r()
{
	int state = 1;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbd_p1, i) == 0)
			state &= BIT(m_keys_p1[(m_kbd_bus >> 4) & 7]->read(), i);
		if (BIT(m_kbd_p2, i) == 0)
			state &= BIT(m_keys_p2[(m_kbd_bus >> 4) & 7]->read(), i);
	}

	return state;
}

void qvt102_state::mcu_p1_w(uint8_t data)
{
	m_kbd_p1 = data;
}

void qvt102_state::mcu_p2_w(uint8_t data)
{
	m_kbd_p2 = data;

	m_kbd_data = !BIT(data, 7);

	// The keyboard serial data is read by the host using a bit banger in
	// the IRQ handler and it is a tight loop requiring good
	// synchronization between the host CPU and keyboard controller.  A
	// word starts with a raising of the line which (inverted) triggers
	// the IRQ and the handler then waits and synchronizes to a falling
	// edge. There is a delay of about 189us before this falling edge. The
	// IRQ handler then reads the bits in a tight loop, reading the last
	// after around 332 usec.
	//
	// The strategy here is to boost the interleave when the line is
	// written high, and to hold this boost for 350us. This ensures that
	// the boost lasts for the critical section of the IRQ handler.
	//
	if (m_kbd_data)
		machine().scheduler().perfect_quantum(attotime::from_usec(350));

	m_irqs->in_w<2>(m_kbd_data);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint8_t qvt102_state::vsync_ack_r()
{
	m_irqs->in_w<0>(CLEAR_LINE);
	return 0;
}

void qvt102_state::vsync_w(int state)
{
	if (state)
		m_irqs->in_w<0>(ASSERT_LINE);
}

MC6845_UPDATE_ROW( qvt102_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	// line attribute (active for the rest of the line)
	uint8_t attr = 0;

	for (int x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_vram[mem];

		// address lines 10 and 11 for the character rom get a special handling
		int a10 = (chr & 0x40) || ((chr & 0xe0) == 0);
		int a11 = 0;

		switch (m_jumper->read() & 0x30)
		{
		case 0x00: a11 = 0; break;    // US
		case 0x10: a11 = a10; break;  // German
		case 0x20: a11 = !a10; break; // UK
		}

		uint16_t gfx = m_char_rom[(a11 << 11) | (a10 << 10) | ((chr << 4) & 0x3f0) | ra] << 1;

		// d0 for drawing codes (avoids gap)
		if (chr >= 0x80 && chr <= 0x9f)
			gfx |= BIT(gfx, 1);

		// half intensity is defined per character
		int half = BIT(chr, 7);

		// check for new attribute
		if (chr >= 0x90 && chr <= 0x9f)
		{
			attr = chr;
			half ^= BIT(m_jumper->read(), 6);
		}

		// draw 9 pixels of the character
		for (int i = 0; i < 9; i++)
		{
			// from schematics
			bool p = !(BIT(m_latch, 3) & BIT(attr, 1)); // blink
			p = !(p & BIT(gfx, i) & (!BIT(attr, 0))); // rom/blank
			p = !(p & !((BIT(attr, 3) & (ra == 11)))); // underline
			p = p ^ (BIT(attr, 2) ^ ((x == cursor_x) | BIT(m_latch, 2))); // reverse

			bitmap.pix(y, x*9 + (8-i)) = palette[p ? 2 - half : 0];
		}

	}
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

void qvt102_state::latch_w(uint8_t data)
{
	// 7------- host to keyboard
	// -6------ aux print
	// --5----- ctc cs0
	// ---4---- dtri
	// ----3--- blink
	// -----2-- rv
	// ------1- eia/cl
	// -------0 aux en

	m_latch = data;

	// jumper w3 enables this connection
	if (machine().phase() != machine_phase::INIT)
		if (BIT(m_jumper->read(), 2))
			m_host->write_dtr(BIT(data, 4));

	m_kbdmcu->set_input_line(MCS48_INPUT_IRQ, BIT(data, 7) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t qvt102_state::kbd_r()
{
	// 7------- keyboard to host
	// -6------ dtr2

	uint8_t data = 0;

	data |= !(m_aux->dsr_r() || m_aux->dcd_r()) << 6;
	data |= !m_kbd_data << 7;

	return data;
}

uint8_t qvt102_state::ctc_r()
{
	return m_ctc->read(BIT(m_latch, 5));
}

void qvt102_state::ctc_w(uint8_t data)
{
	m_ctc->write(BIT(m_latch, 5), data);
}

void qvt102_state::acia_txd_w(int state)
{
	if (BIT(m_latch, 6) == 0)
	{
		// print
		m_aux->write_txd(state);
	}
	else
	{
		// aux enable
		if (BIT(m_latch, 0) == 0)
			m_aux->write_txd(state);

		m_host->write_txd(state);
	}
}

void qvt102_state::acia_rts_w(int state)
{
	m_host->write_rts(state);

	// jumper w4 enables rts output to dtr
	if (machine().phase() != machine_phase::INIT)
		if (BIT(m_jumper->read(), 3))
			m_host->write_dtr(state);
}

void qvt102_state::dsr_w(int state)
{
	if (machine().phase() != machine_phase::INIT)
		if (BIT(m_jumper->read(), 0))
			m_acia->write_dcd(state);
}

void qvt102_state::host_rxd_w(int state)
{
	m_acia->write_rxd(state);

	// aux enable
	if (BIT(m_latch, 0) == 0)
		m_aux->write_txd(state);
}

void qvt102_state::host_dcd_w(int state)
{
	if (machine().phase() != machine_phase::INIT)
		if (BIT(m_jumper->read(), 1))
			m_acia->write_dcd(state);
}

void qvt102_state::machine_start()
{
	m_kbd_data = 0;
	m_kbd_bus = m_kbd_p1 = m_kbd_p2 = 0xff;

	// register for save states
	save_item(NAME(m_latch));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_kbd_bus));
	save_item(NAME(m_kbd_p1));
	save_item(NAME(m_kbd_p2));
}

void qvt102_state::machine_reset()
{
	m_latch = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void qvt102_state::qvt102(machine_config &config)
{
	M6800(config, m_maincpu, 16.6698_MHz_XTAL / 18);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt102_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x TC5514-APL + 3V battery

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(16.6698_MHz_XTAL, 882, 9, 729, 315, 0, 300); // 80x24+1
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	HD6845S(config, m_crtc, 16.6698_MHz_XTAL / 9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(qvt102_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(qvt102_state::vsync_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set(FUNC(qvt102_state::acia_txd_w));
	m_acia->rts_handler().set(FUNC(qvt102_state::acia_rts_w));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->rxd_handler().set(FUNC(qvt102_state::host_rxd_w));
	m_host->cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	m_host->dsr_handler().set(FUNC(qvt102_state::dsr_w));
	m_host->dcd_handler().set(FUNC(qvt102_state::host_dcd_w));

	RS232_PORT(config, m_aux, default_rs232_devices, nullptr);
	m_aux->dsr_handler().set(FUNC(qvt102_state::dsr_w));

	Z80CTC(config, m_ctc, 16.6698_MHz_XTAL / 9);
	m_ctc->set_clk<0>(16.6698_MHz_XTAL / 18);
	m_ctc->set_clk<1>(16.6698_MHz_XTAL / 18);
	m_ctc->zc_callback<0>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_ctc->zc_callback<1>().set(m_acia, FUNC(acia6850_device::write_rxc));

	I8748(config, m_kbdmcu, 6_MHz_XTAL);
	m_kbdmcu->bus_in_cb().set(FUNC(qvt102_state::mcu_bus_r));
	m_kbdmcu->bus_out_cb().set(FUNC(qvt102_state::mcu_bus_w));
	m_kbdmcu->t0_in_cb().set(FUNC(qvt102_state::mcu_t0_r));
	m_kbdmcu->t1_in_cb().set(FUNC(qvt102_state::mcu_t1_r));
	m_kbdmcu->p1_out_cb().set(FUNC(qvt102_state::mcu_p1_w));
	m_kbdmcu->p2_out_cb().set(FUNC(qvt102_state::mcu_p2_w));

	// sound hardware (on keyboard)
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( qvt102 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("t205m.u8", 0x0000, 0x2000, CRC(59cc04f6) SHA1(ee2e3a3ea7b57a231483fcc74266f0f3f51204af))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("c3205m.u32", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb))

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("k301.u302",  0x000, 0x400, CRC(67564b20) SHA1(5897ff920f8fae4aa498d3a4dfd45b58183c041d))
ROM_END

ROM_START( qvt102a )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("t205p.u8", 0x0000, 0x2000, CRC(2e375abc) SHA1(12ad1e49c5773c36c3a8d65845c9a50f9dec141f))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("c3205m.u32", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb))

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("k301.u302", 0x000, 0x400, CRC(67564b20) SHA1(5897ff920f8fae4aa498d3a4dfd45b58183c041d))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME    FLAGS
COMP( 1983, qvt102,  0,      0,      qvt102,  qvt102,  qvt102_state, empty_init, "Qume",  "QVT-102",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, qvt102a, qvt102, 0,      qvt102,  qvt102,  qvt102_state, empty_init, "Qume",  "QVT-102A", MACHINE_SUPPORTS_SAVE )
