// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Texas Instruments TI-74 BASICALC
Texas Instruments TI-95 PROCALC
hardware family: CC-40 -> TI-74 BASICALC -> TI-95 PROCALC

TI-74 PCB layout:
note: TI-95 PCB is nearly the same, just with a different size LCD screen,
its CPU is labeled C70011, and the system ROM is labeled HN61256PC95.

        DOCK-BUS
      --||||||||---
  C  ==           |
  a  ==           |
  r  ==  HN61256  |
  t  ==           ----------------------------
      |                                      |
-------            C70009          4MHz      |
|        HM6264                    RC4193N   |
|                                            |
|                                            |
|                                            |
|                                            |
---------------||||||||||||||||||||||||-------
               ||||||||||||||||||||||||
---------------||||||||||||||||||||||||-------
|              *HD44100H   *HD44780A00       |
|                                            |
|                                            |
|                                            |
|                                            |
----------                                   |
         | ----------------------------------|
         | |                                ||
         | |          LCD screen            ||
         | |                                ||
         | ----------------------------------|
         -------------------------------------

IC1 HN61256PC93 - Hitachi DIP-28 32KB CMOS Mask PROM
IC2 C70009      - Texas Instruments TMS70C46, 54 pins. Basically a TMS70C40 with some TI custom I/O mods.
                  128 bytes internal RAM, 4KB internal ROM, running at max 4MHz.
IC3 HM6264LP-15 - Hitachi 8KB SRAM (battery backed)
RC4193N         - Micropower Switching Regulator
HD44100H        - 60-pin QFP Hitachi HD44100 LCD Driver
HD44780A00      - 80-pin TFP Hitachi HD44780 LCD Controller

*               - indicates that it's on the other side of the PCB


Overall, the hardware is very similar to TI CC-40. A lot has been shuffled around
to cut down on complexity. To reduce power usage even more, the OS often idles while
waiting for any keypress that triggers an interrupt and wakes the processor up.

The machine is powered by 4 AAA batteries. These will also save internal RAM,
provided that the machine is turned off properly.


TODO:
- it runs too fast due to missing clock divider emulation in TMS70C46
- external ram cartridge (HM6264LFP-15 + coin battery)
- DOCK-BUS interface and peripherals, compatible with both TI-74 and TI-95
  * CI-7 cassette interface
  * PC-324 thermal printer
  (+ old Hexbus devices can be connected via a converter cable)
- verify ti74(d12) rom label

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tms7000/tms7000.h"
#include "machine/nvram.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#include "ti74.lh"
#include "ti95.lh"


namespace {

class ti74_state : public driver_device
{
public:
	ti74_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sysbank(*this, "sysbank"),
		m_cart(*this, "cartslot"),
		m_key_matrix(*this, "IN.%u", 0),
		m_battery_inp(*this, "BATTERY"),
		m_segs(*this, "seg%u", 0U)
	{ }

