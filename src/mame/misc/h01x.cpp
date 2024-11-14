// license:BSD-3-Clause
// copyright-holders:zzemu-cn, Robbbert
/***************************************************************************
NF500A (TRS80 Level II Basic)
        09/01/2019

H-01B (TRS80 Level II Basic)
        10/05/2019

Despite the references to the TRS-80, the machines are entirely incompatible.

TODO:
- Need schematics and technical info.
- Need confirmation of clock speeds for each machine.
  (Cassettes made on machines of different clocks are not shareable)
- JCE's 16KB extended ROM functionality is not understood, functionality is
  unemulated
- Need software.

****************************************************************************/

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "emupal.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "imagedev/cassette.h"


namespace {

class h01x_state : public driver_device
{
public:
	h01x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_vram(*this, "vram")
		, m_rom(*this, "maincpu")
		, m_hzrom(*this, "hzrom")
		, m_exrom(*this, "exrom")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void h01x(machine_config &config);
	void h01b(machine_config &config);
	void nf500a(machine_config &config);
	void h01jce(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void h01x_mem_map(address_map &map) ATTR_COLD;
	void h01x_io_map(address_map &map) ATTR_COLD;

	uint8_t mem_0000_r(offs_t offset);
	void mem_0000_w(uint8_t data);
	uint8_t mem_4000_r(offs_t offset);
	void mem_4000_w(offs_t offset, uint8_t data);
	uint8_t mem_8000_r(offs_t offset);
	void mem_8000_w(offs_t offset, uint8_t data);
	uint8_t mem_c000_r(offs_t offset);
	void mem_c000_w(offs_t offset, uint8_t data);

	void port_70_w(uint8_t data);
	uint8_t port_50_r();

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ram_device> m_vram;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_hzrom;
	optional_region_ptr<u8> m_exrom;
	required_device<palette_device> m_palette;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<11> m_io_keyboard;

	uint8_t m_bank = 0U;
	MC6845_UPDATE_ROW(crtc_update_row);

	uint8_t *m_ram_ptr = nullptr;
	uint8_t *m_vram_ptr = nullptr;

	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	bool m_cassette_data = false;
	emu_timer *m_cassette_data_timer = nullptr;
};


void h01x_state::h01x(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &h01x_state::h01x_mem_map);
	m_maincpu->set_addrmap(AS_IO, &h01x_state::h01x_io_map);

	// RAM
	RAM(config, m_ram).set_default_size("32K").set_default_value(0x00);
	// VRAM 16K 4bit
	RAM(config, m_vram).set_default_size("16K").set_default_value(0xf0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.6445_MHz_XTAL, 336, 0, 336, 192, 0, 192);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, 10.6445_MHz_XTAL / 8);   // freq guess
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(4);
	m_crtc->set_update_row_callback(FUNC(h01x_state::crtc_update_row));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_default_state(CASSETTE_STOPPED);
}

void h01x_state::h01b(machine_config &config)
{
	// H-01B CPU: 2MHz ROM: 16KB + 32KB
	h01x(config);
	m_maincpu->set_clock(16_MHz_XTAL/8);  // to confirm
}

void h01x_state::nf500a(machine_config &config)
{
	// NF-500A CPU: 4MHz ROM: 16KB + 32KB
	h01x(config);
	m_maincpu->set_clock(16_MHz_XTAL/4);  // to confirm
}

void h01x_state::h01jce(machine_config &config)
{
	// JCE CPU: 4MHz ROM: 16KB + 32KB + 16KB
	h01x(config);
	m_maincpu->set_clock(16_MHz_XTAL/4);  // to confirm
}

void h01x_state::h01x_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(h01x_state::mem_0000_r), FUNC(h01x_state::mem_0000_w));
	map(0x4000, 0x7fff).rw(FUNC(h01x_state::mem_4000_r), FUNC(h01x_state::mem_4000_w));
	map(0x8000, 0xbfff).rw(FUNC(h01x_state::mem_8000_r), FUNC(h01x_state::mem_8000_w));
	map(0xc000, 0xffff).rw(FUNC(h01x_state::mem_c000_r), FUNC(h01x_state::mem_c000_w));
}

void h01x_state::h01x_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x60, 0x60).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x64, 0x64).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x50, 0x50).r(FUNC(h01x_state::port_50_r));
	map(0x70, 0x70).w(FUNC(h01x_state::port_70_w));
}

