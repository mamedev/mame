// license:???
// copyright-holders:Scott Stone
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
#include "machine/serflash.h"

//#include "machine/i2cmem.h"



class nexus3d_state : public driver_device
{
public:
	nexus3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu"),
		m_serflash(*this, "flash")
	{ }

	required_shared_ptr<UINT32> m_mainram;
	required_device<cpu_device> m_maincpu;
	required_device<serflash_device> m_serflash;

	DECLARE_READ32_MEMBER(nexus3d_unk_r);
//  DECLARE_READ32_MEMBER(nexus3d_unk2_r);
//  DECLARE_READ32_MEMBER(nexus3d_unk3_r);
//  DECLARE_WRITE32_MEMBER(nexus3d_unk2_w);
//  DECLARE_WRITE32_MEMBER(nexus3d_unk3_w);

	DECLARE_DRIVER_INIT(nexus3d);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_nexus3d(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};






READ32_MEMBER(nexus3d_state::nexus3d_unk_r)
{
	return machine().rand() ^ (machine().rand() << 16);
}

//READ32_MEMBER(nexus3d_state::nexus3d_unk2_r)
//{
//  return 0x00000000;//machine().rand() ^ (machine().rand() << 16);
//}
//
//READ32_MEMBER(nexus3d_state::nexus3d_unk3_r)
//{
//  return 0x00000000;//machine().rand() ^ (machine().rand() << 16);
//}
//
//WRITE32_MEMBER(nexus3d_state::nexus3d_unk2_w)
//{
//
//}
//
//WRITE32_MEMBER(nexus3d_state::nexus3d_unk3_w)
//{
//
//}

static ADDRESS_MAP_START( nexus3d_map, AS_PROGRAM, 32, nexus3d_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0x00400000, 0x01ffffff) AM_RAM // ?? uploads various data, + pointers to data in the 0x01ffxxxx range, might be video system related

	// flash
	AM_RANGE(0x9C000000, 0x9C000003) AM_DEVREAD8( "flash", serflash_device, n3d_flash_r, 0xffffffff)
	AM_RANGE(0x9C000010, 0x9C000013) AM_DEVWRITE8("flash", serflash_device, n3d_flash_cmd_w, 0xffffffff)
	AM_RANGE(0x9C000018, 0x9C00001b) AM_DEVWRITE8("flash", serflash_device, n3d_flash_addr_w, 0xffffffff)

	// lots of accesses in this range
//  AM_RANGE(0xC0000F44, 0xC0000F47) AM_READWRITE(nexus3d_unk2_r, nexus3d_unk2_w ) // often
//  AM_RANGE(0xC0000F4C, 0xC0000F4f) AM_READWRITE(nexus3d_unk3_r, nexus3d_unk3_w ) // often

	AM_RANGE(0xE0000014, 0xE0000017) AM_READ(nexus3d_unk_r ) // sits waiting for this


ADDRESS_MAP_END

static INPUT_PORTS_START( nexus3d )

INPUT_PORTS_END


void nexus3d_state::video_start()
{
}

UINT32 nexus3d_state::screen_update_nexus3d(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void nexus3d_state::machine_reset()
{
}

static MACHINE_CONFIG_START( nexus3d, nexus3d_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM920T, 200000000)
	MCFG_CPU_PROGRAM_MAP(nexus3d_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(nexus3d_state, screen_update_nexus3d)

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_SERFLASH_ADD("flash")

MACHINE_CONFIG_END



ROM_START( acheart )
	ROM_REGION( 0x10800898, "flash", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheart.u1",     0x000000, 0x10800898, CRC(109bf439) SHA1(33fd39355923ef384d5eaeec8ae3f296509bde93) )

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//  ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END


ROM_START( acheartf )
	ROM_REGION( 0x10800898, "flash", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheartfull.u1",     0x000000, 0x10800898, CRC(54b57a9d) SHA1(dee5a43b3aea854d2b98869dca74c57b66fb06eb))

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//  ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END

DRIVER_INIT_MEMBER(nexus3d_state,nexus3d)
{
	// the first part of the flash ROM automatically gets copied to RAM
	memcpy( m_mainram, memregion("flash")->base(), 4 * 1024);
}

GAME( 2005, acheart,  0, nexus3d, nexus3d, nexus3d_state, nexus3d, ROT0, "Examu", "Arcana Heart",MACHINE_IS_SKELETON )
GAME( 2006, acheartf, 0, nexus3d, nexus3d, nexus3d_state, nexus3d, ROT0, "Examu", "Arcana Heart Full",MACHINE_IS_SKELETON )
