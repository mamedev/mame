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
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

static UINT8 *mjsiyoub_sub_rom;

static VIDEO_START(mjsiyoub)
{
}

static VIDEO_UPDATE(mjsiyoub)
{
	return 0;
}



static READ8_HANDLER( test_r )
{
	return 0;
}

static ADDRESS_MAP_START( main_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_BASE(&mjsiyoub_sub_rom)
	AM_RANGE(0x8000, 0xffff) AM_RAM //probably shared ram (all of it?)
ADDRESS_MAP_END

/*this doesn't appear to be just sound cpu,there are clearly inputs too and probably something else too.*/
static ADDRESS_MAP_START( sub_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(ay8910_read_port_0_r) //read mux
	AM_RANGE(0x02, 0x02) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x10, 0x10) AM_READ(test_r)
	AM_RANGE(0x11, 0x11) AM_READ(test_r)
	AM_RANGE(0x11, 0x11) AM_WRITENOP //xor-ed mux
	AM_RANGE(0x13, 0x13) AM_READ(test_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( mjsiyoub )
INPUT_PORTS_END


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	test_r,
	test_r,
	NULL,
	NULL
};

static const msm5205_interface msm5205_config =
{
	0,							/* IRQ handler */
	MSM5205_S48_4B				/* 8 KHz, 4 Bits  ?? */
};

static MACHINE_RESET( mjsiyoub )
{
//	cpu_set_input_line(machine->cpu[0], INPUT_LINE_HALT, ASSERT_LINE);
}


static PALETTE_INIT( mjsiyoub )
{
	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static MACHINE_DRIVER_START( mjsiyoub )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80,18432000/4)
	MDRV_CPU_PROGRAM_MAP(main_mem,0)
//  MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("sub", Z80,18432000/4)
	MDRV_CPU_PROGRAM_MAP(sub_mem,0)
	MDRV_CPU_IO_MAP(sub_io,0)
  	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_MACHINE_RESET( mjsiyoub )
	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MDRV_PALETTE_INIT(mjsiyoub)

	MDRV_PALETTE_LENGTH(0x20)

	MDRV_VIDEO_START(mjsiyoub)
	MDRV_VIDEO_UPDATE(mjsiyoub)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 18432000/16) // ??
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("msm", MSM5205, 18432000/32) // ??
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END


ROM_START( mjsiyoub )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "1.1k", 0x00000, 0x8000, CRC(a1083321) SHA1(b36772e90be60270234df16cf92d87f8d950190d) )
	ROM_LOAD( "2.1g", 0x08000, 0x4000, CRC(cfe5de1d) SHA1(4acf9a752aa3c02b0889b0b49d3744359fa24460) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "3.3g", 0x00000, 0x8000, CRC(47d0f16e) SHA1(a125be052668ba93756bf940af31a10e91a3d307) )
	ROM_LOAD( "4.3f", 0x08000, 0x8000, CRC(6cd6a200) SHA1(1c53e5caacdb9c660bd98f5331bf5354581f74c9) )

	ROM_REGION( 0x40000, "proms", 0 )
	ROM_LOAD( "color.bpr", 0x00, 0x20,  CRC(d21367e5) SHA1(b28321ac8f99abfebe2ef4da0c751cefe9f3f3b6) )
ROM_END

static DRIVER_INIT( mjsiyoub )
{
	UINT8 *ROM = memory_region(machine, "sub");

	memcpy(mjsiyoub_sub_rom, ROM, 0x8000);
}

GAME( 1986, mjsiyoub,  0,    mjsiyoub, mjsiyoub, mjsiyoub, ROT0, "Visco", "Mahjong Shiyou", GAME_NOT_WORKING )
