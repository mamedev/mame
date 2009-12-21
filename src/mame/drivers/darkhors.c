/***************************************************************************

                            -= Dark Horse =-

                driver by   Luca Elia (l.elia@tin.it)

A bootleg of 1997 Seta's "Jockey Club II" on inferior hardware:

|-----------------------------------------------------------|
|  M6295    SND              GM76C512 GM76C512  |-------|   |
|                            GM76C512 GM76C512  |ACTEL  |   |
|      EEPROM         PRG1   GM76C512 GM76C512  |A40MX04|   |
|                            GM76C512 GM76C512  |       |   |
|                     PRG2                      |-------|   |
|   |-------|                          62256   GM71C4260    |
|   |ACTEL  |          3.6V_BATT       62256   GM71C4260    |
|   |A40MX04| 68EC020      GAL                              |
|   |       |              GAL                         GAL  |
|J  |-------|                                               |
|A                                           GFX1    GFX2   |
|M                                                          |
|M                                                   GFX3   |
|A                                                          |
|                                                    GFX4   |
|  62256                                                    |
|                                                    GFX5   |
|  62256                       |-------|                    |
|           62256              |ACTEL  |             GFX6   |
|                              |A42MX09|                    |
|           62256              |       |             GFX7   |
|                              |-------|                    |
|           GAL                                      GFX8   |
|           GAL                                             |
|           36MHz                                           |
|-----------------------------------------------------------|

Notes:

- Sprite ram is parsed and converted from the original format (gdfs zooming sprites +
  ssv row sprites?, see ssv.c) to this harware's plain sprites + 2 tilemaps.
- During boot press: Test (F1) for test menu; Service (9) for config menu.
- This game uses 72 input keys!

To do:

- Emulate coin double sensor. At the moment you will need to enter the config menu
  (9 during boot), change to single sensor, press 9 again then 1 to save.
- Remove the patch to pass the eeprom test.
- Extend palette to 0x20000 entries (palette_start prevents more than 0x10000).
- Fix the disalignment between sprites and tilemap (gap in the fence) during play,
  without breaking the other screens, which are fine.
- Implement a few missing inputs / outputs.
- Correct clocks

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "sound/st0016.h"
#include "includes/st0016.h"
#include "cpu/z80/z80.h"

#define DARKHORS_DEBUG	0

/***************************************************************************


                                Video Hardware


***************************************************************************/

static VIDEO_START( darkhors );
static VIDEO_UPDATE( darkhors );

static tilemap *darkhors_tmap, *darkhors_tmap2;
static UINT32 *darkhors_tmapram,  *darkhors_tmapscroll;
static UINT32 *darkhors_tmapram2, *darkhors_tmapscroll2;

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 tile		=	darkhors_tmapram[tile_index] >> 16;
	UINT16 color	=	darkhors_tmapram[tile_index] & 0xffff;
	SET_TILE_INFO(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 tile		=	darkhors_tmapram2[tile_index] >> 16;
	UINT16 color	=	darkhors_tmapram2[tile_index] & 0xffff;
	SET_TILE_INFO(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

static WRITE32_HANDLER( darkhors_tmapram_w )
{
	COMBINE_DATA(&darkhors_tmapram[offset]);
	tilemap_mark_tile_dirty(darkhors_tmap, offset);
}
static WRITE32_HANDLER( darkhors_tmapram2_w )
{
	COMBINE_DATA(&darkhors_tmapram2[offset]);
	tilemap_mark_tile_dirty(darkhors_tmap2, offset);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT32 *s		=	machine->generic.spriteram.u32;
	UINT32 *end		=	machine->generic.spriteram.u32 + 0x02000/4;

	for ( ; s < end; s += 8/4 )
	{
		int attr, code, color, sx, sy, flipx, flipy;

		sx		=		(s[ 0 ] >> 16);
		sy		=		(s[ 0 ] & 0xffff);
		attr	=		(s[ 1 ] >> 16);
		code	=		(s[ 1 ] & 0xffff);

		// Last sprite
		if (sx & 0x8000) break;

		flipx	=	0;
		flipy	=	0;
		color	=	(attr & 0x0200) ? (attr & 0x1ff) : (attr & 0x1ff) * 4;

		// Sign extend the position
		sx	=	(sx & 0x1ff) - (sx & 0x200);
		sy	=	(sy & 0x1ff) - (sy & 0x200);

		sy	=	-sy;
		sy	+=	0xf8;

		drawgfx_transpen(	bitmap,	cliprect, machine->gfx[0],
					code/2,	color,
					flipx,	flipy,	sx,	sy, 0);
	}
}

static VIDEO_START( darkhors )
{
	darkhors_tmap			=	tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows,
												16,16, 0x40,0x40	);

	darkhors_tmap2			=	tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows,
												16,16, 0x40,0x40	);

	tilemap_set_transparent_pen(darkhors_tmap, 0);
	tilemap_set_transparent_pen(darkhors_tmap2, 0);

	machine->gfx[0]->color_granularity = 64; /* 256 colour sprites with palette selectable on 64 colour boundaries */
}