	void ti74(machine_config &config);
	void ti95(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(battery_status_changed);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<tms70c46_device> m_maincpu;
	required_memory_bank m_sysbank;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<8> m_key_matrix;
	required_ioport m_battery_inp;
	output_finder<80> m_segs;

	u8 m_key_select = 0;
	u8 m_power = 0;

	void update_lcd_indicator(u8 y, u8 x, int state);
	void update_battery_status(int state);

	u8 keyboard_r();
	void keyboard_w(u8 data);
	void bankswitch_w(u8 data);

	void ti74_palette(palette_device &palette) const;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	HD44780_PIXEL_UPDATE(ti74_pixel_update);
	HD44780_PIXEL_UPDATE(ti95_pixel_update);
	void main_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Initialisation
*******************************************************************************/

void ti74_state::machine_start()
{
	m_segs.resolve();

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0xbfff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	m_sysbank->configure_entries(0, 4, memregion("system")->base(), 0x2000);

	// register for savestates
	save_item(NAME(m_key_select));
	save_item(NAME(m_power));
}

void ti74_state::machine_reset()
{
	m_power = 1;

	m_sysbank->set_entry(0);
	update_battery_status(m_battery_inp->read());
}

void ti74_state::update_battery_status(int state)
{
	// battery ok/low status is on int1 line!
	m_maincpu->set_input_line(TMS7000_INT1_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(ti74_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");

	// max size is 32KB
	if (size > 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid file size (must be no more than 32K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    Video
*******************************************************************************/

void ti74_state::ti74_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(50, 45, 60)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

void ti74_state::update_lcd_indicator(u8 y, u8 x, int state)
{
	// TI-74 ref._________________...
	// output#  |10     11     12     13     14      2      3      4
	// above    | <    SHIFT   CTL    FN     I/O    UCL    _LOW    >
	// ---- raw lcd screen here ----
	// under    |      BASIC   CALC   DEG    RAD    GRAD   STAT
	// output#  |       63     64      1     62     53     54
	//
	// TI-95 ref._________________...
	// output#  |  40   43     41   44   42     12  11  10/13/14  0    1    2
	// above    | _LOW _ERROR  2nd  INV  ALPHA  LC  INS  DEGRAD  HEX  OCT  I/O
	// screen-  | _P{70} <{71}                                             RUN{3}
	//   area   .                                                          SYS{4}
	m_segs[y * 10 + x] = state ? 1 : 0;
}

HD44780_PIXEL_UPDATE(ti74_state::ti74_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 14 lcd indicators
		update_lcd_indicator(y, x, state);
	}
	else if (line < 2 && pos < 16)
	{
		// internal: 2*16, external: 1*31
		if (y == 7) y++; // the cursor is slightly below the character
		bitmap.pix(1 + y, 1 + line*16*6 + pos*6 + x) = state ? 1 : 2;
	}
}

HD44780_PIXEL_UPDATE(ti74_state::ti95_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 17 lcd indicators
		update_lcd_indicator(y, x, state);
	}
	else if (line == 0 && pos < 16)
	{
		// 1st line is simply 16 chars
		if (y == 7) y++; // the cursor is slightly below the char
		bitmap.pix(10 + y, 1 + pos*6 + x) = state ? 1 : 2;
	}
	else if (line == 1 && pos < 15 && y < 7)
	{
		// 2nd line is segmented into 5 groups of 3 chars, there is no cursor
		// note: the chars are smaller than on the 1st line (this is handled in .lay file)
		const int gap = 9;
		int group = pos / 3;
		bitmap.pix(1 + y, 1 + group*gap + pos*6 + x) = state ? 1 : 2;
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 ti74_state::keyboard_r()
{
	u8 data = 0;

	// read selected keyboard rows
	for (int i = 0; i < 8; i++)
	{
		if (m_key_select >> i & 1)
			data |= m_key_matrix[i]->read();
	}

	return data;
}

void ti74_state::keyboard_w(u8 data)
{
	// d0-d7: select keyboard column
	m_key_select = data;
}

void ti74_state::bankswitch_w(u8 data)
{
	// d0-d1: system rom bankswitch
	m_sysbank->set_entry(data & 3);

	// d2: power-on latch
	if (~data & 4 && m_power)
	{
		m_power = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // stop running
	}

	// d3: N/C
}

void ti74_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x1001).rw("hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x2000, 0x3fff).ram().share("sysram.ic3");
	//map(0x4000, 0xbfff) // mapped by the cartslot
	map(0xc000, 0xdfff).bankr("sysbank");
}



/*******************************************************************************
    Inputs
*******************************************************************************/

INPUT_CHANGED_MEMBER(ti74_state::battery_status_changed)
{
	if (machine().phase() == machine_phase::RUNNING)
		update_battery_status(newval);
}

static INPUT_PORTS_START( ti74 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti74_state, battery_status_changed, 0)
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	// 8x8 keyboard matrix, RESET and ON buttons are not on it. Unused entries are not connected, but some have a purpose for factory testing.
	// For convenience, number keys are mapped to number row too.
	// PORT_NAME lists functions under [SHIFT] and [MODE] or [STAT] as secondaries.
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_NAME("m  M  Frac")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_NAME("k  K  Frq")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_NAME(u8"i  I  \u221ax") // U+221A = √
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190     \u2190") // U+2190 = ←
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_NAME(u8"u  U  x²")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_NAME("j  J  nCr")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_NAME("n  N  Intg")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_NAME("l  L  (x,y)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_NAME("o  O  1/x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192     EE" ) // U+2192 = →
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_NAME("y  Y  log")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_NAME("h  H  nPr")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_NAME("b  B  EXC")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('\'') PORT_NAME(u8"SPACE  '  Δ%")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') PORT_NAME(u8";  :  Σ+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_NAME(u8"p  P  yˣ")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('(') PORT_NAME(u8"\u2191  (") // U+2191 = ↑
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_NAME("t  T  ln(x)")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_NAME("g  G  n!")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_NAME("v  V  SUM")

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_CHAR('=') PORT_NAME("ENTER  =")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("CLR  UCL  CE/C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(')') PORT_NAME(u8"\u2193  )") // U+2193 = ↓
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME(u8"RUN     x\u2194y") // U+2194 = ↔
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_NAME(u8"r  R  π")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_NAME("f  F  P>R")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_NAME("c  C  RCL")

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('~') PORT_CHAR('?') PORT_NAME("+/-  ?  CSR")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_NAME("1  !  r")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_NAME(u8"4  $  Σx")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_NAME(u8"7  DEL  Σx²")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_NAME("BREAK")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_NAME("e  E  tan")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_NAME("d  D  DRG>")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_NAME("x  X  STO")

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('<') PORT_NAME(u8"0  <  xʹ")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_NAME("2  \"  a")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('&') PORT_NAME(u8"5  &  Σy")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME(u8"8  INS  Σy²")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("MODE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_NAME("w  W  cos")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_NAME("s  S  DRG")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_NAME("z  Z  PRINT")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>') PORT_NAME(u8".  >  yʹ") // 2 on the keyboard, same scancode
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_NAME("3  #  b")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_NAME("6  ^  n")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME(u8"9  PB  Σxy")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("OFF")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_NAME("q  Q  sin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_NAME("a  A  DMS>DD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_NAME("+     s(y)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("-     s(x)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_NAME(u8"*     ȳ")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME(u8"/     x̄")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("FN     hyp")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("CTL     STAT")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT     INV")
INPUT_PORTS_END

static INPUT_PORTS_START( ti95 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti74_state, battery_status_changed, 0)
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	// 8x8 keyboard matrix, RESET and ON buttons are not on it.
	// For convenience, number keys are mapped to number row too.
	// PORT_NAME lists functions under [ALPHA] and [2nd] as secondaries.
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("OFF")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("BREAK  Q")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_NAME("SIN  A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_NAME("I/O  Z")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("HELP  ASM")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')') PORT_NAME(")  ]  DRG")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME(u8"÷  \\  DFN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('=') PORT_NAME("=  ~  TRACE")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("EE  {  ENG")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("HALT  W")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("COS  S")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("FILES  X")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_NAME("ALPHA  PART")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("3  ;  SBL")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("+  &  RTN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("2  :  GTL")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME(u8"Σ+  E")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("TAN  D")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("STAT  C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("LEARN  PC")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_NAME("6  @  CP")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("-  _  13d")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("5  %  CMS")

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("x~t  R  AH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_NAME("LN  F")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_NAME("CONV  V")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("OLD  NOP")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME("9  >  x!")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME(u8"×  ^  π")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_NAME("8  <  nCr")

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("HYP  T  BH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("LOG  G")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("NUM  B")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("RUN  SPACE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_NAME("0  $  PAUSE")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_NAME(".  ?  ADV")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("+/-  !  PRINT")

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INCR  Y  CH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME(u8"x²  H")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("FLAGS  N")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME(u8"\u2190  DEL") // U+2190 = ←
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("RCL  O  FH")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME("INV  P")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_NAME("7  }  nPr")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("EXC  U  DH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME(u8"\u221ax  J") // U+221A = √
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("TESTS  M")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME(u8"\u2192  INS") // U+2192 = →
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME(u8"yˣ  L")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("2nd")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4' ) PORT_NAME("4     IND")

	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(') PORT_NAME("(  [  FIX")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("STO  I  EH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_NAME("1/x  K")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("FUNC  ,  \"")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("CE  F:CLR")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("LIST  .  '")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("CLEAR")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_NAME("1  #  LBL")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ti74_state::ti74(machine_config &config)
{
	// basic machine hardware
	TMS70C46(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti74_state::main_map);
	m_maincpu->in_porta().set(FUNC(ti74_state::keyboard_r));
	m_maincpu->out_portb().set(FUNC(ti74_state::bankswitch_w));
	m_maincpu->out_porte().set(FUNC(ti74_state::keyboard_w));

	NVRAM(config, "sysram.ic3", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60); // arbitrary
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(6*31+1, 9*1+1+1);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_ti74);

	PALETTE(config, "palette", FUNC(ti74_state::ti74_palette), 3);

	hd44780_device &hd44780(HD44780(config, "hd44780", 270'000)); // OSC = 91K resistor
	hd44780.set_lcd_size(2, 16); // 2*16 internal
	hd44780.set_pixel_update_cb(FUNC(ti74_state::ti74_pixel_update));

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "ti74_cart", "bin,rom,256").set_device_load(FUNC(ti74_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("ti74_cart");
}

void ti74_state::ti95(machine_config &config)
{
	// basic machine hardware
	TMS70C46(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti74_state::main_map);
	m_maincpu->in_porta().set(FUNC(ti74_state::keyboard_r));
	m_maincpu->out_portb().set(FUNC(ti74_state::bankswitch_w));
	m_maincpu->out_porte().set(FUNC(ti74_state::keyboard_w));

	NVRAM(config, "sysram.ic3", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60); // arbitrary
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(200, 20);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_ti95);

	PALETTE(config, "palette", FUNC(ti74_state::ti74_palette), 3);

	hd44780_device &hd44780(HD44780(config, "hd44780", 270'000)); // OSC = 91K resistor
	hd44780.set_lcd_size(2, 16);
	hd44780.set_pixel_update_cb(FUNC(ti74_state::ti95_pixel_update));

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "ti95_cart", "bin,rom,256").set_device_load(FUNC(ti74_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("ti95_cart");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ti74 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "c70009.ic2", 0x0000, 0x1000, CRC(55a2f7c0) SHA1(530e3de42f2e304c8f4805ad389f38a459ec4e33) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "1060281-101_hn61256pd12.ic1", 0x0000, 0x8000, CRC(019aaa2f) SHA1(04a1e694a49d50602e45a7834846de4d9f7d587d) ) // system rom, banked
ROM_END

ROM_START( ti74a )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "c70009.ic2", 0x0000, 0x1000, CRC(55a2f7c0) SHA1(530e3de42f2e304c8f4805ad389f38a459ec4e33) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "001060281-1_hn61256pc93.ic1", 0x0000, 0x8000, CRC(499b69d1) SHA1(ba333959bd047ac18f461066816c4d56fe73de85) ) // system rom, banked
ROM_END


ROM_START( ti95 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "c70011.ic2", 0x0000, 0x1000, CRC(b4d0a5c1) SHA1(3ff41946d014f72220a88803023b6a06d5086ce4) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "01060281-11_hn61256pc95.ic1", 0x0000, 0x8000, CRC(c46d29ae) SHA1(c653f08590dbc28241a9f5a6c2541641bdb0208b) ) // system rom, banked
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, ti74,  0,      0,      ti74,    ti74,  ti74_state, empty_init, "Texas Instruments", "TI-74 Basicalc (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1985, ti74a, ti74,   0,      ti74,    ti74,  ti74_state, empty_init, "Texas Instruments", "TI-74 Basicalc (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

SYST( 1986, ti95,  0,      0,      ti95,    ti95,  ti74_state, empty_init, "Texas Instruments", "TI-95 Procalc", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
