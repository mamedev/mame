// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for gambling (?) games running on very similar PCBs manufactured by 'Videos A A'.

    Probably manufactured in Italy since PCBs are marked 'lato componenti' (components side) and 'lato
    saldature' (solder side). Lady Gum's 68HC705 ROM contains strings in French, though.

    PCBs couldn't be tested so game titles are taken from ROM stickers.

    The only complete dump is Lady Gum, all the others are missing at least the 68HC705 internal ROM.

    Hardware overview:
    Main CPU: MC68HC705C8ACS (verified on Winner, stickers and PCB marks on other games say 68HC705)
    CRTC: HD46505SP-2
    Sound: NEC D7759C
    RAM: 2 x 6116 or equivalent
    OSC: 10 MHz
    Dips: 1 x 8 dips bank
*/

#include "emu.h"
#include "emupal.h"
#include "cpu/m6805/m68hc05.h"
#include "sound/upd7759.h"
#include "video/mc6845.h"
#include "screen.h"

class videosaa_state : public driver_device
{
public:
	videosaa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void videosaa(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void videosaa_state::video_start()
{
}

uint32_t videosaa_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( videosaa )
INPUT_PORTS_END

static const gfx_layout tiles_layout = // seems to give reasonable results but needs verifying
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "gfx", 0, tiles_layout, 0, 1 )
GFXDECODE_END

void videosaa_state::machine_start()
{
}

void videosaa_state::machine_reset()
{
}

void videosaa_state::videosaa(machine_config &config)
{
	/* basic machine hardware */
	M68HC705C8A(config, m_maincpu, 10_MHz_XTAL); // unknown divider

	/* video hardware */
	// all wrong
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(videosaa_state::screen_update));

	H46505(config, "crtc", 10_MHz_XTAL); // unknown divider

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	/* sound hardware */
	UPD7759(config, "upd");
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( jokrlady )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-jokerlady.0d", 0x0000, 0x2000, NO_DUMP ) // was missing on the PCB

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "jokerlady-video0.5b", 0x0000, 0x2000, NO_DUMP ) // damaged, reads weren't consistent
	ROM_LOAD( "jokerlady-video1.7b", 0x2000, 0x2000, CRC(196932d8) SHA1(5130f03dd88f841a00ef328f12c6211bec377c77) )
	ROM_LOAD( "jokerlady-video2.8b", 0x4000, 0x2000, CRC(58d62dbd) SHA1(cfcab2ca0c081f62185ce59e98124e2c5d368f49) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "jokerlady-msg0.5a", 0x00000, 0x10000, NO_DUMP ) // 27512, damaged, reads weren't consistent

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "st16as15hb1.9c", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( ladygum )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-ladygum-4.0d", 0x0000, 0x2000, CRC(ea099edf) SHA1(eb0e4ccb025cf2a71d2016d501b4da11f8d21677) ) // actual label 68hc705-ladygum-#4.0d

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "ladygum-video0.5b", 0x0000, 0x2000, CRC(c1b72a5b) SHA1(abb9c19be474a83e4f4568a5431634ba7a61d8db) )
	ROM_LOAD( "ladygum-video1.7b", 0x2000, 0x2000, CRC(70448e1f) SHA1(3c0a94284193e7d0f4efb8ffa746b9150d0119e4) )
	ROM_LOAD( "ladygum-video2.8b", 0x4000, 0x2000, CRC(58d62dbd) SHA1(cfcab2ca0c081f62185ce59e98124e2c5d368f49) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "ladygum-msg0.5a", 0x00000, 0x10000, CRC(c1f23f58) SHA1(c89579e53545291638be66e51512458b67251f67) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "st16as15hb1.9c", 0x000, 0x117, CRC(a76721d4) SHA1(c9f14107eceeeed9fa788916e3c40c9dd2fe5c41) )
ROM_END

ROM_START( paradar )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-paradar.0d", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "paradar-video0.5b", 0x0000, 0x2000, CRC(b66a8b54) SHA1(aecc7b8ceed6189954a7cd3550d90dabd3bb23d6) )
	ROM_LOAD( "paradar-video1.7b", 0x2000, 0x2000, CRC(72b05246) SHA1(65988850188915583870a7fbb23b84193c1e753d) )
	ROM_LOAD( "paradar-video2.8b", 0x4000, 0x2000, CRC(b5eb61a0) SHA1(3ce63c7a68cf3e5343ceec53d477a1322e7f2929) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "paradar-msg0.5a", 0x00000, 0x10000, CRC(81f7c0cb) SHA1(15ee0c12a8f9c94beac7e5fe894e5f82a53d9fc1) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "gal16v8b.9c", 0x000, 0x117, CRC(3e031d55) SHA1(827c5380ec54bb162873bf19a19218255bd73e9d) )
ROM_END

ROM_START( winner ) // dump confirmed from 3 different PCBs
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-winner.0d", 0x0000, 0x2000, NO_DUMP ) // programmer gives a 'BOOTLOADER NOT RESPONDING' error

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "winner-video0.5b", 0x0000, 0x2000, CRC(14740cf5) SHA1(ac3dc4de1d3135a33eb68e73e527059d53638354) )
	ROM_LOAD( "winner-video1.7b", 0x2000, 0x2000, CRC(caa4ef76) SHA1(2dab5bbe2a4bd60247219a9fc38cae017b81cdbf) )
	ROM_LOAD( "winner-video2.8b", 0x4000, 0x2000, CRC(ed520709) SHA1(10707aa2985eea06724c1f32c55c3c6b17e57333) )

	ROM_REGION( 0x10000, "upd", 0 )
	ROM_LOAD( "winner-msg0.5a", 0x00000, 0x10000, CRC(81f7c0cb) SHA1(15ee0c12a8f9c94beac7e5fe894e5f82a53d9fc1) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "palce16v8h-25pc-4.9c", 0x000, 0x117, CRC(ac0347ee) SHA1(26b27d2037400c06e63528e0e59be6d7d1a81cab) )
ROM_END

GAME( 1995?, jokrlady,  0,   videosaa, videosaa, videosaa_state, empty_init, ROT0, "Videos A A", "Joker Lady",  MACHINE_IS_SKELETON )
GAME( 1995?, ladygum,   0,   videosaa, videosaa, videosaa_state, empty_init, ROT0, "Videos A A", "Lady Gum",    MACHINE_IS_SKELETON )
GAME( 1995?, paradar,   0,   videosaa, videosaa, videosaa_state, empty_init, ROT0, "Videos A A", "Paradar",     MACHINE_IS_SKELETON )
GAME( 1995?, winner,    0,   videosaa, videosaa, videosaa_state, empty_init, ROT0, "Videos A A", "Winner",      MACHINE_IS_SKELETON )
