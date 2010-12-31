/*

    MPU5

    Skeleton Driver

     -- there are a wide range of titles running on this hardware, the recent ones are said to be encrypted
     -- the driver does nothing, and currently only serves to act as a placeholder to document what existed on this hardware

     -- the main CPU is a 68340, which is a 32-bit 680xx variant with modified opcodes etc.

     -- Much of the communication is done via a 68681 DUART.
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"


static ADDRESS_MAP_START( mpu5_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  mpu5 )
INPUT_PORTS_END

static VIDEO_START(mpu5)
{

}

static VIDEO_UPDATE(mpu5)
{
	return 0;
}

static MACHINE_CONFIG_START( mpu5, driver_device )
	MCFG_CPU_ADD("maincpu", M68EC020, 16000000)	 // ?
	MCFG_CPU_PROGRAM_MAP(mpu5_map)

	/* actually non-video? */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(mpu5)
	MCFG_VIDEO_UPDATE(mpu5)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END

ROM_START( m_honmon )
	ROM_REGION( 0x300000, "maincpu", 0 ) /* Code */
	ROM_LOAD16_BYTE( "hmo_23s.p1", 0x000000, 0x80000, CRC(b3a44b47) SHA1(f54399bb1cc01fd4d615bd2c1a539c132b99d811) )
	ROM_LOAD16_BYTE( "hmo_23l.p2", 0x000001, 0x80000, CRC(09e116f7) SHA1(0d94b957f4bb3ef6aa10511c69e51d5400698622) )
	ROM_LOAD16_BYTE( "hmo_23l.p3", 0x100000, 0x80000, CRC(072b4af0) SHA1(799280cd27e53e167f28c5ad71868e8cd29b0200) )
	ROM_LOAD16_BYTE( "hmo_23l.p4", 0x100001, 0x80000, CRC(8443e257) SHA1(55c9dda40368914481b40b7224081c67758c5717) )
	ROM_LOAD16_BYTE( "hmo_23l.p5", 0x200000, 0x80000, CRC(e1cdf0f4) SHA1(0a1d09fefced246ce0c7692171c56f45050c49fd) )
	ROM_LOAD16_BYTE( "hmo_23l.p6", 0x200001, 0x80000, CRC(32496a48) SHA1(7c29020c1b35dd078e0807b86f67ec41432d84a1) )
ROM_END


GAME( 199?, m_honmon,    0,         mpu5,     mpu5,    0, ROT0,  "Vivid", "Honey Money", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
