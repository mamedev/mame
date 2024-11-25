// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Lola 8 and Lola 8A

Ivo Lola Ribar Institute

2013-08-28 Skeleton driver.


Lola 8 and Lola 8 NK
====================

Lola 8 is predecessor Lola 8A, but essentially a different computer with more
differences than similarities.

D1 and D2 are sockets for ROMs, but 4K or 8K can be used (solder jumper)
D3 and D4 can be used for ROM or RAM chips.
D5,D6 and D7 only for RAM, all are 2K (HM6116 or MB8416)
F6 for video RAM is 1K or 2K, note that only 1K is actually used

Quartz crystal can be 15 or 16MHz (noted on board, default 16)

There are jumper solder connections on board named Z1-Z7 (that many I have
managed to detect on board), that are used to configure if a certain socket
is used for RAM or ROM and also size. This for sure is used to configure
video memory start, but hard to detect all differences on images.

PCB revision 112964

Lola 8 has an integrated matrix keyboard that appears to be with very strange
layout, but please note that char rom content, even being a good read, does not
have proper characters for ASCII codes of lowercase letters on expected places.

Lola 8 NK is using the same keyboard as Lola 8A later, just column 0 and 1 are
switched. PCB is same revision but it is missing lower part for integrated
matrix keyboard and new keyboard is connected over connector.

Lola 8A
=======

BASIC commands : (must be in UPPERcase)

    LET NEXT IF GOTO GOSUB RETURN READ DATA FOR CLS INPUT DIM STOP END RESTORE
    REM CLEAR PUSH POKE PRINT OUT ERROR USR CURSOR NORMAL INVERSE PLOT UNPLOT
    ELSE WIPE COLOUR CENTRE RANGE DRAW CIRCLE LOAD SAVE VERIFY HLOAD HSAVE HVERIFY
    DLOAD DSAVE DVERIFY MERGE CAT RUN NEW ON LIST DEF MON GWIND TWIND UNDER
    SPC OFF TAB THEN TO STEP AND OR XOR NOT ABS LEN SQR INT ASC CHR VAL STR MID
    ARG CALL RND LEFT RIGHT DOT SGN SIN FREE PI FN TAN COS POP PEEK INP LN EXP ATN

Image display is mono, COLOUR x (x = 0 to 3) command switch drawing in mode
    0 - nothing
    1 - set 1 on drawn pixel
    2 - set 0 on drawn pixel
    3 - xor drawn pixel with current value

Unknown how to produce sound - there's no commands.

MON: (Guesswork by trying things)
    - A0 : display addresses A0 to A7
    - 0,FFF : display addresses 0 to FFE (yes it leaves one out)
    - G,R : displays registers (R B=3 : set B register to 3)
    - M,N,Q,T : display or set a specific "register"? (Q : display Q ; Q=6 : set Q to 6)
    - SP : display or set SP (SP=3B00 : set SP)
    - PC : display or set PC
    - L : disassembler (L 0,FF : disassemble range 0 to FF)
    - K : single-step? (address set by G) (K 99 : single-step at 99)

Control Keys
    - A : adds additional line for each char (making image higher)
    - G : back to normal
    - C : break? (only does a newline)
    - L : clear screen
    - M : same as pressing Enter
    - Y : invert screen

TO DO
    - How to use the sound?
    - Need software
    - Need manuals & schematics

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define AY8910_TAG "g12"
#define HD46505SP_TAG "h45"


namespace {

class lola8_base_state : public driver_device
{
public:
	lola8_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_cass(*this, "cassette")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_hd6845(*this, HD46505SP_TAG)
		, m_ay8910(*this, AY8910_TAG)
		, m_p_videoram(*this, "videoram")
		, m_io_keyboard(*this, "KEY.%u", 0U)
		, m_io_modifier(*this, "MODIFIER")
	{ }

	void lola_base(machine_config &config);

protected:
	void crtc_vsync(int state);
	int cass_r();
	void cass_w(int state);

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<i8085a_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cass;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<hd6845s_device> m_hd6845;
	required_device<ay8910_device> m_ay8910;
	required_shared_ptr<u8> m_p_videoram;
	required_ioport_array<10> m_io_keyboard;
	required_ioport m_io_modifier;
};

class lola8_state : public lola8_base_state
{
public:
	lola8_state(const machine_config &mconfig, device_type type, const char *tag)
		: lola8_base_state(mconfig, type, tag)
		, m_p_chargen(*this, "chargen")
	{ }

