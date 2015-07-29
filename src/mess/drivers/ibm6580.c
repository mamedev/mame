// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

IBM 6580 DisplayWriter.

2013-08-19 Skeleton driver.

A green-screen dedicated word-processing workstation. It uses 20cm floppy
disks. It could have up to 224k of ram.

ToDo:
- Everything!
- The roms need to be loaded properly.
- Chargen rom is not dumped.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"

class ibm6580_state : public driver_device
{
public:
	ibm6580_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	const UINT8 *m_p_chargen;
	DECLARE_PALETTE_INIT(ibm6580);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(ibm6580_mem, AS_PROGRAM, 16, ibm6580_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xfdfff) AM_RAM
	AM_RANGE(0xfe000,0xfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibm6580_io, AS_IO, 16, ibm6580_state)
	//ADDRESS_MAP_UNMAP_HIGH
	//ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ibm6580 )
INPUT_PORTS_END

UINT32 ibm6580_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

PALETTE_INIT_MEMBER( ibm6580_state, ibm6580 )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

void ibm6580_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
}

static MACHINE_CONFIG_START( ibm6580, ibm6580_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 4000000) // no idea
	MCFG_CPU_PROGRAM_MAP(ibm6580_mem)
	MCFG_CPU_IO_MAP(ibm6580_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(ibm6580_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(ibm6580_state, ibm6580)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ibm6580 )
	ROM_REGION16_LE( 0x2000, "user1", 0 )
	ROM_LOAD16_BYTE("8493823.bin", 0x0001, 0x1000, CRC(0bea066f) SHA1(8a42e24b609df7d9ca9cd52929702a61f7024635))
	ROM_LOAD16_BYTE("8493822.bin", 0x0000, 0x1000, CRC(6e67f41a) SHA1(600fee505efe5cbcc8bdbab91d233378c7be4f12))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "8493383.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE   INPUT    CLASS          INIT  COMPANY            FULLNAME       FLAGS */
COMP( 1980, ibm6580, 0,      0,       ibm6580,  ibm6580, driver_device,  0,  "IBM", "IBM 6580 DisplayWriter", MACHINE_IS_SKELETON)
