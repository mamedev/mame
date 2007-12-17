/* Rotary Fighter

driver by Barry Rodewald
 based on Initial work by David Haywood

 todo:

 sound
 verify game speed if possible (related to # of interrupts)

*/

#include "driver.h"
#include "cpu/i8085/i8085.h"


static UINT8 *rotaryf_videoram;
static size_t rotaryf_videoram_size;



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( rotaryf_interrupt )
{
	if (video_screen_get_vblank(0))
		cpunum_set_input_line(0, I8085_RST55_LINE,HOLD_LINE);
	else
		cpunum_set_input_line(0, I8085_RST75_LINE,HOLD_LINE);

}



/*************************************
 *
 *  Video system
 *
 *************************************/

static VIDEO_UPDATE( rotaryf )
{
	offs_t offs;

	for (offs = 0; offs < rotaryf_videoram_size; offs++)
	{
		int i;

		UINT8 x = offs << 3;
		int y = offs >> 5;
		UINT8 data = rotaryf_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			data = data >> 1;
			x = x + 1;
		}
	}

	return 0;
}


static ADDRESS_MAP_START( rotaryf_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_MIRROR(0x4000) AM_ROM
//  AM_RANGE(0x6ffb, 0x6ffb) AM_READ(random_r) ??
//  AM_RANGE(0x6ffd, 0x6ffd) AM_READ(random_r) ??
//  AM_RANGE(0x6fff, 0x6fff) AM_READ(random_r) ??
	AM_RANGE(0x7000, 0x73ff) AM_RAM // clears to 1ff ?
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x4000) AM_RAM AM_BASE(&rotaryf_videoram) AM_SIZE(&rotaryf_videoram_size)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rotaryf_io_map, ADDRESS_SPACE_IO, 8 )
//  AM_RANGE(0x00, 0x00) AM_READ(input_port_0_r)
	AM_RANGE(0x21, 0x21) AM_READ(input_port_1_r)
	AM_RANGE(0x26, 0x26) AM_READ(input_port_3_r)
	AM_RANGE(0x29, 0x29) AM_READ(input_port_2_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( rotaryf )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x81, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x81, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING( 0x04, "1000" )
	PORT_DIPSETTING( 0x00, "1500" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, DEF_STR( On ) )
//  PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
//  PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
//  PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START		/* Dummy port for cocktail mode */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static MACHINE_DRIVER_START( rotaryf )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main",8085A,2000000) /* 8080? */ /* 2 MHz? */
	MDRV_CPU_PROGRAM_MAP(rotaryf_map,0)
	MDRV_CPU_IO_MAP(rotaryf_io_map,0)
	MDRV_CPU_VBLANK_INT(rotaryf_interrupt,5)
	MDRV_SCREEN_REFRESH_RATE(60)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_UPDATE(rotaryf)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 262)		/* vert size is a guess, taken from mw8080bw */
	MDRV_SCREEN_VISIBLE_AREA(1*8, 30*8-1, 0*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)

MACHINE_DRIVER_END


ROM_START( rotaryf )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "krf-1.bin", 0x0000, 0x0400, CRC(f7b2d3e6) SHA1(be7afc1a14be60cb895fc4180167353c7156fc4c) )
	ROM_LOAD( "krf-2.bin", 0x0400, 0x0400, CRC(be9f047a) SHA1(e5dd2b5b4fda7f178e7f1137592ba49fbc9cc82e) )
	ROM_LOAD( "krf-3.bin", 0x0800, 0x0400, CRC(c7629eb6) SHA1(03aae964783ce4b1de77737e83fd2094483fbda4) )
	ROM_LOAD( "krf-4.bin", 0x0c00, 0x0400, CRC(b4703093) SHA1(9239d6da818049bc98a631c3bf5b962b5df5b2ea) )
	ROM_LOAD( "krf-5.bin", 0x1000, 0x0400, CRC(ae233f07) SHA1(a7bbd2ee4477ee041d170e2fc4e94c99c3b564fc) )
	ROM_LOAD( "krf-6.bin", 0x1400, 0x0400, CRC(e28b3713) SHA1(428f73891125f80c722357f1029b18fa9416bcfd) )
ROM_END


GAME( 19??, rotaryf, 0, rotaryf, rotaryf, 0, ROT270, "<unknown>", "Rotary Fighter", GAME_NO_SOUND )
