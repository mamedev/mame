// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Jon Guidry
/*******************************************************************************

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
HN61256PC09   - Hitachi DIP-28 32KB CMOS Mask PROM (also seen with HN61256PB02, earlier version?)
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

As for the CC-40+, the product was finalized, but in the end it wasn't released.
The hardware is very similar to CC-40. The main differences are a TMS70C40 CPU
(twice larger internal ROM), and a cassette port separate from Hexbus. The
controller chip is a TI TP0373 this time, it appears that the basic functionality
is the same as the one by AMI. Like the CC-40, it had either 6KB or 18KB RAM.

The CC-40+ cassette device number is 1, eg. SAVE"1.FILENAME" to save, and
OLD"1.FILENAME" to load.


TODO:
- external RAM cartridge (bus_control_w cartridge memory addressing)
- auto clock divider on slow memory access
- Hexbus interface and peripherals
  * HX-1000: color plotter
  * HX-1010: thermal printer
  * HX-3000: RS-232 interface
  * HX-3100: modem
  * HX-3200: Centronics printer interface

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/tms7000/tms7000.h"
#include "imagedev/cassette.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "cc40.lh"


namespace {

class cc40_state : public driver_device
{
public:
	cc40_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "sysram.%u", 1U),
		m_sysbank(*this, "sysbank"),
		m_cartbank(*this, "cartbank"),
		m_cart(*this, "cartslot"),
		m_cass(*this, "cassette"),
		m_key_matrix(*this, "IN.%u", 0),
		m_segs(*this, "seg%u", 0U)
	{
		m_sysram[0] = nullptr;
		m_sysram[1] = nullptr;
	}

	DECLARE_INPUT_CHANGED_MEMBER(sysram_size_changed);

	void cc40(machine_config &config);
	void cc40p(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	required_device<tms7000_device> m_maincpu;
	required_device_array<nvram_device, 2> m_nvram;
	required_memory_bank m_sysbank;
	required_memory_bank m_cartbank;
	required_device<generic_slot_device> m_cart;
	optional_device<cassette_image_device> m_cass;
	required_ioport_array<8> m_key_matrix;
	output_finder<80> m_segs;

	memory_region *m_cart_rom;

	u8 m_bus_control = 0;
	u8 m_power = 0;
	u8 m_banks = 0;
	u8 m_clock_control = 0;
	u8 m_clock_divider = 0;
	u8 m_key_select = 0;

	std::unique_ptr<u8[]> m_sysram[2];
	u16 m_sysram_size[2];
	u16 m_sysram_end[2];
	u16 m_sysram_mask[2];

	void init_sysram(int chip, u16 size);
	void update_lcd_indicator(u8 y, u8 x, int state);
	void update_clock_divider();

	u8 sysram_r(offs_t offset);
	void sysram_w(offs_t offset, u8 data);
	u8 bus_control_r();
	void bus_control_w(u8 data);
	u8 power_r();
	void power_w(u8 data);
	u8 bankswitch_r();
	void bankswitch_w(u8 data);
	u8 clock_control_r();
	void clock_control_w(u8 data);
	u8 keyboard_r();
	void keyboard_w(u8 data);
	u8 cass_r();
	void cass_w(u8 data);

	void cc40_palette(palette_device &palette) const;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	HD44780_PIXEL_UPDATE(cc40_pixel_update);

	void cc40_map(address_map &map) ATTR_COLD;
	void cc40p_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Initialisation
*******************************************************************************/

void cc40_state::init_sysram(int chip, u16 size)
{
	if (m_sysram[chip] == nullptr)
	{
		// init to largest possible
		m_sysram[chip] = std::make_unique<u8[]>(0x2000);
		save_pointer(NAME(m_sysram[chip]), 0x2000, chip);

		save_item(NAME(m_sysram_size[chip]), chip);
		save_item(NAME(m_sysram_end[chip]), chip);
		save_item(NAME(m_sysram_mask[chip]), chip);
	}

	m_nvram[chip]->set_base(m_sysram[chip].get(), size);
	m_sysram_size[chip] = size;
}

void cc40_state::device_post_load()
{
	init_sysram(0, m_sysram_size[0]);
	init_sysram(1, m_sysram_size[1]);

	update_clock_divider();
}

void cc40_state::machine_start()
{
	// init
	m_segs.resolve();
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	m_sysbank->configure_entries(0, 4, memregion("system")->base(), 0x2000);
	if (m_cart_rom)
		m_cartbank->configure_entries(0, 4, m_cart_rom->base(), 0x8000);
	else
		m_cartbank->set_base(memregion("maincpu")->base() + 0x5000);

	init_sysram(0, 0x800); // default to 6KB
	init_sysram(1, 0x800); // "

	bus_control_w(0);
	bankswitch_w(0);

	// register for savestates
	save_item(NAME(m_bus_control));
	save_item(NAME(m_power));
	save_item(NAME(m_banks));
	save_item(NAME(m_clock_control));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_key_select));
}

