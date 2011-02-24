/*
  ATV Track
  (c)2002 Gaelco

  2x SH4 (HD64177506), bus clock of 33 MHz
  There is a chip hidden under a heatsink with RAM connected, probably GFX
*/

#include "emu.h"
#include "cpu/sh4/sh4.h"

VIDEO_START(atvtrack)
{
}

SCREEN_UPDATE(atvtrack)
{
	return 0;
}

static ADDRESS_MAP_START( atvtrack_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( atvtrack_port, ADDRESS_SPACE_IO, 64 )
ADDRESS_MAP_END


static INPUT_PORTS_START( atvtrack )
INPUT_PORTS_END

// ?
#define ATV_CPU_CLOCK 200000000
// ?
static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, ATV_CPU_CLOCK };

static MACHINE_CONFIG_START( atvtrack, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_map)
	MCFG_CPU_IO_MAP(atvtrack_port)

	MCFG_CPU_ADD("subcpu", SH4, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_map)
	MCFG_CPU_IO_MAP(atvtrack_port)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(atvtrack)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(atvtrack)
MACHINE_CONFIG_END

ROM_START( atvtrack )
	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASEFF) // NAND roms, contain additional data hence the sizes
	ROM_LOAD32_BYTE("15.bin", 0x0000000, 0x1080000, CRC(84eaede7) SHA1(6e6230165c3bb35e49c660dfd0d07c132ed89e6a) )
	ROM_LOAD32_BYTE("20.bin", 0x0000001, 0x1080000, CRC(649dc331) SHA1(0cac2d0c15dd564c7fdebdf4365422958f453d63) )
	ROM_LOAD32_BYTE("14.bin", 0x0000002, 0x1080000, CRC(67983453) SHA1(05389a0ffc1a1bae9bac16a53a97d78b6eccc626) )
	ROM_LOAD32_BYTE("19.bin", 0x0000003, 0x1080000, CRC(9fc5c579) SHA1(8829329ef229564952aea2108ef1750dc226cbac) )
ROM_END

GAME( 2002, atvtrack, 0,   atvtrack,    atvtrack,    0, ROT0, "Gaelco", "ATV Track", GAME_NOT_WORKING | GAME_NO_SOUND )
