// license:BSD-3-Clause
// copyright-holders:

/*
Sealy Z80-based games

Main components:
Z80C0006PEC or equivalent
12.000 MHz XTAL
6264 RAM (manufacturer and latency may vary)
62256 RAM (manufacturer and latency may vary)
Altera MAX EPM3256AOC208-10
93C46 EEPROM or Watchdata e2829 ESAM
U6295 (Oki M6295 clone)

TODO:
- check controls for all games. Should be working, but definitely needs
  someone who knows how to play these games
- Dou Dizhu games: emulate the dedicated control panel (twenty card keys plus
  action keys, scanned as a 5-row matrix selected through port a0).  The games
  use it instead of the direct controls when test mode switch 1 (EEPROM byte
  0x38 on bbddz/djddz, 0x36 on ddz2) is OFF
- dump the missing jzuanshi program
- dump the lucky168 secure chip if possible
- verify display timing, sound clock and remaining I/O against hardware
*/


#include "emu.h"

#include "sealy_z80_esam.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <memory>


namespace {

class sealy_z80_state : public driver_device
{
public:
	sealy_z80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fgtiles(*this, "fgtiles"),
		m_bgtiles(*this, "bgtiles"),
		m_oki(*this, "oki"),
		m_rombank(*this, "rombank"),
		m_opfixed(*this, "opfixed"),
		m_opbank(*this, "opbank"),
		m_videoram(*this, "videoram"),
		m_in0(*this, "IN0")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void sealy_base(machine_config &config) ATTR_COLD;

	void dizhu_program_map(address_map &map) ATTR_COLD;
	void dizhu_opcodes_map(address_map &map) ATTR_COLD;
	void alt_program_map(address_map &map) ATTR_COLD;
	void alt_opcodes_map(address_map &map) ATTR_COLD;
	void common_io_map(address_map &map) ATTR_COLD;

	void decrypt_dizhu(const uint8_t (&selectors)[64]) ATTR_COLD;
	void decrypt_pljh() ATTR_COLD;
	void descramble_tiles(uint16_t *tiles, uint32_t count, uint16_t xor_mask, int r_field, int g_field, int b_field) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	required_region_ptr<uint16_t> m_fgtiles;
	required_region_ptr<uint16_t> m_bgtiles;

private:
	required_device<okim6295_device> m_oki;

	required_memory_bank m_rombank;
	required_memory_bank m_opfixed;
	required_memory_bank m_opbank;
	required_shared_ptr<uint8_t> m_videoram;

	required_ioport m_in0;

	std::unique_ptr<uint8_t[]> m_decrypted_opcodes;
	bool m_oki_banked = false;
	uint8_t m_nmi_enable = 0;
	uint8_t m_input_select = 0xff;
	bool m_video_enable = true;

	void draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *ram, const uint16_t *tiles, bool transparent);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void rombank_w(uint8_t data);
	void input_select_w(uint8_t data);
	uint8_t inputs_r();
	void nmi_enable_w(uint8_t data) { m_nmi_enable = BIT(data, 0); }
	void video_enable_w(uint8_t data) { m_video_enable = BIT(data, 0); }
	void vblank(int state);
};


// boards with a 93C46 serial EEPROM
class sealy_z80_eeprom_state : public sealy_z80_state
{
public:
	sealy_z80_eeprom_state(const machine_config &mconfig, device_type type, const char *tag) :
		sealy_z80_state(mconfig, type, tag)
	{ }

	void sealy(machine_config &config) ATTR_COLD;
	void pljh(machine_config &config) ATTR_COLD;

	void init_bbddz() ATTR_COLD;
	void init_ddz2() ATTR_COLD;
	void init_djddz() ATTR_COLD;
	void init_pljh() ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	void pljh_program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


// lucky168: the 93C46 lines drive a secure chip speaking T=0 instead
class sealy_z80_esam_state : public sealy_z80_state
{
public:
	sealy_z80_esam_state(const machine_config &mconfig, device_type type, const char *tag) :
		sealy_z80_state(mconfig, type, tag),
		m_esam(*this, "esam")
	{ }

	void lucky168(machine_config &config) ATTR_COLD;

