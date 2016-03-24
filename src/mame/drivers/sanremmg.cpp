// license:BSD-3-Clause
// copyright-holders:David Haywood

/* San Remo / Elsy Multigame? */

// presumably a gambling game, maybe missing a sub-board?
// http://www.citylan.it/wiki/index.php/Multigame_ID


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"


class sanremmg_state : public driver_device
{
public:
	sanremmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void video_start() override;
	UINT32 screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

};


void sanremmg_state::video_start()
{
}

UINT32 sanremmg_state::screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( sanremmg_map, AS_PROGRAM, 32, sanremmg_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_ROM

ADDRESS_MAP_END


static INPUT_PORTS_START( sanremmg )
INPUT_PORTS_END


static MACHINE_CONFIG_START( sanremmg, sanremmg_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 50000000) // ??? doesn't seem to be ARM, but what is it?
	MCFG_CPU_PROGRAM_MAP(sanremmg_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sanremmg_state, screen_update_sanremmg)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
MACHINE_CONFIG_END

ROM_START( sanremmg )
	ROM_REGION(0x400000, "maincpu", 0 ) // start of 1.bin has 'Tue Sep 03 11:37:03' Not sure if 03 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "1.bin",   0x000000, 0x200000, CRC(67fa5e76) SHA1(92beb90e1b370763966017d47cb748106014d371) ) // HY29LV160BT
	ROM_LOAD( "2.bin",   0x200000, 0x200000, CRC(61f69735) SHA1(ff46362ce6fe239089c85e698add1b8090bb39bb) )
	// there is space for what looks like a 3rd rom
ROM_END


GAME( 2003, sanremmg, 0,        sanremmg,  sanremmg, driver_device,  0,  ROT0, "San Remo Games", "unknown San Remo / Elsy Multigame", MACHINE_NO_SOUND | MACHINE_IS_SKELETON )
