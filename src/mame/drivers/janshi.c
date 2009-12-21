/* Janshi */

/*

Janshi
(c)1992 Eagle

CPU: HD647180X0P6 (16K EPROM internal rom)
Sound: AY-3-8910, M6295
Others: Battery
OSC: 32MHz, 21MHz & 12.2880MHz

ROMs:
1.1A         [92b140a5]
2.1B         [6de7e086]
3.1D         [4e94d8f2]
4.1F         [a5f6e3ef]
5.1H         [ff2cc769]
6.1K         [8197034d]
11.1L        [a7692ddf]



--- Team Japump!!! ---
Dumped by Chackn
04/May/2007

*/

#include "driver.h"
#include "includes/news.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"


static ADDRESS_MAP_START( mainmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static VIDEO_START(janshi)
{

}

static VIDEO_UPDATE(janshi)
{
	return 0;
}


static INPUT_PORTS_START( janshi )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5),RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( janshi )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



static MACHINE_DRIVER_START( janshi )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,6000000) /* Z180 */		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(mainmap)
//  MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(janshi)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(janshi)
	MDRV_VIDEO_UPDATE(janshi)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



ROM_START( janshi )
	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "11.1l", 0x00000, 0x20000, CRC(a7692ddf) SHA1(5e7f43d8337583977baf22a28bbcd9b2182c0cde) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_COPY( "user1", 0x10000, 0x00000, 0x4000 )// Missing 16K internal rom (might need decapping)

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "1.1a", 0x000000, 0x40000, CRC(92b140a5) SHA1(f3b38563f74650604ed0faaf84460e0b04b386b7) )
	ROM_LOAD( "2.1b", 0x040000, 0x40000, CRC(6de7e086) SHA1(e87426264f0181c17383ffe0f7ec7ff5fce3d809) )
	ROM_LOAD( "3.1d", 0x080000, 0x40000, CRC(4e94d8f2) SHA1(a25f542943d74915fc82910baafb9ff9db1ffd70) )
	ROM_LOAD( "4.1f", 0x0c0000, 0x40000, CRC(a5f6e3ef) SHA1(f1f3d28b27eea682aa71855a311fb3abdf9af2cd) )
	ROM_LOAD( "5.1h", 0x100000, 0x40000, CRC(ff2cc769) SHA1(ba4cf2923cf3d4d815a9327595f8e1801c3c8a2b) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "6.1k", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )
ROM_END

GAME( 1992, janshi,  0,    janshi, janshi,  0, ROT0, "Eagle", "Janshi", GAME_NOT_WORKING )
