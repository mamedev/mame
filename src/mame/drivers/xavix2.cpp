// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    XaviX 2

    unknown architecture, does not appear to be 6502 derived like XaviX / SuperXaviX

    die is marked  "SSD 2002-2004 NEC 800208-51"

*******************************************************************************/

#include "emu.h"

#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"

class xavix2_state : public driver_device
{
public:
	xavix2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void xavix2(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
};

uint32_t xavix2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void xavix2_state::machine_start()
{
}

void xavix2_state::machine_reset()
{
}

static INPUT_PORTS_START( xavix2 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout char16layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4, 9*4,8*4,11*4,10*4,13*4,12*4,15*4,14*4 },
	{ STEP16(0,4*16) },
	16*16*4
};

static const gfx_layout charlayout8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static const gfx_layout char16layout8bpp =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( gfx_xavix )
	GFXDECODE_ENTRY( "maincpu", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "maincpu", 0, char16layout, 0, 16 )
	GFXDECODE_ENTRY( "maincpu", 0, charlayout8bpp, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0, char16layout8bpp, 0, 1 )
GFXDECODE_END


void xavix2_state::xavix2(machine_config &config)
{
	// unknown CPU 'SSD 2002-2004 NEC 800208-51'

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(xavix2_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_xavix);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// unknown sound hardware
}


ROM_START( ltv_naru )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "naruto.bin", 0x000000, 0x800000, CRC(e3465ad2) SHA1(13e3d2de5d5a084635cab158f3639a1ea73265dc) )
ROM_END

ROM_START( domfitad )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "xpfitnessadventure.bin", 0x000000, 0x1000000, CRC(a7917081) SHA1(95ae5dc6e64a78ae060cb0e61d8b0af34a93c4ce) )
ROM_END

ROM_START( dombikec )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbikeconcept.bin", 0x000000, 0x1000000, CRC(3447fce5) SHA1(c7e9e9cd789a17ac886ecf253f67753213cf8d21) )
ROM_END


CONS( 2006, ltv_naru, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Bandai / SSD Company LTD", "Let's TV Play Naruto", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// These are for the 'Domyos Interactive System' other Domyos Interactive System games can be found in xavix.cpp (the SoC is inside the cartridge, base acts as a 'TV adapter' only)

// Has SEEPROM and an RTC.  Adventure has the string DOMYSSDCOLTD a couple of times.
CONS( 2008, domfitad, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Fitness Adventure (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2008, dombikec, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Bike Concept (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )




