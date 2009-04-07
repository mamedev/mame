/* Unknown - Poker

Anno: 1982
Produttore:
N.revisione:

CPU:
1x unknown DIP40 (1ef)(missing)
1x TBA820 (14e)(sound)
1x oscillator 12.000 (2f) 

ROMs:
2x TMS2532 (5b,5e)
1x TMS2516 (8b)
1x PROM SN74S288N (8a)
1x RAM MWS5101AEL2 (11e)
4x RAM AM9114EPC (2b,3b,8e,9e) 

Note:
1x 22x2 edge connector
1x trimmer (volume)
1x 8x2 switches dip
1x empty DIP14 socket (close to sound)

Funzionamento: Non Funzionante
In vendita: No
Dumped: 06/04/2009 f205v
*/

#include "driver.h"
#include "cpu/z80/z80.h"

static UINT8* unkpoker_video;

static READ8_HANDLER(test1_r)
{
if (input_code_pressed(KEYCODE_Q)) return 0x01; // unselect cards
if (input_code_pressed(KEYCODE_W)) return 0x02; // deal
if (input_code_pressed(KEYCODE_E)) return 0x04; // replay
if (input_code_pressed(KEYCODE_R)) return 0x08; // select card #5
if (input_code_pressed(KEYCODE_T)) return 0x10;
if (input_code_pressed(KEYCODE_Y)) return 0x20;
if (input_code_pressed(KEYCODE_U)) return 0x40; // 1 credit
if (input_code_pressed(KEYCODE_I)) return 0x80; // 5/10 credits
return 0;
}

static READ8_HANDLER(test2_r)
{
if (input_code_pressed(KEYCODE_A)) return 0x01; // 
if (input_code_pressed(KEYCODE_S)) return 0x02; // 
if (input_code_pressed(KEYCODE_D)) return 0x04; // 
if (input_code_pressed(KEYCODE_F)) return 0x08; // 
if (input_code_pressed(KEYCODE_G)) return 0x10;
if (input_code_pressed(KEYCODE_H)) return 0x20;
if (input_code_pressed(KEYCODE_J)) return 0x40; // 
if (input_code_pressed(KEYCODE_K)) return 0x80; // 
return 0;
}

static READ8_HANDLER(test3_r)
{
if (input_code_pressed(KEYCODE_1)) return 0x01; // select card #1
if (input_code_pressed(KEYCODE_2)) return 0x02; // select card #2
if (input_code_pressed(KEYCODE_3)) return 0x04; // select card #3
if (input_code_pressed(KEYCODE_4)) return 0x08; // select card #4
if (input_code_pressed(KEYCODE_5)) return 0x10;
if (input_code_pressed(KEYCODE_6)) return 0x20;
if (input_code_pressed(KEYCODE_7)) return 0x40; // 
if (input_code_pressed(KEYCODE_8)) return 0x80; // 
return 0;
}

static READ8_HANDLER(test4_r)
{
if (input_code_pressed(KEYCODE_Z)) return 0x01; // 
if (input_code_pressed(KEYCODE_X)) return 0x02; // 
if (input_code_pressed(KEYCODE_C)) return 0x04; // 
if (input_code_pressed(KEYCODE_V)) return 0x08; // 
if (input_code_pressed(KEYCODE_B)) return 0x10;
if (input_code_pressed(KEYCODE_N)) return 0x20;
if (input_code_pressed(KEYCODE_M)) return 0x40; // 
if (input_code_pressed(KEYCODE_L)) return 0x80; // 
return 0;
}

static WRITE8_HANDLER(test_w)
{

}

static ADDRESS_MAP_START( unkpoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fFf) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM
	AM_RANGE(0x5800, 0x5bff) AM_RAM AM_BASE(&unkpoker_video)
	AM_RANGE(0x5c00, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(test1_r,test_w)
	AM_RANGE(0x6800, 0x6800) AM_READWRITE(test2_r,test_w)
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(test3_r,test_w)
	AM_RANGE(0x7800, 0x7800) AM_READWRITE(test4_r,test_w)
ADDRESS_MAP_END

static VIDEO_START(unkpoker)
{

}

static VIDEO_UPDATE(unkpoker)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;

	int y, x;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int tile = unkpoker_video[count];
			drawgfx(bitmap, gfx, tile, 0, 0, 0, x * 8, y * 8, cliprect, TRANSPARENCY_NONE, 0);

			count++;
		}
	}
	return 0;
}

static INPUT_PORTS_START( unkpoker )
INPUT_PORTS_END

static const gfx_layout layout8x8x2 =
{
 	8,8,
 	RGN_FRAC(1,2),
 	2,
 	{
		RGN_FRAC(0,2),RGN_FRAC(1,2)
 	},
 	{ STEP8(0,1) },
 	{ STEP8(0,8) },
 	8*8
};

static GFXDECODE_START( unkpoker )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x2,  0x0, 1 )
GFXDECODE_END

static MACHINE_DRIVER_START( unkpoker )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 1000000) /* Z80? */
	MDRV_CPU_PROGRAM_MAP(unkpoker_map, 0)

	MDRV_GFXDECODE(unkpoker)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(unkpoker)
	MDRV_VIDEO_UPDATE(unkpoker)
MACHINE_DRIVER_END



ROM_START(unkpoker)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("2532.5e", 0x0000, 0x1000, CRC(093d4560) SHA1(d5401b5f7a2ebe5099fefc5b09f8710886e243b2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD("2532.8b", 0x0000, 0x0800, CRC(4427ffc0) SHA1(45f5fd0ae967cdb6abbf2e6c6d12d787556488ef) )
	ROM_CONTINUE(0x0000, 0x0800)
	ROM_LOAD("2516.5b", 0x0800, 0x0800, CRC(496ad48c) SHA1(28380c9d02b64e7d5ef2763de92cd2ca8861eceb) )
ROM_END

GAME( 200?, unkpoker,  0,   unkpoker, unkpoker, 0, ROT0, "???", "unkpoker", GAME_NOT_WORKING|GAME_NO_SOUND )
