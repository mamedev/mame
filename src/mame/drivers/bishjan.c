/*********************************************************************************************************************

Bishou Jan (Laugh World)  (C)1999 Subsino

driver by Luca Elia

PCB Layout
----------

|------------------------------------------------------|
|TDA1519A           28-WAY                             |
|     VOL                                              |
|                HM86171                       ULN2003 |
|   LM324                                              |
|           S-1                                ULN2003 |
|                                                      |
|                                   |-------|  DSW1(8) |
|                       |-------|   |SUBSINO|          |
|            2-V201.U9  |SUBSINO|   |SS9802 |          |
|                       |SS9904 |   |       |          |
|                       |       |   |-------|          |
|                       |-------|                      |
|                                                      |
|                         44.1MHz             CXK58257 |
|  3-V201.U25                                          |
|                                  1-V203.U21          |
|  4-V201.U26                                       SW1|
|             |-------|    |-------|   |-----|         |
|  5-V201.U27 |SUBSINO|    |SUBSINO|   |H8   |         |
|             |SS9601 |    |SS9803 |   |3044 |         |
|  6-V201.U28 |       |    |       |   |-----|         |
|             |-------|    |-------|                   |
|          62256  62256   BATTERY                      |
|------------------------------------------------------|
Notes:
      H8/3044 - Subsino re-badged Hitachi H8/3044 HD6433044A22F Microcontroller (QFP100)
                The H8/3044 is a H8/3002 with 24bit address bus and has 32k MASKROM and 2k RAM, clock input is 14.7MHz [44.1/3]
                MD0,MD1 & MD2 are configured to MODE 6 16MByte Expanded Mode with the on-chip 32k MASKROM enabled.
     CXK58257 - Sony CXK58257 32k x8 SRAM (SOP28)
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
          S-1 - ?? Probably some kind of audio OP AMP or DAC? (DIP8)
          SW1 - Push Button Test Switch
        HSync - 15.75kHz
        VSync - 60Hz

*********************************************************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/h83002/h83002.h"

#define BISHJAN_DEBUG	0

/***************************************************************************
                                Video Hardware
***************************************************************************/

static int clr_offset;

static UINT16 *bishjan_scroll, *bishjan_layers;

static UINT16	*bishjan_videoram1,	*bishjan_videoram2;
static tilemap	*tmap1,				*tmap2;

static TILE_GET_INFO( get_tile_info1 )	{	SET_TILE_INFO(0, bishjan_videoram1[ tile_index ], 0, 0);	}
static TILE_GET_INFO( get_tile_info2 )	{	SET_TILE_INFO(0, bishjan_videoram2[ tile_index ], 0, 0);	}

static VIDEO_START(bishjan)
{
	tmap1 = tilemap_create(	get_tile_info1, tilemap_scan_rows,
							 8,8, 0x80,0x40	);

	tmap2 = tilemap_create(	get_tile_info2, tilemap_scan_rows,
							 8,8, 0x80,0x40	);

	tilemap_set_transparent_pen(tmap1, 0);
	tilemap_set_transparent_pen(tmap2, 0);

	bishjan_videoram1 = (UINT16*)auto_malloc(sizeof(UINT16) * 0x80 * 0x40);
	bishjan_videoram2 = (UINT16*)auto_malloc(sizeof(UINT16) * 0x80 * 0x40);

	colorram = auto_malloc(256*3);
}

static VIDEO_UPDATE( bishjan )
{
	int layers_ctrl = ~bishjan_layers[0];

	int scroll1_x = ((bishjan_scroll[1] & 0x0f00)>>0) | (bishjan_scroll[0] >> 8);
	int scroll1_y = ((bishjan_scroll[1] & 0xf000)>>4) | (bishjan_scroll[0] & 0xff);

	int scroll2_x = ((bishjan_scroll[2] & 0x0f)<<8) | (bishjan_scroll[1] & 0xff);
	int scroll2_y = ((bishjan_scroll[2] & 0xf0)<<4) | (bishjan_scroll[2] >> 8);

#if BISHJAN_DEBUG
if (input_code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	scroll1_y += 1;
	scroll2_y += 1;

	tilemap_set_scrollx( tmap1, 0, scroll1_x);	tilemap_set_scrolly( tmap1, 0, scroll1_y );
	tilemap_set_scrollx( tmap2, 0, scroll2_x);	tilemap_set_scrolly( tmap2, 0, scroll2_y );

	fillbitmap(bitmap,get_black_pen(screen->machine),cliprect);

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tmap1, 0, 0);
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tmap2, 0, 0);

