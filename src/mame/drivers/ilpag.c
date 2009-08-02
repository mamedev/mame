/*************************************************************************************************************

Il Pagliaccio (c) 19?? unknown
Strip Teaser (c) 1993 unknown

preliminary driver by David Haywood & Angelo Salese
original steaser.c driver by David Haywood & Tomasz Slanina

Notes:
-(ilpag) at start-up a "initialize request" pops-up, press Service Mode and the Service switch, and
reset with F3 for doing it.

TODO:
-ilpag: Protection not yet checked at all.My guess it's that communicates via irq 3 and/or 6;
-steaser: Understand the shared ram inputs better and find the coin chutes;
-Some minor issues with the blitter emulation;
-ilpag: Sound is composed by a DAC (MP7524 is an 8-bit DAC (bottom right, by the edge connector -PJB)),
but the MCU controls the sound writes?
-steaser: Sound is composed by an OkiM6295 (controlled by the sub MCU), check if it can be
simulated;

===============================================================================================================
Il Pagliaccio (unknown manufacturer)
------------------------------------
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

===============================================================================================================
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

*****************************************************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "deprecat.h"

static UINT16 *blit_romaddr,*blit_attr1_ram,*blit_dst_ram_loword,*blit_attr2_ram,*blit_dst_ram_hiword,*blit_vregs,*blit_transpen;
static UINT8 *blit_buffer;

static VIDEO_START(ilpag)
{
	blit_buffer = auto_alloc_array(machine, UINT8, 512*512*4); //just to be sure,number is wrong
}

static VIDEO_UPDATE(ilpag)
{
	int x,y;
	int count;

	UINT8 *blit_rom = memory_region(screen->machine, "blit_data");

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

	return 0;
}

static WRITE16_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs,pal_data;

	switch(offset*2)
	{
		case 0:
			pal_offs = (data & 0xff00) >> 8;
			break;
		case 4:
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

/*
(TODO: register names should be properly renamed)
"transpen" 8/2 is for layer clearance;
"transpen" 10/2 is the trasparency pen;
"vregs" are pen selects, they select the proper color to render, and are tied to the first three gfx offsets;
*/
/*
Blitter TODO:
-shrinking bit? (some pictures in steaser, "00" on top of ilpag)
-draw direction (card choose in steaser, in service mode)
-line draw? (for the "Game Over" msg in steaser)
-"random" pens? (9d0000 read in steaser)
-(anything else?)
*/
static WRITE16_HANDLER( blit_copy_w )
{
	UINT8 *blit_rom = memory_region(space->machine, "blit_data");
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;
	int x,y,x_size,y_size;
	UINT32 src;

	printf("blit copy %04x %04x %04x %04x %04x\n", blit_romaddr[0], blit_attr1_ram[0], blit_dst_ram_loword[0], blit_attr2_ram[0], blit_dst_ram_hiword[0] );
	printf("blit vregs %04x %04x %04x %04x\n",blit_vregs[0/2],blit_vregs[2/2],blit_vregs[4/2],blit_vregs[6/2]);
	printf("blit transpen %04x %04x %04x %04x %04x %04x %04x %04x\n",blit_transpen[0/2],blit_transpen[2/2],blit_transpen[4/2],blit_transpen[6/2],
	                                                               blit_transpen[8/2],blit_transpen[10/2],blit_transpen[12/2],blit_transpen[14/2]);

	blit_dst_xpos = (blit_dst_ram_loword[0] & 0x00ff)*2;
	blit_dst_ypos = ((blit_dst_ram_loword[0] & 0xff00)>>8);

	y_size = (0x100-((blit_attr2_ram[0] & 0xff00)>>8));
	x_size = (blit_attr2_ram[0] & 0x00ff)*2;

	/* rounding around for 0 size */
	if(x_size == 0) { x_size = 0x200; }

	/* TODO: used by steaser "Game Over" msg on attract mode*/
//  if(y_size == 1) { y_size = 32; }

	src = blit_romaddr[0] | (blit_attr1_ram[0] & 0x1f00)<<8;
//  src|= (blit_transpen[0xc/2] & 0x0100)<<12;

	for(y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			int drawx = (blit_dst_xpos+x)&0x1ff;
			int drawy = (blit_dst_ypos+y)&0x1ff;
			if(blit_transpen[0x8/2] & 0x100)
				blit_buffer[drawy*512+drawx] = ((blit_vregs[0] & 0xf00)>>8);
			else
			{
				UINT8 pen_helper;

				pen_helper = blit_rom[src] & 0xff;
				if(blit_transpen[0xa/2] & 0x100) //pen is opaque register
				{
					if(pen_helper)
						blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
				}
				else
					blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
			}

			src++;
		}
	}
}

