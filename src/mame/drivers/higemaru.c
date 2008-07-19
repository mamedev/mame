/****************************************************************************

Higemaru

driver by Mirko Buffoni

****************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"


extern WRITE8_HANDLER( higemaru_videoram_w );
extern WRITE8_HANDLER( higemaru_colorram_w );
extern WRITE8_HANDLER( higemaru_c800_w );

extern PALETTE_INIT( higemaru );
extern VIDEO_START( higemaru );
extern VIDEO_UPDATE( higemaru );


static INTERRUPT_GEN( higemaru_interrupt )
{
	if (cpu_getiloops() == 0)
		cpunum_set_input_line_and_vector(machine, 0,0,HOLD_LINE,0xcf);	/* RST 08h */
	else
		cpunum_set_input_line_and_vector(machine, 0,0,HOLD_LINE,0xd7);	/* RST 10h */
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xc000) AM_READ(input_port_0_r)
	AM_RANGE(0xc001, 0xc001) AM_READ(input_port_1_r)
	AM_RANGE(0xc002, 0xc002) AM_READ(input_port_2_r)
	AM_RANGE(0xc003, 0xc003) AM_READ(input_port_3_r)
	AM_RANGE(0xc004, 0xc004) AM_READ(input_port_4_r)
	AM_RANGE(0xd000, 0xd7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xe000, 0xefff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(higemaru_c800_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xc802, 0xc802) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xc803, 0xc803) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0xd000, 0xd3ff) AM_WRITE(higemaru_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xd400, 0xd7ff) AM_WRITE(higemaru_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xd880, 0xd9ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


static INPUT_PORTS_START( higemaru )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )	/* could be Tilt? */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0e, "10000 50000 50000" )
	PORT_DIPSETTING(    0x0c, "10000 60000 60000" )
	PORT_DIPSETTING(    0x0a, "20000 60000 60000" )
	PORT_DIPSETTING(    0x08, "20000 70000 70000" )
	PORT_DIPSETTING(    0x06, "30000 70000 70000" )
	PORT_DIPSETTING(    0x04, "30000 80000 80000" )
	PORT_DIPSETTING(    0x02, "40000 100000 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Demo Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( higemaru )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,       0, 32 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout,  32*4, 16 )
GFXDECODE_END


static MACHINE_DRIVER_START( higemaru )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, 4000000)	/* 4 MHz ? Main xtal is 12MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT_HACK(higemaru_interrupt,2)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(higemaru)
	MDRV_PALETTE_LENGTH(32*4+16*16)

	MDRV_PALETTE_INIT(higemaru)
	MDRV_VIDEO_START(higemaru)
	MDRV_VIDEO_UPDATE(higemaru)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ay2", AY8910, 12000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( higemaru )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "hg4",          0x0000, 0x2000, CRC(dc67a7f9) SHA1(701875e2e85efbe84bf66515117861563f3883c0) )
	ROM_LOAD( "hg5",          0x2000, 0x2000, CRC(f65a4b68) SHA1(687d46406de389c8bad6cc052a2516135db93d4a) )
	ROM_LOAD( "hg6",          0x4000, 0x2000, CRC(5f5296aa) SHA1(410ee1df63492e488b3578b9c4cfbfbd2f41c888) )
	ROM_LOAD( "hg7",          0x6000, 0x2000, CRC(dc5d455d) SHA1(7d253d6680d35943792746da11d91d7be57367cc) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hg3",          0x0000, 0x2000, CRC(b37b88c8) SHA1(7933270969806154f0774d31fda75a5352cf26ad) )	/* characters */

	ROM_REGION( 0x4000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "hg1",          0x0000, 0x2000, CRC(ef4c2f5d) SHA1(247ce819cdc4ed4ec99c25c9006bac1911354bc8) )	/* tiles */
	ROM_LOAD( "hg2",          0x2000, 0x2000, CRC(9133f804) SHA1(93661c028709a7134537321e52da85e3c0f917ba) )

	ROM_REGION( 0x0420, REGION_PROMS, 0 )
	ROM_LOAD( "hgb3",         0x0000, 0x0020, CRC(629cebd8) SHA1(c28cd0f341f4f1c7be97f4d8c289860db8ac0857) )	/* palette */
	ROM_LOAD( "hgb5",         0x0020, 0x0100, CRC(dbaa4443) SHA1(cca2f9b187abd735f2309b38570edcd745042b3e) )	/* char lookup table */
	ROM_LOAD( "hgb1",         0x0120, 0x0100, CRC(07c607ce) SHA1(c048602d62f47129152bbc7ccd38627d78a4392f) )	/* sprite lookup table */
	ROM_LOAD( "hgb4",         0x0220, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "hgb2",         0x0320, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )	/* video timing? (not used) */
ROM_END


GAME( 1984, higemaru, 0, higemaru, higemaru, 0, ROT0, "Capcom", "Pirate Ship Higemaru", 0 )
