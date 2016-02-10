// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Texas Instruments Compact Computer 40 (aka CC-40)
  hardware family: CC-40 -> CC-40+(unreleased) -> TI-74 BASICALC -> TI-95 PROCALC

  ---------------------------------------------
  | ---------------------------------------   |
  | |                                     |   |
  | |             LCD screen              |   |
  | |                                     |   ---------------
  | ---------------------------------------                 |
  |                                                         |
  |                                                         |
  |                                                         |
  |                           *HD44100H                     |
  |       *HD44780A00                                       |
  |                                                         |
  |                                                         |
  |                                                         |
  ----|||||||||||-----------------------------|||||||||||----
      |||||||||||                             |||||||||||
  ----|||||||||||-----------------------------|||||||||||----
  |                                                         |
  |            HM6116LP-4    HM6116LP-4                     |
  |                                                         |
  |                                                         |
  |             HM6116LP-4                    TMX70C20N2L   |
  |                                      5MHz               |
  |                             AMI 1041036-1               |
  |             HN61256PC09                                 |
  |                                             *Cartridge  |
  |                                           ---------------
  |                                           |
  |       -------------------------------------
  |*HEXBUS|
  ---------

  HM6116LP-4    - Hitachi 2KB SRAM (newer 18KB version has two HM6264 8KB chips)
  HN61256PC09   - Hitachi DIP-28 32KB CMOS Mask PROM
  TMX70C20N2L   - Texas Instruments TMS70C20 CPU (128 bytes RAM, 2KB ROM) @ 2.5MHz, 40 pins - "X" implies prototype
  AMI 1041036-1 - 68-pin QFP AMI Gate Array
  HD44100H      - 60-pin QFP Hitachi HD44100 LCD Driver
  HD44780A00    - 80-pin TFP Hitachi HD44780 LCD Controller

  *             - indicates that it's on the other side of the PCB


  CC-40 is powered by 4 AA batteries. These will also save internal RAM,
  provided that the machine is turned off properly. If a program is running,
  you may have to press [BREAK] before turning the CC-40 off. If RAM contents
  ends up dodgy somehow, just delete the nvram files.

  Officially, minimum total RAM size is 6KB. The system will still boot with less,
  but don't expect all software to work properly.

  To run a cartridge, usually the command RUN "DIR" shows which program(s)
  can be loaded. Load a program by pressing the [RUN] key while viewing the list,
  or manually with the command RUN "<shortname of program in list>"


  TODO:
  - external RAM cartridge (bus_control_w cartridge memory addressing)
  - auto clock divider on slow memory access
  - Hexbus interface and peripherals
    * HX-1000: color plotter
    * HX-1010: thermal printer
    * HX-3000: RS-232 interface
    * HX-3100: modem
    * HX-3200: Centronics printer interface

***************************************************************************/

#include "emu.h"
#include "cpu/tms7000/tms7000.h"
#include "video/hd44780.h"
#include "sound/dac.h"
#include "machine/nvram.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#include "cc40.lh"


class cc40_state : public driver_device
{
public:
	cc40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_key_matrix(*this, "IN"),
		m_battery_inp(*this, "BATTERY")
	{
		m_sysram[0] = nullptr;
		m_sysram[1] = nullptr;
	}

	required_device<tms70c20_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<8> m_key_matrix;
	required_ioport m_battery_inp;

	nvram_device *m_nvram[2];

	memory_region *m_cart_rom;

	UINT8 m_bus_control;
	UINT8 m_power;
	UINT8 m_banks;
	UINT8 m_clock_control;
	UINT8 m_clock_divider;
	UINT8 m_key_select;

	std::unique_ptr<UINT8[]> m_sysram[2];
	UINT16 m_sysram_size[2];
	UINT16 m_sysram_end[2];
	UINT16 m_sysram_mask[2];

	void postload();
	void init_sysram(int chip, UINT16 size);
	void update_lcd_indicator(UINT8 y, UINT8 x, int state);
	void update_clock_divider();