	void init_lucky168() ATTR_COLD;

private:
	required_device<sealy_z80_esam_device> m_esam;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	// the host drives the I/O line through either of the two low bits
	void esam_io_w(uint8_t data) { m_esam->io_w(BIT(data, 0) | BIT(data, 1)); }
};


void sealy_z80_state::machine_start()
{
	uint8_t *const rom = memregion("maincpu")->base();
	m_rombank->configure_entries(0, 8, rom, 0x4000);
	uint8_t *const opcodes = m_decrypted_opcodes ? m_decrypted_opcodes.get() : rom; // jzuanshi has no program
	m_opfixed->set_base(opcodes);
	m_opbank->configure_entries(0, 8, opcodes, 0x4000);
	m_oki_banked = memregion("oki")->bytes() > 0x40000;

	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_input_select));
	save_item(NAME(m_video_enable));
}

void sealy_z80_state::machine_reset()
{
	m_rombank->set_entry(2);
	m_opbank->set_entry(2);
	m_nmi_enable = 0;
	m_video_enable = true;
	input_select_w(0xff);
}

void sealy_z80_state::rombank_w(uint8_t data)
{
	m_rombank->set_entry((data >> 2) & 7);
	m_opbank->set_entry((data >> 2) & 7);
}

void sealy_z80_state::input_select_w(uint8_t data)
{
	m_input_select = data;

	if (m_oki_banked)
		m_oki->set_rom_bank(BIT(data, 5));
}

uint8_t sealy_z80_state::inputs_r()
{
	// TODO: the Dou Dizhu games can also scan a key matrix here, with the rows
	// selected by the low five bits written to port a0.  It is enabled by a test
	// mode switch and needs the dedicated panel, which isn't emulated yet.
	return m_in0->read();
}

void sealy_z80_state::vblank(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


// Each layer is 64x32 cells; the low and high bytes of the tile codes sit in two
// separate 0x800-byte planes.  Tiles are 8x8 pixels of one 16-bit word each.
void sealy_z80_state::draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *ram, const uint16_t *tiles, bool transparent)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *const dst = &bitmap.pix(y);
		const uint8_t *const row = &ram[(y >> 3) * 64];

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const uint32_t cell = x >> 3;
			const uint32_t code = (row[cell] | (row[cell + 0x800] << 8)) & 0x3fff;
			const uint16_t pix = tiles[code * 64 + (y & 7) * 8 + (x & 7)];

			if (!transparent || BIT(pix, 15))
				dst[x] = pix & 0x7fff;
		}
	}
}

uint32_t sealy_z80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_video_enable)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	draw_layer(bitmap, cliprect, &m_videoram[0x1000], &m_bgtiles[0], false);
	draw_layer(bitmap, cliprect, &m_videoram[0x0000], &m_fgtiles[0], true);
	return 0;
}


void sealy_z80_state::dizhu_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram().share(m_videoram);
	map(0xc000, 0xffff).bankr(m_rombank);
}

void sealy_z80_state::dizhu_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_opfixed);
	map(0xc000, 0xffff).bankr(m_opbank);
}


void sealy_z80_state::alt_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xffff).ram().share(m_videoram);
}

void sealy_z80_state::alt_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_opfixed);
	map(0x8000, 0xbfff).bankr(m_opbank);
}

void sealy_z80_state::common_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x30, 0x30).r(FUNC(sealy_z80_state::inputs_r));
	map(0x40, 0x40).portr("IN1");
	// NMI entry and shared-work critical sections write 0, then write 1 on exit.
	map(0x70, 0x70).w(FUNC(sealy_z80_state::nmi_enable_w));
	// lucky168/PLJH bracket full tilemap clears with 0/1 writes here.
	map(0x80, 0x80).w(FUNC(sealy_z80_state::video_enable_w));
	map(0x90, 0x90).w(FUNC(sealy_z80_state::rombank_w));
	map(0xa0, 0xa0).w(FUNC(sealy_z80_state::input_select_w));
	map(0xb0, 0xb0).nopw(); // periodic watchdog strobe
	map(0xc0, 0xc0).nopw(); // bbddz writes 80, djddz writes 02; function unknown
}


void sealy_z80_eeprom_state::program_map(address_map &map)
{
	dizhu_program_map(map);
	map(0x6ff0, 0x6ff0).portr("EEPROMIN");
}

void sealy_z80_eeprom_state::pljh_program_map(address_map &map)
{
	alt_program_map(map);
	map(0xafd0, 0xafd0).portr("EEPROMIN");
}

