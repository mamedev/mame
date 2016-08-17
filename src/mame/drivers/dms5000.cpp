// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Digital Microsystems DMS-5000

        11/01/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"


class dms5000_state : public driver_device
{
public:
	dms5000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_dms5000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( dms5000_mem, AS_PROGRAM, 16, dms5000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dms5000_io, AS_IO, 16, dms5000_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dms5000 )
INPUT_PORTS_END


void dms5000_state::machine_reset()
{
}

void dms5000_state::video_start()
{
}

UINT32 dms5000_state::screen_update_dms5000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( dms5000, dms5000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(dms5000_mem)
	MCFG_CPU_IO_MAP(dms5000_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(dms5000_state, screen_update_dms5000)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dms5000 )
	ROM_REGION( 0x4000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "dms-5000_54-8673o.bin", 0x0001, 0x2000, CRC(dce9823e) SHA1(d36ab87d2e6f5e9f02d59a6a7724ad3ce2428a2f))
	ROM_LOAD16_BYTE( "dms-5000_54-8672e.bin", 0x0000, 0x2000, CRC(94d64c06) SHA1(be5a53da7bb29a5fa9ac31efe550d5d6ff8b77cd))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY             FULLNAME       FLAGS */
COMP( 1982, dms5000,  0,      0,       dms5000,   dms5000, driver_device, 0,  "Digital Microsystems", "DMS-5000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
