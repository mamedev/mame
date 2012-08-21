/***************************************************************************

    Commodore Amiga 3000

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class a3000_state : public driver_device
{
public:
	a3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}
};



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define XTAL_U104	XTAL_32MHz


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( a3000_mem, AS_PROGRAM, 32, a3000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK("chipram")
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( a3000 )
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( a3000, a3000_state )
	MCFG_CPU_ADD("maincpu", M68030, XTAL_U104 / 2)
	MCFG_CPU_PROGRAM_MAP(a3000_mem)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(a3000_state, screen_update)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512*2, 312)
	MCFG_SCREEN_VISIBLE_AREA((129-8-8)*2, (449+8-1+8)*2, 44-8, 300+8-1)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

/* Note: I think those ROMs are correct, but they should be verified */
ROM_START( a3000 )
	ROM_REGION32_BE(0x80000, "kickstart", 0)
	ROM_DEFAULT_BIOS("kick14")
	ROM_SYSTEM_BIOS(0, "kick14", "Kickstart 1.4 (36.16)")
	/* COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 0 CS=9713 */
	ROMX_LOAD("390629-02.u182", 0x00000, 0x40000, CRC(58327536) SHA1(d1713d7f31474a5948e6d488e33686061cf3d1e2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	/* COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 1 CS=9B21 */
	ROMX_LOAD("390630-02.u183", 0x00002, 0x40000, CRC(fe2f7fb9) SHA1(c05c9c52d014c66f9019152b3f2a2adc2c678794), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390629-03.u182", 0x00000, 0x40000, CRC(a245dbdf) SHA1(83bab8e95d378b55b0c6ae6561385a96f638598f), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("390630-03.u183", 0x00002, 0x40000, CRC(7db1332b) SHA1(48f14b31279da6757848df6feb5318818f8f576c), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "kick31", "Kickstart 3.1 (40.68)")
	ROMX_LOAD("kick31.u182",    0x00000, 0x40000, CRC(286b9a0d) SHA1(6763a2258ec493f7408cf663110dae9a17803ad1), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROMX_LOAD("kick31.u183",    0x00002, 0x40000, CRC(0b8cde6a) SHA1(5f02e97b48ebbba87d516a56b0400c6fc3434d8d), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  INIT  COMPANY      FULLNAME      FLAGS */
COMP( 1990, a3000, 0,      0,      a3000,   a3000, driver_device, 0,    "Commodore Business Machines", "Amiga 3000", GAME_NOT_WORKING | GAME_NO_SOUND )