void sealy_z80_eeprom_state::io_map(address_map &map)
{
	common_io_map(map);
	map(0x50, 0x50).portw("EEPROMCLK");
	map(0x60, 0x60).portw("EEPROMCS");
	map(0xe0, 0xe0).portw("EEPROMOUT");
}


void sealy_z80_esam_state::program_map(address_map &map)
{
	alt_program_map(map);
	map(0x7ff0, 0x7ff0).portr("ESAMIN");
}

void sealy_z80_esam_state::io_map(address_map &map)
{
	common_io_map(map);
	map(0x50, 0x50).portw("ESAMCLK");
	map(0x60, 0x60).portw("ESAMRST");
	map(0xe0, 0xe0).w(FUNC(sealy_z80_esam_state::esam_io_w));
}


static INPUT_PORTS_START( sealy_eeprom )
	PORT_START("EEPROMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))

	PORT_START("EEPROMCLK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))

	PORT_START("EEPROMCS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END

// Direct controls, used when test mode switch 1 is ON (as in the default EEPROMs).
// Functions observed running bbddz, djddz and ddz2; the bonus game confirm only
// on bbddz.  The original button legends are unknown.
static INPUT_PORTS_START( djddz )
	PORT_INCLUDE(sealy_eeprom)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select Card / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bonus Game Confirm")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Bookkeeping / Next")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout / Exit")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pass / Give Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky168 )
	PORT_START("ESAMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::io_r))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ESAMCLK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::clk_w))

	PORT_START("ESAMRST")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::rst_w))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Gamble / Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Play / Collect (alternate)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Menu Confirm") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop Reel 3 / Double Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Play / Menu Exit")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop Reel 1 / Half Gamble")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Bookkeeping / Small / Next")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Collect / Confirm")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Menu Increment")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Menu 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pljh )
	PORT_INCLUDE(sealy_eeprom)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Confirm / Collect Winnings")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Action / Menu Next")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Clear Bet / Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Bet 1 / Menu Increment")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Fold")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bet 5 / Menu Increment")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Raise")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Menu 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Menu Decrement") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Menu Increment") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END


void sealy_z80_state::sealy_base(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // divider not verified, but part rated for 6 MHz

	screen_device &screen(SCREEN(config, "screen"));
	// lucky168's 60-interrupt prompt blink matches ~1 s in a hardware recording;
	// the actual pixel clock, scan totals and blanking duration remain unknown.
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(sealy_z80_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(sealy_z80_state::vblank));

	// the tiles hold BGR555 values directly, so palette entry n decodes to n
	PALETTE(config, "palette", palette_device::BGR_555);

	SPEAKER(config, "mono").front_center();
	// lucky168 and djddz hardware recordings match the effective ~7.57 kHz rate.
	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divider and pin 7 not verified
}


void sealy_z80_eeprom_state::sealy(machine_config &config)
{
	sealy_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_z80_eeprom_state::program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &sealy_z80_eeprom_state::dizhu_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &sealy_z80_eeprom_state::io_map);

	EEPROM_93C46_16BIT(config, "eeprom").default_value(0);
}

void sealy_z80_eeprom_state::pljh(machine_config &config)
{
	sealy(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_z80_eeprom_state::pljh_program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &sealy_z80_eeprom_state::alt_opcodes_map);
}


void sealy_z80_esam_state::lucky168(machine_config &config)
{
	sealy_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_z80_esam_state::program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &sealy_z80_esam_state::alt_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &sealy_z80_esam_state::io_map);

	// clocked like the CPU so that its bit period matches the host's bit-banging loop
	SEALY_Z80_ESAM(config, m_esam, 12_MHz_XTAL / 2);
}


// 百变斗地主 (Bǎi Biàn Dòu Dìzhǔ). All labels have 百变斗地主
ROM_START( bbddz )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u7", 0x00000, 0x20000, CRC(d82df292) SHA1(f354a3d9b29abb61a447d507b37d28f49983e59d) ) // 27c1001a

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u13", 0x000000, 0x200000, CRC(1ee033bb) SHA1(14ab0702e17add44dfc82ce21dd5a37c05a1b2a2) ) // 29f1611

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "u15", 0x000000, 0x200000, CRC(937e9e76) SHA1(95907ddadc2bf260d88a391ab1f61e8931ec7cc3) ) // 29f1611

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u9", 0x00000, 0x80000, CRC(249b1a34) SHA1(94af1a9c64fb7d06a7510d527c176b2fa6845885) ) // 29f040

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, BAD_DUMP CRC(6ca3a909) SHA1(2f55f4040bcab30387d7cc7fecbdcf4d5762a3ed) ) // default with standard control panel
ROM_END

