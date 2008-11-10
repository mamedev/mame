/* Strip Teaser
 this has *2* 68705 MCUs..  neither dumped

Strip Teaser (unknown manufacturer)
------------------------------------

lower board (Dk-B)

TS68000CP12 (main cpu)
osc. 11.0592MHz
MC68HC705C8P (MCU)
UM6845RA (CRT controller-Supports alphanumeric and graphics modes.Addresses up to 16 KB of video memory-2 MHz)
Lithium battery 3,6V


upper board (8L74) (soundboard?)

MC68HC705C8P (MCU)
osc. 4.0000MHz
non JAMMA connector
1x dipswitch (4 switch)


ROMs
1x AT27c010 (u31.1)(program)
1x AM27C010 (u32.6)(program)
4x M27C4001 (u46.2 - u51.3 - u61.4 - u66.5)(GFX)
1x M27C4001 (u18.7)(sound)


2008.07.01 - Tomasz Slanina - preliminary gfx blitter emulation

TODO:
- fix blitter writes (missing fill ?)
- controls (probably one of the MCUS is used to read controls)
- sound

*/


#include "driver.h"
#include "deprecat.h"


static int clr_offset=0;

static UINT16 *mainram;

static UINT16 data_9a0000, data_998000, data_9b0000, data_9a8000, data_990000, data_988000;


static WRITE16_HANDLER(blitter_9b8000_w)
{
	UINT8 *rom = memory_region(machine, "gfx1");
	int len=memory_region_length(machine, "gfx1");
	int w = video_screen_get_width(machine->primary_screen);
	int h = video_screen_get_height(machine->primary_screen);
	int x,y;
	UINT16 pix;

	int x0=(data_9a8000&0xff)<<1;
	int y0=data_9a8000>>8;
	int dx=data_9b0000&0xff;
	int dy=((data_9b0000>>8)^0xff)+1;
	UINT32 of=data_998000|(data_9a0000<<8);

	if(of==0)
	{
		fillbitmap(tmpbitmap,get_black_pen(machine),0);
		return;
	}

	if(dx==0)
	{
		dx=0x100;
	}

	dx<<=1;

#if 0
	printf("[%x] %x blit %x -> %x,%x  (%x*%x ) [%x] %x - %.4x %.4x - %.4x - %.4x\n",data,cpu_get_pc(machine->activecpu),of,x0,y0,dx,dy,data_9b0000,data_9a8000,data_9a0000,data_998000,data_990000,data_988000);
#endif

	for(y=0;y<dy;++y)
	{
		for(x=0;x<dx;++x)
		{
			if(of<len && y+y0<h && x+x0<w)
			{
				pix=*(rom+of);

				if(pix==0)
				{
					pix=0x100; //dirty hack
				}


				{
					*BITMAP_ADDR16(tmpbitmap, y+y0, x+x0) = pix;
				}
			}
			++of;
		}
	}
}



static VIDEO_START(steaser)
{
	tmpbitmap = auto_bitmap_alloc(32*8*2,32*8*2,video_screen_get_format(machine->primary_screen));
}

static VIDEO_UPDATE(steaser)
{
	copybitmap(bitmap,tmpbitmap,0,0,0,0,cliprect);
	return 0;
}

static READ16_HANDLER( steaser_bd0000_r )
{
	return 0xffff;
}

static READ16_HANDLER( steaser_9e0000_r )
{
	return 0;
}

static WRITE16_HANDLER(mcu_w)
{
	//communciation with MCU ?
}

static WRITE16_HANDLER(unk_w)
{
	switch(cpu_get_pc(machine->activecpu))
	{
		case 0x1348: mainram[0x00932/2]=0xffff; break; //dirt way to exit loop
/*      case 0x16ce:
        {
            int addr=(cpu_get_reg(machine->activecpu, M68K_D2)+0x089c)>>1;
            mainram[addr&0x7fff]=0xffff;//mame_rand(machine);
        }
        break;*/
	}
}

static WRITE16_HANDLER(blitter_990000_w)
{
	COMBINE_DATA(&data_990000);
}

static WRITE16_HANDLER(blitter_998000_w)
{
	COMBINE_DATA(&data_998000);
}

static WRITE16_HANDLER(blitter_9a0000_w)
{
	COMBINE_DATA(&data_9a0000);
}

static WRITE16_HANDLER(blitter_9a8000_w)
{
	COMBINE_DATA(&data_9a8000);
}

static WRITE16_HANDLER(blitter_9b0000_w)
{
	COMBINE_DATA(&data_9b0000);
}

static WRITE16_HANDLER(blitter_988000_w)
{
	COMBINE_DATA(&data_988000);
}

