// license:BSD-3-Clause
// copyright-holders:David Haywood

// the BBL 380 - 180 in 1 features similar menus / presentation / games to the 'ORB Gaming Retro Arcade Pocket Handheld Games Console with 153 Games' (eg has Matchstick Man, Gang Tie III etc.)
// https://www.youtube.com/watch?v=NacY2WHd-CY

// BIOS calls are made very frequently to the undumped firmware.
// The most common call ($6058 in bbl380, $6062 in ragc153 & dphh8630) seems to involve downloading a snippet of code from Flash and executing it from RAM.
// A variant of this call ($60d2 in bbl380, $60e3 in ragc153 & dphh8630) is invoked with jsr.
// For these calls, a 24-bit starting address is specified in $82:$81:$80, and the length in bytes is twice the number specified in $84:$83.
// ST2205U cannot execute code directly from Flash, but has a built-in DMA-compatible NAND interface on Port F ($05).

#include "emu.h"

#include "cpu/m6502/st2205u.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

class bbl380_state : public driver_device
{
public:
	bbl380_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void bbl380(machine_config &config);
	void init_ragc153();

private:
	void lcdc_command_w(u8 data);
	u8 lcdc_data_r();
	void lcdc_data_w(u8 data);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bbl380_map(address_map &map);

	required_device<st2xxx_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

uint32_t bbl380_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void bbl380_state::machine_start()
{
}

void bbl380_state::machine_reset()
{
}

void bbl380_state::lcdc_command_w(u8 data)
{
	logerror("%s: LCDC command $%02X\n", machine().describe_context(), data);
}

u8 bbl380_state::lcdc_data_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: LCDC data read\n", machine().describe_context());
	return 0;
}

void bbl380_state::lcdc_data_w(u8 data)
{
	logerror("%s: LCDC data $%02X\n", machine().describe_context(), data);
}

void bbl380_state::bbl380_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("maincpu", 0); // FIXME: probably not directly mapped (ST2205U has serial Flash interface on port F)
	map(0x600000, 0x600000).w(FUNC(bbl380_state::lcdc_command_w));
	map(0x604000, 0x604000).rw(FUNC(bbl380_state::lcdc_data_r), FUNC(bbl380_state::lcdc_data_w));
}

static INPUT_PORTS_START( bbl380 )
INPUT_PORTS_END

void bbl380_state::bbl380(machine_config &config)
{
	ST2205U(config, m_maincpu, 8000000); // unknown clock; type guessed
	m_maincpu->set_addrmap(AS_DATA, &bbl380_state::bbl380_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // TFT color LCD
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(132, 162);
	m_screen->set_visarea(0, 132-1, 0, 162-1);
	m_screen->set_screen_update(FUNC(bbl380_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	// LCD controller seems to be either Sitronix ST7735R or (if RDDID bytes match) Ilitek ILI9163C
	// (unless the SoC's built-in one is used and the routines which program these are leftovers)
	// Several other LCDC models are identified by ragc153 and dphh8630
}

ROM_START( bbl380 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbl380_st2205u.bin", 0x000000, 0x004000, NO_DUMP ) // internal OTPROM BIOS
	ROM_LOAD( "bbl 380 180 in 1.bin", 0x000000, 0x400000, CRC(146c88da) SHA1(7f18526a6d8cf991f86febce3418d35aac9f49ad) BAD_DUMP )
	// 0x0022XX, 0x0026XX, 0x002AXX, 0x002CXX, 0x002DXX, 0x0031XX, 0x0036XX, etc. should not be FF fill
ROM_END


ROM_START( ragc153 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ragc153_st2205u.bin", 0x000000, 0x004000, NO_DUMP ) // internal OTPROM BIOS (addresses are different from bbl380)
	ROM_LOAD( "25q32ams.bin", 0x000000, 0x400000, CRC(de328d73) SHA1(d17b97e9057be4add68b9f5a26e04c9f0a139673) ) // first 0x100 bytes would read as 0xff at regular speed, but give valid looking consistent data at a slower rate
ROM_END

ROM_START( dphh8630 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ragc153_st2205u.bin", 0x000000, 0x004000, NO_DUMP ) // internal OTPROM BIOS
	ROM_LOAD( "bg25q16.bin", 0x000000, 0x200000, CRC(277850d5) SHA1(740087842e1e63bf99b4ca9c1b2053361f267269) )
ROM_END



void bbl380_state::init_ragc153()
{
	uint8_t *ROM = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size; i++)
	{
		ROM[i] = ROM[i] ^ 0xe4;
	}
}

CONS( 200?, bbl380,        0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "BaoBaoLong", "BBL380 - 180 in 1", MACHINE_IS_SKELETON )
CONS( 200?, ragc153,       0,       0,      bbl380,   bbl380, bbl380_state, init_ragc153, "Orb", "Retro Arcade Game Controller 153-in-1", MACHINE_IS_SKELETON )
CONS( 200?, dphh8630,      0,       0,      bbl380,   bbl380, bbl380_state, init_ragc153, "<unknown>", "Digital Pocket Hand Held System Model: 8630 - 230-in-1", MACHINE_IS_SKELETON )
