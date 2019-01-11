// license:BSD-3-Clause
// copyright-holders:David Haywood

/* San Remo / Elsy Multigame? */

// presumably a gambling game, maybe missing a sub-board?
// http://www.citylan.it/wiki/index.php/Unknown_San_Remo_/_Elsy_Multigame
// M30624FG (M16C/62A family) based, needs CPU core and dumping of internal ROM


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "screen.h"


class sanremmg_state : public driver_device
{
public:
	sanremmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sanremmg(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sanremmg_map(address_map &map);
};


void sanremmg_state::video_start()
{
}

uint32_t sanremmg_state::screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



void sanremmg_state::sanremmg_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom();
}


static INPUT_PORTS_START( sanremmg )
INPUT_PORTS_END

MACHINE_CONFIG_START(sanremmg_state::sanremmg)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", ARM7, 50000000) // wrong, this is an M30624FG (M16C/62A family) with 256K internal ROM, no CPU core available
	MCFG_DEVICE_PROGRAM_MAP(sanremmg_map)

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "sanremmg_m30624fg.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION(0x400000, "data", 0 ) // start of 1.bin has 'Tue Sep 03 11:37:03' Not sure if 03 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "1.bin",   0x000000, 0x200000, CRC(67fa5e76) SHA1(92beb90e1b370763966017d47cb748106014d371) ) // HY29LV160BT
	ROM_LOAD( "2.bin",   0x200000, 0x200000, CRC(61f69735) SHA1(ff46362ce6fe239089c85e698add1b8090bb39bb) )
	// there is space for what looks like a 3rd rom
ROM_END

ROM_START( elsypokr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "elsypokr_m30624fg.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION(0x400000, "data", 0 ) // start of first ROM has 'Mon Apr 05 10:25:38'. Not sure if 05 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "mx29lv160bb.1.bin",   0x000000, 0x200000, CRC(da620fa6) SHA1(f2eea0146f6ddcaa4049f6fe7797d755faeace88) ) // MX29LV160BBTC-70
	ROM_LOAD( "mx29lv160bb.2.bin",   0x200000, 0x200000, CRC(7a0c3e38) SHA1(dd98d6f56272bf3cc0ed1a14234a8c6e0bc4dd37) )
	// there is space for what looks like a 3rd rom
ROM_END

GAME( 2003, sanremmg, 0,        sanremmg,  sanremmg, sanremmg_state, empty_init,  ROT0, "San Remo Games", "unknown San Remo / Elsy Multigame", MACHINE_IS_SKELETON )
GAME( 200?, elsypokr, 0,        sanremmg,  sanremmg, sanremmg_state, empty_init,  ROT0, "Electro System (Elsy)", "unknown Elsy poker", MACHINE_IS_SKELETON )