// 斗地主Ⅱ (Dòu Dìzhǔ II). All labels prepend 斗地主Ⅱ to what's below
ROM_START( ddz2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(01cbe7a5) SHA1(f46339bec4e898afaa78831632cea4013877258d) ) // 27c4010

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(200ece45) SHA1(ab9b19464850c9e75646e382334a5d28a360195c) ) // 27c4096

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(c62be0a4) SHA1(1cbfecba43b475f1a175f69fda498e662ac26720) ) // 27c4096

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x40000, CRC(e089cf82) SHA1(567736b1418b86ea35e29fb9f8a408436c8a03c8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, BAD_DUMP CRC(25e71bc6) SHA1(af3cfd71e949cba4605316225e35c0629522ca22) ) // default with standard control panel
ROM_END

// 顶级斗地主 (Dǐngjí Dòu Dìzhǔ). All labels prepend 顶级斗地主 to what's below
// same data was also found in ROMs with 顶级100分 (Dǐngjí 100 Fēn) labels
ROM_START( djddz )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(54abc7a0) SHA1(25494e0862aa6b03398270efe2a3659180be38ec) ) // 27c010

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(6fa8f11e) SHA1(731f90a929b5b638fa45de24918df6377136276d) ) // 27c4096, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(74997b0f) SHA1(1c2b0aeaf71fa000856b8aa405d7853f8e652257) ) // 27c4096

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x80000, CRC(249b1a34) SHA1(94af1a9c64fb7d06a7510d527c176b2fa6845885) ) // 27c040

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, BAD_DUMP CRC(6ca3a909) SHA1(2f55f4040bcab30387d7cc7fecbdcf4d5762a3ed) ) // default with standard control panel
ROM_END

// 漂亮金花 (Piàoliang Jīnhuā). All labels prepend 漂亮金花 to what's below
ROM_START( pljh )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(18b6d64d) SHA1(a17e298098a44a4ffd19c008c45eef09aa35b110) ) // 27c010, 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(8e4cbc34) SHA1(2dcc9ff890f90a440da210742b6564894a627c3b) ) // 27c4096

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(0b774cdd) SHA1(f9b192a67538596d295550ad6316d3fe1e5f7f6a) ) // 27c4096

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x40000, CRC(8cbb5623) SHA1(90169df14264c1e53040bc43106fd8b86b4f1d59) ) // 27c020
ROM_END

// 金钻石 (Jīn Zuànshí)
ROM_START( jzuanshi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, NO_DUMP ) // was stripped from the PCB

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "2.u18", 0x000000, 0x200000, CRC(0fe25de1) SHA1(ddd92d00c1402824c370d8bf29eed2f81b5a916d) )

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "1.u21", 0x000000, 0x200000, CRC(f7cecfb7) SHA1(5469b838301cc2457b65b7ba610db66092cbac87) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u6", 0x00000, 0x40000, CRC(e1b8d758) SHA1(2be84474ab2e16a394db9d899d62dd452696ef0e) )
ROM_END

// 幸运 (Xìngyùn 168 / Lucky 168)
ROM_START( lucky168 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(64cb5ebb) SHA1(1b7f6c7ad1c065058464614ba1b03896a8d8b006) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION16_LE( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "2.u18", 0x000000, 0x200000, CRC(db3b22f3) SHA1(73e32d88a5c463a68927d44932317fd3046c7112) )

	ROM_REGION16_LE( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "1.u21", 0x000000, 0x200000, CRC(bf627727) SHA1(65669dc735b019095dc971d8c5e4df2450d03a02) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u6", 0x00000, 0x40000, CRC(d3106d94) SHA1(d6c8abd57350156b56805d23d4dafe7489d1ac10) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x120, "esam", 0 )
	ROM_LOAD( "esam.bin", 0x000, 0x120, BAD_DUMP CRC(a20f4a52) SHA1(fd3ea1d18853ce31c91553d4b22e90bdedff58ec) ) // reconstructed, not dumped
ROM_END


/*
Opcode fetches (M1) and ordinary reads go through two different transforms, each
an XOR followed by a bit permutation.  The transform pair is selected by a few
address lines: A12, A10, A7, A6, A3 and A1 pick one of five pairs on the Dou Dizhu
boards and lucky168 (same five pairs, different selector table on lucky168), A9, A7, A6,
A5 and A3 pick one of four different pairs on pljh.  A0-A2 never take part, so
the same transform applies to aligned groups of bytes.
*/

// opcode fetches, Dou Dizhu boards and lucky168
uint8_t dizhu_decrypt_opcode(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0xc0, 0, 5, 1, 2, 7, 6, 3, 4);
		case 1: return bitswap<8>(data ^ 0x41, 2, 3, 7, 0, 5, 1, 4, 6);
		case 2: return bitswap<8>(data ^ 0x30, 5, 0, 2, 4, 3, 6, 7, 1);
		case 3: return bitswap<8>(data ^ 0x64, 4, 7, 5, 0, 1, 3, 6, 2);
		case 4: return bitswap<8>(data ^ 0x81, 2, 3, 6, 5, 0, 1, 4, 7);
	}
}

// ordinary reads, Dou Dizhu boards and lucky168
uint8_t dizhu_decrypt_data(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0x24, 1, 2, 3, 6, 0, 4, 5, 7);
		case 1: return bitswap<8>(data ^ 0x0a, 6, 1, 4, 5, 7, 0, 2, 3);
		case 2: return bitswap<8>(data ^ 0x81, 3, 4, 0, 1, 2, 7, 6, 5);
		case 3: return bitswap<8>(data ^ 0x82, 7, 0, 6, 3, 4, 5, 1, 2);
		case 4: return bitswap<8>(data ^ 0xa2, 0, 1, 2, 3, 7, 6, 5, 4);
	}
}

// opcode fetches, pljh
uint8_t pljh_decrypt_opcode(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0x60, 1, 6, 7, 5, 3, 0, 4, 2);
		case 1: return bitswap<8>(data ^ 0x01, 6, 7, 3, 1, 0, 2, 4, 5);
		case 2: return bitswap<8>(data ^ 0x13, 1, 5, 6, 4, 2, 7, 3, 0);
		case 3: return bitswap<8>(data ^ 0x8e, 3, 4, 5, 6, 7, 1, 0, 2);
	}
}

// ordinary reads, pljh
uint8_t pljh_decrypt_data(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0x4e, 2, 4, 1, 7, 0, 5, 3, 6);
		case 1: return bitswap<8>(data ^ 0x46, 0, 2, 4, 6, 5, 1, 7, 3);
		case 2: return bitswap<8>(data ^ 0x81, 5, 1, 0, 3, 7, 6, 2, 4);
		case 3: return bitswap<8>(data ^ 0xd4, 6, 3, 2, 0, 1, 4, 5, 7);
	}
}

// transform pair used for each value of bitswap<6>(address, 12, 10, 7, 6, 3, 1)
constexpr uint8_t DIZHU_SELECTORS[64] =
{
	0, 1, 0, 2, 3, 1, 2, 1,
	2, 4, 1, 3, 2, 4, 1, 2,
	0, 2, 4, 0, 3, 1, 3, 4,
	1, 4, 2, 0, 3, 4, 0, 3,
	2, 4, 0, 3, 0, 2, 4, 1,
	4, 3, 0, 1, 2, 3, 4, 3,
	1, 0, 1, 4, 2, 0, 1, 3,
	0, 4, 2, 0, 1, 3, 2, 4,
};

constexpr uint8_t LUCKY168_SELECTORS[64] =
{
	0, 4, 0, 3, 2, 4, 3, 4,
	3, 1, 4, 2, 3, 1, 4, 3,
	0, 3, 1, 0, 2, 4, 2, 1,
	4, 1, 3, 0, 2, 1, 0, 2,
	3, 1, 0, 2, 0, 3, 1, 4,
	1, 2, 0, 4, 3, 1, 2, 1,
	4, 0, 4, 2, 3, 0, 4, 1,
	0, 2, 3, 0, 4, 1, 3, 2,
};

// transform pair used for each value of bitswap<5>(address, 9, 7, 6, 5, 3)
constexpr uint8_t PLJH_SELECTORS[32] =
{
	0, 1, 0, 2, 3, 0, 1, 2,
	1, 0, 1, 3, 2, 3, 1, 0,
	2, 0, 2, 3, 1, 1, 3, 0,
	3, 2, 0, 1, 3, 2, 0, 1,
};