static WRITE16_HANDLER( color_offset_w )
{
		data>>=8;
		clr_offset=data*3;
}

static WRITE16_HANDLER( color_data_w )
{
		data>>=8;
		colorram[clr_offset]=data;
		palette_set_color_rgb(machine,clr_offset/3,pal6bit(colorram[(clr_offset/3)*3]),pal6bit(colorram[(clr_offset/3)*3+1]),pal6bit(colorram[(clr_offset/3)*3+2]));
		clr_offset=(clr_offset+1)%768;
}


static ADDRESS_MAP_START( steaser_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xbd0000, 0xbd0001) AM_READ( steaser_bd0000_r )
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&mainram)
	AM_RANGE(0x800000, 0x800001) AM_READ( steaser_bd0000_r )
	AM_RANGE(0x880000, 0x880001) AM_READ( steaser_bd0000_r )
	AM_RANGE(0x840000, 0x840001) AM_WRITE(mcu_w)
//  AM_RANGE(0x8c0000, 0x8c0001) AM_WRITENOP
//  AM_RANGE(0x900000, 0x900003) AM_RAM

	AM_RANGE(0x900000, 0x900001) AM_WRITE(color_offset_w)
	AM_RANGE(0x900002, 0x900003) AM_WRITE(color_data_w)
	AM_RANGE(0x900004, 0x900005) AM_WRITENOP
	AM_RANGE(0x940000, 0x940001) AM_WRITENOP
	AM_RANGE(0x980000, 0x98000f) AM_WRITENOP //blitter related
	AM_RANGE(0x988000, 0x988001) AM_WRITE(blitter_988000_w)
	AM_RANGE(0x990000, 0x990001) AM_WRITE(blitter_990000_w)
	AM_RANGE(0x998000, 0x998001) AM_WRITE(blitter_998000_w)
	AM_RANGE(0x9a0000, 0x9a0001) AM_WRITE(blitter_9a0000_w)
	AM_RANGE(0x9a8000, 0x9a8001) AM_WRITE(blitter_9a8000_w)
	AM_RANGE(0x9b0000, 0x9b0001) AM_WRITE(blitter_9b0000_w)
	AM_RANGE(0x9b8000, 0x9b8001) AM_WRITE(blitter_9b8000_w)
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ( steaser_9e0000_r )
	AM_RANGE(0x9f0000, 0x9f0001) AM_WRITE(unk_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( steaser )
INPUT_PORTS_END



static INTERRUPT_GEN( steaser_irq )
{
	int num=cpu_getiloops()+3;
	cpunum_set_input_line(machine, 0, num, HOLD_LINE);
}

static MACHINE_DRIVER_START( steaser )
	MDRV_CPU_ADD("main", M68000, 11059200 )
	MDRV_CPU_PROGRAM_MAP(steaser_map,0)
	MDRV_CPU_VBLANK_INT_HACK(steaser_irq,4)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8*2, 32*8*2)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 2*32*8-1, 0*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(steaser)
	MDRV_VIDEO_UPDATE(steaser)
MACHINE_DRIVER_END


ROM_START( steaser )
	ROM_REGION( 0x40000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u31.1", 0x00001, 0x20000, CRC(7963e960) SHA1(2a1c68265e0a3909ccd097ea784e3e179f528844) )
	ROM_LOAD16_BYTE( "u32.6", 0x00000, 0x20000, CRC(c0ab5fb1) SHA1(15b3dbf0242e885b7009c21479544a821d4e5a7d) )

	ROM_REGION( 0x1000, "cpu1", 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_main.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_sub.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "samples", 0 ) /* Sound Samples */
	ROM_LOAD( "u18.7", 0x00000, 0x80000, CRC(ee942232) SHA1(b9c1fc73c6006bcad0dd177e0f30a96f1063a993) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "u46.2", 0x000000, 0x80000, CRC(c4a5e47b) SHA1(9f3d3124c76c0bdf8cdca849e1d921a335e433b6) )
	ROM_LOAD( "u51.3", 0x080000, 0x80000, CRC(4dc57435) SHA1(7dfa6f9e35986dd48869786abbe70103f336bcb1) )
	ROM_LOAD( "u61.4", 0x100000, 0x80000, CRC(d8d8dc6f) SHA1(5a76b1fd1a3a532e5ff2de127286ace7d3567c58) )
	ROM_LOAD( "u66.5", 0x180000, 0x80000, CRC(da309671) SHA1(66baf8a83024547c471da39748ff99a9a9013ea4) )
ROM_END

static DRIVER_INIT(steaser)
{
	colorram=auto_malloc(768);
}

GAME( 1993, steaser, 0, steaser, steaser, steaser, ROT0,  "Unknown", "Strip Teaser (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
