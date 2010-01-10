/*

1945 K-3 driver
---------------

1945K-III
Oriental, 2000

This game is a straight rip-off of Psikyo's Strikers 1945 III.

PCB Layout
----------

ORIENTAL SOFT INC., -OPCX2-
|--------------------------------------------|
|    AD-65   SND-1.SU7            M16M-1.U62 |
|                     PAL                    |
|    AD-65   SND-2.SU4                       |
|                                 M16M-2.U63 |
|                                            |
|                    KM681000                |
|J                   KM681000     6116       |
|A                                           |
|M          62256    |-------|    6116       |
|M          62256    |SPR800E|               |
|A                   |OP-CX1 |    6116  6116 |
|    6116   PRG-1.U51|QFP208 |               |
|                    |-------|    6116  6116 |
|    6116   PRG-2.U52                        |
|                 |-----| |------|           |
|           PAL   |     | |QL2003| M16M-3.U61|
|           PAL   |68000| |PLCC84|           |
|DSW1 DSW2        |-----| |------| PAL       |
|             16MHz        27MHz             |
|--------------------------------------------|
Notes:
     68000 clock : 16.000MHz
    M6295 clocks : 1.000MHz (both), sample rate = 1000000 / 132
           VSync : 60Hz


SAVE STATE (lee@lmservers.com):
No code changes required to support save state
1945kiii uses the 68000 and OKIM6295 which both support save state.
The rationale for saving/not saving are as follows:

static UINT16* k3_spriteram_1;  Saved via reference to AM_BASE
static UINT16* k3_spriteram_2;  Saved via reference to AM_BASE
static UINT16* k3_bgram;        Saved via reference to AM_BASE
static tilemap_t *k3_bg_tilemap;  Saved due to tilemap supporting save

There are no static local variables.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#define MASTER_CLOCK	XTAL_16MHz


typedef struct _k3_state k3_state;
struct _k3_state
{
	/* memory pointers */
	UINT16 *  spriteram_1;
	UINT16 *  spriteram_2;
	UINT16 *  bgram;
//  UINT16 *  paletteram16; // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* devices */
	const device_config *oki1;
	const device_config *oki2;
};


