/***************************************************************************

    Midway Quicksilver skeleton driver

    Main CPU : Intel Celeron 333/366MHz
    Motherboard : Intel SE44BX-2
    RAM : 64MB
    Graphics Chips : Quantum Obsidian 3DFX
    Storage : Hard Drive
***************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"

static VIDEO_START(midqslvr)
{
}

static VIDEO_UPDATE(midqslvr)
{
	return 0;
}

static ADDRESS_MAP_START(midqslvr_map, ADDRESS_SPACE_PROGRAM, 32)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(midqslvr_io, ADDRESS_SPACE_IO, 32)
ADDRESS_MAP_END

static INPUT_PORTS_START( midqslvr )
INPUT_PORTS_END

static MACHINE_CONFIG_START( midqslvr, driver_device )
	MDRV_CPU_ADD("maincpu", PENTIUM, 333000000)	// actually Celeron 333
	MDRV_CPU_PROGRAM_MAP(midqslvr_map)
	MDRV_CPU_IO_MAP(midqslvr_io)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 199)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(midqslvr)
	MDRV_VIDEO_UPDATE(midqslvr)
MACHINE_CONFIG_END

ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "user1", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "offrthnd", 0, SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "user1", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "hydro", 0, SHA1(3e9956dd4a62f540ea2900f857611ff9f035b76a) )
ROM_END

GAME(1999, hydrthnd, 0, midqslvr, midqslvr, 0, ROT0, "Midway Games", "Hydro Thunder", GAME_NO_SOUND|GAME_NOT_WORKING)
GAME(2000, offrthnd, 0, midqslvr, midqslvr, 0, ROT0, "Midway Games", "Offroad Thunder", GAME_NO_SOUND|GAME_NOT_WORKING)