//  popmessage("23: %03x,%03x - %03x,%03x LY: %02x", scroll1_x,scroll1_y, scroll2_x,scroll2_y, (int)*bishjan_layers);

	return 0;
}

// Palette: HMC HM86171 VGA 256 colour RAMDAC

static WRITE16_HANDLER(colordac_w)
{
	if (ACCESSING_BITS_0_7)
	{
		colorram[clr_offset] = data;
		palette_set_color_rgb(space->machine, clr_offset/3,
			pal6bit(colorram[(clr_offset/3)*3+0]),
			pal6bit(colorram[(clr_offset/3)*3+1]),
			pal6bit(colorram[(clr_offset/3)*3+2])
		);
		clr_offset = (clr_offset+1) % (256*3);
	}

	if (ACCESSING_BITS_8_15)
	{
		clr_offset = (data>>8) * 3;
	}
}

// Tilemaps

static UINT16 bishjan_low;

static WRITE16_HANDLER( bishjan_low_w )
{
	if (ACCESSING_BITS_8_15)	bishjan_low = data >> 8;
}

static WRITE16_HANDLER( bishjan_tmap1_w )
{
	if (ACCESSING_BITS_8_15)
	{
		bishjan_videoram1[offset*2] = data | bishjan_low;
		tilemap_mark_tile_dirty(tmap1, offset*2);
	}
	if (ACCESSING_BITS_0_7)
	{
		bishjan_videoram1[offset*2+1] = (data << 8) | bishjan_low;
		tilemap_mark_tile_dirty(tmap1, offset*2+1);
	}
}
static WRITE16_HANDLER( bishjan_tmap2_w )
{
	if (ACCESSING_BITS_8_15)
	{
		bishjan_videoram2[offset*2] = data | bishjan_low;
		tilemap_mark_tile_dirty(tmap2, offset*2);
	}
	if (ACCESSING_BITS_0_7)
	{
		bishjan_videoram2[offset*2+1] = (data << 8) | bishjan_low;
		tilemap_mark_tile_dirty(tmap2, offset*2+1);
	}
}

static READ16_HANDLER( bishjan_tmap1_lo_r )
{
	return (bishjan_videoram1[offset*2+0] << 8) | (bishjan_videoram1[offset*2+1] & 0x00ff);
}
static READ16_HANDLER( bishjan_tmap1_hi_r )
{
	return (bishjan_videoram1[offset*2+0] & 0xff00) | (bishjan_videoram1[offset*2+1] >> 8);
}

static READ16_HANDLER( bishjan_tmap2_lo_r )
{
	return (bishjan_videoram2[offset*2+0] << 8) | (bishjan_videoram2[offset*2+1] & 0x00ff);
}
static READ16_HANDLER( bishjan_tmap2_hi_r )
{
	return (bishjan_videoram2[offset*2+0] & 0xff00) | (bishjan_videoram2[offset*2+1] >> 8);
}

// Inputs

static UINT16 bishjan_sel, bishjan_input, bishjan_hopper;

static WRITE16_HANDLER( bishjan_sel_w )
{
	if (ACCESSING_BITS_8_15)	bishjan_sel = data >> 8;
}

static READ16_HANDLER( bishjan_unk_r )
{
	return
		(mame_rand(space->machine) & 0x9800)	|	// bit 7 eeprom?
		(((bishjan_sel==0x12) ? 0x40:0x00) << 8) |
//      (mame_rand() & 0xff);
//      (((video_screen_get_frame_number(space->machine->primary_screen)%60)==0)?0x18:0x00);
		0x18;
}

static WRITE16_HANDLER( bishjan_input_w )
{
	if (ACCESSING_BITS_8_15)	bishjan_input = data >> 8;
}

static READ16_HANDLER( bishjan_input_r )
{
	int i;
	UINT16 res = 0xff;
	static const char *const port[] = { "KEYB_0", "KEYB_1", "KEYB_2", "KEYB_3", "KEYB_4" };

	for (i = 0; i < 5; i++)
		if (bishjan_input & (1 << i))
			res = input_port_read(space->machine, port[i]);

	return	(res << 8) |
			input_port_read(space->machine, "SYSTEM") |
			((bishjan_hopper && !(video_screen_get_frame_number(space->machine->primary_screen)%10)) ? 0x00 : 0x04)	// bit 2: hopper sensor
	;
}

