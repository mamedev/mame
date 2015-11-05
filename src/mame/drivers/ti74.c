// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

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
  to cut down on complexity (and probably for protection too). To reduce power usage
  even more, the OS often idles while waiting for any keypress that triggers an interrupt
  and wakes the processor up.

  The machine is powered by 4 AAA batteries. These will also save internal RAM,
  provided that the machine is turned off properly.


  TODO:
  - it runs too fast due to missing clock divider emulation in TMS70C46
  - external ram cartridge
  - DOCK-BUS interface and peripherals, compatible with both TI-74 and TI-95
    * CI-7 cassette interface
    * PC-324 thermal printer
    (+ old Hexbus devices can be connected via a converter cable)

***************************************************************************/

#include "emu.h"
#include "cpu/tms7000/tms7000.h"
#include "video/hd44780.h"
#include "machine/nvram.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#include "ti74.lh"
#include "ti95.lh"


class ti74_state : public driver_device
{
public:
	ti74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_key_matrix(*this, "IN"),
		m_battery_inp(*this, "BATTERY")
	{ }

	required_device<tms70c46_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<8> m_key_matrix;
	required_ioport m_battery_inp;

	UINT8 m_key_select;
	UINT8 m_power;

	void update_lcd_indicator(UINT8 y, UINT8 x, int state);
	void update_battery_status(int state);

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);

	virtual void machine_reset();
	virtual void machine_start();
	DECLARE_PALETTE_INIT(ti74);
	DECLARE_INPUT_CHANGED_MEMBER(battery_status_changed);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(ti74_cartridge);
};



/***************************************************************************

  File Handling

***************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(ti74_state, ti74_cartridge)
{
	UINT32 size = m_cart->common_get_size("rom");

	// max size is 32KB
	if (size > 0x8000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}



/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(ti74_state, ti74)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

void ti74_state::update_lcd_indicator(UINT8 y, UINT8 x, int state)
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
	output_set_lamp_value(y * 10 + x, state);
}

static HD44780_PIXEL_UPDATE(ti74_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 14 lcd indicators
		ti74_state *driver_state = device.machine().driver_data<ti74_state>();
		driver_state->update_lcd_indicator(y, x, state);
	}
	else if (line < 2 && pos < 16)
	{
		// internal: 2*16, external: 1*31
		if (y == 7) y++; // the cursor is slightly below the character
		bitmap.pix16(1 + y, 1 + line*16*6 + pos*6 + x) = state ? 1 : 2;
	}
}

static HD44780_PIXEL_UPDATE(ti95_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 17 lcd indicators
		ti74_state *driver_state = device.machine().driver_data<ti74_state>();
		driver_state->update_lcd_indicator(y, x, state);
	}
	else if (line == 0 && pos < 16)
	{
		// 1st line is simply 16 chars
		if (y == 7) y++; // the cursor is slightly below the char
		bitmap.pix16(10 + y, 1 + pos*6 + x) = state ? 1 : 2;
	}
	else if (line == 1 && pos < 15 && y < 7)
	{
		// 2nd line is segmented into 5 groups of 3 chars, there is no cursor
		// note: the chars are smaller than on the 1st line (this is handled in .lay file)
		const int gap = 9;
		int group = pos / 3;
		bitmap.pix16(1 + y, 1 + group*gap + pos*6 + x) = state ? 1 : 2;
	}
}



/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

READ8_MEMBER(ti74_state::keyboard_r)
{
	UINT8 ret = 0;

	// read selected keyboard rows
	for (int i = 0; i < 8; i++)
	{
		if (m_key_select >> i & 1)
			ret |= m_key_matrix[i]->read();
	}

	return ret;
}

WRITE8_MEMBER(ti74_state::keyboard_w)
{
	// d(0-7): select keyboard column
	m_key_select = data;
}

WRITE8_MEMBER(ti74_state::bankswitch_w)
{
	// d0-d1: system rom bankswitch
	membank("sysbank")->set_entry(data & 3);

	// d2: power-on latch
	if (~data & 4 && m_power)
	{
		m_power = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // stop running
	}

	// d3: N/C
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, ti74_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE("hd44780", hd44780_device, read, write)
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("sysram.ic3")
	//AM_RANGE(0x4000, 0xbfff)      // mapped by the cartslot
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("sysbank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, ti74_state )
	AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(keyboard_r)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(bankswitch_w)
	AM_RANGE(TMS7000_PORTE, TMS7000_PORTE) AM_WRITE(keyboard_w) AM_READNOP
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(ti74_state::battery_status_changed)
{
	if (machine().phase() == MACHINE_PHASE_RUNNING)
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_NAME("i  I  " UTF8_SQUAREROOT"x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(UTF8_LEFT"     " UTF8_LEFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_NAME("u  U  x" UTF8_POW_2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_NAME("j  J  nCr")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_NAME("n  N  Intg")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_NAME("l  L  (x,y)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_NAME("o  O  1/x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(UTF8_RIGHT"     EE")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_NAME("y  Y  log")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_NAME("h  H  nPr")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_NAME("b  B  EXC")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('\'') PORT_NAME("SPACE  '  " UTF8_CAPITAL_DELTA"%")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') PORT_NAME(";  :  " UTF8_CAPITAL_SIGMA"+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_NAME("p  P  y" UTF8_POW_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('(') PORT_NAME(UTF8_UP"  (")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_NAME("t  T  ln(x)")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_NAME("g  G  n!")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_NAME("v  V  SUM")

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_CHAR('=') PORT_NAME("ENTER  =")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("CLR  UCL  CE/C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(')') PORT_NAME(UTF8_DOWN"  )")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("RUN     x<>y")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_NAME("r  R  " UTF8_SMALL_PI)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_NAME("f  F  P>R")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_NAME("c  C  RCL")

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('~') PORT_CHAR('?') PORT_NAME("+/-  ?  CSR")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_NAME("1  !  r")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_NAME("4  $  " UTF8_CAPITAL_SIGMA"x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_NAME("7  DEL  " UTF8_CAPITAL_SIGMA"x" UTF8_POW_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_NAME("BREAK")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_NAME("e  E  tan")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_NAME("d  D  DRG>")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_NAME("x  X  STO")

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('<') PORT_NAME("0  <  x" UTF8_PRIME)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_NAME("2  \"  a")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('&') PORT_NAME("5  &  " UTF8_CAPITAL_SIGMA"y")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("8  INS  " UTF8_CAPITAL_SIGMA"y" UTF8_POW_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("MODE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_NAME("w  W  cos")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_NAME("s  S  DRG")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_NAME("z  Z  PRINT")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>') PORT_NAME(".  >  y" UTF8_PRIME) // 2 on the keyboard, same scancode
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_NAME("3  #  b")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_NAME("6  ^  n")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME("9  PB  " UTF8_CAPITAL_SIGMA"xy")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("OFF")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_NAME("q  Q  sin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_NAME("a  A  DMS>DD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("+     s(y)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("-     s(x)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("*     y" UTF8_NONSPACE_MACRON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("/     x" UTF8_NONSPACE_MACRON)
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME(UTF8_DIVIDE"  \\  DFN")
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME(UTF8_CAPITAL_SIGMA"+  E")
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME(UTF8_MULTIPLY"  ^  " UTF8_SMALL_PI)
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("x" UTF8_POW_2"  H")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("FLAGS  N")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME(UTF8_LEFT"  DEL")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("RCL  O  FH")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME("INV  P")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_NAME("7  }  nPr")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("EXC  U  DH")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME(UTF8_SQUAREROOT"x  J")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("TESTS  M")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME(UTF8_RIGHT"  INS")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("y" UTF8_POW_X"  L")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("2nd")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4' )PORT_NAME("4     IND")

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



/***************************************************************************

  Machine Config

***************************************************************************/

