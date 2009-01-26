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

-Inputs;

-Unknown sound chip (but seems to be a ym-something with 8-bits wide as
 control port);

-Visible Area;

-Protection,especially in magicrda;

-Many unknown memory maps;

*******************************************************************************/


#define CLOCK_A	XTAL_30MHz
#define CLOCK_B	XTAL_19_6608MHz

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/2413intf.h"

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

static WRITE16_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset*2)
	{
		case 0:
			pal_offs = data;
			break;
		case 4:
			internal_pal_offs = 0;
			break;
		case 2:
			switch(internal_pal_offs)
			{
				case 0:
					r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

static ADDRESS_MAP_START( magicard_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_RAM AM_BASE(&magicram)
	AM_RANGE(0x020000, 0x03ffff) AM_READ(test_r) AM_WRITENOP
	AM_RANGE(0x040000, 0x05ffff) AM_RAM
	AM_RANGE(0x060000, 0x07ffff) AM_RAM AM_BASE(&blit_ram)
//  AM_RANGE(0x100000, 0x17ffff) AM_RAM AM_REGION("main", 0)
	AM_RANGE(0x180000, 0x1ffbff) AM_ROM AM_REGION("main", 0)
	AM_RANGE(0x1ffc00, 0x1ffc01) AM_READ(test_r) //protection read,magicrda tests here a lot
	AM_RANGE(0x1ffc40, 0x1ffc41) AM_READ(test_r)
	AM_RANGE(0x1ffd00, 0x1ffd05) AM_WRITE(paletteram_io_w) //RAMDAC
	AM_RANGE(0x1ffd40, 0x1ffd41) AM_WRITE(ym2413_register_port_0_lsb_w) //not the right sound chip,unknown type.
	AM_RANGE(0x1ffd42, 0x1ffd43) AM_WRITE(ym2413_data_port_0_lsb_w)
	AM_RANGE(0x1ffd80, 0x1ffd81) AM_READ(test_r)
	AM_RANGE(0x1ffd80, 0x1ffd81) AM_WRITENOP //?
	AM_RANGE(0x1fffe0, 0x1fffe1) AM_READ(test_r)
	AM_RANGE(0x1ffff0, 0x1ffff1) AM_WRITENOP //protection?
	AM_RANGE(0x1ffff2, 0x1fffff) AM_WRITENOP //?
	AM_RANGE(0x200000, 0x27ffff) AM_RAM AM_REGION("main",0)//protection ram?
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
	device_reset(cputag_get_cpu(machine, "main"));
}

/*************************
*    Machine Drivers     *
*************************/

/*Probably there's a mask somewhere if it REALLY uses irqs at all...irq vectors dynamically changes after some time.*/
static INTERRUPT_GEN( magicard_irq )
{
//	if(input_code_pressed(KEYCODE_Z))
//		cpu_set_input_line(device->machine->cpu[0], 1, HOLD_LINE);
//	magicram[0x2004/2]^=0xffff;
}

static MACHINE_DRIVER_START( magicard )
	MDRV_CPU_ADD("main", M68000, CLOCK_A/2)	/* SCC-68070 CCA84 datasheet */
	MDRV_CPU_PROGRAM_MAP(magicard_mem,0)
 	MDRV_CPU_VBLANK_INT("main", magicard_irq) /* no interrupts? (it erases the vectors..) */

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1) //might be 320x240

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(magicard)
	MDRV_VIDEO_UPDATE(magicard)

	MDRV_MACHINE_RESET(magicard)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym", YM2413, CLOCK_A/12)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( magicrda )
	MDRV_IMPORT_FROM(magicard)
	MDRV_CPU_MODIFY("main")


MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( magicard )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code & GFX */
	ROM_LOAD16_WORD_SWAP( "magicorg.bin", 0x000000, 0x80000, CRC(810edf9f) SHA1(0f1638a789a4be7413aa019b4e198353ba9c12d9) )

//  ROM_REGION( 0x400, "boot_prg", 0 )
//  ROM_COPY( "main", 0x00000, 0x00000, 0x400)

	ROM_REGION( 0x0100, "proms", 0 ) /* Color PROM?? */
	ROM_LOAD16_WORD_SWAP("mgorigee.bin",	0x0000,	0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END

ROM_START( magicrda )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code & GFX */
	ROM_LOAD16_WORD_SWAP( "mcorigg2.bin", 0x00000, 0x20000, CRC(48546aa9) SHA1(23099a5e4c9f2c3386496f6d7f5bb7d435a6fb16) )
	ROM_LOAD16_WORD_SWAP( "mcorigg1.bin", 0x20000, 0x20000, CRC(c9e4a38d) SHA1(812e5826b27c7ad98142a0f52fbdb6b61a2e31d7) )

//  ROM_REGION( 0x400, "boot_prg", 0 )
//  ROM_COPY( "main", 0x00000, 0x00000, 0x400)

	ROM_REGION( 0x0100, "proms", 0 ) /* Color PROM?? */
	ROM_LOAD("mgorigee.bin",	0x0000,	0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

static DRIVER_INIT( magicard )
{
	//...
}

/*    YEAR  NAME      PARENT MACHINE   INPUT  INIT  ROT    COMPANY   FULLNAME             FLAGS... */

GAME( 199?, magicard, 0,     magicard, 0,     magicard,    ROT0, "Impera", "Magic Card (set 1)", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 199?, magicrda, 0,     magicrda, 0,     magicard,    ROT0, "Impera", "Magic Card (set 2)", GAME_NO_SOUND | GAME_NOT_WORKING )
