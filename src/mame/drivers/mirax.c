/*
****************************************************
Mirax (C)1985 Current Technologies

skeleton driver by Tomasz Slanina analog[AT]op[DOT]pl


CPU: Ceramic potted module, Z80C
Sound: AY-3-8912 (x2)
RAM: 74S201, 74S201 (x6), 2148 (x6), 2114 (x2), 58725 (x2), 6116
PROMS: 82S123 (x2)
XTAL: 12 MHz

Here comes a high energy space game of its own kind.

Mirax(TM)
* First person perspective
* Fully integrated game play.
* Continually changing 3-D graphics plus powerful sound effects.

HOW TO PLAY
1. Your goal is to terminate Mirax City - a giant enemy central station shown at stage 10.
2. Destroy all enemy objects (air or ground) as many as you can. Avoid indestructible building blocks.
3. Shooting a group of ground targets causes satellites to rise.
4. Enemy flagship appears after a wave of far ground vessels completely destroyed.
5. Hitting flagship puts you into power shooting mode - no fire button required.
6. Bonus flag is given for each flagship hit. 5 or 8 flags award you an extra fighter.
7. Use joystick to take sight before firing.


Pinouts

Parts          Solder
1 Gnd          Gnd
2 Gnd          Gnd
3 Gnd          Video gnd
4 Gnd          Gnd
5
6
7
8
9 Coin 1       Video sync
10 Up
11 Fire
12 Down
13 start1      start2
14 left
15 right
16 Video green
17 Video blue
18 Video red
19 Speaker-    Speaker+
20 +5          +5
21 +5          +5
22 +12         +12


The End

************************************************
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/ay8910.h"

static UINT8 nSndNum;
static UINT8 nAyCtrl, nAyData;

static VIDEO_START(mirax)
{
}


static VIDEO_UPDATE(mirax)
{
#ifdef MAME_DEBUG
	//audio tester
	if(input_code_pressed_once(KEYCODE_Q))
	{
		cputag_set_input_line(screen->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
	}
#endif
	return 0;
}


static SOUND_START(mirax)
{
	nSndNum = 0x10;
	nAyCtrl = 0x00;
	nAyData = 0x00;
}

static READ8_HANDLER(snd_read)
{
	/* bit 6 is used to select AY (1st or 2nd) */

	return nSndNum++;
}

static WRITE8_HANDLER(audio_w)
{
	if(cpu_get_previouspc(space->cpu)==0x2fd)
	{
		nAyCtrl=offset;
		nAyData=data;
	}
}


static WRITE8_DEVICE_HANDLER(ay_sel)
{
	if(cpu_get_previouspc(cputag_get_cpu(device->machine, "maincpu"))==0x309)
	{
		ay8910_address_w(device,0,nAyCtrl);
		ay8910_data_w(device,0,nAyData);
	}
}

static ADDRESS_MAP_START( memory_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(snd_read)

	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("ay1", ay_sel) //1st ay ?
	AM_RANGE(0xe001, 0xe001) AM_WRITENOP
	AM_RANGE(0xe003, 0xe003) AM_WRITENOP

	AM_RANGE(0xe400, 0xe400) AM_DEVWRITE("ay2", ay_sel) //2nd ay ?
	AM_RANGE(0xe401, 0xe401) AM_WRITENOP
	AM_RANGE(0xe403, 0xe403) AM_WRITENOP

	AM_RANGE(0xf900, 0xf9ff) AM_WRITE(audio_w)

ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )

ADDRESS_MAP_END

static const gfx_layout layout16 =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3)},
	{ 0, 1, 2, 3, 4, 5, 6, 7 ,
	  0+8*8,1+8*8,2+8*8,3+8*8,4+8*8,5+8*8,6+8*8,7+8*8},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	0*8+8*8*2, 1*8+8*8*2, 2*8+8*8*2, 3*8+8*8*2, 4*8+8*8*2, 5*8+8*8*2, 6*8+8*8*2, 7*8+8*8*2},
	16*16
};

static const gfx_layout layout8 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3)},
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8
};

static GFXDECODE_START( mirax )
	GFXDECODE_ENTRY( "gfx1", 0, layout8,     0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, layout16,    0, 8 )
	GFXDECODE_ENTRY( "gfx3", 0, layout16,    0, 8 )
GFXDECODE_END

static INPUT_PORTS_START( mirax )

INPUT_PORTS_END


PALETTE_INIT( mirax )
{
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static MACHINE_DRIVER_START( mirax )
	MDRV_CPU_ADD("maincpu", Z80, 12000000) // audio cpu ?
	MDRV_CPU_PROGRAM_MAP(memory_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold, 2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x40)
	MDRV_PALETTE_INIT(mirax)
	MDRV_GFXDECODE(mirax)
	MDRV_VIDEO_START(mirax)
	MDRV_VIDEO_UPDATE(mirax)

	MDRV_SOUND_START(mirax)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay1", AY8910, 12000000/6 /* guess */)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MDRV_SOUND_ADD("ay2", AY8910, 12000000/6 /* guess */)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

MACHINE_DRIVER_END

ROM_START( mirax )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mxr2-4v.rom",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mxe3-4v.rom",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "mxh3-4v.rom",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "mxk3-4v.rom",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0xc000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "mxe2-4v.rom",   0x0000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "mxf2-4v.rom",   0x4000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "mxh2-4v.rom",   0x8000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )

	ROM_REGION( 0xc000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "mxf3-4v.rom",   0x0000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "mxi3-4v.rom",   0x4000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "mxl3-4v.rom",   0x8000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0xc000, "user1", 0 )
	ROM_LOAD( "mxp5-42.rom",   0x0000, 0x4000, CRC(716410a0) SHA1(55171376e1e164b1d5e728789da6e04a3a33c172) )
	ROM_LOAD( "mxr5-4v.rom",   0x4000, 0x4000, CRC(c9484fc3) SHA1(101c5e4b9d49d2424ad80970eb3bdb87949a9966) )
	ROM_LOAD( "mxs5-4v.rom",   0x8000, 0x4000, CRC(e0085f91) SHA1(cf143b94048e1ebb5c899b94b500e193dfd42e18) )

	ROM_REGION( 0x0060, "proms", 0 ) // data ? encrypted roms for cpu1 ?
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
	ROM_LOAD( "mirax.prm",	0x0040, 0x0020, NO_DUMP )

ROM_END

GAME( 1985, mirax,  0,		mirax, mirax, 0, ROT90, "Current Technologies", "Mirax", GAME_NOT_WORKING)
