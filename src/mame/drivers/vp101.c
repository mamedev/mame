/***************************************************************************

    Play Mechanix / Right Hand Tech "VP101" platform
    (PCB also marked "Raw Thrills" but all RT games appear to be on PC hardware)

    Skeleton driver by R. Belmont

    MIPS VR5500 at 300 to 400 MHz
    Xilinx Virtex-II FPGA with custom 3D hardware and 1 or 2 PowerPC 405 CPU cores
    AC97 audio with custom DMA frontend which streams 8 stereo channels
    PIC18c442 protection chip (not readable)

****************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/idectrl.h"

static VIDEO_UPDATE( vp101 )
{
	return 0;
}

static VIDEO_START( vp101 )
{
}

static READ32_HANDLER(tty_ready_r)
{
	return 0x60;	// must return &0x20 for output at tty_w to continue
}

static WRITE32_HANDLER(tty_w)	// set breakpoint at bfc01430 to catch when it's printing things
{
// uncomment to see startup messages - it says "RAM OK" and "EPI RSS Ver 4.5.1" followed by "<RSS active>" and then lots of dots
//  printf("%c", data);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM				// this is a sufficient amount to get "RAM OK"
	AM_RANGE(0x1c000000, 0x1c000003) AM_WRITE(tty_w)		// RSS OS code uses this one
	AM_RANGE(0x1c000014, 0x1c000017) AM_READ(tty_ready_r)
	AM_RANGE(0x1c400000, 0x1c400003) AM_WRITE(tty_w)		// boot ROM code uses this one
	AM_RANGE(0x1c400014, 0x1c400017) AM_READ(tty_ready_r)
	AM_RANGE(0x1fc00000, 0x1fcfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( vp101 )
INPUT_PORTS_END

static const mips3_config r5000_config =
{
	32768,				/* code cache size */
	32768,				/* data cache size */
	100000000			/* system (bus) clock */
};

static MACHINE_CONFIG_START( vp101, driver_device )
	MDRV_CPU_ADD("maincpu", R5000LE, 300000000)	/* actually VR5500 with added NEC VR-series custom instructions */
	MDRV_CPU_CONFIG(r5000_config)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_VIDEO_START(vp101)
	MDRV_VIDEO_UPDATE(vp101)
MACHINE_CONFIG_END

ROM_START(jnero)
	ROM_REGION(0x100000, "maincpu", 0)	/* Boot ROM */
	ROM_LOAD( "d710.05523.bin", 0x000000, 0x100000, CRC(6054a066) SHA1(58e68b7d86e6f24c79b99c8406e86e3c14387726) )

	ROM_REGION(0x80000, "pic", 0)		/* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8722a-1206.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE_READONLY("jn010108", 0, SHA1(4f3e9c6349c9be59213df1236dba7d79e7cd704e) )
ROM_END

GAME( 2004, jnero, 0, vp101, vp101, 0, ROT0, "ICE/Play Mechanix", "Johnny Nero Action Hero", GAME_NOT_WORKING|GAME_NO_SOUND )