static WRITE16_HANDLER( k3_bgram_w )
{
	k3_state *state = (k3_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bgram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static TILE_GET_INFO( get_k3_bg_tile_info )
{
	k3_state *state = (k3_state *)machine->driver_data;
	int tileno = state->bgram[tile_index];
	SET_TILE_INFO(1, tileno, 0, 0);
}

static VIDEO_START(k3)
{
	k3_state *state = (k3_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_k3_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 64);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	k3_state *state = (k3_state *)machine->driver_data;
	const gfx_element *gfx = machine->gfx[0];
	UINT16 *source = state->spriteram_1;
	UINT16 *source2 = state->spriteram_2;
	UINT16 *finish = source + 0x1000 / 2;

	while (source < finish)
	{
		int xpos, ypos;
		int tileno;
		xpos = (source[0] & 0xff00) >> 8;
		ypos = (source[0] & 0x00ff) >> 0;
		tileno = (source2[0] & 0x7ffe) >> 1;
		xpos |=  (source2[0] & 0x0001) << 8;
		drawgfx_transpen(bitmap, cliprect, gfx, tileno, 1, 0, 0, xpos, ypos, 0);
		drawgfx_transpen(bitmap, cliprect, gfx, tileno, 1, 0, 0, xpos, ypos - 0x100, 0); // wrap
		drawgfx_transpen(bitmap, cliprect, gfx, tileno, 1, 0, 0, xpos - 0x200, ypos, 0); // wrap
		drawgfx_transpen(bitmap, cliprect, gfx, tileno, 1, 0, 0, xpos - 0x200, ypos - 0x100, 0); // wrap

		source++;
		source2++;
	}
}

static VIDEO_UPDATE(k3)
{
	k3_state *state = (k3_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


static WRITE16_HANDLER( k3_scrollx_w )
{
	k3_state *state = (k3_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, 0, data);
}

static WRITE16_HANDLER( k3_scrolly_w )
{
	k3_state *state = (k3_state *)space->machine->driver_data;
	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}

static WRITE16_HANDLER( k3_soundbanks_w )
{
	k3_state *state = (k3_state *)space->machine->driver_data;
	okim6295_set_bank_base(state->oki1, (data & 4) ? 0x40000 : 0);
	okim6295_set_bank_base(state->oki2, (data & 2) ? 0x40000 : 0);
}

static ADDRESS_MAP_START( k3_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0009ce, 0x0009cf) AM_WRITENOP	// bug in code? (clean up log)
	AM_RANGE(0x0009d2, 0x0009d3) AM_WRITENOP	// bug in code? (clean up log)

	AM_RANGE(0x000000, 0x0fffff) AM_ROM	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM	// Main Ram
	AM_RANGE(0x200000, 0x200fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)	// palette
	AM_RANGE(0x240000, 0x240fff) AM_RAM AM_BASE_MEMBER(k3_state, spriteram_1)
	AM_RANGE(0x280000, 0x280fff) AM_RAM AM_BASE_MEMBER(k3_state, spriteram_2)
	AM_RANGE(0x2c0000, 0x2c0fff) AM_RAM_WRITE(k3_bgram_w) AM_BASE_MEMBER(k3_state, bgram)
	AM_RANGE(0x340000, 0x340001) AM_WRITE(k3_scrollx_w)
	AM_RANGE(0x380000, 0x380001) AM_WRITE(k3_scrolly_w)
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITE(k3_soundbanks_w)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x440000, 0x440001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x480000, 0x480001) AM_READ_PORT("DSW")
	AM_RANGE(0x4c0000, 0x4c0001) AM_DEVREADWRITE8("oki2", okim6295_r, okim6295_w, 0xff00)
	AM_RANGE(0x500000, 0x500001) AM_DEVREADWRITE8("oki1", okim6295_r, okim6295_w, 0xff00)
	AM_RANGE(0x8c0000, 0x8cffff) AM_RAM	// not used?
ADDRESS_MAP_END

static INPUT_PORTS_START( k3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* Are these used at all? */

	PORT_START("DSW")
	PORT_DIPNAME( 0x007,  0x0007, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0002, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static const gfx_layout k3_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
	  8*128, 9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128
};

static GFXDECODE_START( 1945kiii )
	GFXDECODE_ENTRY( "gfx1", 0, k3_layout,   0x0, 2  ) /* bg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, k3_layout,   0x0, 2  ) /* bg tiles */
GFXDECODE_END


static MACHINE_START( 1945kiii )
{
	k3_state *state = (k3_state *)machine->driver_data;

	state->oki1 = devtag_get_device(machine, "oki1");
	state->oki2 = devtag_get_device(machine, "oki2");
}

static MACHINE_DRIVER_START( k3 )

	/* driver data */
	MDRV_DRIVER_DATA(k3_state)

	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(k3_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_MACHINE_START(1945kiii)

	MDRV_GFXDECODE(1945kiii)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(k3)
	MDRV_VIDEO_UPDATE(k3)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki1", OKIM6295, MASTER_CLOCK/16) /* dividers? */
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("oki2", OKIM6295, MASTER_CLOCK/16) /* dividers? */
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



ROM_START( 1945kiii )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "prg-1.u51", 0x00001, 0x80000, CRC(6b345f27) SHA1(60867fa0e2ea7ebdd4b8046315ee0c83e5cf0d74) )
	ROM_LOAD16_BYTE( "prg-2.u52", 0x00000, 0x80000, CRC(ce09b98c) SHA1(a06bb712b9cf2249cc535de4055b14a21c68e0c5) )

	ROM_REGION( 0x080000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "snd-2.su4", 0x00000, 0x80000, CRC(47e3952e) SHA1(d56524621a3f11981e4434e02f5fdb7e89fff0b4) )

	ROM_REGION( 0x080000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "snd-1.su7", 0x00000, 0x80000, CRC(bbb7f0ff) SHA1(458cf3a0c2d42110bc2427db675226c6b8d30999) )

	ROM_REGION( 0x400000, "gfx1", 0 ) // sprites
	ROM_LOAD32_WORD( "m16m-1.u62", 0x000000, 0x200000, CRC(0b9a6474) SHA1(6110ecb17d0fef25935986af9a251fc6e88e3993) )
	ROM_LOAD32_WORD( "m16m-2.u63", 0x000002, 0x200000, CRC(368a8c2e) SHA1(4b1f360c4a3a86d922035774b2c712be810ec548) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // bg tiles
	ROM_LOAD( "m16m-3.u61", 0x00000, 0x200000, CRC(32fc80dd) SHA1(bee32493a250e9f21997114bba26b9535b1b636c) )
ROM_END

GAME( 2000, 1945kiii, 0, k3, k3, 0, ROT270, "Oriental", "1945k III", GAME_SUPPORTS_SAVE )