static VIDEO_UPDATE( darkhors )
{
	int layers_ctrl = -1;

#if DARKHORS_DEBUG
	if (input_code_pressed(screen->machine, KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(screen->machine, KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(screen->machine, KEYCODE_W))	mask |= 2;
		if (input_code_pressed(screen->machine, KEYCODE_A))	mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	tilemap_set_scrollx(darkhors_tmap,0, (darkhors_tmapscroll[0] >> 16) - 5);
	tilemap_set_scrolly(darkhors_tmap,0, (darkhors_tmapscroll[0] & 0xffff) - 0xff );
	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, darkhors_tmap, TILEMAP_DRAW_OPAQUE, 0);

	tilemap_set_scrollx(darkhors_tmap2,0, (darkhors_tmapscroll2[0] >> 16) - 5);
	tilemap_set_scrolly(darkhors_tmap2,0, (darkhors_tmapscroll2[0] & 0xffff) - 0xff );
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, darkhors_tmap2, 0, 0);

	if (layers_ctrl & 4)	draw_sprites(screen->machine,bitmap,cliprect);

#if DARKHORS_DEBUG
#if 0
	popmessage("%04X-%04X %04X-%04X %04X-%04X %04X-%04X %04X-%04X %04X-%04X",
		darkhors_tmapscroll[0] >> 16, darkhors_tmapscroll[0] & 0xffff,
		darkhors_tmapscroll[1] >> 16, darkhors_tmapscroll[1] & 0xffff,
		darkhors_tmapscroll[2] >> 16, darkhors_tmapscroll[2] & 0xffff,
		darkhors_tmapscroll[3] >> 16, darkhors_tmapscroll[3] & 0xffff,
		darkhors_tmapscroll[4] >> 16, darkhors_tmapscroll[4] & 0xffff,
		darkhors_tmapscroll[5] >> 16, darkhors_tmapscroll[5] & 0xffff
	);
#endif
#endif
	return 0;
}

/***************************************************************************


                                Memory Map


***************************************************************************/

static const eeprom_interface eeprom_intf =
{
	7,				// address bits 7
	8,				// data bits    8
	"*110",			// read         1 10 aaaaaaa
	"*101",			// write        1 01 aaaaaaa dddddddd
	"*111",			// erase        1 11 aaaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

static NVRAM_HANDLER( darkhors )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(machine, &eeprom_intf);

		if (file) eeprom_load(file);
		else
		{
			// Set the EEPROM to Factory Defaults
			eeprom_set_data(memory_region(machine, "user1"),(1<<7));
		}
	}
}