static WRITE16_HANDLER( bishjan_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// coin out         data & 0x01;
		bishjan_hopper	=	data & 0x02;	// hopper
		coin_counter_w( 1,	data & 0x10 );
	}
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

/*
432000 & 436000 W + lo (80*20=1000)
412000 & 422000 R (lo & hi)
416000 & 426000 R (lo & hi)

433000 & 437000 W
413000 & 423000 R
417000 & 427000 R
*/

static ADDRESS_MAP_START( bishjan_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM AM_REGION("main", 0)
	AM_RANGE( 0x080000, 0x0fffff ) AM_ROM AM_REGION("main", 0)

	AM_RANGE( 0x200000, 0x207fff ) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)	// battery

	AM_RANGE( 0x412000, 0x412fff ) AM_READ( bishjan_tmap2_lo_r )	//?
	AM_RANGE( 0x413000, 0x413fff ) AM_READ( SMH_RAM ) AM_SHARE(1)
	AM_RANGE( 0x416000, 0x416fff ) AM_READ( bishjan_tmap1_lo_r )	//?
	AM_RANGE( 0x417000, 0x417fff ) AM_READ( SMH_RAM ) AM_SHARE(1)
	AM_RANGE( 0x422000, 0x422fff ) AM_READ( bishjan_tmap2_hi_r )	//?
	AM_RANGE( 0x423000, 0x423fff ) AM_READ( SMH_RAM ) AM_SHARE(2)
	AM_RANGE( 0x426000, 0x426fff ) AM_READ( bishjan_tmap1_hi_r )
	AM_RANGE( 0x427000, 0x427fff ) AM_READ( SMH_RAM ) AM_SHARE(2)
	AM_RANGE( 0x430000, 0x431fff ) AM_WRITE( bishjan_tmap2_w )
	AM_RANGE( 0x432000, 0x432fff ) AM_WRITE( bishjan_tmap2_w )		//?
	AM_RANGE( 0x433000, 0x433fff ) AM_WRITE( SMH_RAM ) AM_SHARE(1)
	AM_RANGE( 0x434000, 0x435fff ) AM_WRITE( bishjan_tmap1_w )
	AM_RANGE( 0x436000, 0x436fff ) AM_WRITE( bishjan_tmap1_w )		//?
	AM_RANGE( 0x437000, 0x437fff ) AM_WRITE( SMH_RAM ) AM_SHARE(2)

	AM_RANGE( 0x600000, 0x600001 ) AM_READWRITE( SMH_NOP, bishjan_sel_w )
	AM_RANGE( 0x600060, 0x600061 ) AM_WRITE( colordac_w )
	AM_RANGE( 0x600062, 0x600063 ) AM_WRITE( SMH_NOP )	// ff to 600062
	AM_RANGE( 0x6000a0, 0x6000a1 ) AM_WRITE( bishjan_low_w )

	AM_RANGE( 0xa0001e, 0xa0001f ) AM_WRITE( SMH_RAM ) AM_BASE( &bishjan_layers )

	AM_RANGE( 0xa00020, 0xa00025 ) AM_WRITE( SMH_RAM ) AM_BASE( &bishjan_scroll )

	AM_RANGE( 0xc00000, 0xc00001 ) AM_READ_PORT("DSW")	// c00001 sw1
	AM_RANGE( 0xc00002, 0xc00003 ) AM_READ_PORT("JOY") AM_WRITE( bishjan_input_w )	// in c
	AM_RANGE( 0xc00004, 0xc00005 ) AM_READ( bishjan_input_r )	// in a & b
	AM_RANGE( 0xc00006, 0xc00007 ) AM_READ( bishjan_unk_r )		// c00006 in d ($18)
	AM_RANGE( 0xc00008, 0xc00009 ) AM_READ_PORT("RESET") AM_WRITE( bishjan_coin_w )	// c00009 reset
ADDRESS_MAP_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout bishjan_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static GFXDECODE_START( bishjan )
	GFXDECODE_ENTRY( "gfx1", 0, bishjan_8x8_layout, 0, 1 )