/*bit 0 is the blitter busy flag*/
static READ16_HANDLER( blitter_status_r )
{
	return 0;
}

/*TODO*/
static WRITE16_HANDLER( lamps_w )
{
//  popmessage("%02x",data);
}

static READ16_HANDLER( test_r )
{
	return 0xffff;//mame_rand(space->machine);
}

#if 0
static WRITE16_HANDLER( irq_callback_w )
{
//  popmessage("%02x",data);
	cputag_set_input_line(space->machine, "maincpu", 3, HOLD_LINE );
}

static WRITE16_HANDLER( sound_write_w )
{
	popmessage("%02x",data);
	dac_data_w(0, data & 0x0f);		/* Sound DAC? */
}
#endif

static ADDRESS_MAP_START( ilpag_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blit_data", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)

//  AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x880000, 0x880001) AM_READ(test_r)

	AM_RANGE(0x900000, 0x900005) AM_WRITE( paletteram_io_w ) //RAMDAC
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_BASE(&blit_transpen) //video registers for the blitter write
	AM_RANGE(0x990000, 0x990007) AM_RAM AM_BASE(&blit_vregs) //pens
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_BASE(&blit_romaddr)
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_BASE(&blit_attr1_ram)
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_BASE(&blit_dst_ram_loword)
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_BASE(&blit_attr2_ram)
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE( blit_copy_w ) AM_BASE(&blit_dst_ram_hiword)
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)

	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xc00180, 0xc00181) AM_READ_PORT("IN2")
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
	AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( steaser_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blit_data", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)

 	AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x840000, 0x840001) AM_WRITE(sound_write_w)
	AM_RANGE(0x880000, 0x880001) AM_READ(test_r)
//  AM_RANGE(0x8c0000, 0x8c0001) AM_WRITE(sound_write_w)

	AM_RANGE(0x900000, 0x900005) AM_WRITE( paletteram_io_w ) //RAMDAC
	AM_RANGE(0x940000, 0x940001) AM_WRITENOP //? Seems a dword write for some read, written consecutively
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_BASE(&blit_transpen)//probably transparency pens
	AM_RANGE(0x990000, 0x990005) AM_RAM AM_BASE(&blit_vregs)
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_BASE(&blit_romaddr)
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_BASE(&blit_attr1_ram)
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_BASE(&blit_dst_ram_loword)
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_BASE(&blit_attr2_ram)
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE( blit_copy_w ) AM_BASE(&blit_dst_ram_hiword)
	AM_RANGE(0x9c0002, 0x9c0003) AM_READNOP //pen control?
	AM_RANGE(0x9d0000, 0x9d0001) AM_READNOP //?
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)
	AM_RANGE(0x9f0000, 0x9f0001) AM_WRITENOP //???

