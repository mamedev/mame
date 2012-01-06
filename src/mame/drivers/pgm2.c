

/* PGM 2 hardware.

 Motherboard is bare bones stuff, probably only contains the video processor, no ROMs.

 Makes use of internal ROM ASICS, newer than those found on the best protected PGM games.  Games actually boot to a warning screen even if you remove all program roms!

 Encrypted.

 I'm guessing it's ARM based, but not sure.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

};

static ADDRESS_MAP_START( pgm2_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( pgm2 )
INPUT_PORTS_END

static SCREEN_UPDATE(pgm2)
{
	return 0;
}

static SCREEN_EOF(pgm2)
{

}

static VIDEO_START(pgm2)
{

}

static MACHINE_START( pgm2 )
{

}

static MACHINE_RESET( pgm2 )
{

}

static MACHINE_CONFIG_START( pgm2, pgm2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 20000000)	// ??
	MCFG_CPU_PROGRAM_MAP(pgm2_map)
	MCFG_DEVICE_DISABLE()


	MCFG_MACHINE_START( pgm2 )
	MCFG_MACHINE_RESET( pgm2 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE(pgm2)
	MCFG_SCREEN_EOF(pgm2)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(pgm2)
MACHINE_CONFIG_END

ROM_START( orleg2 )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "xyj2_v104cn.u7",  0x000000, 0x800000, CRC(7c24a4f5) SHA1(3cd9f9264ef2aad0869afdf096e88eb8d74b2570) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_REGION( 0x1c00000, "sprcol", ROMREGION_ERASEFF ) /* Sprite Colour Data */
	ROM_REGION( 0x1000000, "sprmask", ROMREGION_ERASEFF ) /* Sprite Masks + Colour Indexes */
	ROM_REGION( 0x1000000, "ics", ROMREGION_ERASEFF ) /* Samples - (8 bit mono 11025Hz) - */

	ROM_REGION( 0x2000000, "others", 0 )
	ROM_LOAD16_WORD_SWAP( "ig-a.u26",  0x000000, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u35",  0x000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u36",  0x000000, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u2",   0x000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u4",   0x000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u12",  0x000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u16",  0x000000, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )
	ROM_LOAD16_WORD_SWAP( "ig-a.u18",  0x000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
ROM_END

/* PGM2 */
GAME( 2007, orleg2,       0,         pgm2,    pgm2,     0,       ROT0, "IGS", "Oriental Legend 2", GAME_IS_SKELETON )