/*
H-01B
   KD7 KD6 KD5 KD4 KD3 KD2 KD1 KD0  scan address
A0            space Z   A   Q   1    BFFEH
A1             BRK  X   S   W   2    BFFDH
A2             (16) C   D   E   3    BFFBH
A3             E/C  V   F   R   4    BFF7H
A4              -   B   G   T   5    BFEFH
A5             右   N   H   Y   6    BFDFH
A6            RETN  M   J   U   7    BFBFH
A7              [   ,   K   I   8    BF7FH
A8            BS 左 .   L   O   9    BEFFH
A9              :   /   ;   P   0    BDFFH
A10           SHIFT            下    BBFFH

need to determine keys:
Ctrl ESC
*/

static INPUT_PORTS_START( h01b )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('C') PORT_CHAR('c')
//  PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)   PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E/C") PORT_CODE(KEYCODE_TILDE)    PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)      PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)          PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)    PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/*

模拟器中的按键位置安排参照 TRS-80

NF-500A

   KD7 KD6 KD5 KD4 KD3 KD2 KD1 KD0  scan address
A0      R   E   5 CTRL? 6   T   W    BFFEH
A1      3   2   Y  E/C  U   4   1    BFFDH
A2      9   :   8   -   7   0   下   BFFBH
A3      D   S   G  ESC  H   F   Q    BFF7H
A4      X   A   V   Z       C  BRK   BFEFH
A5      L BS 左 K space J   ;   右   BFDFH
A6      M   .   N       B   ,   /    BFBFH
A7      P  RETN O SHIFT I   [        BF7FH

3 56 34 51 key functions not verified, temporarily mapped to ] TAB \ '

E/C E汉 = ~  translates to ASCII 20
ESC translates to ASCII 31
BS translates to ASCII 8
61 translates to ASCII 13 enter
16 translates to ASCII 10 down
56 translates to ASCII 16
27 3 34 51  no action

Program to test key ASCII codes
10 A$=INKEY$
20 IF LEN(A$)>0 THEN PRINT ASC(A$)
30 GOTO 10

TRS-80
left 8 right 9 up 91 down 10
Clear 31
@ 64
backspace 8

*/

/* Input ports */
static INPUT_PORTS_START( h01x )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E/C") PORT_CODE(KEYCODE_TILDE)    PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)          PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)   PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)    PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)      PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/*  Machine     */

void h01x_state::machine_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_cassette_data));

	m_cassette_data_timer = timer_alloc(FUNC(h01x_state::cassette_data_callback), this);
	m_cassette_data_timer->adjust(attotime::zero, 0, attotime::from_hz(48000));
}

void h01x_state::machine_reset()
{
	m_bank = 0x00;

	m_ram_ptr = m_ram->pointer();
	m_vram_ptr = m_vram->pointer();
}

