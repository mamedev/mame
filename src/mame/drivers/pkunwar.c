/***************************************************************************

Notes:
- Currently the Flip Screen dip switch only flips the screen horizontally.
  This might not be the correct behaviour. Verification of the real board
  would be necessary to emulate this accurately.

***************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"


PALETTE_INIT( nova2001 );
WRITE8_HANDLER( pkunwar_flipscreen_w );
VIDEO_UPDATE( pkunwar );



static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x8fff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa001, 0xa001) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0xa003, 0xa003) AM_READ(AY8910_read_port_1_r)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x8800, 0x8bff) AM_WRITE(MWA8_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x8c00, 0x8fff) AM_WRITE(MWA8_RAM) AM_BASE(&colorram)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(&AY8910_control_port_0_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(&AY8910_write_port_0_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(&AY8910_control_port_1_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(&AY8910_write_port_1_w)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(pkunwar_flipscreen_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( pkunwar )
	PORT_START	/* IN0 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START	/* IN1 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	2048,	/* 2048 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2048*16*8, 2048*16*8 + 4, 2*4,  3*4, 2048*16*8 + 8, 2048*16*8 + 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	512,	/* 512 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 512*64*8, 512*64*8 + 4, 2*4,  3*4, 512*64*8 + 8, 512*64*8 + 12,
			0*4+16*8, 1*4+16*8, 512*64*8+16*8, 512*64*8 + 4+16*8, 2*4+16*8,  3*4+16*8, 512*64*8 + 8 + 16*8, 512*64*8 + 12 + 16*8 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8,
			0*8+32*8, 2*8+32*8, 4*8+32*8, 6*8+32*8, 8*8+32*8, 10*8+32*8, 12*8+32*8, 14*8 + 32*8  },
	64*8
};

static GFXDECODE_START( pkunwar )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,   16*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, spritelayout,     0, 16 )
GFXDECODE_END



static const struct AY8910interface ay8910_interface_1 = {
	input_port_0_r,
	input_port_1_r
};

static const struct AY8910interface ay8910_interface_2 = {
	input_port_2_r,
	input_port_3_r
};



static MACHINE_DRIVER_START( pkunwar )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/4)	/* 3 MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(0,writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MDRV_GFXDECODE(pkunwar)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(32*16)

	MDRV_PALETTE_INIT(nova2001)
	MDRV_VIDEO_UPDATE(pkunwar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pkunwar )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pkwar.01r",    0x0000, 0x4000, CRC(ce2d2c7b) SHA1(2ffe2eb339fd668ec4fe90eff66124a334db0693) )
	ROM_LOAD( "pkwar.02r",    0x4000, 0x4000, CRC(abc1f661) SHA1(c4bf4a345efd4271617de9f334303d81c6885aa5) )
	ROM_LOAD( "pkwar.03r",    0xe000, 0x2000, CRC(56faebea) SHA1(dd0406c723a08f5d1120655857a115ab8c2d2a11) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pkwar.01y",    0x0000, 0x2000, CRC(428d3b92) SHA1(7fe11e8d785fe829d34e512f233bb9ccc70cd431) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "pkwar.02y",    0x2000, 0x2000, CRC(ce1da7bc) SHA1(a2357b61703a689ce63aec7dd44702b119894f8e) )
	ROM_CONTINUE(             0xa000, 0x2000 )
	ROM_LOAD( "pkwar.03y",    0x4000, 0x2000, CRC(63204400) SHA1(1ba87ad3425c51150cb65408f04ee0147ef332d3) )
	ROM_CONTINUE(             0xc000, 0x2000 )
	ROM_LOAD( "pkwar.04y",    0x6000, 0x2000, CRC(061dfca8) SHA1(0a2dd8fc790d607195ca18dfc55575c2b9ddc58a) )
	ROM_CONTINUE(             0xe000, 0x2000 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "pkwar.col",    0x0000, 0x0020, CRC(af0fc5e2) SHA1(480908bf893211b580ae19cfb40dc35ad1bbc343) )
ROM_END

ROM_START( pkunwarj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pgunwar.6",    0x0000, 0x4000, CRC(357f3ef3) SHA1(bc651fb7701b395ae8cda1888814af5c5aa325a6) )
	ROM_LOAD( "pgunwar.5",    0x4000, 0x4000, CRC(0092e49e) SHA1(7945361036f7679e4f4bb6b94f60f3ca09c077dc) )
	ROM_LOAD( "pkwar.03r",    0xe000, 0x2000, CRC(56faebea) SHA1(dd0406c723a08f5d1120655857a115ab8c2d2a11) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pkwar.01y",    0x0000, 0x2000, CRC(428d3b92) SHA1(7fe11e8d785fe829d34e512f233bb9ccc70cd431) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "pkwar.02y",    0x2000, 0x2000, CRC(ce1da7bc) SHA1(a2357b61703a689ce63aec7dd44702b119894f8e) )
	ROM_CONTINUE(             0xa000, 0x2000 )
	ROM_LOAD( "pgunwar.2",    0x4000, 0x2000, CRC(a2a43443) SHA1(4e10569886d364eb2539928ea81dc1565b60b590) )
	ROM_CONTINUE(             0xc000, 0x2000 )
	ROM_LOAD( "pkwar.04y",    0x6000, 0x2000, CRC(061dfca8) SHA1(0a2dd8fc790d607195ca18dfc55575c2b9ddc58a) )
	ROM_CONTINUE(             0xe000, 0x2000 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "pkwar.col",    0x0000, 0x0020, CRC(af0fc5e2) SHA1(480908bf893211b580ae19cfb40dc35ad1bbc343) )
ROM_END



GAME( 1985?, pkunwar,  0,       pkunwar, pkunwar, 0, ROT0, "UPL", "Penguin-Kun Wars (US)", 0 )
GAME( 1985?, pkunwarj, pkunwar, pkunwar, pkunwar, 0, ROT0, "UPL", "Penguin-Kun Wars (Japan)", 0 )
