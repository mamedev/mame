// license:BSD-3-Clause
// copyright-holders:David Haywood

// the BBL 380 - 180 in 1 features similar menus / presentation / games to the 'ORB Gaming Retro Arcade Pocket Handheld Games Console with 153 Games' (eg has Matchstick Man, Gang Tie III etc.)
// https://www.youtube.com/watch?v=NacY2WHd-CY

// contains 6502 code

#include "emu.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"

class bbl380_state : public driver_device
{
public:
	bbl380_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void bbl380(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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

static INPUT_PORTS_START( bbl380 )
INPUT_PORTS_END

void bbl380_state::bbl380(machine_config &config)
{
	// unknown CPU, 6502 based

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(bbl380_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
}

ROM_START( bbl380 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbl 380 180 in 1.bin", 0x000000, 0x400000, CRC(146c88da) SHA1(7f18526a6d8cf991f86febce3418d35aac9f49ad) )
ROM_END

CONS( 200?, bbl380,        0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "BaoBaoLong", "BBL380 - 180 in 1", MACHINE_IS_SKELETON )