void sealy_z80_state::decrypt_dizhu(const uint8_t (&selectors)[64])
{
	uint8_t *const rom = memregion("maincpu")->base();
	const uint32_t length = memregion("maincpu")->bytes();
	m_decrypted_opcodes = std::make_unique<uint8_t[]>(length);

	for (uint32_t a = 0; a < length; a++)
	{
		const uint8_t key = selectors[bitswap<6>(a, 12, 10, 7, 6, 3, 1)];
		m_decrypted_opcodes[a] = dizhu_decrypt_opcode(rom[a], key);
		rom[a] = dizhu_decrypt_data(rom[a], key);
	}
}

void sealy_z80_state::decrypt_pljh()
{
	uint8_t *const rom = memregion("maincpu")->base();
	const uint32_t length = memregion("maincpu")->bytes();
	m_decrypted_opcodes = std::make_unique<uint8_t[]>(length);

	for (uint32_t a = 0; a < length; a++)
	{
		const uint8_t key = PLJH_SELECTORS[bitswap<5>(a, 9, 7, 6, 5, 3)];
		m_decrypted_opcodes[a] = pljh_decrypt_opcode(rom[a], key);
		rom[a] = pljh_decrypt_data(rom[a], key);
	}
}

// Convert the stored tile words to the common format: BGR555 with bit 15 kept as
// the opacity flag.  The *_field arguments give the 5-bit field (0 = bits 0-4,
// 1 = bits 5-9, 2 = bits 10-14) holding each component once the XOR is removed.
void sealy_z80_state::descramble_tiles(uint16_t *tiles, uint32_t count, uint16_t xor_mask, int r_field, int g_field, int b_field)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const uint16_t v = tiles[i] ^ xor_mask;
		tiles[i] = (v & 0x8000)
				| (((v >> (r_field * 5)) & 0x1f) << 0)
				| (((v >> (g_field * 5)) & 0x1f) << 5)
				| (((v >> (b_field * 5)) & 0x1f) << 10);
	}
}

void sealy_z80_eeprom_state::init_bbddz()
{
	decrypt_dizhu(DIZHU_SELECTORS);
	descramble_tiles(&m_fgtiles[0], m_fgtiles.length(), 0x0000, 1, 0, 2);
	descramble_tiles(&m_bgtiles[0], m_bgtiles.length(), 0x0000, 1, 0, 2);
}

void sealy_z80_eeprom_state::init_ddz2()
{
	decrypt_dizhu(DIZHU_SELECTORS);
}

void sealy_z80_eeprom_state::init_djddz()
{
	decrypt_dizhu(DIZHU_SELECTORS);
	descramble_tiles(&m_fgtiles[0], m_fgtiles.length(), 0x0610, 1, 0, 2);
	descramble_tiles(&m_bgtiles[0], m_bgtiles.length(), 0x0430, 1, 2, 0);
}

void sealy_z80_esam_state::init_lucky168()
{
	decrypt_dizhu(LUCKY168_SELECTORS);
}

void sealy_z80_eeprom_state::init_pljh()
{
	decrypt_pljh();
}

} // anonymous namespace


GAME( 2005, bbddz,    0, sealy,    djddz,    sealy_z80_eeprom_state, init_bbddz,    ROT0, "Sealy", "Bai Bian Dou Dizhu", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2003, ddz2,     0, sealy,    djddz,    sealy_z80_eeprom_state, init_ddz2,     ROT0, "Sealy", "Dou Dizhu II",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2004, djddz,    0, sealy,    djddz,    sealy_z80_eeprom_state, init_djddz,    ROT0, "Sealy", "Dingji Dou Dizhu",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2000, pljh,     0, pljh,     pljh,     sealy_z80_eeprom_state, init_pljh,     ROT0, "Sealy", "Piaoliang Jinhua",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2003, jzuanshi, 0, lucky168, lucky168, sealy_z80_esam_state,   empty_init,    ROT0, "Sealy", "Jin Zuanshi",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // missing program ROM
GAME( 2002, lucky168, 0, lucky168, lucky168, sealy_z80_esam_state,   init_lucky168, ROT0, "Sealy", "Lucky 168",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
