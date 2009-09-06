/***************************************************************************

Galaxi (C)2000 B.R.L.

driver by Luca Elia

Hardware info (29/07/2008 f205v):

Chips:
    1x missing main CPU (u1)(from the socket I would say it's a 68000)
    1x A40MX04-PL84 (u29)
    1x AD-65 (equivalent to M6295) (u9)(sound)
    1x MC1458P (u10)(sound)
    1x TDA2003 (u8)(sound)
    1x oscillator 10.000MHz (QZ1)
    1x oscillator 16.000000 (QZ2)
ROMs:
    1x AT27C020 (1)
    2x M27C4001 (2,3)
    2x AT49F010 (4,5)
    2x DS1230Y (non volatile SRAM)
Notes:
    1x 28x2 edge connector
    1x trimmer (volume)

- This hardware is almost identical to that in magic10.c


[31/08/2008] (Roberto Fresca)

- Added Magic Joker.
- Fixed the 3rd background offset to Galaxi.
- Remapped inputs to match the standard poker games.

[12/09/2008] (Roberto Fresca)

- Added lamps support to magjoker & galaxi.


***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "galaxi.lh"

/***************************************************************************
                                Video Hardware
***************************************************************************/

static UINT16  *bg1_ram,  *bg2_ram,  *bg3_ram,  *bg4_ram,  *fg_ram;
static tilemap *bg1_tmap, *bg2_tmap, *bg3_tmap, *bg4_tmap, *fg_tmap;

static TILE_GET_INFO( get_bg1_tile_info )
{
	UINT16 code = bg1_ram[tile_index];
	SET_TILE_INFO(0, code, 0x10+(code >> 12), 0);
}
static TILE_GET_INFO( get_bg2_tile_info )
{
	UINT16 code = bg2_ram[tile_index];
	SET_TILE_INFO(0, code, 0x10+(code >> 12), 0);
}
static TILE_GET_INFO( get_bg3_tile_info )
{
	UINT16 code = bg3_ram[tile_index];
	SET_TILE_INFO(0, code, (code >> 12), 0);
}
static TILE_GET_INFO( get_bg4_tile_info )
{
	UINT16 code = bg4_ram[tile_index];
	SET_TILE_INFO(0, code, (code >> 12), 0);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	UINT16 code = fg_ram[tile_index];
	SET_TILE_INFO(1, code, 0x20+(code >> 12), 0);
}


static WRITE16_HANDLER( galaxi_bg1_w )
{
	COMBINE_DATA( &bg1_ram[offset] );
	tilemap_mark_tile_dirty( bg1_tmap, offset );
}
static WRITE16_HANDLER( galaxi_bg2_w )
{
	COMBINE_DATA( &bg2_ram[offset] );
	tilemap_mark_tile_dirty( bg2_tmap, offset );
}
static WRITE16_HANDLER( galaxi_bg3_w )
{
	COMBINE_DATA( &bg3_ram[offset] );
	tilemap_mark_tile_dirty( bg3_tmap, offset );
}
static WRITE16_HANDLER( galaxi_bg4_w )
{
	COMBINE_DATA( &bg4_ram[offset] );
	tilemap_mark_tile_dirty( bg4_tmap, offset );
}

static WRITE16_HANDLER( galaxi_fg_w )
{
	COMBINE_DATA( &fg_ram[offset] );
	tilemap_mark_tile_dirty( fg_tmap, offset );
}

static VIDEO_START(galaxi)
{
	bg1_tmap = tilemap_create( machine, get_bg1_tile_info, tilemap_scan_rows, 16,16, 0x20,0x10 );
	bg2_tmap = tilemap_create( machine, get_bg2_tile_info, tilemap_scan_rows, 16,16, 0x20,0x10 );
	bg3_tmap = tilemap_create( machine, get_bg3_tile_info, tilemap_scan_rows, 16,16, 0x20,0x10 );
	bg4_tmap = tilemap_create( machine, get_bg4_tile_info, tilemap_scan_rows, 16,16, 0x20,0x10 );

	fg_tmap  = tilemap_create( machine, get_fg_tile_info, tilemap_scan_rows, 8,8,   0x40,0x20 );

	tilemap_set_transparent_pen(bg1_tmap, 0);
	tilemap_set_transparent_pen(bg2_tmap, 0);
	tilemap_set_transparent_pen(bg3_tmap, 0);
	tilemap_set_transparent_pen(bg4_tmap, 0);

	tilemap_set_transparent_pen(fg_tmap, 0);

	tilemap_set_scrolldx( bg3_tmap, -8, 0);
}

