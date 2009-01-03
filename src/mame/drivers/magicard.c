/******************************************************************************


    MAGIC CARD - IMPERA
    -------------------

    Preliminary driver by Roberto Fresca, David Haywood & Angelo Salese


    Games running on this hardware:

    * Magic Card (set 1),  Impera, 199?
    * Magic Card (set 2),  Impera, 199?

*******************************************************************************


    *** Hardware Notes ***

    (All ICs on this board are surface scratched, so we're guessing...)

    Identified:

    - CPU:  1x Philips SCC 68070 CCA84 (16 bits Microprocessor, PLCC) @ 15 MHz
    - VSC:  1x Philips SCC 66470 CAB (Video and System Controller, QFP)

    - Protection: 1x Dallas TimeKey DS1207-1 (for book-keeping protection)

    - Crystals:   1x 30.0000 MHz.
                  1x 19.6608 MHz.

    - PLDs:       1x PAL16L8ACN
	              1x PALCE18V8H-25


*******************************************************************************


    *** General Notes ***

    Impera released "Magic Card" in a custom 16-bits PCB.
    The hardware was so expensive and they never have reached the expected sales,
    so... they ported the game to Impera/Funworld 8bits boards, losing part of
    graphics and sound/music quality. The new product was named "Magic Card II".

*******************************************************************************

TODO:

-Understand these two things:
 -bpset 1a2a82
  do pc=1a2a92 ;tests the 260000-27ffff region
  After the boot screen (the angel with torch),will come a screen with a circle
  and a counter in the middle like a warning screen.I think is checksuming
  something -RF

 -bpset 75600
  do pc=75648 ;there are various tests/tight hangs while testing the 2005 area
  Chances are that the inputs are located in a RAM-based fashion,similar of the
  way used by spool99 (probably the latter copied from this one). -AS

-Fix properly the starting vectors;

-Colors;

-Inputs;

-Sound;

-Visible Area;

-Clone romset magicrda not checked at all;

*******************************************************************************/


#define CLOCK_A	XTAL_30MHz
#define CLOCK_B	XTAL_19_6608MHz

#include "driver.h"
#include "cpu/m68000/m68000.h"

static UINT16 *magicram;
static UINT16 *blit_ram;

/*************************
*     Video Hardware     *
*************************/

static VIDEO_START(magicard)
{

}

static VIDEO_UPDATE(magicard)
{
	int x,y,count;

	count = (0/2);

	for(y=0;y<300;y++)
	{
		for(x=0;x<168;x++)
		{
			UINT32 color;

			color = ((blit_ram[count]) & 0x00ff)>>0;

			if((x*2)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+1) = screen->machine->pens[color];

			color = ((blit_ram[count]) & 0xff00)>>8;

			if(((x*2)+1)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+0) = screen->machine->pens[color];

			count++;
		}
	}
	return 0;
}


/*************************
* Memory map information *
*************************/

