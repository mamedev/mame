/***************************************************************************

        DEC VT220

        30/06/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ram.h"


class vt220_state : public driver_device
{
public:
	vt220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};


static ADDRESS_MAP_START(vt220_mem, AS_PROGRAM, 8, vt220_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt220_io, AS_IO, 8, vt220_state)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vt220 )
INPUT_PORTS_END

void vt220_state::machine_reset()
{
	memset(m_ram->pointer(),0,16*1024);
}

void vt220_state::video_start()
{
}

UINT32 vt220_state::screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static MACHINE_CONFIG_START( vt220, vt220_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(vt220_mem)
	MCFG_CPU_IO_MAP(vt220_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(vt220_state, screen_update_vt220)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vt220 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-178e6.bin", 0x0000, 0x8000, CRC(cce5088c) SHA1(4638304729d1213658a96bb22c5211322b74d8fc))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                      FULLNAME       FLAGS */
COMP( 1983, vt220,  0,      0,       vt220,     vt220, driver_device,   0,  "Digital Equipment Corporation", "VT220", GAME_NOT_WORKING | GAME_NO_SOUND)
