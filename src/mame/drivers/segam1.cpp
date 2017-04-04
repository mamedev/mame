// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Sega M1 hardware (837-7571) (PCB)

Sega Bingo Multicart (837-10675) (Sticker on PCB)

used for redemption / gambling style machines in a satellite setup

based on Caribbean Boule the following hardware setup is used

One X-Board (segaxbd.cpp) drives a large rear-projection monitor which all players view to see the main game progress.

Multiple M1 boards ("satellite" board) for each player for them to view information privately.

One 'link' board which connects everything together.  The link board has audio hardware, a 68K, and a Z80 as
well as a huge bank of UARTS and toslink connectors, but no video. It's possible the main game logic runs
on the 'link' board.


Unfortunately we don't have any dumps of anything other than an M1 board right now.

---

is this related to (or a component of?) bingoc.cpp, the EPR numbers are much lower there tho
so it's probably an earlier version of the same thing or one of the 'link' boards?

uses s24 style tilemaps (ram based?)


*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"



class segam1_state : public driver_device
{
public:
	segam1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	virtual void video_start() override;
	uint32_t screen_update_segam1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

void segam1_state::video_start()
{
}

uint32_t segam1_state::screen_update_segam1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( segam1_map, AS_PROGRAM, 16, segam1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( segam1 )
INPUT_PORTS_END




static MACHINE_CONFIG_START( segam1, segam1_state )

	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2)
	MCFG_CPU_PROGRAM_MAP(segam1_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", segam1_state,  irq1_line_hold)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segam1_state, screen_update_segam1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	// YM3438
MACHINE_CONFIG_END


ROM_START( bingpty )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "epr-16648b.bin", 0x00000, 0x20000, CRC(e4fceb4c) SHA1(0a248bb328d2f6d72d540baefbe62838f4b76585) )
	ROM_LOAD16_BYTE( "epr-16649b.bin", 0x00001, 0x20000, CRC(736d8bbd) SHA1(c359ad513d4a7693cbb1a27ce26f89849e894d05) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14845.bin", 0x00000, 0x20000, CRC(90d47101) SHA1(7bc002c104e3dbde1986aaec54112d5658eab523) )

	ROM_REGION( 0x8000, "m1comm", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14221a.bin", 0x00000, 0x8000, CRC(a13e67a4) SHA1(4cd269c7f04a64ae7806c8784f86bf6553a25d85) )

	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

ROM_START( unkm1 ) // 1992.01.31 string
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "epr-14427.ic8", 0x00000, 0x40000, CRC(2d904fc6) SHA1(7062f47d77d09906420118c85e1cb565bec345a7) )
	ROM_LOAD16_BYTE( "epr-14428.ic7", 0x00001, 0x40000, CRC(97a317f4) SHA1(19bc4cf6b6c580caa44f36c929b445ed94b2d9eb) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "epr-14429.ic104", 0x00000, 0x20000, CRC(1ff8262d) SHA1(fb90bd877b2dc65eb3e5495d6e21dee1f871fb44) )

	// ROM_REGION( 0x8000, "m1comm", 0 ) // not present?
	
	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "315-5472-01.ic22", 0x000, 0x0eb, CRC(828ee6e2) SHA1(f32dd0f6297cc8bd3049be4bca502c0f8ec738cf) )
	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

GAME( 199?, bingpty,    0,        segam1,    segam1, driver_device,    0, ROT0,  "Sega", "Bingo Party Multicart (Rev B) (M1 Satellite board)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1992, unkm1,      0,        segam1,    segam1, driver_device,    0, ROT0,  "Sega", "Unknown Sega gambling game (M1 Satellite board)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