	void lola8(machine_config &config);

private:
	void machine_start() override ATTR_COLD;

	u8 port_b_r();
	void port_a_w(u8 data);

	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_region_ptr<u8> m_p_chargen;
protected:
	u8 m_porta = 0U;
};

class lola8nk_state : public lola8_state
{
public:
	lola8nk_state(const machine_config &mconfig, device_type type, const char *tag)
		: lola8_state(mconfig, type, tag)
	{ }

	void lola8nk(machine_config &config);

private:
	void machine_start() override ATTR_COLD;

	u8 port_b_r();
	void mem_map(address_map &map) ATTR_COLD;
};

class lola8a_state : public lola8_base_state
{
public:
	lola8a_state(const machine_config &mconfig, device_type type, const char *tag)
		: lola8_base_state(mconfig, type, tag)
	{ }

	void lola8a(machine_config &config);

private:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	u8 port_a_r();
	void port_b_w(u8 data);

	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_portb = 0U;
};

/* Memory maps */
void lola8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x5000, 0x53ff).ram().share("videoram"); // MK4801AN at E6
	// RAM starts at 0x5800
}

void lola8nk_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x6000, 0x63ff).ram().share("videoram"); // MB8146 at E6, only 1K used
	// RAM starts at 0x6800
}

void lola8_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf9, 0xf9).w(m_ay8910, FUNC(ay8910_device::address_w));
	map(0xfa, 0xfa).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xfc, 0xfc).rw(m_hd6845, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xfd, 0xfd).rw(m_hd6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

/* I/O maps */
void lola8a_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// Sockets for RAM 6264 at G45, F45, E45 and D45
	// should be populated in order, usual configuration
	// is 16K with G45 and F45 populated
	map(0x8000, 0x9fff).rom().region("maincpu", 0); // 2764A at B45
	map(0xa000, 0xbfff).rom().region("maincpu", 0x2000); // 2764A at C45
	map(0xc000, 0xdfff).rom().region("maincpu", 0x4000); // 2764A at H67
	map(0xe000, 0xffff).ram().share("videoram"); // 6264 at G67
}

void lola8a_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x80, 0x80).w(m_ay8910, FUNC(ay8910_device::address_w));
	map(0x84, 0x84).w(m_ay8910, FUNC(ay8910_device::data_w));
	map(0x88, 0x88).r(m_ay8910, FUNC(ay8910_device::data_r));
	map(0x90, 0x90).rw(m_hd6845, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x92, 0x92).rw(m_hd6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

/* Input ports */
static INPUT_PORTS_START( lola8 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('o')
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('l')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('g')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('t')
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('n')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('p')
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('x')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('m')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('k')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('h')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('e')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('b')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('v')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('f')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('r')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIER")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
INPUT_PORTS_END

static INPUT_PORTS_START( lola8a )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'Ž') PORT_CHAR(U'ž')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Č') PORT_CHAR(U'č')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(U'Š') PORT_CHAR(U'š')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'Ć') PORT_CHAR(U'ć')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(':') PORT_CHAR('@')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RET") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIER")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
INPUT_PORTS_END

/* Video */
/* F4 Character Displayer */
static const gfx_layout lola8_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_lola8 )
	GFXDECODE_ENTRY( "chargen", 0x0800, lola8_charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( lola8_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	for (u8 x = 0; x < x_count; x++)
	{
		u16 mem = (ma + x) & 0x3ff;
		u8 chr = m_p_videoram[mem];
		u8 gfx = m_p_chargen[0x0800 | (chr<<4) | ra] ^ ((x == (cursor_x)) ? 0xff : 0);

		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

MC6845_UPDATE_ROW( lola8a_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	u8 inv = BIT(ma,13) ? 0xff : 0x00;
	ma &= 0x7ff;

	for (u8 x = 0; x < x_count; x++)
	{
		u16 mem = (x+ma)*8 + ra;
		u8 gfx = m_p_videoram[mem] ^ ((cursor_x == x) ? 0xff : 0) ^ inv;
		if (ra == 8) // empty line when Ctrl-A is used
			gfx = inv;

		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

/* Keyboard */
u8 lola8_state::port_b_r()
{
	u8 data = 0xff;
	for(int i=0;i<8;i++)
	{
		if (BIT(m_porta,i)==0)
		{
			data = m_io_keyboard[i]->read();
			break;
		}
	}
	return data & m_io_modifier->read();
}

// On Lola 8 NK same keyboard as on 8A is used
// but column 0 and 1 wires are switched
u8 lola8nk_state::port_b_r()
{
	u8 data = 0xff, kbrow = m_porta & 15;
	if (kbrow < 10)
		data = m_io_keyboard[kbrow]->read() & m_io_modifier->read();
	return bitswap<8>(data, 7, 6, 5, 4, 3, 2, 0, 1);
}

void lola8_state::port_a_w(u8 data)
{
	m_porta = data;
}

u8 lola8a_state::port_a_r()
{
	u8 data = 0xff, kbrow = m_portb & 15;

	if (kbrow < 10)
		data = m_io_keyboard[kbrow]->read() & m_io_modifier->read();

	return data;
}

void lola8a_state::port_b_w(u8 data)
{
	m_portb = data;
}

/* Cassette */
int lola8_base_state::cass_r()
{
	return (m_cass->input() < 0.03);
}

void lola8_base_state::cass_w(int state)
{
	m_cass->output(state ? -1.0 : +1.0);
}

/* Machine start / reset */
void lola8a_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x1fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x8000, 0x9fff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x1fff, m_ram->pointer());
				}
			},
			&m_rom_shadow_tap);
}

void lola8_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x5800, 0x5800 + m_ram->size() - 1, m_ram->pointer());
	save_item(NAME(m_porta));
}

void lola8nk_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x6800, 0x6800 + m_ram->size() - 1, m_ram->pointer());
	save_item(NAME(m_porta));
}

void lola8a_state::machine_start()
{
	// Add just RAM over 8K, first 8K will be added after reset
	if (m_ram->size() > 8 * 1024)
		m_maincpu->space(AS_PROGRAM).install_ram(0x2000, m_ram->size() - 1, m_ram->pointer() + 0x2000);
	save_item(NAME(m_portb));
}

void lola8_base_state::crtc_vsync(int state)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state? ASSERT_LINE : CLEAR_LINE);
}

/* Machine configuration */
void lola8_base_state::lola_base(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 0);
	m_maincpu->in_sid_func().set(FUNC(lola8a_state::cass_r));
	m_maincpu->out_sod_func().set(FUNC(lola8a_state::cass_w));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910, 0);
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(m_hd6845, FUNC(hd6845s_device::screen_update));

	HD6845S(config, m_hd6845, 0); // HD6845 == HD46505S
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(8);
	m_hd6845->out_vsync_callback().set(FUNC(lola8a_state::crtc_vsync));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void lola8_state::lola8(machine_config &config)
{
	lola_base(config);

	/* basic machine hardware */
	m_maincpu->set_clock(XTAL(16'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &lola8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lola8_state::io_map);

	m_ay8910->set_clock(XTAL(16'000'000) / 16);
	m_ay8910->port_a_write_callback().set(FUNC(lola8_state::port_a_w));
	m_ay8910->port_b_read_callback().set(FUNC(lola8_state::port_b_r));

	/* video hardware */
	m_screen->set_raw(16_MHz_XTAL, 480, 0, 320, 312, 0, 250);

	m_hd6845->set_clock(XTAL(16'000'000) / 16);
	m_hd6845->set_update_row_callback(FUNC(lola8_state::crtc_update_row));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_lola8);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("10K").set_extra_options("2K,4K,6K,8K,10K");
}

void lola8nk_state::lola8nk(machine_config &config)
{
	lola8(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &lola8nk_state::mem_map);

	m_ay8910->port_b_read_callback().set(FUNC(lola8nk_state::port_b_r));

	/* video hardware */
	m_screen->set_raw(16_MHz_XTAL, 512, 0, 320, 312, 0, 225);

	/* internal ram */
	RAM(config.replace(), RAM_TAG).set_default_size("6K").set_extra_options("2K,4K,6K");
}

void lola8a_state::lola8a(machine_config &config)
{
	lola_base(config);

	/* basic machine hardware */
	m_maincpu->set_clock(XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &lola8a_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lola8a_state::io_map);

	m_ay8910->set_clock(XTAL(4'915'200) / 4);
	m_ay8910->port_a_read_callback().set(FUNC(lola8a_state::port_a_r));
	m_ay8910->port_b_write_callback().set(FUNC(lola8a_state::port_b_w));

	/* video hardware */
	m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 313, 0, 200);

	m_hd6845->set_clock(XTAL(8'000'000) / 8);
	m_hd6845->set_update_row_callback(FUNC(lola8a_state::crtc_update_row));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("8K,16K,24K,32K");
}

/* ROM definition */
ROM_START( lola8 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "l83es2 r0+r1 8.5.85.d1", 0x0000, 0x2000, CRC(c036595f) SHA1(72447b55f053d90dfd31c335b68637fd7b942a41))
	ROM_LOAD( "l83es2 r2+r3 8.5.85.d2", 0x2000, 0x2000, CRC(17adfefd) SHA1(d20e52348ac9b1a60d2a845b5f804d93bfb00b0f))
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "lo8 03.01 crt r0 17.4.85.fg6", 0x0000, 0x1000, CRC(3fe1e7c4) SHA1(1e76b61f5cb6800f8f263fd970afe88a7b2bab64))
ROM_END

ROM_START( lola8nk )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "l8bmny r0 v04 15.5.85.d1", 0x0000, 0x1000, CRC(ec16f0b8) SHA1(9d909eccfbeba07d4520cf369fe514764841f6f9))
	ROM_LOAD( "l8bmny r1 v04 15.5.85.d2", 0x1000, 0x1000, CRC(d79fb5e4) SHA1(1b0c04a34472cc456575760193a6b9b26a749c32))
	ROM_LOAD( "l8bmny r2 v04 15.5.85.d3", 0x2000, 0x1000, CRC(c7f32f39) SHA1(1ef1475502894f43c13f244ec3f60672b6d6db2b))
	ROM_LOAD( "l8bmny r3 v04 15.5.85.d4", 0x3000, 0x1000, CRC(879e4c31) SHA1(4cd9701e502308c8ef732a53dc6055f75060ea84))
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chl8ny crt 14.5.85.fg6", 0x0800, 0x0800, CRC(f078c61a) SHA1(08d0072a52b5de03e46264a05eb49699191976b5))
	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom.e12", 0x00, 0x20, CRC(6887a88c) SHA1(de34ea9345ba4a5b7728602d7c074424765a9635))
	ROM_LOAD( "prom.b5",  0x20, 0x20, CRC(afbe3aa1) SHA1(757c115971013381f6aeb5c5c566150e715267d1))
ROM_END

ROM_START( lola8a )
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lola 8a r0 w06 22.11.86.b45", 0x0000, 0x2000, CRC(aca1fc08) SHA1(f7076d937bb53b0addcba2a5b7c05ab75d6d0d93))
	ROM_LOAD( "lola 8a r1 w06 22.11.86.c45", 0x2000, 0x2000, CRC(99f8ec9b) SHA1(88eafd09c479f177525fa0039cf04d74bae39dab))
	ROM_LOAD( "lola 8a r2 w06 22.11.86.h67", 0x4000, 0x2000, CRC(1e7cd46b) SHA1(048b2583ee7baeb9621e629b79ed64583ac5d554))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME     FLAGS
COMP( 1985, lola8,  0,      0,      lola8,   lola8,  lola8_state,  empty_init, "Institut Ivo Lola Ribar", "Lola 8",    MACHINE_SUPPORTS_SAVE )
COMP( 1985, lola8nk,lola8,  0,      lola8nk, lola8a, lola8nk_state,empty_init, "Institut Ivo Lola Ribar", "Lola 8 NK", MACHINE_SUPPORTS_SAVE )
COMP( 1986, lola8a, lola8,  0,      lola8a,  lola8a, lola8a_state, empty_init, "Institut Ivo Lola Ribar", "Lola 8A",   MACHINE_SUPPORTS_SAVE )