static WRITE32_HANDLER( darkhors_eeprom_w )
{
	if (data & ~0xff000000)
		logerror("CPU #0 PC: %06X - Unknown EEPROM bit written %08X\n",cpu_get_pc(space->cpu),data);

	if ( ACCESSING_BITS_24_31 )
	{
		// latch the bit
		eeprom_write_bit(data & 0x04000000);

		// reset line asserted: reset.
		eeprom_set_cs_line((data & 0x01000000) ? CLEAR_LINE : ASSERT_LINE );

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

static WRITE32_HANDLER( paletteram32_xBBBBBGGGGGRRRRR_dword_w )
{
	if (ACCESSING_BITS_16_31)	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset*2, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset*2+1, data, mem_mask);
}

static UINT32 input_sel;
static WRITE32_HANDLER( darkhors_input_sel_w )
{
	COMBINE_DATA(&input_sel);
//  if (ACCESSING_BITS_16_31)    popmessage("%04X",data >> 16);
}

static int mask_to_bit( int mask )
{
	switch( mask )
	{
		default:
		case 0x01:	return 0;
		case 0x02:	return 1;
		case 0x04:	return 2;
		case 0x08:	return 3;
		case 0x10:	return 4;
		case 0x20:	return 5;
		case 0x40:	return 6;
		case 0x80:	return 7;
	}
}

static READ32_HANDLER( darkhors_input_sel_r )
{
	// from bit mask to bit number
	int bit_p1 = mask_to_bit((input_sel & 0x00ff0000) >> 16);
	int bit_p2 = mask_to_bit((input_sel & 0xff000000) >> 24);
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7" };

	return	(input_port_read(space->machine, portnames[bit_p1]) & 0x00ffffff) |
			(input_port_read(space->machine, portnames[bit_p2]) & 0xff000000) ;
}

static WRITE32_HANDLER( darkhors_unk1_w )
{
	// 0x1000 lockout
//  if (ACCESSING_BITS_16_31)    popmessage("%04X",data >> 16);
}

static ADDRESS_MAP_START( darkhors_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x490040, 0x490043) AM_WRITE(darkhors_eeprom_w)
	AM_RANGE(0x4e0080, 0x4e0083) AM_READ_PORT("4e0080") AM_WRITE(darkhors_unk1_w)

	AM_RANGE(0x580000, 0x580003) AM_READ_PORT("580000")
	AM_RANGE(0x580004, 0x580007) AM_READ_PORT("580004")
	AM_RANGE(0x580008, 0x58000b) AM_READ(darkhors_input_sel_r)
	AM_RANGE(0x58000c, 0x58000f) AM_WRITE(darkhors_input_sel_w)
	AM_RANGE(0x580084, 0x580087) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0xff000000)
	AM_RANGE(0x580200, 0x580203) AM_READNOP
	AM_RANGE(0x580400, 0x580403) AM_READ_PORT("580400")
	AM_RANGE(0x580420, 0x580423) AM_READ_PORT("580420")

	AM_RANGE(0x800000, 0x86bfff) AM_RAM
	AM_RANGE(0x86c000, 0x86ffff) AM_RAM_WRITE(darkhors_tmapram_w) AM_BASE(&darkhors_tmapram)
	AM_RANGE(0x870000, 0x873fff) AM_RAM_WRITE(darkhors_tmapram2_w) AM_BASE(&darkhors_tmapram2)
	AM_RANGE(0x874000, 0x87dfff) AM_RAM
	AM_RANGE(0x87e000, 0x87ffff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x880000, 0x89ffff) AM_WRITE(paletteram32_xBBBBBGGGGGRRRRR_dword_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x8a0000, 0x8bffff) AM_WRITEONLY	// this should still be palette ram!
	AM_RANGE(0x8c0120, 0x8c012f) AM_WRITEONLY AM_BASE(&darkhors_tmapscroll)
	AM_RANGE(0x8c0130, 0x8c013f) AM_WRITEONLY AM_BASE(&darkhors_tmapscroll2)
ADDRESS_MAP_END

static UINT32* jclub2_tileram;
static int jclub2_gfx_index;

static WRITE32_HANDLER( jclub2_tileram_w )
{
	COMBINE_DATA(&jclub2_tileram[offset]);
	gfx_element_mark_dirty(space->machine->gfx[jclub2_gfx_index], offset/(256/4));

}

static ADDRESS_MAP_START( jclub2_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x490040, 0x490043) AM_WRITE(darkhors_eeprom_w)
	AM_RANGE(0x4e0080, 0x4e0083) AM_READ_PORT("4e0080") AM_WRITE(darkhors_unk1_w)

	AM_RANGE(0x580000, 0x580003) AM_READ_PORT("580000")
	AM_RANGE(0x580004, 0x580007) AM_READ_PORT("580004")
	AM_RANGE(0x580008, 0x58000b) AM_READ(darkhors_input_sel_r)
	AM_RANGE(0x58000c, 0x58000f) AM_WRITE(darkhors_input_sel_w)
	AM_RANGE(0x580200, 0x580203) AM_READNOP
	AM_RANGE(0x580400, 0x580403) AM_READ_PORT("580400")
	AM_RANGE(0x580420, 0x580423) AM_READ_PORT("580420")

	AM_RANGE(0x800000, 0x87ffff) AM_RAM AM_BASE_GENERIC(spriteram)

	AM_RANGE(0x880000, 0x89ffff) AM_WRITE(paletteram32_xBBBBBGGGGGRRRRR_dword_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x8a0000, 0x8bffff) AM_WRITEONLY	// this should still be palette ram!

	AM_RANGE(0x8C0000, 0x8C01ff) AM_RAM
	AM_RANGE(0x8E0000, 0x8E01ff) AM_RAM

	AM_RANGE(0x900000, 0x90ffff) AM_RAM_WRITE(jclub2_tileram_w) AM_BASE(&jclub2_tileram) // tile data gets decompressed here by main cpu?