static VIDEO_UPDATE(galaxi)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(screen->machine, KEYCODE_R))	// remapped due to inputs changes.
	{
		int msk = 0;
		if (input_code_pressed(screen->machine, KEYCODE_T))	msk |= 1;
		if (input_code_pressed(screen->machine, KEYCODE_Y))	msk |= 2;
		if (input_code_pressed(screen->machine, KEYCODE_U))	msk |= 4;
		if (input_code_pressed(screen->machine, KEYCODE_I))	msk |= 8;
		if (input_code_pressed(screen->machine, KEYCODE_O))	msk |= 16;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, bg1_tmap,  TILEMAP_DRAW_OPAQUE, 0);
	else					bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, bg2_tmap,  0, 0);
	if (layers_ctrl & 4)	tilemap_draw(bitmap,cliprect, bg3_tmap,  0, 0);
	if (layers_ctrl & 8)	tilemap_draw(bitmap,cliprect, bg4_tmap,  0, 0);

	if (layers_ctrl & 16)	tilemap_draw(bitmap,cliprect, fg_tmap, 0, 0);

	return 0;
}

/***************************************************************************
                            Memory Maps
***************************************************************************/

static int hopper, ticket;

static UINT16 out[3];

static void show_out(void)
{
//  popmessage("%04x %04x %04x", out[0], out[1], out[2]);
}

static WRITE16_HANDLER( galaxi_500000_w )
{
	COMBINE_DATA( &out[0] );
	show_out();
}

static WRITE16_HANDLER( galaxi_500002_w )
{
	COMBINE_DATA( &out[1] );
	show_out();
}

static WRITE16_HANDLER( galaxi_500004_w )
{
	if ( ACCESSING_BITS_0_7 )
	{
	/*
        - Lbits -
        7654 3210
        =========
        ---- ---x  Hold1 lamp.
        ---- --x-  Hold2 lamp.
        ---- -x--  Hold3 lamp.
        ---- x---  Hold4 lamp.
        ---x ----  Hold5 lamp.
        --x- ----  Start lamp.
        -x-- ----  Payout.

    */
		output_set_lamp_value(1, (data & 1));			/* Lamp 1 - HOLD 1 */
		output_set_lamp_value(2, (data >> 1) & 1);		/* Lamp 2 - HOLD 2 */
		output_set_lamp_value(3, (data >> 2) & 1);		/* Lamp 3 - HOLD 3 */
		output_set_lamp_value(4, (data >> 3) & 1);		/* Lamp 4 - HOLD 4 */
		output_set_lamp_value(5, (data >> 4) & 1);		/* Lamp 5 - HOLD 5 */
		output_set_lamp_value(6, (data >> 5) & 1);		/* Lamp 6 - START  */
	}
	if ( ACCESSING_BITS_8_15 )
	{
		ticket = data & 0x0100;
		hopper = data & 0x1000;
		coin_counter_w(0, data & 0x2000);	// coins
	}

	COMBINE_DATA( &out[2] );
	show_out();
}

static CUSTOM_INPUT( ticket_r )
{
	return ticket && !(video_screen_get_frame_number(field->port->machine->primary_screen)%10);
}
static CUSTOM_INPUT( hopper_r )
{
	return hopper && !(video_screen_get_frame_number(field->port->machine->primary_screen)%10);
}


static ADDRESS_MAP_START( galaxi_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM

	AM_RANGE( 0x100000, 0x1003ff ) AM_RAM_WRITE( galaxi_bg1_w ) AM_BASE( &bg1_ram )
	AM_RANGE( 0x100400, 0x1007ff ) AM_RAM_WRITE( galaxi_bg2_w ) AM_BASE( &bg2_ram )
	AM_RANGE( 0x100800, 0x100bff ) AM_RAM_WRITE( galaxi_bg3_w ) AM_BASE( &bg3_ram )
	AM_RANGE( 0x100c00, 0x100fff ) AM_RAM_WRITE( galaxi_bg4_w ) AM_BASE( &bg4_ram )

	AM_RANGE( 0x101000, 0x101fff ) AM_RAM_WRITE( galaxi_fg_w  ) AM_BASE( &fg_ram  )
	AM_RANGE( 0x102000, 0x1047ff ) AM_READNOP	// unknown

	AM_RANGE( 0x300000, 0x3007ff ) AM_RAM_WRITE( paletteram16_xRRRRRGGGGGBBBBB_word_w ) AM_BASE( &paletteram16 )

	AM_RANGE( 0x500000, 0x500001 ) AM_READ_PORT( "INPUTS" )
	AM_RANGE( 0x500000, 0x500001 ) AM_WRITE( galaxi_500000_w )
	AM_RANGE( 0x500002, 0x500003 ) AM_WRITE( galaxi_500002_w )
	AM_RANGE( 0x500004, 0x500005 ) AM_WRITE( galaxi_500004_w )

	AM_RANGE( 0x700000, 0x700001 ) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0x00ff )

	AM_RANGE( 0x600000, 0x607fff ) AM_RAM AM_BASE( &generic_nvram16 ) AM_SIZE( &generic_nvram_size )	// 2x DS1230Y (non volatile SRAM)
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( galaxi )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM( hopper_r, (void *)0 )	// hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)	// coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)	// coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN3 )	// pin 25LC
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( ticket_r, (void *)0 )	// ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )	// test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// (pin 15LS)
INPUT_PORTS_END

