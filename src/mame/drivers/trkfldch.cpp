// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Track & Field Challenge TV Game
https://www.youtube.com/watch?v=wjn1lLylqog

HELP!  what architecture is this?  CPU type doesn't seem obvious

*/

#include "emu.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class trkfldch_state : public driver_device
{
public:
	trkfldch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void trkfldch(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:

	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trkfldch_map(address_map &map);
};

void trkfldch_state::video_start()
{
}

uint32_t trkfldch_state::screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void trkfldch_state::trkfldch_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

static INPUT_PORTS_START( trkfldch )
INPUT_PORTS_END

// dummy, doesn't appear to be tile based
static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( gfx_trkfldch )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

void trkfldch_state::machine_start()
{
}

void trkfldch_state::machine_reset()
{
}

MACHINE_CONFIG_START(trkfldch_state::trkfldch)
	/* basic machine hardware */
//	MCFG_DEVICE_ADD("maincpu", Z80,8000000)
//	MCFG_DEVICE_PROGRAM_MAP(trkfldch_map)
//	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", trkfldch_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(trkfldch_state, screen_update_trkfldch)
	MCFG_SCREEN_PALETTE("palette")

	GFXDECODE(config, m_gfxdecode, "palette", gfx_trkfldch); // dummy
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG); // dummy
MACHINE_CONFIG_END

ROM_START( trkfldch )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "trackandfield.bin", 0x000000, 0x400000,  CRC(f4f1959d) SHA1(344dbfe8df1897adf77da6e5ca0435c4d47d6842) )
ROM_END

CONS( 2007, trkfldch,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "Track & Field Challenge", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