ADDRESS_MAP_END


static ADDRESS_MAP_START( jclub2o_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( darkhors )
	PORT_START("580000")
	PORT_BIT( 0xff7fffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("?") PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("580004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BILL1 )	// bill in
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN1 )	// coin in s1
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )	// coin in s2
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1 )	// coin drop
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SPECIAL )	// hopper full
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_ENTER)	// bill in p2
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )	// coin in s1 p2
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_COIN4 )	// coin in s2 p2
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_COIN2 )	// coin drop p2
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SPECIAL)	// hopper full p2

	PORT_START("580400")
	PORT_BIT( 0xfffcffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("580420")
	PORT_BIT( 0xfffcffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0*") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9*") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("4e0080")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_SERVICE1 )	// config
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_SERVICE2 )	// reset
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_SERVICE3 )	// meter
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_SERVICE4 )	// lastgame
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1) // test
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )	// door 1
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )	// door 2
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_CUSTOM(eeprom_bit_r, NULL)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_START1   )	// start
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Payout") PORT_CODE(KEYCODE_LCONTROL)	// payout
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_LALT)		// cancel
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_START2   ) 	// start p2
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Payout") PORT_CODE(KEYCODE_RCONTROL)	// payout p2
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_CODE(KEYCODE_RALT)		// cancel p2
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("IN0")	/* 580008(0) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 8") //PORT_CODE(KEYCODE_)

	PORT_START("IN1")	/* 580008(1) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-7") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-8") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")	/* 580008(2) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-8") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* 580008(3) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-7") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-8") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")	/* 580008(4) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-7") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-8") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")	/* 580008(5) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-7") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-8") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")	/* 580008(6) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-7") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-8") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")	/* 580008(7) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7-8") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************


                                Gfx Layouts


***************************************************************************/

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{	RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0	},
	{ STEP8(0,1), STEP8(16,1) },
	{ STEP16(0,16*2)},
	16*16*2
};

static GFXDECODE_START( darkhors )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0, 0x10000/64 )	// color codes should be doubled
GFXDECODE_END

// wrong
static const gfx_layout layout_16x16x8_jclub2 =
{
	16,16,
	0x10000/256,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 24,16,8,0,  56,48,40,32,  88,80,72,64,  120, 112, 104, 96},
	{ 0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	256*8
};

static GFXDECODE_START( jclub2 )
	//GFXDECODE_ENTRY( "maincpu", 0, layout_16x16x8_jclub2, 0, 0x10000/64 ) // color codes should be doubled
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static INTERRUPT_GEN( darkhors )
{
	switch (cpu_getiloops(device))
	{
		case 0:	cpu_set_input_line(device, 3, HOLD_LINE);	break;
		case 1:	cpu_set_input_line(device, 4, HOLD_LINE);	break;
		case 2:	cpu_set_input_line(device, 5, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( darkhors )
	MDRV_CPU_ADD("maincpu", M68EC020, 12000000) // 36MHz/3 ??
	MDRV_CPU_PROGRAM_MAP(darkhors_map)
	MDRV_CPU_VBLANK_INT_HACK(darkhors,3)

	MDRV_NVRAM_HANDLER(darkhors)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x190, 0x100)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)

	MDRV_GFXDECODE(darkhors)
	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(darkhors)
	MDRV_VIDEO_UPDATE(darkhors)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, 528000)	// ??
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