static READ16_HANDLER( test_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( magicard_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_RAM AM_BASE(&magicram)
	AM_RANGE(0x020000, 0x020001) AM_READ(test_r)
	AM_RANGE(0x020002, 0x020003) AM_READ(test_r)
	AM_RANGE(0x040000, 0x05ffff) AM_RAM
	AM_RANGE(0x060000, 0x07ffff) AM_RAM AM_BASE(&blit_ram)
//	AM_RANGE(0x100000, 0x17ffff) AM_RAM AM_REGION("main", 0)
	AM_RANGE(0x180000, 0x1ffbff) AM_ROM AM_REGION("main", 0)
	AM_RANGE(0x1ffc00, 0x1fffff) AM_RAM
	AM_RANGE(0x260000, 0x27ffff) AM_RAM /*???*/
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( magicard )
INPUT_PORTS_END


MACHINE_RESET( magicard )
{
	UINT16 *src    = (UINT16*)memory_region( machine, "main" );
	UINT16 *dst    = magicram;
	memcpy (dst, src, 0x20000);
}

static PALETTE_INIT( magicard )
{
/*	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x100; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}*/
}

/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout mca_charlayout =
{
/* text layer is only 1bpp??
   there are 2 charsets, but very different
*/
	8, 10,
	190,
	1,		/* 1bpp? */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8, 9*8, 8*8 },
	8*10

};

static const gfx_layout mca_tilelayout =
{
/* this is only to see the GFX and find
   a proper way to decode
*/
	8, 8,
	0x8000,
	1,		/* just for testing */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( magicard )
	GFXDECODE_ENTRY( "main", 0x21562, mca_charlayout,  0, 32 )
	GFXDECODE_ENTRY( "main", 0x16470, mca_tilelayout,  0, 32 )
//	GFXDECODE_ENTRY( "main", 0x22efa, mca_tilelayout,  0, 32 )
GFXDECODE_END

static GFXDECODE_START( magicrda )
	GFXDECODE_ENTRY( "main", 0x21562 + 0x792, mca_charlayout,  0, 32 )
	GFXDECODE_ENTRY( "main", 0x16470, mca_tilelayout,  0, 32 )
//	GFXDECODE_ENTRY( "main", 0x22efa, mca_tilelayout,  0, 32 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

#if 0
/*Probably there's a mask somewhere if it REALLY uses irqs at all...irq vectors dynamically changes after some time.*/
static INTERRUPT_GEN( magicard_irq )
{
	if(input_code_pressed(KEYCODE_Z))
		cpu_set_input_line(device->machine->cpu[0], 1, HOLD_LINE);
}
#endif

static MACHINE_DRIVER_START( magicard )
	MDRV_CPU_ADD("main", M68000, CLOCK_A/2)	/* SCC-68070 CCA84 datasheet */
	MDRV_CPU_PROGRAM_MAP(magicard_mem,0)
//	MDRV_CPU_VBLANK_INT("main", magicard_irq) /* no interrupts? (it erases the vectors..) */

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 300-1)
	MDRV_PALETTE_INIT(magicard)

	MDRV_PALETTE_LENGTH(0x20000/2)
	MDRV_GFXDECODE(magicard)

	MDRV_VIDEO_START(magicard)
	MDRV_VIDEO_UPDATE(magicard)

	MDRV_MACHINE_RESET(magicard)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( magicrda )
	MDRV_IMPORT_FROM(magicard)
	MDRV_CPU_MODIFY("main")

	MDRV_GFXDECODE(magicrda)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( magicard )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code & GFX */
	ROM_LOAD16_WORD_SWAP( "magicorg.bin", 0x000000, 0x80000, CRC(810edf9f) SHA1(0f1638a789a4be7413aa019b4e198353ba9c12d9) )

//	ROM_REGION( 0x400, "boot_prg", 0 )
//	ROM_COPY( "main", 0x00000, 0x00000, 0x400)

	ROM_REGION( 0x0100, "proms", 0 ) /* Color PROM?? */
	ROM_LOAD16_WORD_SWAP("mgorigee.bin",	0x0000,	0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END

ROM_START( magicrda )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code & GFX */
	ROM_LOAD16_WORD_SWAP( "mcorigg2.bin", 0x00000, 0x20000, CRC(48546aa9) SHA1(23099a5e4c9f2c3386496f6d7f5bb7d435a6fb16) )
	ROM_LOAD16_WORD_SWAP( "mcorigg1.bin", 0x20000, 0x20000, CRC(c9e4a38d) SHA1(812e5826b27c7ad98142a0f52fbdb6b61a2e31d7) )

//	ROM_REGION( 0x400, "boot_prg", 0 )
//	ROM_COPY( "main", 0x00000, 0x00000, 0x400)

	ROM_REGION( 0x0100, "proms", 0 ) /* Color PROM?? */
	ROM_LOAD("mgorigee.bin",	0x0000,	0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

static DRIVER_INIT( magicard )
{
	UINT16 *src    = (UINT16*)memory_region( machine, "main" );
	UINT16 *dst    = magicram;
	memcpy (dst, src, 0x20000);
}

/*    YEAR  NAME      PARENT MACHINE   INPUT  INIT  ROT    COMPANY   FULLNAME             FLAGS... */

GAME( 199?, magicard, 0,     magicard, 0,     magicard,    ROT0, "Impera", "Magic Card (set 1)", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 199?, magicrda, 0,     magicrda, 0,     magicard,    ROT0, "Impera", "Magic Card (set 2)", GAME_NO_SOUND | GAME_NOT_WORKING )