static INPUT_PORTS_START( magjoker )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( hopper_r, (void *)0 )	// hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)	// coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)	// coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Hopper Refill") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( ticket_r, (void *)0 )	// ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )	// test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )	// (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// (pin 15LS)
INPUT_PORTS_END


/***************************************************************************
                               Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8, 8,
	0x1000,	// 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout layout_16x16x4 =
{
	16, 16,
	0x1000,	// 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4), STEP4(4*4+8*16*4,4), STEP4(0+8*16*4,4) },
	{ STEP16(0,4*8) },
	16*16*4
};

static GFXDECODE_START( galaxi )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_16x16x4, 0, 0x400/0x10 )
	GFXDECODE_ENTRY( "gfx1", 0x80000, layout_8x8x4,   0, 0x400/0x10 )
GFXDECODE_END


/***************************************************************************
                              Machine Drivers
***************************************************************************/

static MACHINE_DRIVER_START( galaxi )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_10MHz)	// ?
	MDRV_CPU_PROGRAM_MAP(galaxi_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(16*5, 512-16*2-1, 16*1, 256-1)

	MDRV_GFXDECODE(galaxi)
	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(galaxi)
	MDRV_VIDEO_UPDATE(galaxi)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_16MHz/16)	// ?
	MDRV_SOUND_CONFIG(okim6295_interface_pin7low)	// ?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( magjoker )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxi)

	/* sound hardware */
	MDRV_SOUND_MODIFY("oki")

	/* ADPCM samples are recorded with extremely low volume */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 4.0)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( galaxi )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.u48", 0x00000, 0x20000, CRC(53d86ed0) SHA1(d04ad4c79b0ae46d3d5820b16481ea95c1370e6d) )
	ROM_LOAD16_BYTE( "4.u47", 0x00001, 0x20000, CRC(ddd67683) SHA1(68f8969949e1db90a765c1f31cb8957eef505d5f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "3.u34", 0x00000, 0x80000, CRC(4a59ad63) SHA1(34fc1a948fc205f8c55a8e99d143bbdf4d1b220f) )
	ROM_LOAD16_BYTE( "2.u33", 0x00001, 0x80000, CRC(a8b29a97) SHA1(835c6885d5adf0e7600810ad9fcda88c22077495) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u38", 0x00000, 0x40000, CRC(50e289db) SHA1(43c576c014f4c3d22bfa4c932e161d7558d483f6) )
ROM_END

ROM_START( magjoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "25.u48", 0x00000, 0x20000, CRC(505bdef2) SHA1(9c2a525f2eb3cc39bdd6219bad7c5a1a8bc0b274) )
	ROM_LOAD16_BYTE( "24.u47", 0x00001, 0x20000, CRC(380fd0cd) SHA1(bcd6d23e41e249c7e587b253958eec180440639a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u34", 0x00000, 0x80000, CRC(952b7c84) SHA1(a28e1b79444331837ffc07c8d3c16c1d9a3c974c) )
	ROM_LOAD16_BYTE( "22.u33", 0x00001, 0x80000, CRC(41866733) SHA1(257d77f89fcf1e8f36fb6a8fcb8ad48b1127e457) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 4-bit ADPCM mono @ 6 kHz.*/
	ROM_LOAD( "21.u38", 0x00000, 0x40000, CRC(199baf33) SHA1(006708d955481fe1ae44555d27896d18e1ff8440) )
ROM_END


/***************************************************************************
                               Game Drivers
***************************************************************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT  ROT    COMPANY   FULLNAME                      FLAGS  LAYOUT  */
GAMEL( 2000, galaxi,   0,      galaxi,   galaxi,   0,    ROT0, "B.R.L.", "Galaxi (v2.0)",               0,     layout_galaxi )
GAMEL( 2000, magjoker, 0,      magjoker, magjoker, 0,    ROT0, "B.R.L.", "Magic Joker (v1.25.10.2000)", 0,     layout_galaxi )