MC6845_UPDATE_ROW( h01x_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = ((ma+x)*16 + ra) & 0x3fff;
		u8 gfx = 0;
		if (ra < 16)
			gfx = m_vram_ptr[mem] ^ ((x == cursor_x) ? 15 : 0);

		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

void h01x_state::port_70_w(uint8_t data)
{
	m_bank = data & 0xc0;

	// bit5, speaker
	m_speaker->level_w(BIT(data, 5));

	// bit4, cassette
	m_cassette->output(BIT(data, 4) ? 1.0 : -1.0);
}

uint8_t h01x_state::port_50_r()
{
	// bit 7, cassette input
	return m_cassette_data ? 0xff : 0x7f;
}


// 0x0000 --- 0x3FFF
uint8_t h01x_state::mem_0000_r(offs_t offset)
{
	return m_rom[offset];
}

void h01x_state::mem_0000_w(uint8_t data)
{
}

// 0x4000 --- 0x7FFF
uint8_t h01x_state::mem_4000_r(offs_t offset)
{
	return m_ram_ptr[offset];
}

void h01x_state::mem_4000_w(offs_t offset, uint8_t data)
{
	m_ram_ptr[offset] = data;
}

// 0x8000 --- 0xBFFF
uint8_t h01x_state::mem_8000_r(offs_t offset)
{
	switch (m_bank) {
	case 0xc0:
		return m_hzrom[offset];
	case 0x40:
		if ((offset & 0xf000) == 0x3000) {
			u8 result = 0xff;
			for (int i = 0; i < 11; i++) {
				if (!BIT(offset, i))
					result &= m_io_keyboard[i]->read();
			}
			return result;
		} else {
			return 0xff;
		}
	case 0x00:
		return m_ram_ptr[offset + 0x4000];
	default:
		return 0xff;
	}
}

void h01x_state::mem_8000_w(offs_t offset, uint8_t data)
{
	if (m_bank == 0x00)
		m_ram_ptr[offset + 0x4000] = data;
}


// 0xC000 --- 0xFFFF
uint8_t h01x_state::mem_c000_r(offs_t offset)
{
	if (m_bank == 0xc0)
		return m_hzrom[offset + 0x4000];
	else if (m_bank == 0x40)
		return m_vram_ptr[offset];
	else
		return 0xff;
}

void h01x_state::mem_c000_w(offs_t offset, uint8_t data)
{
	if (m_bank == 0x40)
		m_vram_ptr[offset] = (data & 0x0f) | 0xf0;
}


TIMER_CALLBACK_MEMBER(h01x_state::cassette_data_callback)
{
	m_cassette_data = false;

	if (m_cassette->input() > 0.2)
		m_cassette_data = true;
}


/* ROM definition */
ROM_START(h01b)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("h-01b_sysrom.bin",   0x0000, 0x4000, CRC(b52093a7) SHA1(8c874765033444688c906b1a987a73f2c3ec83fb))

	ROM_REGION(0x8000,"hzrom",0)
	ROM_LOAD("h-01b_hzrom.bin",   0x0000, 0x8000, CRC(f0d6a7ac) SHA1(72151d3215bc8b26f983466221fe5f4009727ce8))
ROM_END

ROM_START(nf500a)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("u2-hn27128-9adc.bin",   0x0000, 0x4000, CRC(147dae83) SHA1(856b0970c603e88707ce8638be5dbd8ab1c42a1b))

	ROM_REGION(0x8000,"hzrom",0)
	ROM_LOAD("u4-hn27256-32aa.bin",   0x0000, 0x8000, CRC(9ecfddaa) SHA1(54b6e1b43f79b7705e95edda845b21d7326d48e2))
ROM_END

ROM_START(h01jce)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("m5l27128k_9b99.bin",   0x0000, 0x4000, CRC(59be30df) SHA1(21ccc765d13992753ec0457e09ac97cea82888a9))

	ROM_REGION(0x8000,"hzrom",0)
	ROM_LOAD("u4-hn27256-32aa.bin",   0x0000, 0x8000, CRC(9ecfddaa) SHA1(54b6e1b43f79b7705e95edda845b21d7326d48e2))

	ROM_REGION(0x4000, "exrom",0)
	ROM_LOAD("hn4827128g_f0f9.bin",   0x0000, 0x4000, CRC(36bffec0) SHA1(5b4b24c54eba0a8b69f291ca656ea27a3685f42e))
ROM_END

} // anonymous namespace


/* Driver */

// H-01B中文教育电脑
// 普乐电器公司
// cpu      Z-80A 2MHz

// NF500A教学电脑
// 国营八三〇厂制造
// cpu      Z-80A 4MHz
// video    MC6845P
// sysrom   16KB EPROM
// hzrom    32KB EPROM
// ram      32KB SRAM
// vram     16Kx4bit DRAM

// JCE
// 广东江门计算机应用设备厂
// video    HD6845SP
// sysrom   16KB EPROM
// hzrom    32KB EPROM
// extrom   16KB EPROM

// Startup screens
// H-01B  : H-01型中文教育电脑 普乐电器公司制造
// NF500A : H-01型汉字微电脑   中国科学院H电脑公司
// JCE    : H-01型中文普及电脑 北岳电子有限公司制造

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT          COMPANY                                      FULLNAME     FLAGS
COMP( 1985, h01b,    0,      0,      h01b,    h01b,    h01x_state,   empty_init,   "China H Computer Company",                  "H-01B",     MACHINE_SUPPORTS_SAVE )
COMP( 1985, nf500a,  0,      0,      nf500a,  h01x,    h01x_state,   empty_init,   "China State-owned 830 Factory",             "NF500A",    MACHINE_SUPPORTS_SAVE )
COMP( 1987, h01jce,  0,      0,      h01jce,  h01x,    h01x_state,   empty_init,   "China Jiangmen Computer Equipment Factory", "H-01 JCE",  MACHINE_SUPPORTS_SAVE )
