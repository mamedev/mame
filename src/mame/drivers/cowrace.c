/*************************************************************************************************************

    Cow Race

    preliminary driver by Luca Elia

*************************************************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/2203intf.h"

static tilemap *tmap;

static WRITE8_HANDLER( cowrace_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static WRITE8_HANDLER( cowrace_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static TILE_GET_INFO( get_tile_info )
{
	UINT16 code = videoram[ tile_index ] + (colorram[ tile_index ] << 8) ;
	SET_TILE_INFO(1, code & 0x1ff, 0, TILE_FLIPYX( 0 ));;
}

static VIDEO_START( cowrace )
{
	tmap = tilemap_create(	machine, get_tile_info, tilemap_scan_rows,
							8,8, 0x20,0x20	);

	tilemap_set_transparent_pen(tmap, 0);
}

static VIDEO_UPDATE( cowrace )
{
	bitmap_fill(bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect, tmap, 0, 0);
	return 0;
}

#ifdef UNUSED_FUNCTION
static WRITE8_HANDLER( cowrace_soundlatch_w )
{
	soundlatch_w(0, data);
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
}
#endif

static READ8_HANDLER( ret_ff )
{
	return 0xff;
}

static READ8_HANDLER( ret_00 )
{
	return 0x00;
}

static UINT8 cowrace_38c2;

static WRITE8_HANDLER( cowrace_38c2_w )
{
	cowrace_38c2 = data;
}

static READ8_HANDLER( cowrace_30c3_r )
{
	switch( cowrace_38c2 )
	{
		case 0x02:	return 0x03;
		case 0x04:	return 0x00;
	}

	return 0xff;
}

static ADDRESS_MAP_START( mem_map_cowrace, ADDRESS_SPACE_PROGRAM, 8 )
AM_RANGE(0x302f, 0x302f) AM_READ( ret_00 )
AM_RANGE(0x30c3, 0x30c3) AM_READ( cowrace_30c3_r )
AM_RANGE(0x38c2, 0x38c2) AM_READWRITE( ret_ff, cowrace_38c2_w )

	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM
	AM_RANGE(0x4000, 0x43ff) AM_READWRITE( SMH_RAM, cowrace_videoram_w ) AM_BASE( &videoram )
	AM_RANGE(0x5000, 0x53ff) AM_READWRITE( SMH_RAM, cowrace_colorram_w ) AM_BASE( &colorram )
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map_cowrace, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( mem_map_sound_cowrace, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map_sound_cowrace, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE(SOUND, "ym", ym2203_w)
ADDRESS_MAP_END


static const gfx_layout layout8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(0,2),
		RGN_FRAC(1,2)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{
		RGN_FRAC(0,4),
		RGN_FRAC(1,4),
		RGN_FRAC(2,4),
		RGN_FRAC(3,4),
	},
	{ 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7,0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8  },
	16*16
};

static GFXDECODE_START( cowrace )
	GFXDECODE_ENTRY( "gfx1", 0x000000, layout16x16x4, 0, 0x1 )
	GFXDECODE_ENTRY( "gfx2", 0x000000, layout8x8x2, 0, 0x1 )
GFXDECODE_END

static INPUT_PORTS_START( cowrace )
	PORT_START("IN0")
INPUT_PORTS_END

static const ym2203_interface ym2203_interface_1 =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_MEMORY_HANDLER("audio", PROGRAM, soundlatch_r),	// read A
		DEVCB_DEVICE_HANDLER(SOUND, "oki", okim6295_r),			// read B
		DEVCB_NULL,												// write A
		DEVCB_DEVICE_HANDLER(SOUND, "oki", okim6295_w)			// write B
	},
	NULL
};


static MACHINE_DRIVER_START( cowrace )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(mem_map_cowrace,0)
	MDRV_CPU_IO_MAP(io_map_cowrace,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* missing slave z80? (like in King Derby)*/

	MDRV_CPU_ADD("audio", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(mem_map_sound_cowrace,0)
	MDRV_CPU_IO_MAP(io_map_sound_cowrace,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)	// NMI by main CPU

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(cowrace)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(cowrace)
	MDRV_VIDEO_UPDATE(cowrace)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")
	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.80)

	MDRV_SOUND_ADD("ym", YM2203, 3000000)
	MDRV_SOUND_CONFIG(ym2203_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.80)
MACHINE_DRIVER_END


ROM_START( cowrace )
	ROM_REGION( 0x8000, "main", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x8000, CRC(c05c3bd3) SHA1(b7199a069ab45edd25e021589b79105cdfa5511a) )

	ROM_REGION( 0x2000, "audio", 0 )
	ROM_LOAD( "u164.bin", 0x0000, 0x2000, CRC(9affa1c8) SHA1(bfc07693e8f749cbf20ab8cda33975b66f567962) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u94.bin", 0x0000, 0x8000, CRC(945dc115) SHA1(bdd145234e6361c42ed20e8ca4cac64f07332748) )
	ROM_LOAD( "u95.bin", 0x8000, 0x8000, CRC(fc1fc006) SHA1(326a67c1ea0f487ecc8b7aef2d90124a01e6dee3) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "u139.bin", 0x0000, 0x2000, CRC(b746bb2f) SHA1(5f5f48752689079ed65fe7bb4a69512ada5db05d) )
	ROM_LOAD( "u140.bin", 0x2000, 0x2000, CRC(7e24b674) SHA1(c774efeb8e4e833e73c29007d5294c93df1abef4) )

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD( "u4.bin", 0x00000, 0x20000, CRC(f92a3ab5) SHA1(fc164492793597eadb8a50154410936edb74fa23) )
ROM_END

GAME( 2000, cowrace, kingdrby, cowrace, cowrace, 0,	ROT0, "bootleg", "Cow Race (hack of King Derby)", GAME_NOT_WORKING )