static VIDEO_START(jclub2)
{
 	/* find first empty slot to decode gfx */
	for (jclub2_gfx_index = 0; jclub2_gfx_index < MAX_GFX_ELEMENTS; jclub2_gfx_index++)
		if (machine->gfx[jclub2_gfx_index] == 0)
			break;

	assert(jclub2_gfx_index != MAX_GFX_ELEMENTS);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine->gfx[jclub2_gfx_index] = gfx_element_alloc(machine, &layout_16x16x8_jclub2, (UINT8 *)jclub2_tileram, machine->config->total_colors / 16, 0);


}

static VIDEO_UPDATE(jclub2)
{
	return 0;
}

static MACHINE_DRIVER_START( jclub2 )
	MDRV_CPU_ADD("maincpu", M68EC020, 12000000)
	MDRV_CPU_PROGRAM_MAP(jclub2_map)
	MDRV_CPU_VBLANK_INT_HACK(darkhors,3)

	MDRV_NVRAM_HANDLER(darkhors)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x190, 0x100)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)

	MDRV_GFXDECODE(jclub2)
	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(jclub2)
	MDRV_VIDEO_UPDATE(jclub2)
MACHINE_DRIVER_END

static ADDRESS_MAP_START( st0016_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE("stsnd", st0016_snd_r, st0016_snd_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( st0016_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	//AM_RANGE(0xc0, 0xc0) AM_READ(cmd1_r)
	//AM_RANGE(0xc1, 0xc1) AM_READ(cmd2_r)
	//AM_RANGE(0xc2, 0xc2) AM_READ(cmd_stat8_r)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe7, 0xe7) AM_WRITE(st0016_rom_bank_w)
	//AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END

static const st0016_interface st0016_config =
{
	&st0016_charram
};

static VIDEO_START(jclub2o)
{

}

static VIDEO_UPDATE(jclub2o)
{
	return 0;
}

static MACHINE_DRIVER_START( jclub2o )
	MDRV_CPU_ADD("st0016",Z80,8000000)
	MDRV_CPU_PROGRAM_MAP(st0016_mem)
	MDRV_CPU_IO_MAP(st0016_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("maincpu", M68EC020, 12000000)
	MDRV_CPU_PROGRAM_MAP(jclub2o_map)
	//MDRV_CPU_VBLANK_INT_HACK(darkhors,3)

	MDRV_NVRAM_HANDLER(darkhors)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x190, 0x100)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)

	MDRV_GFXDECODE(jclub2)
	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(jclub2o)
	MDRV_VIDEO_UPDATE(jclub2o)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("stsnd", ST0016, 0)
	MDRV_SOUND_CONFIG(st0016_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

ROM_START( darkhors )
	ROM_REGION( 0x100000, "maincpu", 0 )	// 68EC020 code
	ROM_LOAD32_WORD_SWAP( "prg2", 0x00000, 0x80000, CRC(f2ec5818) SHA1(326937a331496880f517f41b0b8ab54e55fd7af7) )
	ROM_LOAD32_WORD_SWAP( "prg1", 0x00002, 0x80000, CRC(b80f8f59) SHA1(abc26dd8b36da0d510978364febe385f69fb317f) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "gfx1", 0x000000, 0x80000, CRC(e9fe9967) SHA1(a79d75c09f0eac6372de8d6e98c5eecf38ef750c) )
	ROM_LOAD( "gfx2", 0x080000, 0x80000, CRC(0853c5c5) SHA1(2b49ffe607278817f1f8219a79f5906be53ee6f4) )
	ROM_LOAD( "gfx3", 0x100000, 0x80000, CRC(6e89278f) SHA1(044c15e00ea95fd3f108fa916000a1000789c8e8) )
	ROM_LOAD( "gfx4", 0x180000, 0x80000, CRC(f28407ab) SHA1(47933719cff8099fc079fd736b4b08176f3aff66) )
	ROM_LOAD( "gfx5", 0x200000, 0x80000, CRC(281402cd) SHA1(77f8e5e02c6e7161299c06e65a078c1cdda1ba66) )
	ROM_LOAD( "gfx6", 0x280000, 0x80000, CRC(8ea0149b) SHA1(7792fd7e07a7baa4e15f50b6528c78fb15b40b40) )
	ROM_LOAD( "gfx7", 0x300000, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // FIXED BITS (11111111)
	ROM_FILL(         0x300000, 0x80000, 0 ) // a zero-fill seems fine
	ROM_LOAD( "gfx8", 0x380000, 0x80000, CRC(590bec2a) SHA1(7fdbb21f1a3eccde65e91eb2443a0e01487c59c3) ) // 000xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 )	// Samples
	ROM_LOAD( "snd", 0x00000, 0x80000, CRC(7aeb12d3) SHA1(3e81725fc206baa7559da87552a0cd73b7616155) )

	ROM_REGION( 0x80000, "user1", ROMREGION_BE )	// EEPROM
	ROM_LOAD( "eeprom", 0x00000, 0x80000, CRC(45314fdb) SHA1(c4bd5508e5b51a6e0356c049f1ccf2b5d94caee9) )
ROM_END

/*

Jockey Club II Original sets

 -- unlike the bootleg these appear to have the graphics compressed in the main program roms.  (CPS3 / SRMP6 -like)  (I think CPS3 is a SETA design)
 -- the older hardware uses an st-0016 for sound (see st0016.c)
 -- the program can be upgraded by placing a ROM in the socket next to the main program ROM.

 */



 /*
Jockey Club II by SETA 1996

PCB E79-001 rev 01a (Newer)

Main CPU : SETA ST-0032 70C600JF505

Others : MC68EC020FG16
     SETA ST-0013
     SETA ST-0017

Xtal : 42.9545 MHz, 60.0000 MHz, 33.3333 MHz


Rams :  Toshiba : TC5118160CJ-60 (x3)
    NKK N341256SJ-15 (x2)
    NEC D43001GU-70LL (x4)

Gals : gal16V8B(x2) ref : M88-03 M88-04

Eeprom : 93c46

Eproms : M88-01.u38,M88-023.u6 (read as 578200) (1st set)
     M88-01A.u38,M88-02.u6 (2nd set)
     M88-01B.u38,M88-02.u6 (3rd set)
     M88-01.u38,M88-02.u6,Z201x.u39 (4th set)
     M88-01A.u38,M88-02.u6 M88-03D.u39 (5th set)
     M88-01B.u38,M88-02.u6,M88-03D.u39 (6th set)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.
*/


// ToDo: check for other similarities with CPS3 hardware, there are more than a few things which look the same.

// this contains mutliple sets, although splitting them as listed above makes no sense.. especially not the 'subcpu' roms
ROM_START( jclub2 )
	ROM_REGION( 0x200000, "maincpu", 0 )	// 68EC020 code  + compressed GFX
	// main program (similar to main program of bootleg
	ROM_LOAD16_WORD_SWAP( "m88-01b.u38",0x00000, 0x200000, CRC(f1054c69) SHA1(be6d92653f0d3cc0a36a2ff0798043f4a95439bc) )
	ROM_LOAD16_WORD_SWAP( "m88-01a.u38",0x00000, 0x200000, CRC(c1243e1c) SHA1(2a5857738b8950daf77ddaa8304b765f809f8241) ) // alt revision?
	ROM_LOAD16_WORD_SWAP( "m88-01.u38", 0x00000, 0x200000, CRC(84476b68) SHA1(1014d23d3cebbfa9aa3bfb90505529989a8eedfa) ) // alt revision?

	ROM_REGION( 0x200000, "patch", 0 )	// 68EC020 code
	// it appears that the operator could place a ROM in the socket next to the main CPU rom to update the main program rom by
	// overriding the initial 0x80000 bytes of the program rom.
	ROM_LOAD16_WORD_SWAP( "m88-03d.u39",0x00000, 0x080000, CRC(723dd22b) SHA1(0ca622e0dd315f29e72dd9b82fb419d306ec5df8) )
	ROM_LOAD16_WORD_SWAP( "z201x.u39",0x00000, 0x080000, CRC(1fb79c16) SHA1(c8914f7dfc17c412f6ca756f8eb6d6a35e3b6214) )

	ROM_REGION( 0x100000, "oki", 0 )	// Samples? (not oki probably one of the ST-xx customs, no idea if the dump is good)
	// data distribution would indicate this is a sound rom
	ROM_LOAD( "m88-02.u6", 0x00000, 0x100000, CRC(0dd3436a) SHA1(809d3b7a26d36f71da04036fd8ab5d0c5089392a) )

	ROM_REGION( 0x80000, "user1", ROMREGION_ERASEFF | ROMREGION_BE )	// EEPROM

	ROM_REGION( 0x80000, "misc", ROMREGION_ERASEFF )
	ROM_LOAD( "gal16v8b-m88-03.bin", 0x000, 0x117, CRC(6d9c882e) SHA1(84cb95ab540290c2f8b740668360e9c643a67dcf) )
	ROM_LOAD( "gal16v8b-m88-04.bin", 0x000, 0x117, CRC(5e79f292) SHA1(5e44c234e2b15d486a1af71fee986892aa245b4d) )
ROM_END

/*
Jockey Club II by SETA 1994-1995

Other hardware version (older):

Main PCB: E06-00409
Sub PCb : E06-00407 (I/O nothing else)

Main CPU : SETA ST-0020

Many XTAL : 48.0000 MHz,33.3333 MHz,4.91520 MHz,42.9545 MHz(x2),105.0000 MHz (this 105.0000 Xtal is sometimes replaced by a tiny pcb silscreened 108.0000 MHz(!), with ICS ICS1494N, MB3771 and 14.3181 MHz Xtal)

Others : MC68EC020FG16
     SETA ST-0013
     SETA ST-0016  <-- z80 core + simple gfx + sound, see st0016.c
     SETA ST-0017

Rams :  Toshiba TC514800AJ-70
    Toshiba TC514000ASJ-70 (x8)
    Sharp LH5168D

Eproms : SX006A-01.u26 read as 5716200,JC2-110x.u27,SX006-04.u87 (sound)
     SX006B-01.u26,JC2-110x.u27,SX006-04.u87 (sound)
     SX006A-01.u26,JC2-112x.u27,SX006-04.u87 (sound)
     SX006B-01.u26,JC2-112x.u27,SX006-04.u87 (sound)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.

*/

// this contains mutliple sets
ROM_START( jclub2o )
	ROM_REGION( 0x200000, "maincpu", 0 )	// 68EC020 code + compressed gfx
	ROM_LOAD16_WORD_SWAP( "sx006a-01.106",0x00000, 0x200000, CRC(55e249bc) SHA1(ed0f066ed17f047760b712cbbfba1a62d4b452ba) )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26",0x00000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) )  // alt revision?

	ROM_REGION( 0x200000, "patch", 0 )	// 68EC020 code
	// it appears that the operator could place a ROM in the socket next to the main CPU rom to update the main program rom by
	// overriding the initial 0x80000 bytes of the program rom.
	ROM_LOAD16_WORD_SWAP( "jc2-110x.u27",0x00000, 0x080000, CRC(03aa6882) SHA1(e0343bc77a19994ddafa614891663b40e1476332) )
	ROM_LOAD16_WORD_SWAP( "jc2-112x.u27",0x00000, 0x080000, CRC(e1ab93bd) SHA1(78b618b3f7819bd5351ebf949f328fec7795cec9) ) // alt revision?

	ROM_REGION( 0x90000, "st0016", 0 ) // z80 core (used for sound?)
	ROM_LOAD( "sx006-04.u87", 0x10000, 0x80000, CRC(a87adedd) SHA1(1cd5af2d03738fff2230b46241659179467c828c) )
	ROM_COPY( "st0016",  0x10000, 0x00000, 0x08000 )

	ROM_REGION( 0x80000, "user1", ROMREGION_ERASEFF | ROMREGION_BE )	// EEPROM
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

static DRIVER_INIT( darkhors )
{
	UINT32 *rom    = (UINT32 *) memory_region(machine, "maincpu");
	UINT8  *eeprom = (UINT8 *)  memory_region(machine, "user1");
	int i;

#if 1
	// eeprom error patch
	rom[0x0444c/4]	=	0x02b34e71;
	rom[0x04450/4]	=	0x4e710839;

	rom[0x045fc/4]	=	0xbe224e71;
	rom[0x04600/4]	=	0x4e714eb9;
#endif

	for (i = 0; i < (1<<7); i++)
		eeprom[i] = eeprom[i*2];
}

GAME( 199?, jclub2,   0,      jclub2,  darkhors, 0,        ROT0, "Seta", "Jockey Club II (newer hardware)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 199?, jclub2o,  jclub2, jclub2o, darkhors, 0,        ROT0, "Seta", "Jockey Club II (older hardware)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, darkhors, jclub2, darkhors, darkhors, darkhors, ROT0, "bootleg", "Dark Horse (bootleg of Jockey Club II)", GAME_IMPERFECT_GRAPHICS )
