// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* Bounty Hunter

 PC hardware.. no dumps of the bios roms are currently available

*/


#include "emu.h"
#include "cpu/i386/i386.h"


class bntyhunt_state : public driver_device
{
public:
	bntyhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
};


void bntyhunt_state::video_start()
{
}

UINT32 bntyhunt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( bntyhunt_map, AS_PROGRAM, 32, bntyhunt_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( bntyhunt )
INPUT_PORTS_END


static MACHINE_CONFIG_START( bntyhunt, bntyhunt_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 200000000) /* Probably a Pentium or higher .. ?? Mhz*/
	MCFG_CPU_PROGRAM_MAP(bntyhunt_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(bntyhunt_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
MACHINE_CONFIG_END


ROM_START(bntyhunt)
	ROM_REGION32_LE(0x20000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("bntyhunt.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "bntyhunt", 0, SHA1(e50937d14d5c6adfb5e0012db5a7df090eebc2e1) )
ROM_END


GAME( 200?, bntyhunt,  0,   bntyhunt,  bntyhunt, driver_device,  0,  ROT0,  "GCTech Co., LTD",    "Bounty Hunter (GCTech Co., LTD)",   MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
