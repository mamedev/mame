// license:BSD-3-Clause
// copyright-holders:Justin Kerk
/***************************************************************************

	Tektronix TekXpress XP33x series X terminals

	Skeleton driver.

****************************************************************************/

/*

	TODO:

	- everything

	Technical info:
	https://www.linux-mips.org/wiki/Tektronix_TekXPress_XP338
	https://web.archive.org/web/20061013084616/http://tekxp-linux.hopto.org/pmwiki/pmwiki.php/Hardware/XP33x
	http://bio.gsi.de/DOCS/NCD/xp.html

	CPU: IDT 79R3052E (MIPS I R3000 embedded core. Big Endian. With MMU, without FPU.)
	Graphic Chipset: Texas Instruments (34020AGBL-40) TI 34010 (TIGA)
	Graphic RAMDAC: Brooktree BT458LPJ135 (Supported in XFree86)
	RAM: 4MB onboard, upgradeable to 52MB, 3 slots for 5V FPM NoParity SIMMs
	ROM: 256Kb, 2 x 27c1024 (64Kx16), with BootMonitor
	Peripherals:
		1x Ethernet Port (AUI and UTP) -- AMD AM79C98 Chipset
		2x Serial Ports -- Philips SCC2692 Dual UART chipset
		Keyboard and Mouse controller is a generic PC-Style i8742 with Phoenix BIOS.
		1x "ERGO" Port (Combined Monitor, Keyboard, Mouse)
	16K (2K*8) EEPROM is SEEQ NQ2816A-250, AT28C16 compatible. Contains Boot Monitor settings. The content is compressed.
	Micron MT56C0816 - ??? Flash

*/


#include "emu.h"
#include "cpu/mips/r3000.h"
#include "cpu/tms34010/tms34010.h"

#define SCREEN_TAG "screen"

class tekxp330_state : public driver_device
{
public:
	tekxp330_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_start() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/* Memory Maps */

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, tekxp330_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tms_map, AS_PROGRAM, 16, tekxp330_state )
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tekxp330 )
INPUT_PORTS_END

/* Video */

void tekxp330_state::video_start()
{
}

UINT32 tekxp330_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Machine Initialization */

void tekxp330_state::machine_start()
{
}

/* Machine Driver */

static MACHINE_CONFIG_START( tekxp330, tekxp330_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R3052, XTAL_20MHz) /* IDT 79R3052E, clock unknown */
	MCFG_R3000_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_CPU_PROGRAM_MAP(cpu_map)

	MCFG_CPU_ADD("tms", TMS34010, XTAL_40MHz) /* clock unknown */
	MCFG_CPU_PROGRAM_MAP(tms_map)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(tekxp330_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_PALETTE_ADD("palette", 64)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( tekxp330 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	/* TekXpress XP300 Boot ROM V1.0  Thu Feb 6 14:50:16 PST 1992 */
	ROM_LOAD32_DWORD( "xp300.bin", 0x000000, 0x200000, CRC(9a324588) SHA1(a6e10275f8215f446be91128bab4c643693da653) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     CLASS          INIT    COMPANY      FULLNAME           FLAGS */
COMP( 1992, tekxp330,   0,          0,      tekxp330,   tekxp330, driver_device,   0,    "Tektronix", "TekXpress XP330", MACHINE_IS_SKELETON )
