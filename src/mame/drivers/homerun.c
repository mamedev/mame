/*
 Moero Pro Yakyuu Homerun - (c) 1988 Jaleco
 Dynamic Shooting - (c) 1988 Jaleco
 Driver by Tomasz Slanina

 *weird* hardware - based on NES version
 (gfx bank changed in the middle of screen,
  sprites in NES format etc)

Todo :
 - voice ( unemulated D7756C )
 - controls/dips
 - better emulation of gfx bank switching
 - is there 2 player mode ?

*/

#include "driver.h"
#include "machine/8255ppi.h"
#include "sound/2203intf.h"

extern int homerun_gc_up;
extern int homerun_gc_down;
extern int homerun_xpa,homerun_xpb,homerun_xpc;
extern UINT8 *homerun_videoram;

WRITE8_HANDLER( homerun_videoram_w );
WRITE8_HANDLER( homerun_color_w );
WRITE8_HANDLER( homerun_banking_w );
VIDEO_START(homerun);
VIDEO_UPDATE(homerun);

static WRITE8_HANDLER(pa_w){homerun_xpa=data;}
static WRITE8_HANDLER(pb_w){homerun_xpb=data;}
static WRITE8_HANDLER(pc_w){homerun_xpc=data;}

static const ppi8255_interface ppi8255_intf =
{
	1,
	{ 0 },
	{ 0  },
	{ 0  },
	{ pa_w },
	{ pb_w },
	{ pc_w },
};


static MACHINE_RESET( homerun )
{
	ppi8255_init(&ppi8255_intf);
}

static const gfx_layout gfxlayout =
{
   8,8,
   RGN_FRAC(1,1),
   2,
   { 8*8,0},
   { 0, 1, 2, 3, 4, 5, 6, 7},
   { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
   8*8*2
};



static const gfx_layout spritelayout =
{
   16,16,
   RGN_FRAC(1,1),
   2,
   { 8*8,0},
   { 0, 1, 2, 3, 4, 5, 6, 7,0+8*8*2,1+8*8*2,2+8*8*2,3+8*8*2,4+8*8*2,5+8*8*2,6+8*8*2,7+8*8*2},
   { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8+2*8*8*2,1*8+2*8*8*2,2*8+2*8*8*2,3*8+2*8*8*2,4*8+2*8*8*2,5*8+2*8*8*2,6*8+2*8*8*2,7*8+2*8*8*2},
   8*8*2*4
};

static GFXDECODE_START( homerun )
	GFXDECODE_ENTRY( REGION_GFX1, 0, gfxlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout,   0, 16 )
GFXDECODE_END

static ADDRESS_MAP_START( homerun_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_READ(MRA8_BANK1)
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_WRITE(homerun_videoram_w) AM_BASE(&homerun_videoram)
	AM_RANGE(0xa000, 0xa0ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xb000, 0xb0ff) AM_WRITE(homerun_color_w)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER(homerun_40_r)
{
	if(video_screen_get_vpos(machine->primary_screen)>116)
		return readinputport(0)|0x40;
	else
		return readinputport(0);
}

static ADDRESS_MAP_START( homerun_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(MWA8_NOP) /* ?? */
	AM_RANGE(0x20, 0x20) AM_WRITE(MWA8_NOP) /* ?? */
	AM_RANGE(0x30, 0x33) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)
	AM_RANGE(0x40, 0x40) AM_READ(homerun_40_r)
	AM_RANGE(0x50, 0x50) AM_READ(input_port_2_r)
	AM_RANGE(0x60, 0x60) AM_READ(input_port_1_r)
	AM_RANGE(0x70, 0x70) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x71, 0x71) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
ADDRESS_MAP_END

static const struct YM2203interface ym2203_interface =
{
	input_port_3_r,
	0,
	0,
	homerun_banking_w
};


static INPUT_PORTS_START( homerun )
	PORT_START
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x80, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1   )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2  )

	PORT_START
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 1C_2C ) )

INPUT_PORTS_END

static INPUT_PORTS_START( dynashot )
	PORT_START
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1   )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_BIT( 0x77, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2  )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 1C_2C ) )


	PORT_DIPNAME( 0x7c, 0x7c, "Collisions ?" ) //not all bits
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x7c, DEF_STR( On ) )

INPUT_PORTS_END


static MACHINE_DRIVER_START( homerun )
	MDRV_CPU_ADD(Z80, 5000000)
	MDRV_CPU_PROGRAM_MAP(homerun_memmap, 0)
	MDRV_CPU_IO_MAP(homerun_iomap, 0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_MACHINE_RESET(homerun)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-25)

	MDRV_GFXDECODE(homerun)
	MDRV_PALETTE_LENGTH(16*4)

	MDRV_VIDEO_START(homerun)
	MDRV_VIDEO_UPDATE(homerun)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 6000000/2)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END

ROM_START( homerun )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "homerun.43",        0x0000, 0x4000, CRC(e759e476) SHA1(ad4f356ff26209033320a3e6353e4d4d9beb59c1) )
	ROM_CONTINUE(        0x10000,0x1c000)

	ROM_REGION( 0x010000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "homerun.60",  0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x020000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "homerun.120",  0x00000, 0x20000, CRC(52f0709b) SHA1(19e675bcccadb774f60ec5929fc1fb5cf0d3f617) )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 )
	ROM_LOAD( "homerun.snd",  0x00000, 0x1000, NO_DUMP ) /* D7756C internal rom */

ROM_END

ROM_START( dynashot )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "1.ic43",        0x0000, 0x4000, CRC(bf3c9586) SHA1(439effbda305f5fa265e5897c81dc1447e5d867d) )
	ROM_CONTINUE(        0x10000,0x1c000)

	ROM_REGION( 0x010000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "3.ic60",  0x00000, 0x10000, CRC(77d6a608) SHA1(a31ff343a5d4d6f20301c030ecc2e252149bcf9d) )

	ROM_REGION( 0x020000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "2.ic120",  0x00000, 0x20000, CRC(bedf7b98) SHA1(cb6c5fcaf8df5f5c7636c3c8f79b9dda78e30c2e) )

	ROM_REGION( 0x01000, REGION_SOUND1, 0 )
	ROM_LOAD( "dynashot.snd",  0x00000, 0x1000, NO_DUMP ) /* D7756C internal rom */

ROM_END


GAME( 1988, homerun, 0, homerun, homerun, 0, ROT0, "Jaleco", "Moero Pro Yakyuu Homerun",GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND)
GAME( 1988, dynashot, 0, homerun, dynashot, 0, ROT0, "Jaleco", "Dynamic Shooting",GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND)


