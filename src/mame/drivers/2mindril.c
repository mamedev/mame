/*
 Two Minute Drill - Taito 1993
 -----------------------------
 Half Video, Half Mechanical?
(video hw + motion/acceleration sensor ?)

 preliminary driver by
  David Haywood
  Tomasz Slanina

TODO:
 - simulate the sensors (and remove rom hack)
 - controls/dips
 - find video control regs (layer enable, scroll)

BG scroll:
BG maps are 2048x256 (128x16 16x16 tiles).
There's some kind of double buffering - odd/even screens are at
x offsets 0 and 512 (it's visible during distance count after throw (odd/even numbers))

looks like regs @460000 are used,  pairs at N, and N+8, so
460000, 460008
460002, 46000a
460004, 46000c
460006, 46000e

*/

/*

TWO MINUTE DRILL - Taito 1993?

No idea what this game is... I do not have the pinout
See pic for more details

 Brief hardware overview:
 ------------------------

 Main processor   - 68000 16Mhz

 Sound            - Yamaha YM2610B

 Taito custom ICs - TC0400YSC
                  - TC0260DAR
                  - TC0630FDP
                  - TC0510NI0

DAC               -26.6860Mhz
                        -32.0000Mhz

*/

#include "driver.h"
#include "sound/2610intf.h"

static UINT16 *unkram;

static UINT16 *map1ram;
static UINT16 *map2ram;
static UINT16 *map3ram;
static UINT16 *map4ram;
static UINT16 *charram;
static UINT16 *textram;

#define DRAW_MAP(map,num) 	{ 	int x,y; \
			  	for(y=0;y<16;y++) \
	 				for(x=0;x<128;x++) \
	 				{ \
	 					UINT16 data0=map[y*128+x*2]; \
	 					UINT16 data1=map[y*128+x*2+1]; \
	 					drawgfx(bitmap,screen->machine->gfx[0], data1, \
		 					data0&0xff, \
							data0&0x4000, data0&0x8000, \
							x*16-512/*+(((INT16)(unkram[0x60000/2+num]))/32)*/, y*16/*+(((INT16)(unkram[0x60008/2+num]))/32)*/, \
							cliprect,TRANSPARENCY_PEN,0); \
	 				}	\
			}

static VIDEO_UPDATE( drill )
{
	int i;
	fillbitmap(bitmap,0,NULL);

	for (i=0; i<256; i++)
	{
		decodechar(screen->machine->gfx[1],i,(UINT8*)&charram[0]);
	}

	DRAW_MAP(map1ram,0)
	DRAW_MAP(map2ram,1)
	DRAW_MAP(map3ram,2)
	DRAW_MAP(map4ram,3)


	{
		int x,y;
		for(y=0;y<64;y++)
	 		for(x=0;x<64;x++)
	 		{
	 			drawgfx(	bitmap,
						screen->machine->gfx[1],
						textram[y*64+x]&0xff, //1ff ??
						((textram[y*64+x]>>9)&0xf),
						0, 0,
						x*8,y*8,
						cliprect,
						TRANSPARENCY_PEN,0);
	 		}
	}
	//printf("%.4X %.4X %.4X %.4X %.4X %.4X\n", unkram[0x60000/2],unkram[0x60000/2+1],unkram[0x60000/2+2],unkram[0x60000/2+3],unkram[0x60000/2+4],unkram[0x60000/2+5]);
	return 0;
}

static VIDEO_START( drill )
{
	machine->gfx[0]->color_granularity=16;
}

static READ16_HANDLER( drill_unk_r )
{
	return 0xffff;
}

static ADDRESS_MAP_START( drill_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x3000ff) AM_RAM
	AM_RANGE(0x410000, 0x411fff) AM_RAM AM_BASE(&map1ram)
	AM_RANGE(0x412000, 0x413fff) AM_RAM AM_BASE(&map2ram)
	AM_RANGE(0x414000, 0x415fff) AM_RAM AM_BASE(&map3ram)
	AM_RANGE(0x416000, 0x417fff) AM_RAM AM_BASE(&map4ram)
	AM_RANGE(0x41c000, 0x41dfff) AM_RAM AM_BASE(&textram)
	AM_RANGE(0x41e000, 0x41ffff) AM_RAM AM_BASE(&charram)
	AM_RANGE(0x400000, 0x4fffff) AM_RAM AM_BASE(&unkram)// video stuff, 460000 - video regs ?
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x502000, 0x503fff) AM_RAM
	AM_RANGE(0x700000, 0x70000f) AM_READ(drill_unk_r) AM_WRITE(SMH_NOP) // i/o
	AM_RANGE(0x600000, 0x600001) AM_READ(YM2610_status_port_0_A_lsb_r) AM_WRITE(YM2610_control_port_0_A_lsb_w)
	AM_RANGE(0x600002, 0x600003) AM_READ(YM2610_read_port_0_lsb_r) AM_WRITE(YM2610_data_port_0_A_lsb_w)
	AM_RANGE(0x600004, 0x600005) AM_READ(YM2610_status_port_0_B_lsb_r) AM_WRITE(YM2610_control_port_0_B_lsb_w)
	AM_RANGE(0x600006, 0x600007) AM_WRITE(YM2610_data_port_0_B_lsb_w)
	AM_RANGE(0x60000c, 0x60000d) AM_READ(SMH_NOP) AM_WRITE(SMH_NOP)
	AM_RANGE(0x60000e, 0x60000f) AM_READ(SMH_NOP) AM_WRITE(SMH_NOP)