void cc40_state::machine_reset()
{
	m_power = 1;

	update_clock_divider();
	bankswitch_w(0);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(cc40_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");

	// max size is 4*32KB
	if (size > 0x2'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid file size (must be no more than 128K)");

	m_cart->rom_alloc(0x2'0000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE); // allocate a larger ROM region to have 4x32K banks
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    Video
*******************************************************************************/

void cc40_state::cc40_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(50, 45, 60)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

void cc40_state::update_lcd_indicator(u8 y, u8 x, int state)
{
	// reference _________________...
	// output#  |10  11     12     13     14      0      1      2      3   4
	// above    | <  SHIFT  CTL    FN     DEG    RAD    GRAD   I/O    UCL  >
	// ---- raw lcd screen here ----
	// under    |    ERROR   v      v      v      v      v      v    _LOW
	// output#  |    60     61     62     63     50     51     52     53
	m_segs[y * 10 + x] = state ? 1 : 0;
}

HD44780_PIXEL_UPDATE(cc40_state::cc40_pixel_update)
{
	// char size is 5x7 + cursor
	if (x > 4 || y > 7)
		return;

	if (line == 1 && pos == 15)
	{
		// the last char is used to control the 18 lcd indicators
		update_lcd_indicator(y, x, state);
	}
	else if (line < 2 && pos < 16)
	{
		// internal: 2*16, external: 1*31
		if (y == 7) y++; // the cursor is slightly below the character
		bitmap.pix(1 + y, 1 + line*16*6 + pos*6 + x) = state ? 1 : 2;
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 cc40_state::sysram_r(offs_t offset)
{
	// read system ram, based on addressing configured in bus_control_w
	if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
		return m_sysram[0][offset & (m_sysram_size[0] - 1)];
	else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
		return m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)];
	else
		return 0xff;
}

void cc40_state::sysram_w(offs_t offset, u8 data)
{
	// write system ram, based on addressing configured in bus_control_w
	if (offset < m_sysram_end[0] && m_sysram_size[0] != 0)
		m_sysram[0][offset & (m_sysram_size[0] - 1)] = data;
	else if (offset < m_sysram_end[1] && m_sysram_size[1] != 0)
		m_sysram[1][(offset - m_sysram_end[0]) & (m_sysram_size[1] - 1)] = data;
}

u8 cc40_state::bus_control_r()
{
	return m_bus_control;
}

void cc40_state::bus_control_w(u8 data)
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

u8 cc40_state::power_r()
{
	return m_power;
}

void cc40_state::power_w(u8 data)
{
	// d0: power-on hold latch
	m_power = data & 1;

	// stop running
	if (!m_power)
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

u8 cc40_state::bankswitch_r()
{
	return m_banks;
}

void cc40_state::bankswitch_w(u8 data)
{
	data &= 0x0f;

	// d0-d1: system rom bankswitch
	m_sysbank->set_entry(data & 3);

	// d2-d3: cartridge 32KB page bankswitch
	if (m_cart_rom)
		m_cartbank->set_entry(data >> 2 & 3);

	m_banks = data;
}

u8 cc40_state::clock_control_r()
{
	return m_clock_control;
}

void cc40_state::update_clock_divider()
{
	// 2.5MHz /3 to /17 in steps of 2
	m_clock_divider = (~m_clock_control & 7) * 2 + 1;
	m_maincpu->set_clock_scale((m_clock_control & 8) ? (1.0 / (double)m_clock_divider) : 1);
}

void cc40_state::clock_control_w(u8 data)
{
	data &= 0x0f;

	// d0-d2: clock divider
	// d3: enable clock divider always
	// other bits: unused?
	if (m_clock_control != data)
	{
		m_clock_control = data;
		update_clock_divider();
	}
}

u8 cc40_state::keyboard_r()
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

void cc40_state::keyboard_w(u8 data)
{
	// d0-d7: select keyboard column
	m_key_select = data;
}

u8 cc40_state::cass_r()
{
	// d3: cass data in
	return (m_cass->input() > 0.04) ? 8 : 0;
}

void cc40_state::cass_w(u8 data)
{
	// d4: cass motor
	m_cass->set_motor((data & 0x10) ? 1 : 0);

	// d3: cass data out
	m_cass->output((data & 8) ? +1.0 : -1.0);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cc40_state::cc40_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0110, 0x0110).rw(FUNC(cc40_state::bus_control_r), FUNC(cc40_state::bus_control_w));
	map(0x0111, 0x0111).rw(FUNC(cc40_state::power_r), FUNC(cc40_state::power_w));
	map(0x0112, 0x0112).noprw(); // d0-d3: Hexbus data
	map(0x0113, 0x0113).noprw(); // d0: Hexbus available
	map(0x0114, 0x0114).noprw(); // d1: Hexbus handshake
	map(0x0115, 0x0115).w("dac", FUNC(dac_bit_interface::data_w));
	map(0x0116, 0x0116).portr("BATTERY");
	map(0x0119, 0x0119).rw(FUNC(cc40_state::bankswitch_r), FUNC(cc40_state::bankswitch_w));
	map(0x011a, 0x011a).rw(FUNC(cc40_state::clock_control_r), FUNC(cc40_state::clock_control_w));
	map(0x011e, 0x011f).rw("hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));

	map(0x0800, 0x0fff).ram().share("sysram.0");
	map(0x1000, 0x4fff).rw(FUNC(cc40_state::sysram_r), FUNC(cc40_state::sysram_w));
	map(0x5000, 0xcfff).bankr("cartbank");
	map(0xd000, 0xefff).bankr("sysbank");
}

void cc40_state::cc40p_map(address_map &map)
{
	cc40_map(map);
	map(0x0121, 0x0121).rw(FUNC(cc40_state::cass_r), FUNC(cc40_state::cass_w));
}



/*******************************************************************************
    Inputs
*******************************************************************************/

INPUT_CHANGED_MEMBER(cc40_state::sysram_size_changed)
{
	init_sysram((int)param, newval << 11);
}

static INPUT_PORTS_START( cc40 )
	PORT_START("RAMSIZE")
	PORT_CONFNAME( 0x07, 0x01, "RAM Chip 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cc40_state, sysram_size_changed, 0)
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x01, "2KB" )
	PORT_CONFSETTING(    0x04, "8KB" )
	PORT_CONFNAME( 0x70, 0x10, "RAM Chip 2") PORT_CHANGED_MEMBER(DEVICE_SELF, cc40_state, sysram_size_changed, 1)
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("/")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("DOWN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_NAME("+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("ENTER")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_NAME("*")

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



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cc40_state::cc40(machine_config &config)
{
	// basic machine hardware
	TMS70C20(config, m_maincpu, 5_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cc40_state::cc40_map);
	m_maincpu->in_porta().set(FUNC(cc40_state::keyboard_r));
	m_maincpu->out_portb().set(FUNC(cc40_state::keyboard_w));

	NVRAM(config, "sysram.0", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "sysram.1", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "sysram.2", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60); // arbitrary
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(6*31+1, 9*1+1+1);
	screen.set_visarea_full();
	config.set_default_layout(layout_cc40);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(cc40_state::cc40_palette), 3);

	hd44780_device &hd44780(HD44780(config, "hd44780", 270'000)); // OSC = 91K resistor
	hd44780.set_lcd_size(2, 16); // 2*16 internal
	hd44780.set_pixel_update_cb(FUNC(cc40_state::cc40_pixel_update));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "cc40_cart", "bin,rom,256").set_device_load(FUNC(cc40_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("cc40_cart");
}

void cc40_state::cc40p(machine_config &config)
{
	cc40(config);

	// basic machine hardware
	TMS70C40(config.replace(), m_maincpu, 5_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cc40_state::cc40p_map);
	m_maincpu->in_porta().set(FUNC(cc40_state::keyboard_r));
	m_maincpu->out_portb().set(FUNC(cc40_state::keyboard_w));

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_MUTED | CASSETTE_MOTOR_DISABLED);
	SPEAKER(config, "cass_output").front_center(); // on data recorder
	m_cass->add_route(ALL_OUTPUTS, "cass_output", 0.05);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cc40 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "c11002", 0x0000, 0x0800, CRC(a21bf6ab) SHA1(3da8435ecbee143e7fa149ee8e1c92949bade1d8) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "hn61256pc09", 0x0000, 0x8000, CRC(f5322fab) SHA1(1b5c4052a53654363c458f75eac7a27f0752def6) ) // system rom, banked
ROM_END

ROM_START( cc40p )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "75305.u200", 0x0000, 0x1000, CRC(42bb6af2) SHA1(642dede16cb4ef2c5b9eaae79e28054f1111eef8) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "hn61256pc09.u205", 0x0000, 0x8000, CRC(f5322fab) SHA1(1b5c4052a53654363c458f75eac7a27f0752def6) ) // system rom, banked
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY              FULLNAME               FLAGS
SYST( 1983, cc40,  0,     0,      cc40,    cc40,  cc40_state, empty_init, "Texas Instruments", "Compact Computer 40", MACHINE_SUPPORTS_SAVE )
SYST( 1984, cc40p, cc40,  0,      cc40p,   cc40,  cc40_state, empty_init, "Texas Instruments", "Compact Computer 40 Plus (prototype)", MACHINE_SUPPORTS_SAVE )
