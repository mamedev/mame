/*
   Mahjong Shiyou driver

   placeholder driver, roms 1.1k & 2.1g might be encrypted, looks like an address based xor on
   a couple of bits at a time, probably not too hard to decrypt

*/


/*

Mahjong Shiyou (BET type)
(c)1986 Visco

Board:  S-0086-001-00
CPU:    Z80-A x2
Sound:  AY-3-8910
        M5205
OSC:    18.432MHz
        400KHz


1.1K       Z80#2 prg.
2.1G

3.3G       Z80#1 prg.
4.3F

COLOR.BPR  color

*/

#include "driver.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"


static VIDEO_START(mjsiyoub)
{
}

static VIDEO_UPDATE(mjsiyoub)
{
	return 0;
}

static ADDRESS_MAP_START( readmem1, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem1, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( readmem2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END

static INPUT_PORTS_START( mjsiyoub )
INPUT_PORTS_END


static const struct AY8910interface ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	NULL,				/* Input A: DSW 2 */
	NULL,				/* Input B: DSW 1 */
	NULL,
	NULL
};

static const struct MSM5205interface msm5205_interface =
{
	0,							/* IRQ handler */
	MSM5205_S48_4B				/* 8 KHz, 4 Bits  ?? */
};


static MACHINE_DRIVER_START( mjsiyoub )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80,18432000/4)
	MDRV_CPU_PROGRAM_MAP(readmem1,writemem1)
//  MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("sub", Z80,18432000/4)
	MDRV_CPU_PROGRAM_MAP(readmem2,writemem2)
//  MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(mjsiyoub)
	MDRV_VIDEO_UPDATE(mjsiyoub)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 18432000/16) // ??
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("msm", MSM5205, 18432000/32) // ??
	MDRV_SOUND_CONFIG(msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END


ROM_START( mjsiyoub )
	ROM_REGION( 0x10000, RGNCLASS_CPU, "main", 0 )
	ROM_LOAD( "1.1k", 0x00000, 0x8000, CRC(a1083321) SHA1(b36772e90be60270234df16cf92d87f8d950190d) )
	ROM_LOAD( "2.1g", 0x08000, 0x4000, CRC(cfe5de1d) SHA1(4acf9a752aa3c02b0889b0b49d3744359fa24460) )

	ROM_REGION( 0x10000, RGNCLASS_CPU, "sub", 0 )
	ROM_LOAD( "3.3g", 0x00000, 0x8000, CRC(47d0f16e) SHA1(a125be052668ba93756bf940af31a10e91a3d307) )
	ROM_LOAD( "4.3f", 0x08000, 0x8000, CRC(6cd6a200) SHA1(1c53e5caacdb9c660bd98f5331bf5354581f74c9) )

	ROM_REGION( 0x40000, RGNCLASS_PROMS, "proms", 0 )
	ROM_LOAD( "color.bpr", 0x00, 0x20,  CRC(d21367e5) SHA1(b28321ac8f99abfebe2ef4da0c751cefe9f3f3b6) )
ROM_END

GAME( 1986, mjsiyoub,  0,    mjsiyoub, mjsiyoub, 0, ROT0, "Visco", "Mahjong Shiyou", GAME_NOT_WORKING )