GFXDECODE_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( bishjan )
	PORT_START("RESET")		/* IN0 - Reset */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)

	PORT_START("DSW")		/* IN1 - DSW(SW1) */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0001, "Keyboard" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
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

	PORT_START("JOY")		/* IN2 - C */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1			) PORT_NAME("1 Player Start (Joy Mode)")	// start (joy)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	)	// down (joy)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	)	// left (joy)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	)	// right (joy)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1		)	// n (joy)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_BET	) PORT_NAME("P1 Mahjong Bet (Joy Mode)")	// bet (joy)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2		)	// select (joy)

	PORT_START("SYSTEM")		/* IN3 - A */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE		)	// test
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,IPT_SPECIAL		)	// hopper sensor
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1		)	// stats
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2		)	// pay out? "hopper empty"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1			)	PORT_IMPULSE(2)	// coin
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE3		)	// pay out? "hopper empty"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2			)	PORT_IMPULSE(2)	// coin

	PORT_START("KEYB_0")	/* IN4 - B(1) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A		)	// a
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E		)	// e
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I		)	// i
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M		)	// m
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// i2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1			)	// b2 (start)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_1")	/* IN5 - B(2) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B		)	// b
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F		)	// f
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J		)	// j
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N		)	// n
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// l2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET	)	// c2 (bet)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_2")	/* IN6 - B(3) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C		)	// c
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G		)	// g
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K		)	// k
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// k2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// m2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_3")	/* IN7 - B(4) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D		)	// d
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H		)	// h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L		)	// l
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// j2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_4")	/* IN8 - B(5) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// g2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// e2
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// d2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// f2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)
INPUT_PORTS_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static INTERRUPT_GEN( bishjan_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, 0, PULSE_LINE);
			break;
		default:
			h8_3002_InterruptRequest(24);
			break;
	}
}

static MACHINE_DRIVER_START( bishjan )
	MDRV_CPU_ADD("main", H83044, 44100000/3)
	MDRV_CPU_PROGRAM_MAP( bishjan_map, 0 )
	MDRV_CPU_VBLANK_INT_HACK(bishjan_interrupt,2)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 512, 256 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MDRV_SCREEN_REFRESH_RATE( 60 )

	MDRV_GFXDECODE(bishjan)
	MDRV_PALETTE_LENGTH( 256 )

	MDRV_VIDEO_START( bishjan )
	MDRV_VIDEO_UPDATE( bishjan )
MACHINE_DRIVER_END

/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( bishjan )
	ROM_REGION( 0x100000, "main", 0 )		// H8/3044 program
	ROM_LOAD( "1-v203.u21", 0x000000, 0x080000, CRC(1f891d48) SHA1(0b6a5aa8b781ba8fc133289790419aa8ea21c400) )

	ROM_REGION( 0x400000, "gfx1", 0 )		// Tiles
	ROM_LOAD32_BYTE( "3-v201.u25", 0x000000, 0x100000, CRC(e013e647) SHA1(a5b0f82f3454393c1ea5e635b0d37735a25e2ea5) )
	ROM_LOAD32_BYTE( "4-v201.u26", 0x000001, 0x100000, CRC(e0d40ef1) SHA1(95f80889103a7b93080b46387274cb1ffe0c8768) )
	ROM_LOAD32_BYTE( "5-v201.u27", 0x000002, 0x100000, CRC(85067d40) SHA1(3ecf7851311a77a0dfca90775fcbf6faabe9c2ab) )
	ROM_LOAD32_BYTE( "6-v201.u28", 0x000003, 0x100000, CRC(430bd9d7) SHA1(dadf5a7eb90cf2dc20f97dbf20a4b6c8e7734fb1) )

	ROM_REGION( 0x100000, "samples", 0 )	// Samples
	ROM_LOAD( "2-v201.u9", 0x000000, 0x100000, CRC(ea42764d) SHA1(13fe1cd30e474f4b092949c440068e9ddca79976) )
ROM_END

static DRIVER_INIT(bishjan)
{
	UINT16 *rom = (UINT16*)memory_region(machine, "main");

	// check
	rom[0x042EA/2] = 0x4008;

	// rts -> rte
	rom[0x33386/2] = 0x5670;
	rom[0x0CC5C/2] = 0x5670;
}

GAME(1999, bishjan, 0, bishjan, bishjan, bishjan, ROT0, "Subsino", "Bishou Jan", GAME_NO_SOUND)
