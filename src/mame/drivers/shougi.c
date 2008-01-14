/***************************************************************************

Driver by Jarek Burczynski, started by Tomasz Slanina  dox@space.pl
ALPHA 8201 MCU handling by Tatsuyuki satoh
Lots of hardware info from Guru

memory map :
0000 - 3fff rom
4000 - 43ff RAM for main CPU
4800 - 480f  control area (connected to 74LS259 8-bit addressable latch; lines A0,A1,A2 - address input, line A3 - data input)
    4800,4808 - control shared RAM access between main CPU/sub CPU. 4800=access for sub CPU, 4808=access for main CPU
    4801,4809 - NMI disable/enable
    4802,480a -
    4803,480b -
    4804,480c -
    4807,480f -?

4800 - 4800 read: input
5000 - 5000 read: input
5800 - 5800 read: input

5800 - 5800 write: watchdog

6000 - AY port write
6800 - AY data write

7000 - 73ff RAM shared: main CPU/ALPHA MCU
7800 - 7bff RAM shared: between main CPU/sub CPU

8000 - bfff colorram
c000 - ffff videoram



TO DO:

------
Both games use custom MCU: ALPHA 8201 (42 pin DIP).
It's connected to the RAM that is shared with the first CPU.
CPU controls MCU (probably to run and stop it).

note: ALPHA 8302 is the MCU used in Exciting Soccer.

Shougi
Alpha Electronics Co. Ltd., 198x

PCB No: 57F1
CPU   : Z80A (x2)
SOUND : AY-3-8910
XTAL  : 10.000MHz
RAM   :
 2114 (x6), one RAM: 0x400 x 4bits = 0x400 bytes (x 3)
mapped at:
1: 0x4000 - 0x43ff (main CPU - stack also here, so it is work RAM)
2: 0x7000 - 0x73ff (main CPU - shared with ALPHA-8201)
3: 0x6000 - 0x63ff (sub CPU)


 4116 (x16), one RAM: 16K x 1bit  = 16K x 8bits *2 = 32K x 8bits
mapped at:
0x8000 - 0xffff



CUSTOM: ALPHA 8201 (42 pin DIP)
DIPSW : 6 position (x1)
       Positions 1, 5 & 6 not used

    4   3   2
       ------------------------------
       OFF  OFF OFF 1 minutes (time for the opponent to make his decision)
       OFF  OFF ON  2
       OFF  ON  OFF 3
       OFF  ON  ON  4
       ON   OFF OFF 5
       ON   OFF ON  10
       ON   ON  OFF 20
       ON   ON  ON  30

ROMs  : All type 2732
PROM  : Type MB7051




**************************************************************************/



#include <math.h>
#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

//VIDEO_START( shougi )
//{
//  generic_vh_start();
//}

#include "video/resnet.h"
/***************************************************************************

  Convert the color PROMs into a more useable format.


  bit 0 -- 1000 ohm resistor--\
  bit 1 -- 470 ohm resistor --+--+--> RED
  bit 2 -- 220 ohm resistor --/  \---------------1000 ohm resistor---\
  bit 3 -- 1000 ohm resistor--\                                      |
  bit 4 -- 470 ohm resistor --+--+--> GREEN                          |
  bit 5 -- 220 ohm resistor --/  \---------------1000 ohm resistor---+--- 1000 Ohm pullup resistor
  bit 6 -- 470 ohm resistor --+--+--> BLUE                           |
  bit 7 -- 220 ohm resistor --/  \---------------1000 ohm resistor---/

***************************************************************************/


static PALETTE_INIT( shougi )
{
	int i;
	static const int resistances_b[2]  = { 470, 220 };
	static const int resistances_rg[3] = { 1000, 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];


	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rg,	weights_r,	0,	1000+1000,
			3,	resistances_rg,	weights_g,	0,	1000+1000,
			2,	resistances_b,	weights_b,	0,	1000+1000);

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}




static VIDEO_UPDATE( shougi )
{
int offs;

	for (offs = 0;offs <0x4000; offs++)
	{
		int sx, sy, x, data1, data2, color, data;

		sx = offs >> 8;		/*00..0x3f (64*4=256)*/
		sy = offs & 0xff;	/*00..0xff*/
//      if (flipscreen[0]) sx = 31 - sx;
//      if (flipscreen[1]) sy = 31 - sy;

		data1 = videoram[offs];				/* color */
		data2 = videoram[0x4000 + offs];	/* pixel data */

		for (x=0; x<4; x++) /*4 pixels per byte (2 bitplanes in 2 nibbles: 1st=bits 7-4, 2nd=bits 3-0)*/
		{
			color= ((data1>>x) & 1) | (((data1>>(4+x)) & 1)<<1);
			data = ((data2>>x) & 1) | (((data2>>(4+x)) & 1)<<1);

			*BITMAP_ADDR16(bitmap, 255-sy, 255-(sx*4 + x)) = color*4 + data;
		}
	}
	/* copy the character mapped graphics */
	//copybitmap(bitmap,tmpbitmap,0,0,0,0,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
	return 0;
}

#if 0

static UINT8 *cpu_sharedram;
static UINT8 cpu_sharedram_control_val = 0;

//to do:
// add separate sharedram/r/w() for both CPUs and use control value to verify access

static WRITE8_HANDLER ( cpu_sharedram_sub_w )
{
	if (cpu_sharedram_control_val!=0) logerror("sub CPU access to shared RAM when access set for main cpu\n");
	cpu_sharedram[offset] = data;
}

static WRITE8_HANDLER ( cpu_sharedram_main_w )
{
	if (cpu_sharedram_control_val!=1) logerror("main CPU access to shared RAM when access set for sub cpu\n");
	cpu_sharedram[offset] = data;
}

static READ8_HANDLER ( cpu_sharedram_r )
{
	return cpu_sharedram[offset];
}

#endif

static WRITE8_HANDLER ( cpu_shared_ctrl_sub_w )
{
//  cpu_sharedram_control_val = 0;
//logerror("cpu_sharedram_ctrl=SUB");
}

static WRITE8_HANDLER ( cpu_shared_ctrl_main_w )
{
//  cpu_sharedram_control_val = 1;
//logerror("cpu_sharedram_ctrl=MAIN");
}

static WRITE8_HANDLER( shougi_watchdog_reset_w )
{
	watchdog_reset_w(0,data);
}

static WRITE8_HANDLER( shougi_mcu_halt_off_w )
{
	/* logerror("mcu HALT OFF"); */
	cpunum_set_input_line(2, INPUT_LINE_HALT, CLEAR_LINE);
}

static WRITE8_HANDLER( shougi_mcu_halt_on_w )
{
	/* logerror("mcu HALT ON"); */
	cpunum_set_input_line(2, INPUT_LINE_HALT,ASSERT_LINE);
}


static int nmi_enabled = 0;

static WRITE8_HANDLER( nmi_disable_and_clear_line_w )
{
	nmi_enabled = 0; /* disable NMIs */

	/* NMI lines are tied together on both CPUs and connected to the LS74 /Q output */
	cpunum_set_input_line(0, INPUT_LINE_NMI, CLEAR_LINE);
	cpunum_set_input_line(1, INPUT_LINE_NMI, CLEAR_LINE);
}

static WRITE8_HANDLER( nmi_enable_w )
{
	nmi_enabled = 1; /* enable NMIs */
}

static INTERRUPT_GEN( shougi_vblank_nmi )
{
	if ( nmi_enabled == 1 )
	{
		/* NMI lines are tied together on both CPUs and connected to the LS74 /Q output */
		cpunum_set_input_line(0, INPUT_LINE_NMI, ASSERT_LINE);
		cpunum_set_input_line(1, INPUT_LINE_NMI, ASSERT_LINE);
	}
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM		/* 2114 x 2 (0x400 x 4bit each) */

	/* 4800-480f connected to the 74LS259, A3 is data line so 4800-4807 write 0, and 4808-480f write 1 */
	AM_RANGE(0x4800, 0x4800) AM_WRITE(cpu_shared_ctrl_sub_w)  AM_READ(input_port_2_r)
	AM_RANGE(0x4808, 0x4808) AM_WRITE(cpu_shared_ctrl_main_w)
	AM_RANGE(0x4801, 0x4801) AM_WRITE(nmi_disable_and_clear_line_w)
	AM_RANGE(0x4809, 0x4809) AM_WRITE(nmi_enable_w)
	AM_RANGE(0x4802, 0x4802) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x480a, 0x480a) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x4803, 0x4803) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x480b, 0x480b) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x4804, 0x4804) AM_WRITE(shougi_mcu_halt_off_w)
	AM_RANGE(0x480c, 0x480c) AM_WRITE(shougi_mcu_halt_on_w)
	AM_RANGE(0x4807, 0x4807) AM_WRITE(MWA8_NOP)//?????? connected to +5v via resistor
	AM_RANGE(0x480f, 0x480f) AM_WRITE(MWA8_NOP)

	AM_RANGE(0x5000, 0x5000) AM_READ(input_port_0_r)
	AM_RANGE(0x5800, 0x5800) AM_WRITE(shougi_watchdog_reset_w) AM_READ(input_port_0_r) /* game won't boot if watchdog doesn't work */
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE(1) /* 2114 x 2 (0x400 x 4bit each) */
	AM_RANGE(0x7800, 0x7bff) AM_RAM AM_SHARE(2) /* 2114 x 2 (0x400 x 4bit each) */
//  AM_RANGE(0x7800, 0x78ff) AM_WRITE(cpu_sharedram_main_w) AM_BASE(&cpu_sharedram)/* sharedram main/sub */
//  AM_RANGE(0x7800, 0x7bff) AM_READ(cpu_sharedram_r)/* 2114 x 2 (0x400 x 4bit each) */

	AM_RANGE(0x8000, 0xffff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)	/* 4116 x 16 (32K) */
ADDRESS_MAP_END

/* sub */
static int r=0;
static READ8_HANDLER ( dummy_r )
{
	r ^= 1;
	if(r)
		return 0xff;
	else
		return 0;
}

static ADDRESS_MAP_START( readport_sub, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READ(dummy_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE(2) /* 2114 x 2 (0x400 x 4bit each) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

static INPUT_PORTS_START( shougi )
	PORT_START	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )//+-
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )//+-
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_COCKTAIL

	PORT_START	/* Coin, Start */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )//+
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )//?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START4 )

INPUT_PORTS_END


static MACHINE_DRIVER_START( shougi )

	MDRV_CPU_ADD(Z80,10000000/4)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT(shougi_vblank_nmi,1)

	MDRV_CPU_ADD(Z80,10000000/4)
	MDRV_CPU_PROGRAM_MAP(sub_map,0)
	MDRV_CPU_IO_MAP(readport_sub,0)
	/* NMIs triggered in shougi_vblank_nmi() */

	/* MCU */
	MDRV_CPU_ADD(ALPHA8201, 10000000/4/8)
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(10)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 255)
	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(shougi)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(shougi)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 10000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END



ROM_START( shougi )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.3a",    0x0000, 0x1000, CRC(b601303f) SHA1(ed07fb09053e15be49f4cb66e8916d1bdff48336) )
	ROM_LOAD( "3.3c",    0x1000, 0x1000, CRC(2b8c7314) SHA1(5d21e425889f8dc118fcd2ba8cfc6fb8f94ddc5f) )
	ROM_LOAD( "2.3b",    0x2000, 0x1000, CRC(09cb831f) SHA1(5a83a22d9245f980fe6a495433e51437d1f95644) )
	ROM_LOAD( "4.3d",    0x3000, 0x1000, CRC(ad1a642a) SHA1(d12b10f94a568d1126384e14af4b53c5e5b1a0d0) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "5.3e",    0x0000, 0x1000, CRC(ff1f07d0) SHA1(ae5bab09916b6d4ad8d3568ea39501850bdc6991) )
	ROM_LOAD( "8.3j",    0x1000, 0x1000, CRC(6230c4c1) SHA1(0b2c81bb02c270ed3bb5b42c4bd4eb25023090cb) )
	ROM_LOAD( "6.3f",    0x2000, 0x1000, CRC(d5a91b16) SHA1(1d21295667c3eb186f9e7f867763f2f2697fd350) )
	ROM_LOAD( "9.3k",    0x3000, 0x1000, CRC(dbbfa66e) SHA1(fcf23fcc65e8253325937acaf7aad4253be5e6df) )
	ROM_LOAD( "7.3h",    0x4000, 0x1000, CRC(7ea8ec4a) SHA1(d3b999a683f49c911871d0ae6bb2022e73e3cfb8) )
	/* shougi has one socket empty */

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "pr.2l",   0x0000, 0x0020, CRC(cd3559ff) SHA1(a1291b06a8a337943660b2ef62c94c49d58a6fb5) )
ROM_END

ROM_START( shougi2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1-2.3a",    0x0000, 0x1000, CRC(16d75306) SHA1(2d090396abd1fe2b31cb8450cc5d2fbde75e0230) )
	ROM_LOAD( "3-2.3c",    0x1000, 0x1000, CRC(35b6d98b) SHA1(fc125acd4d504d9c883e685b9c6e5a509dc75c69) )
	ROM_LOAD( "2-2.3b",    0x2000, 0x1000, CRC(b38affed) SHA1(44529233358923f114285533270b2a3c078b70f4) )
	ROM_LOAD( "4-2.3d",    0x3000, 0x1000, CRC(1abdb6bf) SHA1(9c7630c0e4bcaa4296a442b0e9828b96d91da77f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "5-2.3e",    0x0000, 0x1000, CRC(0ba89dd4) SHA1(d4d3b7bccccf3b7e07e2d9d776426a22b4ff422e) )
	ROM_LOAD( "8-2.3j",    0x1000, 0x1000, CRC(0ae0c8c1) SHA1(91f6f88d38c96c793137e7aaa763cab1b769e098) )
	ROM_LOAD( "6-2.3f",    0x2000, 0x1000, CRC(d98abcae) SHA1(f280b627f81f2c727268b9694d833e487ff6b08d) )
	ROM_LOAD( "9-2.3k",    0x3000, 0x1000, CRC(4e0e6c90) SHA1(b8462eec0a13d8bdf7d314eb285b5bd27d40631c) )
	ROM_LOAD( "7-2.3h",    0x4000, 0x1000, CRC(5f37ebc6) SHA1(2e5c4c2f455979e2ad2c66c5aa9f4d92194796af) )
	ROM_LOAD( "10-2.3l",   0x5000, 0x1000, CRC(a26385fd) SHA1(2adb21bb4f67a378014bc1edda48daca349d17e1) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "pr.2l",   0x0000, 0x0020, CRC(cd3559ff) SHA1(a1291b06a8a337943660b2ef62c94c49d58a6fb5) )
ROM_END

GAME( 198?, shougi,  0,        shougi,  shougi,  0, ROT0, "Alpha Denshi", "Shougi", GAME_UNEMULATED_PROTECTION )
GAME( 198?, shougi2, shougi,   shougi,  shougi,  0, ROT0, "Alpha Denshi", "Shougi 2", GAME_UNEMULATED_PROTECTION )
