/*************************************************************************************************************

    Sky Lancer

    preliminary driver by Luca Elia

    compare with carrera.c & cherrym2.c

*************************************************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"

static tilemap *tmap, *tmap2;

static UINT8 *skylncr_videoram,*skylncr_colorram;
static UINT8 *skylncr_videoram2,*skylncr_colorram2;

static WRITE8_HANDLER( skylncr_videoram_w )
{
	skylncr_videoram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static WRITE8_HANDLER( skylncr_colorram_w )
{
	skylncr_colorram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static TILE_GET_INFO( get_tile_info )
{
	UINT16 code = skylncr_videoram[ tile_index ] + (skylncr_colorram[ tile_index ] << 8);
	SET_TILE_INFO(0, code, 0, TILE_FLIPYX( 0 ));
}


static WRITE8_HANDLER( skylncr_videoram2_w )
{
	skylncr_videoram2[offset] = data;
	tilemap_mark_tile_dirty(tmap2, offset);
}

static WRITE8_HANDLER( skylncr_colorram2_w )
{
	skylncr_colorram2[offset] = data;
	tilemap_mark_tile_dirty(tmap2, offset);
}

static TILE_GET_INFO( get_tile_info2 )
{
	UINT16 code = skylncr_videoram2[ tile_index ] + (skylncr_colorram2[ tile_index ] << 8);
	SET_TILE_INFO(1, code, 0, TILE_FLIPYX( 0 ));
}

#define TILES_PER_PAGE_X	(0x40)
#define TILES_PER_PAGE_Y	(0x8)
#define PAGES_PER_TMAP_X	(0x2)
#define PAGES_PER_TMAP_Y	(0x2)

static TILEMAP_MAPPER( skylncr_tilemap_scan_pages )
{
	return	(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) +

			(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) * TILES_PER_PAGE_X;
}

static VIDEO_START( skylncr )
{

	tmap = tilemap_create(	get_tile_info, tilemap_scan_rows,
							8,8, 0x40,0x20	);

	tmap2 = tilemap_create(	get_tile_info2, skylncr_tilemap_scan_pages,
							8,32,
							TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y );

	tilemap_set_transparent_pen(tmap,  0);
	tilemap_set_transparent_pen(tmap2, 0);
}

static VIDEO_UPDATE( skylncr )
{
	fillbitmap(bitmap,0,cliprect);
	tilemap_draw(bitmap,cliprect, tmap2, 0, 0);
	tilemap_draw(bitmap,cliprect, tmap, 0, 0);
	return 0;
}

static READ8_HANDLER( ret_ff )
{
	return 0xff;
}

#ifdef UNUSED_FUNCTION
static READ8_HANDLER( ret_00 )
{
	return 0x00;
}
#endif

static UINT8 skylncr_nmi_enable;

static WRITE8_HANDLER( skylncr_nmi_enable_w )
{
	skylncr_nmi_enable = data & 0x10;
}

static WRITE8_HANDLER( skylncr_paletteram_w )
{
	static int color;

	if (offset == 0)
	{
		color = data;
	}
	else
	{
		int r,g,b;
		paletteram[color] = data;
		r = paletteram[(color/3*3)+0];	g = paletteram[(color/3*3)+1];	b = paletteram[(color/3*3)+2];
		r = (r << 2) | (r >> 4);		g = (g << 2) | (g >> 4);		b = (b << 2) | (b >> 4);
		palette_set_color(machine,color/3,MAKE_RGB(r,g,b));
		color = (color + 1) % (0x100*3);
	}
}

static WRITE8_HANDLER( skylncr_paletteram2_w )
{
	static int color;

	if (offset == 0)
	{
		color = data;
	}
	else
	{
		int r,g,b;
		paletteram_2[color] = data;
		r = paletteram_2[(color/3*3)+0];	g = paletteram_2[(color/3*3)+1];	b = paletteram_2[(color/3*3)+2];
		r = (r << 2) | (r >> 4);			g = (g << 2) | (g >> 4);			b = (b << 2) | (b >> 4);
		palette_set_color(machine,0x100 + color/3,MAKE_RGB(r,g,b));
		color = (color + 1) % (0x100*3);
	}
}

static ADDRESS_MAP_START( mem_map_skylncr, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM

	AM_RANGE(0x8800, 0x8fff) AM_READWRITE( SMH_RAM, skylncr_videoram_w ) AM_BASE( &skylncr_videoram )
	AM_RANGE(0x9000, 0x97ff) AM_READWRITE( SMH_RAM, skylncr_colorram_w ) AM_BASE( &skylncr_colorram )

	AM_RANGE(0x9800, 0x9fff) AM_READWRITE( SMH_RAM, skylncr_videoram2_w ) AM_BASE( &skylncr_videoram2 )
	AM_RANGE(0xa000, 0xa7ff) AM_READWRITE( SMH_RAM, skylncr_colorram2_w ) AM_BASE( &skylncr_colorram2 )

AM_RANGE(0xaa55, 0xaa55) AM_READ( ret_ff )

	AM_RANGE(0xb000, 0xb7ff) AM_RAM

	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static WRITE8_HANDLER( skylncr_coin_w )
{
	coin_counter_w( 0, data & 0x04 );
//  popmessage("%02x",data);
}

static ADDRESS_MAP_START( io_map_skylncr, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x00, 0x00) AM_READ( input_port_0_r )
	AM_RANGE(0x01, 0x01) AM_READ( input_port_1_r )
	AM_RANGE(0x02, 0x02) AM_READ( input_port_2_r )
	AM_RANGE(0x10, 0x10) AM_READ( input_port_3_r )
	AM_RANGE(0x11, 0x11) AM_READ( input_port_4_r )
	AM_RANGE(0x12, 0x12) AM_READ( input_port_5_r )

	AM_RANGE(0x20, 0x20) AM_WRITE( skylncr_coin_w )

	AM_RANGE(0x30, 0x30) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE(0x31, 0x31) AM_READWRITE( AY8910_read_port_0_r , AY8910_write_port_0_w )

	AM_RANGE(0x40, 0x41) AM_WRITE( skylncr_paletteram_w )
	AM_RANGE(0x50, 0x51) AM_WRITE( skylncr_paletteram2_w )

	AM_RANGE(0x70, 0x70) AM_WRITE( skylncr_nmi_enable_w )
ADDRESS_MAP_END


static const gfx_layout layout8x8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0,8*1,
		RGN_FRAC(1,2)+8*0,RGN_FRAC(1,2)+8*1,
		8*2,8*3,
		RGN_FRAC(1,2)+8*2,RGN_FRAC(1,2)+8*3
	},
	{ STEP8(0,8*4) },
	8*8*4
};

static const gfx_layout layout8x32x8 =
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{ 8*0,8*4,8*1,8*5,8*2,8*6,8*3,8*7 },
	{
	 RGN_FRAC(1,2)+8*8*0, RGN_FRAC(0,2)+8*8*0,
	 RGN_FRAC(1,2)+8*8*1, RGN_FRAC(0,2)+8*8*1,
	 RGN_FRAC(1,2)+8*8*2, RGN_FRAC(0,2)+8*8*2,
	 RGN_FRAC(1,2)+8*8*3, RGN_FRAC(0,2)+8*8*3,
	 RGN_FRAC(1,2)+8*8*4, RGN_FRAC(0,2)+8*8*4,
	 RGN_FRAC(1,2)+8*8*5, RGN_FRAC(0,2)+8*8*5,
	 RGN_FRAC(1,2)+8*8*6, RGN_FRAC(0,2)+8*8*6,
	 RGN_FRAC(1,2)+8*8*7, RGN_FRAC(0,2)+8*8*7,
	 RGN_FRAC(1,2)+8*8*8, RGN_FRAC(0,2)+8*8*8,
	 RGN_FRAC(1,2)+8*8*9, RGN_FRAC(0,2)+8*8*9,
	 RGN_FRAC(1,2)+8*8*10,RGN_FRAC(0,2)+8*8*10,
	 RGN_FRAC(1,2)+8*8*11,RGN_FRAC(0,2)+8*8*11,
	 RGN_FRAC(1,2)+8*8*12,RGN_FRAC(0,2)+8*8*12,
	 RGN_FRAC(1,2)+8*8*13,RGN_FRAC(0,2)+8*8*13,
	 RGN_FRAC(1,2)+8*8*14,RGN_FRAC(0,2)+8*8*14,
	 RGN_FRAC(1,2)+8*8*15,RGN_FRAC(0,2)+8*8*15
	},
	8*16*8
};

static const gfx_layout layout8x32x8_rot =
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0, 8*1,
		RGN_FRAC(1,2)+8*0, RGN_FRAC(1,2)+8*1,
		8*2, 8*3,
		RGN_FRAC(1,2)+8*2, RGN_FRAC(1,2)+8*3
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};

static GFXDECODE_START( skylncr )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8,			0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_rot,	0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8,		0, 2 )
GFXDECODE_END

static INPUT_PORTS_START( skylncr )

	PORT_START("SYSTEM")	// IN0 - $0 "PORT0 A"
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4)

	PORT_START("BUTTONS")	// IN1 - $1 "PORT0 B"
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)	// skip test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)

	PORT_START("DSW1")	// IN2 - $2 "DSW1"
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")	// IN3 - $10 "DSW2"
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN1")	// IN4 - $11 "PORT1 B"
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN2")	// IN5 - $12 "PORT1 C"
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW4")	// IN6 - AY8910 port A
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")	// IN7 - AY8910 port B
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_6_r,
	input_port_7_r,
	NULL,
	NULL
};


// It runs in IM 0, thus needs an opcode on the data bus
static INTERRUPT_GEN( skylncr_vblank_interrupt )
{
	if (skylncr_nmi_enable) cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( skylncr )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 12000000/4)
	MDRV_CPU_PROGRAM_MAP(mem_map_skylncr,0)
	MDRV_CPU_IO_MAP(io_map_skylncr,0)
	MDRV_CPU_VBLANK_INT("main", skylncr_vblank_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(skylncr)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(skylncr)
	MDRV_VIDEO_UPDATE(skylncr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/*
Sky Lancer PCB Layout
---------------------

  |--------------------------------------------|
 _|                          ROM.U33           |
|                                              |
|                            ROM.U32           |
|    WF19054                                   |
|                                              |
|_                                             |
  |                                  6264      |
  |                     |------|     6116      |
 _|           DSW4(8)   |ACTEL |               |
|             DSW3(8)   |A1010B|               |
|             DSW2(8)   |      |          6264 |
|             DSW1(8)   |------|               |
|                                         6264 |
|    M5M82C255                                 |
|                                              |
|       ROM.U35                                |
|3.6V_BATT                                     |
|_          6116              Z80        12MHz |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
*/

static DRIVER_INIT( skylncr )
{
	paletteram   = auto_malloc(0x100*3);
	paletteram_2 = auto_malloc(0x100*3);
}

ROM_START( skylncr )
	ROM_REGION( 0x80000, "main", 0 )
	ROM_LOAD( "27512.u35",  0x00000, 0x10000, CRC(98b1c9fe) SHA1(9ca1706d25038a078fb07ba5c2e6681ed468bc88) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "574200.u32", 0x00000, 0x80000, CRC(b36f11fe) SHA1(1d8660ac1ca44e33976ac14210e4a3a201f8f3c4) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "574200.u33", 0x00000, 0x80000, CRC(19b25221) SHA1(2f32d337125a9fd0bc7f50713b05e564fd4f81b2) )
ROM_END

GAME( 1995, skylncr, 0, skylncr, skylncr, skylncr, ROT0, "Bordun International", "Sky Lancer (Bordun International)", GAME_NOT_WORKING )
