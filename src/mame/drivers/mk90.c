// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Elektronika MK-90

        12/05/2009 Skeleton driver.


    http://www.pisi.com.pl/piotr433/index.htm#mk90
    http://www.taswegian.com/MOSCOW/mk-90.html

This is a Soviet computer-calculator, very similar in looks to the Sharp.
It has a LCD display. It cost about 1500 roubles, which is the wages for 6
months for an average citizen.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"


class mk90_state : public driver_device
{
public:
	mk90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_mk90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(mk90_mem, AS_PROGRAM, 16, mk90_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM // RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROM // Extension ROM
	AM_RANGE(0x8000, 0xffff) AM_ROM // Main ROM
//  AM_RANGE(0xe800, 0xe801) LCD address
//  AM_RANGE(0xe802, 0xe803) LCD data
//  AM_RANGE(0xe810, 0xe810) serial bus controller data
//  AM_RANGE(0xe812, 0xe813) serial bus controller transfer rate
//  AM_RANGE(0xe814, 0xe814) serial bus controller control/status
//  AM_RANGE(0xe816, 0xe816) serial bus controller command
//  AM_RANGE(0xea00, 0xea7e) RTC
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mk90 )
INPUT_PORTS_END


void mk90_state::machine_reset()
{
}

void mk90_state::video_start()
{
}

UINT32 mk90_state::screen_update_mk90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( mk90, mk90_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", K1801VM2, XTAL_4MHz)
	MCFG_T11_INITIAL_MODE(0x8000)
	MCFG_CPU_PROGRAM_MAP(mk90_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mk90_state, screen_update_mk90)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mk90 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "bas1", "Basic 1")
	ROMX_LOAD( "mk90ro10.bin",  0x8000, 0x8000, CRC(fac18038) SHA1(639f09a1be5f781f897603d0f799f7c6efd1b67f), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "bas2", "Basic 2")
	ROMX_LOAD( "mk90ro20.bin",  0x8000, 0x8000, CRC(d8b3a5f5) SHA1(8f7ab2d97c7466392b6354c0ea7017531c2133ae), ROM_BIOS(2))
	ROMX_LOAD( "mk90ro20t.bin", 0x4000, 0x4000, CRC(0f4b9434) SHA1(c74bbde6d201913c9e67ef8e2abe14b784187f8d), ROM_BIOS(2))  // Expansion ROM
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY     FULLNAME       FLAGS */
COMP( 1988, mk90,   0,      0,       mk90,      mk90, driver_device,    0,   "Elektronika", "MK-90", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