void ti74_state::update_battery_status(int state)
{
	// battery ok/low status is on int1 line!
	m_maincpu->set_input_line(TMS7000_INT1_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void ti74_state::machine_reset()
{
	m_power = 1;

	update_battery_status(m_battery_inp->read());
}

void ti74_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0xbfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

	membank("sysbank")->configure_entries(0, 4, memregion("system")->base(), 0x2000);
	membank("sysbank")->set_entry(0);

	// zerofill
	m_key_select = 0;
	m_power = 0;

	// register for savestates
	save_item(NAME(m_key_select));
	save_item(NAME(m_power));
}

static MACHINE_CONFIG_START( ti74, ti74_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS70C46, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_NVRAM_ADD_0FILL("sysram.ic3")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*31+1, 9*1+1+1)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*31, 0, 9*1+1)
	MCFG_DEFAULT_LAYOUT(layout_ti74)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(ti74_state, ti74)

	MCFG_HD44780_ADD("hd44780") // 270kHz
	MCFG_HD44780_LCD_SIZE(2, 16) // 2*16 internal
	MCFG_HD44780_PIXEL_UPDATE_CB(ti74_pixel_update)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "ti74_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom,256")
	MCFG_GENERIC_LOAD(ti74_state, ti74_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "ti74_cart")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ti95, ti74_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS70C46, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_NVRAM_ADD_0FILL("sysram.ic3")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(200, 20)
	MCFG_SCREEN_VISIBLE_AREA(0, 200-1, 0, 20-1)
	MCFG_DEFAULT_LAYOUT(layout_ti95)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(ti74_state, ti74)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
	MCFG_HD44780_PIXEL_UPDATE_CB(ti95_pixel_update)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "ti95_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom,256")
	MCFG_GENERIC_LOAD(ti74_state, ti74_cartridge)

	//MCFG_SOFTWARE_LIST_ADD("cart_list", "ti95_cart")
MACHINE_CONFIG_END



/***************************************************************************

  ROM Definitions

***************************************************************************/

ROM_START( ti74 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c70009.ic2", 0xf000, 0x1000, CRC(55a2f7c0) SHA1(530e3de42f2e304c8f4805ad389f38a459ec4e33) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "hn61256pc93.ic1", 0x0000, 0x8000, CRC(019aaa2f) SHA1(04a1e694a49d50602e45a7834846de4d9f7d587d) ) // system rom, banked
ROM_END


ROM_START( ti95 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c70011.ic2", 0xf000, 0x1000, CRC(b4d0a5c1) SHA1(3ff41946d014f72220a88803023b6a06d5086ce4) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "hn61256pc95.ic1", 0x0000, 0x8000, CRC(c46d29ae) SHA1(c653f08590dbc28241a9f5a6c2541641bdb0208b) ) // system rom, banked
ROM_END


COMP( 1985, ti74, 0, 0, ti74, ti74, driver_device, 0, "Texas Instruments", "TI-74 BASICALC", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1986, ti95, 0, 0, ti95, ti95, driver_device, 0, "Texas Instruments", "TI-95 PROCALC", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
