/*******************************************************************************************

Il Pagliaccio (c) 19?? unknown

preliminary driver by David Haywood & Angelo Salese

Notes:
-at start-up a "initialize request" pops-up,press Service Mode and the "Init eeprom?" button,and
reset with F3 for doing it.

TODO:
-Protection not yet checked at all.My raw guess is that covers input ports or Work RAM
 addresses;
-This game runs all the program snippets by irq callings,check / implement proper irq
 generation;
-Video emulation,blitter based;

CPU

1x MC68HC000FN12 (main)
1x XC95144XL
2x XC9572XL
1x oscillator 11.0592MHz
1x blue resonator CSB400P

ROMs

4x W27E040
1x P87C748EBPN (read protected)


Note

1x 28x2 edge connector (not JAMMA)
1x trimmer (volume)
1x battery
2x pushbutton (MANAGEMENT,STATISTIC)

*******************************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"

static UINT16 *blit_romaddr,*blit_attr1_ram,*blit_dst_ram_loword,*blit_attr2_ram,*blit_dst_ram_hiword;
static UINT8 *blit_buffer;

VIDEO_START(ilpag)
{
	blit_buffer = auto_malloc(512*512*4); //just to be sure,number is wrong
}

VIDEO_UPDATE(ilpag)
{
	int x,y;
//	static int counter = 0;
	int count;

	UINT8 *blit_rom = memory_region(screen->machine, "blit_data");
//	printf("counter %04x\n", counter);

	blit_rom = blit_buffer;

	count = 0;


	for(y=0;y<512;y++)
	{
		for(x=0;x<512;x++)
		{
			UINT32 color;
			color = (blit_rom[count] & 0xff);

			if(x<video_screen_get_visible_area(screen)->max_x && y<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, x) = screen->machine->pens[color];

			count++;
		}
	}
//	counter++;

	return 0;
}

static WRITE16_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs,pal_data;

	switch(offset*2)
	{
		case 4:
			pal_offs = 0;
			break;
		case 0:
			internal_pal_offs = 0;
			break;
		case 2:
			switch(internal_pal_offs)
			{
				case 0:
					pal_data = (data & 0xff00) >> 8;
					r = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					pal_data = (data & 0xff00) >> 8;
					g = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					pal_data = (data & 0xff00) >> 8;
					b = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

static WRITE16_HANDLER( blit_copy_w )
{
	UINT8 *blit_rom = memory_region(space->machine, "blit_data");
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;


	int x,y,x_size,y_size, src;;

	printf("blit copy? %04x %04x %04x %04x %04x %04x\n", data, blit_romaddr[0], blit_attr1_ram[0], blit_dst_ram_loword[0], blit_attr2_ram[0], blit_dst_ram_hiword[0] );


	blit_dst_xpos = (blit_dst_ram_loword[0] & 0x00ff)*2;
	blit_dst_ypos = ((blit_dst_ram_loword[0] & 0xff00)>>8)*2;
	//                                          ff00 = ypos?

//	blit_dst_xpos|= (blit_dst_ram_hiword[0] & 0xffff)<<16;

	y_size = 8;// blit_romaddr[0] & 0x00ff;
	x_size = 14;//blit_romaddr[0] & 0xff00 >> 8;

	src = blit_romaddr[0];


	for(y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			int drawx = (blit_dst_xpos+x)&0x1ff;
			int drawy = (blit_dst_ypos+y)&0x1ff;

			blit_buffer[drawy*512+drawx] = blit_rom[src];
			src+=1;
		}

	}
			//blit_buffer[blit_dst_xpos+(x*0x100)+(y)] = blit_ram[blit_romaddr[0]+(x*0x100)+(y)];
}

#if 0
static READ16_HANDLER( unknown_read )
{
	return mame_rand(space->machine);
}
#endif

static READ16_HANDLER( blitter_status_r )
{
	return 0;
}

#if 0
static UINT16 t1,t2;

static READ16_HANDLER( unk_latch_1_r )
{
	return t1;
}

static READ16_HANDLER( unk_latch_2_r )
{
	return t2;
}

static WRITE16_HANDLER( unk_latch_1_w )
{
	t1 = data;
}

static WRITE16_HANDLER( unk_latch_2_w )
{
	t2 = data;
}
#endif

static ADDRESS_MAP_START( ilpag_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blit_data", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

//	AM_RANGE(0x800000, 0x800001) AM_READ(unk_latch_1_r)
//	AM_RANGE(0x880000, 0x880001) AM_READ(unk_latch_2_r)

	AM_RANGE(0x225000, 0x225fff) AM_RAM // NVRAM?
	AM_RANGE(0x900000, 0x900005) AM_WRITE( paletteram_io_w )
	AM_RANGE(0x980006, 0x980007) AM_WRITE( blit_copy_w )
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_BASE(&blit_romaddr)
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_BASE(&blit_attr1_ram)
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_BASE(&blit_dst_ram_loword)
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_BASE(&blit_attr2_ram)
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM AM_BASE(&blit_dst_ram_hiword)
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)

	AM_RANGE(0xc00180, 0xc00181) AM_READ_PORT("IN2")
//	AM_RANGE(0xc00200, 0xc00201) AM_WRITE(unk_latch_1_w)
	AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//	AM_RANGE(0xc002d0, 0xc002d1) AM_WRITE(unk_latch_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( ilpag )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Init eeprom?" )//enable this for the initialize eeprom thing
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN3")
	PORT_DIPNAME( 0x0001, 0x0001, "IN3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "??? enable this" )//<- this puts the game on the "hi score" screen
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INTERRUPT_GEN( ilpag_hackint )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, 3, HOLD_LINE);
			break;

		case 1:
			cpu_set_input_line(device, 4, HOLD_LINE);
			break;

		case 2:
			cpu_set_input_line(device, 6, HOLD_LINE);
			break;


	}

}


static MACHINE_DRIVER_START( ilpag )
	MDRV_CPU_ADD("main", M68000, 8000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(ilpag_map,0)
	MDRV_CPU_VBLANK_INT_HACK(ilpag_hackint,3)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(ilpag)
	MDRV_VIDEO_UPDATE(ilpag)
MACHINE_DRIVER_END




ROM_START( ilpag )
	ROM_REGION( 0x200000, "main", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.7c-35.u32", 0x000000, 0x80000, CRC(ed99c884) SHA1(b3d2c9fb7765e3c8ff1e0de9c8edb6628e1c79ef) )
	ROM_LOAD16_BYTE( "2.7c-36.u31", 0x000001, 0x80000, CRC(4cd41688) SHA1(a1a15b06aa738cd4154d3c3479a7bf2da0e48426) )

	ROM_REGION( 0x100000, "blit_data", 0 ) // data for the blitter
	ROM_LOAD( "graf2.u51",   0x080000, 0x80000, CRC(2d64d3b5) SHA1(8fdb943d0aedf12706ce0a772c8f5155fa03e8c7) )
	ROM_LOAD( "graf1.u46",   0x000000, 0x80000, CRC(cf745964) SHA1(7af4a6c0b8d01c0d1b71bc5330a257d2fa712611) )

	ROM_REGION( 0x800, "mcu", 0 ) // MCU code
	ROM_LOAD( "87c748.u132", 0x000, 0x800, NO_DUMP )
ROM_END


GAME( 19??, ilpag,    0,        ilpag,    ilpag,    0, ROT0,  "unknown", "Il Pagliaccio", GAME_NOT_WORKING | GAME_NO_SOUND )