ADDRESS_MAP_END

static INPUT_PORTS_START( drill )
INPUT_PORTS_END

static const gfx_layout drill_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{ RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 ,0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8,        52, 48, 60, 56, 36, 32, 44, 40 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout vramlayout=
{
    8,8,
    256,
    4,
    { 0, 1, 2, 3 },
    {20,16,28,24,4,0,12,8},
	  { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
    32*8
};

static GFXDECODE_START( 2mindril )
	GFXDECODE_ENTRY( REGION_GFX1, 0, drill_layout,  0, 256  )
	GFXDECODE_ENTRY( 0,		   	0, vramlayout,   0, 256 )
GFXDECODE_END


static INTERRUPT_GEN( drill_interrupt )
{
	cpunum_set_input_line(machine, 0, 4, HOLD_LINE);
}

static void irqhandler(running_machine *machine, int irq)
{
	cpunum_set_input_line(machine, 0,5,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2610interface ym2610_interface =
{
	irqhandler,
	0,
	REGION_SOUND1
};

static MACHINE_DRIVER_START( drill )
	MDRV_CPU_ADD("main", M68000, 16000000 )
	MDRV_CPU_PROGRAM_MAP(drill_map,0)
	MDRV_CPU_VBLANK_INT("main", drill_interrupt)
	MDRV_GFXDECODE(2mindril)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(128*16, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239-16)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(drill)
	MDRV_VIDEO_UPDATE(drill)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("ym", YM2610, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_interface)
	MDRV_SOUND_ROUTE(0, "left",  0.25)
	MDRV_SOUND_ROUTE(0, "right", 0.25)
	MDRV_SOUND_ROUTE(1, "left",  1.0)
	MDRV_SOUND_ROUTE(2, "right", 1.0)
MACHINE_DRIVER_END


ROM_START( 2mindril )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d58-38.ic11", 0x00000, 0x40000, CRC(c58e8e4f) SHA1(648db679c3bfb5de1cd6c1b1217773a2fe56f11b) )
	ROM_LOAD16_BYTE( "d58-37.ic9",  0x00001, 0x40000, CRC(19e5cc3c) SHA1(04ac0eef893c579fe90d91d7fd55c5741a2b7460) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "d58-11.ic31", 0x000000, 0x200000,  CRC(dc26d58d) SHA1(cffb18667da18f5367b02af85a2f7674dd61ae97) )

	ROM_REGION( 0x800000, REGION_GFX1, ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "d58-09.ic28", 0x000000, 0x200000, CRC(d8f6a86a) SHA1(d6b2ec309e21064574ee63e025ae4716b1982a98) )
	ROM_LOAD32_WORD( "d58-08.ic27", 0x000002, 0x200000, CRC(9f5a3f52) SHA1(7b696bd823819965b974c853cebc1660750db61e) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 )
	ROM_LOAD32_WORD( "d58-10.ic29", 0x000000, 0x200000, CRC(74c87e08) SHA1(f39b3a64f8338ccf5ca6eb76cee92a10fe0aad8f) )
ROM_END

static DRIVER_INIT( drill )
{
	// rearrange gfx roms to something we can decode, two of the roms form 4bpp of the graphics, the third forms another 2bpp but is in a different format
	UINT32 *src = (UINT32*)memory_region( machine, REGION_GFX2 );
	UINT32 *dst = (UINT32*)memory_region( machine, REGION_GFX1 );// + 0x400000;
	UINT8 *rom = memory_region( machine, REGION_CPU1 );
	int i;

	for (i=0; i< 0x400000/4; i++)
	{
		UINT32 dat1 = src[i];
	    dat1 = BITSWAP32(dat1, 3, 11, 19, 27, 2, 10, 18, 26, 1, 9, 17, 25, 0, 8, 16, 24, 7, 15, 23, 31, 6, 14, 22, 30, 5, 13, 21, 29, 4, 12, 20, 28 );
		dst[(0x400000/4)+i] = dat1;
	}

	//enable some kind of debug mode (ignore errors)
	rom[0x7fffb]=0;
	rom[0x7fffc]=0;
	rom[0x7fffd]=0;
	rom[0x7fffe]=0;
}

GAME( 1993, 2mindril,    0,        drill,    drill,    drill, ROT0,  "Taito", "Two Minute Drill", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