//  AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xbd0000, 0xbd0001) AM_READ(test_r)
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
//  AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( ilpag )
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
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
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
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Gioco (Bet)")
	PORT_DIPNAME( 0x0020, 0x0020, "IN3" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
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

static INPUT_PORTS_START( steaser )
	PORT_START("MENU")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)

	PORT_START("STAT")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_IMPULSE(1)

	PORT_START("BET_DEAL")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_IMPULSE(1)

	PORT_START("TAKE_DOUBLE")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score") PORT_IMPULSE(1)

	PORT_START("SMALL_BIG")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Big") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) PORT_NAME("Small") PORT_IMPULSE(1)

	PORT_START("CANCEL_HOLD1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_IMPULSE(1)

	PORT_START("HOLD2_HOLD3")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_IMPULSE(1)

	PORT_START("HOLD4_HOLD5")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static MACHINE_DRIVER_START( ilpag )
	MDRV_CPU_ADD("maincpu", M68000, 11059200 )	// ?
	MDRV_CPU_PROGRAM_MAP(ilpag_map)
	MDRV_CPU_VBLANK_INT("screen",irq4_line_hold) //3 & 6 used, mcu comms?

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(ilpag)
	MDRV_VIDEO_UPDATE(ilpag)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

/*
20089f = 1 -> menu
2008a0 = 1 -> stat
2008a1 = 1 -> (unused)
2008a2 = 1 -> bet
2008a3 = 1 -> deal
2008a4 = 1 -> take
2008a5 = 1 -> double
2008a6 = 1 -> small
2008a7 = 1 -> big
2008a8 = 1 -> cancel
2008a9 = 1 -> hold 1
2008aa = 1 -> hold 2
2008ab = 1 -> hold 3
2008ac = 1 -> hold 4
2008ad = 1 -> hold 5

*/

static TIMER_DEVICE_CALLBACK( steaser_mcu_sim )
{
//  static int i;
	/*first off, signal the "MCU is running" flag*/
	generic_nvram16[0x932/2] = 0xffff;
	/*clear the inputs (they are impulsed)*/
//  for(i=0;i<8;i+=2)
//      generic_nvram16[((0x8a0)+i)/2] = 0;
	/*finally, read the inputs*/
	generic_nvram16[0x89e/2] = input_port_read(timer->machine, "MENU") & 0xffff;
	generic_nvram16[0x8a0/2] = input_port_read(timer->machine, "STAT") & 0xffff;
	generic_nvram16[0x8a2/2] = input_port_read(timer->machine, "BET_DEAL") & 0xffff;
	generic_nvram16[0x8a4/2] = input_port_read(timer->machine, "TAKE_DOUBLE") & 0xffff;
	generic_nvram16[0x8a6/2] = input_port_read(timer->machine, "SMALL_BIG") & 0xffff;
	generic_nvram16[0x8a8/2] = input_port_read(timer->machine, "CANCEL_HOLD1") & 0xffff;
	generic_nvram16[0x8aa/2] = input_port_read(timer->machine, "HOLD2_HOLD3") & 0xffff;
	generic_nvram16[0x8ac/2] = input_port_read(timer->machine, "HOLD4_HOLD5") & 0xffff;
}

/* TODO: remove this hack.*/
static INTERRUPT_GEN( steaser_irq )
{
	int num=cpu_getiloops(device)+3;
	cpu_set_input_line(device, num, HOLD_LINE);
}

static MACHINE_DRIVER_START( steaser )
	MDRV_IMPORT_FROM( ilpag )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(steaser_map)
	MDRV_CPU_VBLANK_INT_HACK(steaser_irq,4)

	MDRV_TIMER_ADD_PERIODIC("coinsim", steaser_mcu_sim, HZ(10000)) // not real, but for simulating the MCU
MACHINE_DRIVER_END


ROM_START( ilpag )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.7c-35.u32", 0x000000, 0x80000, CRC(ed99c884) SHA1(b3d2c9fb7765e3c8ff1e0de9c8edb6628e1c79ef) )
	ROM_LOAD16_BYTE( "2.7c-36.u31", 0x000001, 0x80000, CRC(4cd41688) SHA1(a1a15b06aa738cd4154d3c3479a7bf2da0e48426) )

	ROM_REGION( 0x100000, "blit_data", 0 ) // data for the blitter
	ROM_LOAD( "graf2.u51",   0x080000, 0x80000, CRC(2d64d3b5) SHA1(8fdb943d0aedf12706ce0a772c8f5155fa03e8c7) )
	ROM_LOAD( "graf1.u46",   0x000000, 0x80000, CRC(cf745964) SHA1(7af4a6c0b8d01c0d1b71bc5330a257d2fa712611) )

	ROM_REGION( 0x800, "mcu", 0 ) // MCU code
	ROM_LOAD( "87c748.u132", 0x000, 0x800, NO_DUMP )
ROM_END

ROM_START( steaser )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u31.1", 0x00001, 0x20000, CRC(7963e960) SHA1(2a1c68265e0a3909ccd097ea784e3e179f528844) )
	ROM_LOAD16_BYTE( "u32.6", 0x00000, 0x20000, CRC(c0ab5fb1) SHA1(15b3dbf0242e885b7009c21479544a821d4e5a7d) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_main.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "mcu2", 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_sub.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "oki", 0 ) /* Sound Samples */
	ROM_LOAD( "u18.7", 0x00000, 0x80000, CRC(ee942232) SHA1(b9c1fc73c6006bcad0dd177e0f30a96f1063a993) )

	ROM_REGION( 0x200000, "blit_data", 0 ) /* GFX */
	ROM_LOAD( "u46.2", 0x000000, 0x80000, CRC(c4a5e47b) SHA1(9f3d3124c76c0bdf8cdca849e1d921a335e433b6) )
	ROM_LOAD( "u51.3", 0x080000, 0x80000, CRC(4dc57435) SHA1(7dfa6f9e35986dd48869786abbe70103f336bcb1) )
	ROM_LOAD( "u61.4", 0x100000, 0x80000, CRC(d8d8dc6f) SHA1(5a76b1fd1a3a532e5ff2de127286ace7d3567c58) )
	ROM_LOAD( "u66.5", 0x180000, 0x80000, CRC(da309671) SHA1(66baf8a83024547c471da39748ff99a9a9013ea4) )
ROM_END

GAME( 199?, ilpag,   0, ilpag,   ilpag, 0, ROT0,  "<unknown>", "Il Pagliaccio (Italy, Version 2.7C)", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS )
/* In-game strings are in Italian but service mode is half English / half French?*/
GAME( 1993, steaser, 0, steaser, steaser, 0, ROT0,  "<unknown>", "Strip Teaser (Italy, Version 1.22)", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS )
