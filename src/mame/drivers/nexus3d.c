/************************************************************************

	NEXUS 3D Version 1.0 Board from Interpark

	Games on this platform:
	
	Arcana Heart FULL, Examu Inc, 2006

	MagicEyes VRENDER 3D Soc (200 MHz ARM920T CPU / GFX / Sound)
	Also Has 2x QDSP QS1000 for sound

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
//#include "machine/i2cmem.h"

class nexus3d_state : public driver_device
{
public:
	nexus3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( nexus3d_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000fff) AM_ROM AM_REGION("user1", 0) 
ADDRESS_MAP_END

static INPUT_PORTS_START( nexus3d )

INPUT_PORTS_END


VIDEO_START( nexus3d )
{

}

SCREEN_UPDATE(nexus3d)
{
	return 0;
}

static MACHINE_CONFIG_START( nexus3d, nexus3d_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM9, 200000000)
	MCFG_CPU_PROGRAM_MAP(nexus3d_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
	MCFG_SCREEN_UPDATE(nexus3d)

	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(nexus3d)
MACHINE_CONFIG_END


// The u1 flash on this is clearly recycled from a Happy Fish or Blue Elf multigame.
// Around 75% of the rom is NeoGeo, CPS2, Semicom etc. MAME romsets used by said multigame bootlegs
// which explains why the 1Gb flash rom hardly compresses, it's already compressed data.
//
// I highly suspect this upgrade (to Full) was done at the PCB shop to boost the value of the PCB, and
// that the original game used a smaller flash.  It seems highly unlikely that Examu would ship ROMs
// containing the entire backcatalog of SNK and Capcom material ;-)
// 
// It's possible this set should be marked as a bootleg due to this although I imagine the acutal valid
// part of the data will match a clean dump.
ROM_START( acheartf )
	ROM_REGION( 0x42000898, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcana_heart_full.u1",     0x000000, 0x42000898, CRC(1a171ca3) SHA1(774f3b8d5fb366901d819b5dc15ca49b0cd177b9) )

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//	ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END

static DRIVER_INIT( nexus3d )
{

}

GAME( 2006, acheartf, 0, nexus3d, nexus3d, nexus3d, ROT0, "Examu", "Arcana Heart Full", GAME_NO_SOUND|GAME_NOT_WORKING )
