/* Main Event - SNK 1984
   Canvas Croquis - SNK 1985

   driver by    David Haywood
        Tomasz Slanina

  Notes: Main Event
  -----------------

 ROM doesn't pass its internal checksum

 Todo:
  - fix controls (now you need to press button1 + direction for punch/block ) and DIPs
  - verify position of status bars

  Notes: Canvas Croquis
  ---------------------

 GFX banking is wrong?
 Doesn't have the front layer?
 Doesn't have the status bars?

-----

Canvas Croquis

file : readme.txt
author : Stefan Lindberg
created: 2005-12-24
updated: *
version: 1.0

Canvas Croquis, SNK 1984

Note:

The bproms(MB7054) was read as 74s572.
I have not tested this PCB yet so i have no idea if it's workin.
All Bproms and P1-P8 is on top pcb, P9-P14 on bottom board, see pictures.

Documentation:

Name Size CRC32
-----------------------------------------------------------
cc_top_pcb.jpg 974024 0xd2fb553e
cc_bottom_pcb.jpg 964448 0xe8bad203
Roms:
Name Size CRC32 Chip Type
-----------------------------------------------------------
cc_bprom1.j10 1024 0xfbbbf911 MB7054 (read as 74s572)
cc_bprom2.j9 1024 0x19efe7df MB7054 (read as 74s572)
cc_bprom3.j8 1024 0x21f72498 MB7054 (read as 74s572)
cc_p1.a2 8192 0xfa7109e1 M5L2764k
cc_p2.a3 8192 0x8b8beb34 M5L2764k
cc_p3.a4 8192 0xea342f87 M5L2764k
cc_p4.a5 8192 0x9cf35d98 M5L2764k
cc_p5.a7 8192 0xc5ef1eda M5L2764k
cc_p6.a8 8192 0x7b1dd7fc M5L2764k
cc_p7.h2 16384 0x029b5ea0 M5L27128k
cc_p8.f2 8192 0x0f0368ce M5L2764k
cc_p9.a2 16384 0xb58c5f24 M5L27128k
cc_p10.b2 16384 0x3c0a4eeb M5L27128k
cc_p11.c2 16384 0x4c8c2156 M5L27128k
cc_p12.j8 8192 0x9003a979 M5L2764k
cc_p13.j5 8192 0xa52cd549 M5L2764k
cc_p14.j2 8192 0xedc6a1eb M5L2764k

. Board supplied by Stefan Lindberg
. Board dumped by Stefan Lindberg

*/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/namco.h"

WRITE8_HANDLER( snkwave_w );

extern UINT8 *mainsnk_fgram;
extern UINT8 *mainsnk_bgram;
WRITE8_HANDLER(mainsnk_c600_w);
WRITE8_HANDLER(mainsnk_fgram_w);
WRITE8_HANDLER(mainsnk_bgram_w);
VIDEO_START(mainsnk);
VIDEO_UPDATE(mainsnk);
VIDEO_UPDATE(canvas);

static int sound_cpu_ready;
static int sound_command;
static int sound_fetched;


static WRITE8_HANDLER( sound_command_w )
{
	if( sound_fetched==0 ){
		logerror("missed sound command: %02x\n", sound_command );
	}

	sound_fetched = 0;
	sound_command = data;
	sound_cpu_ready = 0;
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

static READ8_HANDLER( sound_command_r )
{
	sound_fetched = 1;
	return sound_command;
}

static READ8_HANDLER( sound_ack_r )
{
	sound_cpu_ready = 1;
	return 0xff;
}

static READ8_HANDLER( mainsnk_port_0_r )
{
	int result = input_port_0_r( machine, 0 );
	if( !sound_cpu_ready ) result |= 0x20;
	return result;
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ(mainsnk_port_0_r)
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_1_r)
	AM_RANGE(0xc200, 0xc200) AM_READ(input_port_2_r)
	AM_RANGE(0xc300, 0xc300) AM_READ(input_port_3_r)
	AM_RANGE(0xc400, 0xc400) AM_READ(input_port_5_r)
	AM_RANGE(0xc500, 0xc500) AM_READ(input_port_4_r)
	AM_RANGE(0xc600, 0xc600) AM_WRITE(mainsnk_c600_w)
	AM_RANGE(0xc700, 0xc700) AM_WRITE(sound_command_w)
	AM_RANGE(0xd800, 0xdbff) AM_READWRITE(SMH_RAM, mainsnk_bgram_w) AM_BASE(&mainsnk_bgram)
	AM_RANGE(0xdc00, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0xf000, 0xf3ff) AM_READWRITE(SMH_RAM, mainsnk_fgram_w) AM_BASE(&mainsnk_fgram)
	AM_RANGE(0xf400, 0xf7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_BASE(&namco_wavedata)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(sound_command_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(sound_ack_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xe002, 0xe007) AM_WRITE(snkwave_w)
	AM_RANGE(0xe008, 0xe008) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xe009, 0xe009) AM_WRITE(AY8910_write_port_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP
ADDRESS_MAP_END

static INPUT_PORTS_START( mainsnk )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* ?? */

	PORT_START
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) /* also 02,04 */
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x60, 0x60, "Game mode" )
	PORT_DIPSETTING(    0x60, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x20, "Demo Sounds On" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPNAME( 0x08, 0x00, "SW 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "SW 2-4" ) /* $1ecf */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "SW 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( canvas )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status? or vbl? */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	256
};


static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	256
};

static const struct namco_interface snkwave_interface =
{
	1,
	-1
};


static GFXDECODE_START( mainsnk )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0, tile_layout,	0,  8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0, sprite_layout,	0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( mainsnk )
	MDRV_CPU_ADD(Z80, 3360000)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80,4000000)
	/* audio CPU */
 	MDRV_CPU_PROGRAM_MAP(sound_map,0)
 	MDRV_CPU_IO_MAP(sound_portmap,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 244)

	MDRV_GFXDECODE(mainsnk)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.606060)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(34*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 34*8-1, 0*8, 27*8-1)

	MDRV_PALETTE_LENGTH((16+2)*16)

	MDRV_VIDEO_START(mainsnk)
	MDRV_VIDEO_UPDATE(mainsnk)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(NAMCO, 24000)
	MDRV_SOUND_CONFIG(snkwave_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( canvas )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mainsnk)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(2*8, 32*8-1, 0*8, 27*8-1)

	MDRV_VIDEO_UPDATE(canvas)
