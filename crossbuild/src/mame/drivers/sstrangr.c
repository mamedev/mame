/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger/Space Stranger 2"           */
/*                                                     */
/*******************************************************/


#include "driver.h"


#define NUM_PENS	(8)


static UINT8 *sstrngr_ram;
static size_t sstrngr_ram_size;
static UINT8 sstrngr_flip_screen;



/*************************************
 *
 *  Video system
 *
 *************************************/

static VIDEO_UPDATE( sstrangr )
{
	offs_t offs;

	for (offs = 0; offs < sstrngr_ram_size; offs++)
	{
		int i;

		UINT8 x = offs << 3;
		int y = offs >> 5;
		UINT8 data = sstrngr_ram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			if (sstrngr_flip_screen)
			{
				pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
				data = data << 1;
			}
			else
			{
				pen = (data & 0x01) ? RGB_WHITE : RGB_BLACK;
				data = data >> 1;
			}

			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
		}
	}

	return 0;
}


static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}


static VIDEO_UPDATE( sstrngr2 )
{
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;

	get_pens(pens);

	color_map_base = sstrngr_flip_screen ?  memory_region(REGION_PROMS) : &memory_region(REGION_PROMS)[0x0200];

	for (offs = 0; offs < sstrngr_ram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 9 << 5) | (offs & 0x1f);

		UINT8 data = sstrngr_ram[offs];
		UINT8 fore_color = color_map_base[color_address] & 0x07;

		for (i = 0; i < 8; i++)
		{
			UINT8 color;

			if (sstrngr_flip_screen)
			{
				color = (data & 0x80) ? fore_color : 0;
				data = data << 1;
			}
			else
			{
				color = (data & 0x01) ? fore_color : 0;
				data = data >> 1;
			}

			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			x = x + 1;
		}
	}

	return 0;
}


static WRITE8_HANDLER( port_w )
{
	sstrngr_flip_screen = data & 0x20;
}



static ADDRESS_MAP_START( sstrangr_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(15) )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_BASE(&sstrngr_ram) AM_SIZE(&sstrngr_ram_size)
	AM_RANGE(0x6000, 0x63ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sstrangr_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x41, 0x41) AM_READ(input_port_0_r)
	AM_RANGE(0x42, 0x42) AM_READ(input_port_1_r)
	AM_RANGE(0x44, 0x44) AM_READWRITE(input_port_2_r, port_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( sstrangr )
	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Must be ACTIVE_LOW for game to boot */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START_TAG("EXT")      /* External switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_DRIVER_START( sstrangr )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main",8080,1996800)	/* clock is a guess, taken from mw8080bw */
	MDRV_CPU_PROGRAM_MAP(sstrangr_map,0)
	MDRV_CPU_IO_MAP(sstrangr_io_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_UPDATE(sstrangr)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 262)		/* vert size is a guess, taken from mw8080bw */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)

	/* sound hardware */

MACHINE_DRIVER_END



/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger 2"                          */
/*                                                     */
/*******************************************************/

/* color version of Space Stranger, board has Stranger 2 written on it */

static INPUT_PORTS_START( sstrngr2 )
	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Coinage) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START_TAG("EXT")      /* External switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_DIPNAME( 0x02, 0x00, "Player's Bullet Speed (Cheat)" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_DRIVER_START( sstrngr2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(sstrangr)

	/* video hardware */
	MDRV_VIDEO_UPDATE(sstrngr2)

MACHINE_DRIVER_END



ROM_START( sstrangr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "hss-01.58",     0x0000, 0x0400, CRC(feec7600) SHA1(787a6be4e24ce931e7678e777699b9f6789bc199) )
	ROM_LOAD( "hss-02.59",     0x0400, 0x0400, CRC(7281ff0b) SHA1(56649d1362be1b9f517cb8616cbf9e4f955e9a2d) )
	ROM_LOAD( "hss-03.60",     0x0800, 0x0400, CRC(a09ec572) SHA1(9c4ad811a6c0460403f9cdc9fe5381c460249ff5) )
	ROM_LOAD( "hss-04.61",     0x0c00, 0x0400, CRC(ec411aca) SHA1(b72eb6f7c3d69e2829280d1ab982099f6eff0bde) )
	ROM_LOAD( "hss-05.62",     0x1000, 0x0400, CRC(7b1b81dd) SHA1(3fa6e244e203fb75f92b19db7b4b18645b3f66a3) )
	ROM_LOAD( "hss-06.63",     0x1400, 0x0400, CRC(de383625) SHA1(7ec0d7171e771c4b43e026f3f50a88d8ab2236bb) )
	ROM_LOAD( "hss-07.64",     0x1800, 0x0400, CRC(2e41d0f0) SHA1(bba720b0c5a7bd47abb8bc8498a989e17dc52428) )
	ROM_LOAD( "hss-08.65",     0x1c00, 0x0400, CRC(bd14d0b0) SHA1(9665f639afef9c1291f2efc054216ff44c595b45) )
ROM_END

ROM_START( sstrngr2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "4764.09",      0x0000, 0x2000, CRC(d88f86cc) SHA1(9f284ee50caf3c64bd04a79a798de620348881bc) )
	ROM_LOAD( "2708.10",      0x6000, 0x0400, CRC(eba304c1) SHA1(3fa6fbb29fa46c146283f69a712bfc51cbb2a43c) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )		/* color maps player 1/player 2 */
	ROM_LOAD( "2708.15",      0x0000, 0x0400, CRC(c176a89d) SHA1(955dd540dc3787091c3f34ae122a13e6b7523414) )
ROM_END


GAME( 1978, sstrangr, 0,        sstrangr, sstrangr, 0, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger", GAME_NO_SOUND )
GAME( 1979, sstrngr2, sstrangr, sstrngr2, sstrngr2, 0, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger 2", GAME_NO_SOUND )
