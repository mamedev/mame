/* bra$il */

/*

TODO:
-understand how the blitter really works (commands/video registers etc.);
-fix / locate input ports;
-add the sound chip into MAME (OkiM6376),it's already in AGEMAME so it should be easy to merge it.

==================================================================================

CPUs
N80C186XL25 (main)(u1)
1x ispLSI2032-80LJ (u13)(not dumped)
1x ispLSI1032E-70LJ (u18)(not dumped)
1x M6376 (sound)(u17)
1x oscillator 40.000MHz

ROMs
1x MX27C4000 (u16)
2x M27C801 (u7,u8)

Note

1x 28x2 edge connector (cn1)
1x 5 legs connector (cn2)
1x 8 legs connector (cn3)
1x trimmer (volume)
1x pushbutton (k1)
1x battery (b1)


cpu is 80186 based (with extras), see
http://media.digikey.com/pdf/Data%20Sheets/Intel%20PDFs/80C186XL,%2080C188XL.pdf

*/


#include "driver.h"

static UINT16 *blit_ram;

VIDEO_START(brasil)
{

}

VIDEO_UPDATE(brasil)
{
	int x,y,count;

	count = (0/2);

//	popmessage("%d %d",max_x,max_y);

	for(y=0;y<200;y++)
	{
		for(x=0;x<400;x++)
		{
			UINT32 color;
			UINT32 b;
			UINT32 g;
			UINT32 r;

			color = (blit_ram[count]) & 0xffff;

			b = (color & 0x001f) << 3;
			g = (color & 0x07e0) >> 3;
			r = (color & 0xf800) >> 8;
			if(x<cliprect->max_x && ((y*2)+0)<cliprect->max_y)
				*BITMAP_ADDR32(bitmap, (y*2)+0, x) = b | (g<<8) | (r<<16);

			count++;
		}

		for(x=0;x<400;x++)
		{
			UINT32 color;
			UINT32 b;
			UINT32 g;
			UINT32 r;

			color = (blit_ram[count]) & 0xffff;

			b = (color & 0x001f) << 3;
			g = (color & 0x07e0) >> 3;
			r = (color & 0xf800) >> 8;
			if(x<cliprect->max_x && ((y*2)+1)<cliprect->max_y)
				*BITMAP_ADDR32(bitmap, (y*2)+1, x) = b | (g<<8) | (r<<16);

			count++;
		}
	}

	return 0;
}

/*Some sort of status registers,probably blitter/vblank related.*/
static UINT16 unk_latch;

static READ16_HANDLER( unk_status_r )
{
	switch(offset*2)
	{
		case 0: return input_port_read(space->machine, "IN0"); //and 0x23 -> result = 3 at start-up
		case 2: return (unk_latch & 3); //and 0x3f
	}

	return 0;
}

/*bankaddress might be wrong...*/
static WRITE16_HANDLER( unk_status_w )
{
	static UINT32 bankaddress;
	UINT8 *ROM = memory_region(space->machine, "user1");

	switch(data & 3)
	{
		case 0: unk_latch = 1; break;
		case 1: unk_latch = 0; break;
		case 2: unk_latch = 2; break;
	}

	bankaddress = (data & 0x0f) * 0x40000;

	memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);

//	popmessage("%04x",data);
}

static UINT16 t1,t2,t3;

static READ16_HANDLER( read1_r )
{
	return t1;
}

static READ16_HANDLER( read2_r )
{
	return t2;
}

static READ16_HANDLER( read3_r )
{
	return t3;
}

static WRITE16_HANDLER( write1_w )
{
	t1 = data;
}

static WRITE16_HANDLER( write2_w )
{
	t2 = data;
}

static WRITE16_HANDLER( write3_w )
{
	t3 = data;
}

static ADDRESS_MAP_START( brasil_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM /*irq vector area + work ram*/
	AM_RANGE(0x40000, 0x7ffff) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x80000, 0xcffff) AM_ROMBANK(1)
	AM_RANGE(0xd0000, 0xfffff) AM_ROMBANK(2) //not a real rom bank,just for debug
ADDRESS_MAP_END

static ADDRESS_MAP_START( brasil_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0030, 0x0033) AM_READ( unk_status_r )
	AM_RANGE(0x0030, 0x0031) AM_WRITE( unk_status_w )
//	AM_RANGE(0x0000, 0x0001) AM_WRITE //0-f I/O range,appears to be video registers related
//	AM_RANGE(0x0002, 0x0003) AM_WRITE //"map" register for the blitter?
// 	AM_RANGE(0x0006, 0x0007) AM_WRITE //"map" register for the blitter?
 	AM_RANGE(0x0008, 0x0009) AM_READWRITE( read1_r,write1_w ) // commands?
	AM_RANGE(0x000a, 0x000b) AM_READWRITE( read2_r,write2_w ) // ""
	AM_RANGE(0x000e, 0x000f) AM_READWRITE( read3_r,write3_w ) // ""
//	AM_RANGE(0x000e, 0x000f) AM_WRITE
//	AM_RANGE(0xffa2, 0xffa3) AM_WRITE
ADDRESS_MAP_END

static INPUT_PORTS_START( brasil )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x0020, 0x0000, "unknown trigger" )
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

static INTERRUPT_GEN( vblank_irq )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
}

static MACHINE_DRIVER_START( brasil )
	MDRV_CPU_ADD("main", I80186, 25000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(brasil_map,0)
	MDRV_CPU_IO_MAP(brasil_io,0)
	MDRV_CPU_VBLANK_INT("main", vblank_irq)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
//	MDRV_SCREEN_SIZE(64*8, 32*8)
//	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MDRV_SCREEN_SIZE(800, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 300-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(brasil)
	MDRV_VIDEO_UPDATE(brasil)

	//OkiM6376
MACHINE_DRIVER_END

ROM_START( brasil )
	ROM_REGION( 0x200000, "user1", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "record_brasil_hrc7_vers.3.u7", 0x000000, 0x100000, CRC(627e0d58) SHA1(6ff8ba7b21e1ea5c88de3f02a057906c9a7cd808) )
	ROM_LOAD16_BYTE( "record_brasil_hrc8_vers.3.u8", 0x000001, 0x100000, CRC(47f7ba2a) SHA1(0add7bbf771fd0bf205a05e910cb388cf052b09f) )

	ROM_REGION( 0x080000, "samples", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound_brasil_hbr_vers.1.u16", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

DRIVER_INIT( brasil )
{
	UINT8 *ROM = memory_region(machine, "user1");
	memory_set_bankptr(machine, 2, &ROM[0x1d0000]);
}

GAME( 2000, brasil,    0,        brasil,    brasil,    brasil, ROT0,  "unknown", "Bra$il", GAME_NOT_WORKING | GAME_NO_SOUND )