MACHINE_DRIVER_END


ROM_START( mainsnk)
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "snk.p01",      0x000000, 0x002000, CRC(00db1ca2) SHA1(efe83488cf88adc185e6024b8f6ad5f8ef7f4cfd) )
	ROM_LOAD( "snk.p02",      0x002000, 0x002000, CRC(df5c86b5) SHA1(e9c854524e3d8231c874314cdff321e66ec7f0c4) )
	ROM_LOAD( "snk.p03",      0x004000, 0x002000, CRC(5c2b7bca) SHA1(e02c72fcd029999b730abd91f07866418cfe6216) )
	ROM_LOAD( "snk.p04",      0x006000, 0x002000, CRC(68b4b2a1) SHA1(8f3abc826df93f0748151624066e956b9670bc9d) )
	ROM_LOAD( "snk.p05",      0x008000, 0x002000, CRC(580a29b4) SHA1(4a96af92d65f86aca7f3a70032b5e4dc29048483) )
	ROM_LOAD( "snk.p06",      0x00a000, 0x002000, CRC(5f8a60a2) SHA1(88a051e13d6b3bbd3606a4c4cc0395da07e0f109) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "snk.p07",	0x0000, 0x4000, CRC(4208391e) SHA1(d110ca4ff9d21fe7813f04ec43c2c23471c6517f) )

	ROM_REGION( 0x08000, REGION_GFX1, 0 )
	ROM_LOAD( "snk.p12",      0x000000, 0x002000, CRC(ecf87eb7) SHA1(83b8d19070d5930b306a0309ebba05b04c2abebf) )
	ROM_LOAD( "snk.p11",      0x002000, 0x002000, CRC(3f6bc5ba) SHA1(02e49f58f5d94117113b59037fa49b8897d05b4b) )
	ROM_LOAD( "snk.p10",      0x004000, 0x002000, CRC(b5147a96) SHA1(72641fadabd16f2de4f4cf6ff3ef07233de5ddfd) )
	ROM_LOAD( "snk.p09",      0x006000, 0x002000, CRC(0ebcf837) SHA1(7b93cdffd3b8d768b98bb01956114e4ff012d029) )

	ROM_REGION( 0x12000, REGION_GFX2, 0 )
	ROM_LOAD( "snk.p13",      0x000000, 0x002000, CRC(2eb624a4) SHA1(157d7beb6ff0baa9276e388774a85996dc03821d) )
	ROM_LOAD( "snk.p16",      0x002000, 0x002000, CRC(dc502869) SHA1(024c868e8cd74c52f4787a19b9ad292b7a9dcc1c) )
	ROM_LOAD( "snk.p19",      0x004000, 0x002000, CRC(58d566a1) SHA1(1451b223ddb7c975b770f28af6c41775daaf95c1) )
	ROM_LOAD( "snk.p14",      0x006000, 0x002000, CRC(bb927d82) SHA1(ac7ae1850cf22b73e31c92b6f598fb057470a570) )
	ROM_LOAD( "snk.p17",      0x008000, 0x002000, CRC(66f60c32) SHA1(7a08d0a2c1804cdaad702a23ff33128d0b6d8084) )
	ROM_LOAD( "snk.p20",      0x00a000, 0x002000, CRC(d12c6333) SHA1(bed1a0aedaa8f6fe9c33f49b5da00ab1c9045ddd) )
	ROM_LOAD( "snk.p15",      0x00c000, 0x002000, CRC(d242486d) SHA1(0c24a3fdcb604b6231b75069c99009d68023bb8f) )
	ROM_LOAD( "snk.p18",      0x00e000, 0x002000, CRC(838b12a3) SHA1(a3444f9b2aeef70caa93e5f642cb6c3b75e88ea4) )
	ROM_LOAD( "snk.p21",      0x010000, 0x002000, CRC(8961a51e) SHA1(4f9d8358bc76118c4fab631ae73a02ab5aa0c036) )

	ROM_REGION( 0x1800, REGION_PROMS, 0 )
	ROM_LOAD( "main3.bin",    0x000000, 0x000800, CRC(78b29dde) SHA1(c2f93cde6fd8bc175e9e0d38af41b7710d7f1c82) )
	ROM_LOAD( "main2.bin",    0x000800, 0x000800, CRC(7c314c93) SHA1(c6bd2a0eaf617448ef65dcbadced313b0d69ab88) )
	ROM_LOAD( "main1.bin",    0x001000, 0x000800, CRC(deb895c4) SHA1(f1281dcb3471d9627565706ff09ba72f09dc62a4) )
ROM_END

static DRIVER_INIT ( canvas )
{
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xc000, 0, 0, input_port_0_r );
}

ROM_START( canvas )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cc_p1.a2",      0x000000, 0x002000, CRC(fa7109e1) SHA1(23e31e14af2171ee2fd7290194805b95b0f7b35c) )
	ROM_LOAD( "cc_p2.a3",      0x002000, 0x002000, CRC(8b8beb34) SHA1(c678ed7ec302eaac3594950f10f0a170353345e5) )
	ROM_LOAD( "cc_p3.a4",      0x004000, 0x002000, CRC(ea342f87) SHA1(087e1260ba51bf47bf19942b59d21d067515989d) )
	ROM_LOAD( "cc_p4.a5",      0x006000, 0x002000, CRC(9cf35d98) SHA1(08de7863f1a540b69487c87eb0a493ceeacffa1b) )
	ROM_LOAD( "cc_p5.a7",      0x008000, 0x002000, CRC(c5ef1eda) SHA1(31cf3e7fe52718bebffdac9b3666454b0956a6d9) )
	ROM_LOAD( "cc_p6.a8",      0x00a000, 0x002000, CRC(7b1dd7fc) SHA1(1287ab261885d5e9ba957024d7a00c7a0d31235b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "cc_p7.h2",	0x0000, 0x4000, CRC(029b5ea0) SHA1(88f84b4dd01656ded8d983396ded404c9d8186f1) )
	ROM_LOAD( "cc_p8.f2",	0x4000, 0x2000, CRC(0f0368ce) SHA1(a02f066ea024285a931b85709822a50a4099e0b0) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD( "cc_p9.a2",       0x006000, 0x002000, CRC(b58c5f24) SHA1(7026b3d4f8060fd6607eb6d356d6b61cc9cb75c3) ) // ok
	ROM_CONTINUE( 0xe000,0x2000) // ok
	ROM_LOAD( "cc_p10.b2",      0x004000, 0x002000, CRC(3c0a4eeb) SHA1(53742a5bef16e71bebefb0e43a175341f5bf0aa6) )
	ROM_CONTINUE( 0xc000,0x2000)
	ROM_LOAD( "cc_p11.c2",      0x008000, 0x004000, CRC(4c8c2156) SHA1(7f1d9a1e1c6cab91f24c7fc75d0c7ec2702137af) )

	ROM_REGION( 0x6000, REGION_GFX2, 0 )
	ROM_LOAD( "cc_p12.j8",      0x000000, 0x002000, CRC(9003a979) SHA1(f63959a9dc9ee67622865e783d2e501c640a4bed) )
	ROM_LOAD( "cc_p13.j5",      0x002000, 0x002000, CRC(a52cd549) SHA1(1902b8c107c5156113068ced74349ac576ac047c) )
	ROM_LOAD( "cc_p14.j2",      0x004000, 0x002000, CRC(edc6a1e8) SHA1(8c948a5f057e13bb9ed9738b66c702f45586fe59) )

	ROM_REGION( 0x1800, REGION_PROMS, 0 )
	ROM_LOAD( "cc_bprom3.j8",    0x000000, 0x000400, CRC(21f72498) SHA1(a586c869cb4633fec0df92b5646ece78f99b6f2a) )
	ROM_LOAD( "cc_bprom2.j9",    0x000800, 0x000400, CRC(19efe7df) SHA1(7e49af8b8b01fb929b87d6285da32fbe4c58606d) )
	ROM_LOAD( "cc_bprom1.j10",   0x001000, 0x000400, CRC(fbbbf911) SHA1(86394a7f67bc4f89f72b9607ca3733ab3d690289) )
ROM_END


GAME( 1984, mainsnk,      0,          mainsnk, mainsnk, 0,        ROT0, "SNK", "Main Event (1984)", 0)
GAME( 1985, canvas,       0,          canvas,  canvas,  canvas,   ROT0, "SNK", "Canvas Croquis", GAME_IMPERFECT_GRAPHICS) // some gfx banks might be wrong...
