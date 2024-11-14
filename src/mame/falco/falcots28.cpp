// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Falco Endura TS-28 (aka FM-II?)

    VT100/VT102/VT132 terminal

    Hardware:
    - Z80A (Z8400A PS)
    - 6116
    - SY2128-3 x2 (and two empty sockets)
    - 2x 2764 (and one empty socket)
    - 1x 2732 (chargen)
    - Z80ACTC
    - SCN2672 with SCB2673
    - Z80ADART
    - XTAL 16.537 MHz (hard to read)
    - Bell
    - DAC0832LCN (brightness control)

    TODO:
    - Some scrolling issues (GNU nano for example)
    - Bell is really silent

    Notes:
    - Starts with a relatively dark screen. Push FUNCTION (mapped to LALT)
      and cursor up to increase brightness.
    - Keytek inductric keyboard

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "video/scn2674.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ts28_state : public driver_device
{
public:
	ts28_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_pvtc(*this, "pvtc"),
		m_dart(*this, "dart"),
		m_chargen(*this, "chargen"),
		m_keys(*this, "key_%x", 0U)
	{ }

	void ts28(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2672_device> m_pvtc;
	required_device<z80dart_device> m_dart;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport_array<16> m_keys;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void key_scan_w(uint8_t data);

	void crt_brightness_w(uint8_t data);
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	uint8_t m_key_scan = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ts28_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xafff).ram().share("charram");
	map(0xb000, 0xbfff).ram().share("attrram");
}

void ts28_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe1, 0xe1).w(FUNC(ts28_state::crt_brightness_w));
	map(0xe3, 0xe3).w(FUNC(ts28_state::key_scan_w));
	map(0xe8, 0xef).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0xf0, 0xf3).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0xf8, 0xfb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( ts28 )
	PORT_START("key_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)      PORT_NAME("Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)        PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("key_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)     PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)      PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)      PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)      PORT_NAME("Left Shift")

	PORT_START("key_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)    PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)    PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Function")

	PORT_START("key_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("key_4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)    PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("key_5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("key_6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)    PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("key_7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)       PORT_NAME("Edit  Edit F")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)      PORT_NAME("Correct")

	PORT_START("key_8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_NAME("Print  Exten")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Right Shift")

	PORT_START("key_9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(HOME))  PORT_NAME("Home  Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                    PORT_NAME("Back Space  Break")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_NAME("Keypad 7  Ins Char")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_NAME("Keypad 4  Erase Line")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)                   PORT_NAME("Return")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_NAME("Keypad 1  Send Line")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)                                  PORT_NAME("Line Feed")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("key_a")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME("\xe2\x86\x93  Dim")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))    PORT_NAME("\xe2\x86\x91  Brite")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)   PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_NAME("Keypad 8  Del Char")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)   PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_NAME("Keypad 5  Erase Page")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)   PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_NAME("Keypad 2  Send Page")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)   PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("key_b")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME("\xe2\x86\x92")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME("\xe2\x86\x90")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("Keypad -  Del Line")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     PORT_NAME("Keypad 9  Ins Line")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     PORT_NAME("Keypad 6  Erase")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)                                      PORT_NAME("Keypad ,")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START("key_c")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("key_d")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("key_e")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("key_f")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD
//**************************************************************************

void ts28_state::key_scan_w(uint8_t data)
{
	int i = BIT(data, 4, 3);
	int j = BIT(data, 0, 4);

	m_dart->rib_w(BIT(m_keys[j]->read(), i));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void ts28_state::crt_brightness_w(uint8_t data)
{
	m_screen->set_brightness(data);
}

void ts28_state::char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
}

void ts28_state::attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("attrram");
}

SCN2672_DRAW_CHARACTER_MEMBER( ts28_state::draw_character )
{
	// 76------  unknown
	// --5-----  normal/bold
	// ---43---  unknown
	// -----2--  invert
	// ------1-  blink
	// -------0  underline

	uint16_t data = m_chargen[charcode << 4 | linecount];
	const pen_t *const pen = m_palette->pens();

	if (ul && (BIT(attrcode, 0)))
		data = 0x1ff;

	if (blink && (BIT(attrcode, 1)))
		data = 0x000;

	if (BIT(attrcode, 2))
		data = ~data;

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = BIT(attrcode, 5) ? pen[1] : pen[2];
	rgb_t bg = pen[0];

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, i) ? fg : bg;
}

static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP16(0, 8) },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ts28_state::machine_start()
{
	// register for save states
	save_item(NAME(m_key_scan));
}

void ts28_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" }, // interrupts not enabled, might not be part of the chain
	{ "dart" },
	{ nullptr }
};

void ts28_state::ts28(machine_config &config)
{
	Z80(config, m_maincpu, 16.537_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ts28_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ts28_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber()); // unknown color
	m_screen->set_raw(16.537_MHz_XTAL, 864, 0, 720, 319, 0, 300);
	m_screen->set_screen_update(m_pvtc, FUNC(scn2672_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2672(config, m_pvtc, 16.537_MHz_XTAL / 9);
	m_pvtc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9);
	m_pvtc->set_display_callback(FUNC(ts28_state::draw_character));
	m_pvtc->set_addrmap(0, &ts28_state::char_map);
	m_pvtc->set_addrmap(1, &ts28_state::attr_map);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16.537_MHz_XTAL / 4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(16.537_MHz_XTAL / 9);
	ctc.zc_callback<0>().set(m_dart, FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<0>().append(m_dart, FUNC(z80dart_device::txca_w));
	ctc.set_clk<1>(16.537_MHz_XTAL / 9);
	ctc.zc_callback<1>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	ctc.set_clk<2>(16.537_MHz_XTAL / 9);
	ctc.zc_callback<2>().set("bell", FUNC(speaker_sound_device::level_w));

	Z80DART(config, m_dart, 16.537_MHz_XTAL / 4);
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dart->out_txda_callback().set("porta", FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsa_callback().set("porta", FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set("portb", FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsb_callback().set("portb", FUNC(rs232_port_device::write_rts));

	rs232_port_device &porta(RS232_PORT(config, "porta", default_rs232_devices, nullptr));
	porta.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	porta.cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));

	rs232_port_device &portb(RS232_PORT(config, "portb", default_rs232_devices, nullptr));
	portb.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	portb.cts_handler().set(m_dart, FUNC(z80dart_device::ctsb_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SPEAKER_SOUND(config, "bell").add_route(ALL_OUTPUTS, "mono", 1.00);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ts28 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("endura_v100_0.e27", 0x0000, 0x2000, CRC(2fd1ea7f) SHA1(711e09b843ecf2e463bd2b193a66842261512fa4))
	ROM_LOAD("endura_v100_1.e28", 0x2000, 0x2000, CRC(4de83d27) SHA1(a1c0858debfb0c1e82b44c620855a13dc1626302))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("chargen.f3", 0x0000, 0x1000, CRC(f0764798) SHA1(3d4826bfe1f1e533ee2134ad9ae6c2512edbb069))

	ROM_REGION(0x40, "prom", 0)
	ROM_LOAD("82s123.f3", 0x00, 0x20, CRC(862b9680) SHA1(0d19266cb9680e0e5dd92d230b64a5ee86bf6046))
	ROM_LOAD("82s123.h7", 0x00, 0x20, CRC(275a4436) SHA1(e986454f6f0f93f72b1c43a49ff3ded41630e38b))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME        FLAGS
COMP( 1983, ts28, 0,       0,     ts28,    ts28,  ts28_state, empty_init, "Falco", "Endura TS-28", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