	DECLARE_READ8_MEMBER(sysram_r);
	DECLARE_WRITE8_MEMBER(sysram_w);
	DECLARE_READ8_MEMBER(bus_control_r);
	DECLARE_WRITE8_MEMBER(bus_control_w);
	DECLARE_WRITE8_MEMBER(power_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(battery_r);
	DECLARE_READ8_MEMBER(bankswitch_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(clock_control_r);
	DECLARE_WRITE8_MEMBER(clock_control_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(cc40);
	DECLARE_INPUT_CHANGED_MEMBER(sysram_size_changed);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cc40_cartridge);
};



/***************************************************************************

  File Handling

***************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(cc40_state, cc40_cartridge)
{
	UINT32 size = m_cart->common_get_size("rom");

	// max size is 4*32KB
	if (size > 0x20000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(0x20000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);  // allocate a larger ROM region to have 4x32K banks
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}



/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(cc40_state, cc40)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

void cc40_state::update_lcd_indicator(UINT8 y, UINT8 x, int state)
{
	// reference _________________...
	// output#  |10  11     12     13     14      0      1      2      3   4
	// above    | <  SHIFT  CTL    FN     DEG    RAD    GRAD   I/O    UCL  >
	// ---- raw lcd screen here ----
	// under    |    ERROR   v      v      v      v      v      v    _LOW
	// output#  |    60     61     62     63     50     51     52     53
	output().set_lamp_value(y * 10 + x, state);
}

static HD44780_PIXEL_UPDATE(cc40_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 18 lcd indicators
		cc40_state *driver_state = device.machine().driver_data<cc40_state>();
		driver_state->update_lcd_indicator(y, x, state);
	}
	else if (line < 2 && pos < 16)
	{
		// internal: 2*16, external: 1*31
		if (y == 7) y++; // the cursor is slightly below the character
		bitmap.pix16(1 + y, 1 + line*16*6 + pos*6 + x) = state ? 1 : 2;
	}
}



/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

READ8_MEMBER(cc40_state::sysram_r)
{
	// read system ram, based on addressing configured in bus_control_w
	if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
		return m_sysram[0][offset & (m_sysram_size[0] - 1)];
	else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
		return m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)];
	else
		return 0xff;
}

WRITE8_MEMBER(cc40_state::sysram_w)
{
	// write system ram, based on addressing configured in bus_control_w
	if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
		m_sysram[0][offset & (m_sysram_size[0] - 1)] = data;
	else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
		m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)] = data;
}

READ8_MEMBER(cc40_state::bus_control_r)
{
	return m_bus_control;
}

WRITE8_MEMBER(cc40_state::bus_control_w)
{
	// d0,d1: auto enable clock divider on cartridge memory access (d0: area 1, d1: area 2)

	// d2,d3: system ram addressing
	// 00: 8K, 8K @ $1000-$2fff, $3000-$4fff
	// 01: 8K, 2K @ $1000-$2fff, $3000-$37ff
	// 10: 2K, 8K @ $1000-$17ff, $1800-$37ff
	// 11: 2K, 2K @ $1000-$17ff, $1800-$1fff
	int d2 = (data & 4) ? 0x0800 : 0x2000;
	int d3 = (data & 8) ? 0x0800 : 0x2000;
	m_sysram_end[0] = d3;
	m_sysram_mask[0] = d3 - 1;
	m_sysram_end[1] = d3 + d2;
	m_sysram_mask[1] = d2 - 1;

	// d4,d5: cartridge memory addressing
	// 00: 2K @ $5000-$57ff & $5800-$5fff
	// 01: 8K @ $5000-$6fff & $7000-$8fff
	// 10:16K @ $5000-$8fff & $9000-$cfff
	// 11: 8K @ $1000-$2fff & $3000-$4fff - system ram is disabled

	// d6: auto enable clock divider on system rom access

	// d7: unused?
	m_bus_control = data;
}

WRITE8_MEMBER(cc40_state::power_w)
{
	// d0: power-on hold latch
	m_power = data & 1;

	// stop running
	if (!m_power)
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE8_MEMBER(cc40_state::sound_w)
{
	// d0: piezo control
	m_dac->write_signed8((data & 1) ? 0x7f : 0);
}

READ8_MEMBER(cc40_state::battery_r)
{
	// d0: low battery sense line (0 = low power)
	return m_battery_inp->read();
}

READ8_MEMBER(cc40_state::bankswitch_r)
{
	return m_banks;
}

WRITE8_MEMBER(cc40_state::bankswitch_w)
{
	// d0-d1: system rom bankswitch
	membank("sysbank")->set_entry(data & 3);

	// d2-d3: cartridge 32KB page bankswitch
	if (m_cart_rom)
		membank("cartbank")->set_entry(data >> 2 & 3);

	m_banks = data & 0x0f;
}

READ8_MEMBER(cc40_state::clock_control_r)
{
	return m_clock_control;
}

void cc40_state::update_clock_divider()
{
	// 2.5MHz /3 to /17 in steps of 2
	m_clock_divider = (~m_clock_control & 7) * 2 + 1;
	m_maincpu->set_clock_scale((m_clock_control & 8) ? (1.0 / (double)m_clock_divider) : 1);
}

WRITE8_MEMBER(cc40_state::clock_control_w)
{
	// d0-d2: clock divider
	// d3: enable clock divider always
	// other bits: unused?
	if (m_clock_control != (data & 0x0f))
	{
		m_clock_control = data;
		update_clock_divider();
	}
}

READ8_MEMBER(cc40_state::keyboard_r)
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

WRITE8_MEMBER(cc40_state::keyboard_w)
{
	// d(0-7): select keyboard column
	m_key_select = data;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, cc40_state )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0110, 0x0110) AM_READWRITE(bus_control_r, bus_control_w)
	AM_RANGE(0x0111, 0x0111) AM_WRITE(power_w)
	AM_RANGE(0x0112, 0x0112) AM_NOP // d0-d3: Hexbus data
	AM_RANGE(0x0113, 0x0113) AM_NOP // d0: Hexbus available
	AM_RANGE(0x0114, 0x0114) AM_NOP // d0,d1: Hexbus handshake
	AM_RANGE(0x0115, 0x0115) AM_WRITE(sound_w)
	AM_RANGE(0x0116, 0x0116) AM_READ(battery_r)
	AM_RANGE(0x0119, 0x0119) AM_READWRITE(bankswitch_r, bankswitch_w)
	AM_RANGE(0x011a, 0x011a) AM_READWRITE(clock_control_r, clock_control_w)
	AM_RANGE(0x011e, 0x011f) AM_DEVREADWRITE("hd44780", hd44780_device, read, write)

	AM_RANGE(0x0800, 0x0fff) AM_RAM AM_SHARE("sysram.0")
	AM_RANGE(0x1000, 0x4fff) AM_READWRITE(sysram_r, sysram_w)
	AM_RANGE(0x5000, 0xcfff) AM_ROMBANK("cartbank")
	AM_RANGE(0xd000, 0xefff) AM_ROMBANK("sysbank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, cc40_state )
	AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(keyboard_r)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(keyboard_w)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(cc40_state::sysram_size_changed)
{
	init_sysram((int)(FPTR)param, newval << 11);
}

static INPUT_PORTS_START( cc40 )
	PORT_START("RAMSIZE")
	PORT_CONFNAME( 0x07, 0x01, "RAM Chip 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cc40_state, sysram_size_changed, (void *)0)
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x01, "2KB" )
	PORT_CONFSETTING(    0x04, "8KB" )
	PORT_CONFNAME( 0x70, 0x10, "RAM Chip 2") PORT_CHANGED_MEMBER(DEVICE_SELF, cc40_state, sysram_size_changed, (void *)1)
	PORT_CONFSETTING(    0x00, "None" ) // note: invalid configuration, unless Chip 1 is also 0x00
	PORT_CONFSETTING(    0x10, "2KB" )
	PORT_CONFSETTING(    0x40, "8KB" )

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	// 8x8 keyboard matrix, RESET and ON buttons are not on it. Unused entries are not connected, but some might have a purpose for factory testing(?)
	// The numpad number keys are shared with the ones on the main keyboard, also on the real machine.
	// PORT_NAME lists functions under [SHIFT] as secondaries.
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("SPACE")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("CLR  UCL")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_NAME("LEFT  DEL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("RIGHT  INS")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("UP  PB")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("/")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("DOWN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("ENTER")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("*")

	PORT_START("IN.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("CTL")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_NAME("BREAK")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("RUN")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("FN")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("OFF")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void cc40_state::machine_reset()
{
	m_power = 1;

	update_clock_divider();

	address_space &space = m_maincpu->space(AS_PROGRAM);
	bankswitch_w(space, 0, 0);
}

void cc40_state::init_sysram(int chip, UINT16 size)
{
	if (m_sysram[chip] == nullptr)
	{
		// init to largest possible
		m_sysram[chip] = std::make_unique<UINT8[]>(0x2000);
		save_pointer(NAME(m_sysram[chip].get()), 0x2000, chip);

		save_item(NAME(m_sysram_size[chip]), chip);
		save_item(NAME(m_sysram_end[chip]), chip);
		save_item(NAME(m_sysram_mask[chip]), chip);
	}

	m_nvram[chip]->set_base(m_sysram[chip].get(), size);
	m_sysram_size[chip] = size;
}

void cc40_state::postload()
{
	init_sysram(0, m_sysram_size[0]);
	init_sysram(1, m_sysram_size[1]);

	update_clock_divider();
}

void cc40_state::machine_start()
{
	// init
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	membank("sysbank")->configure_entries(0, 4, memregion("system")->base(), 0x2000);
	if (m_cart_rom)
		membank("cartbank")->configure_entries(0, 4, m_cart_rom->base(), 0x8000);
	else
		membank("cartbank")->set_base(memregion("maincpu")->base() + 0x5000);

	m_nvram[0] = machine().device<nvram_device>("sysram.1");
	m_nvram[1] = machine().device<nvram_device>("sysram.2");
	init_sysram(0, 0x800); // default to 6KB
	init_sysram(1, 0x800); // "

	address_space &space = m_maincpu->space(AS_PROGRAM);
	bus_control_w(space, 0, 0);
	bankswitch_w(space, 0, 0);

	// zerofill other
	m_power = 0;
	m_clock_control = 0;
	m_key_select = 0;

	// register for savestates
	save_item(NAME(m_bus_control));
	save_item(NAME(m_power));
	save_item(NAME(m_banks));
	save_item(NAME(m_clock_control));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_key_select));

	machine().save().register_postload(save_prepost_delegate(FUNC(cc40_state::postload), this));
}

static MACHINE_CONFIG_START( cc40, cc40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS70C20, XTAL_5MHz / 2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_NVRAM_ADD_0FILL("sysram.0")
	MCFG_NVRAM_ADD_0FILL("sysram.1")
	MCFG_NVRAM_ADD_0FILL("sysram.2")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*31+1, 9*1+1+1)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*31, 0, 9*1+1)
	MCFG_DEFAULT_LAYOUT(layout_cc40)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(cc40_state, cc40)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16) // 2*16 internal
	MCFG_HD44780_PIXEL_UPDATE_CB(cc40_pixel_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "cc40_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom,256")
	MCFG_GENERIC_LOAD(cc40_state, cc40_cartridge)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "cc40_cart")
MACHINE_CONFIG_END



/***************************************************************************

  ROM Definitions

***************************************************************************/

ROM_START( cc40 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tms70c20.bin", 0xf800, 0x0800, CRC(a21bf6ab) SHA1(3da8435ecbee143e7fa149ee8e1c92949bade1d8) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "hn61256pc09.bin", 0x0000, 0x8000, CRC(f5322fab) SHA1(1b5c4052a53654363c458f75eac7a27f0752def6) ) // system rom, banked
ROM_END


COMP( 1983, cc40, 0, 0, cc40, cc40, driver_device, 0, "Texas Instruments", "Compact Computer 40", MACHINE_SUPPORTS_SAVE )
