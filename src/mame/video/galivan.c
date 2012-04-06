/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

d800-dbff   foreground:         char low bits (1 screen * 1024 chars/screen)
dc00-dfff   foreground attribute:   7       ?
                         6543       color
                             21 ?
                               0    char hi bit


e000-e0ff   spriteram:  64 sprites (4 bytes/sprite)
        offset :    0       1       2       3
        meaning:    ypos(lo)    sprite(lo)  attribute   xpos(lo)
                                7   flipy
                                6   flipx
                                5-2 color
                                1   sprite(hi)
                                0   xpos(hi)


background: 0x4000 bytes of ROM:    76543210    tile code low bits
        0x4000 bytes of ROM:    7       ?
                         6543       color
                             2  ?
                              10    tile code high bits

***************************************************************************/

#include "emu.h"
#include "includes/galivan.h"

/* Layers has only bits 5-6 active.
   6 selects background off/on
   5 controls sprite priority (active only on title screen,
     not for scores or push start nor game)
*/


/* Notes:
     write_layers and layers are used in galivan/dangar but not ninjemak
     ninjemak_dispdisable is used in ninjemak but not galivan/dangar
*/



/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( galivan )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0-0x7f */
	for (i = 0; i < 0x80; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* I think that */
	/* background tiles use colors 0xc0-0xff in four banks */
	/* the bottom two bits of the color code select the palette bank for */
	/* pens 0-7; the top two bits for pens 8-15. */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry;

		if (i & 0x08)
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0xc0) >> 2);
		else
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0x30) >> 0);

		colortable_entry_set_value(machine.colortable, 0x80 + i, ctabentry);
	}

	/* sprites use colors 0x80-0xbf in four banks */
	/* The lookup table tells which colors to pick from the selected bank */
	/* the bank is selected by another PROM and depends on the top 7 bits of */
	/* the sprite code. The PROM selects the bank *separately* for pens 0-7 and */
	/* 8-15 (like for tiles). */
	for (i = 0; i < 0x1000; i++)
	{
		UINT8 ctabentry;
		int i_swapped = ((i & 0x0f) << 8) | ((i & 0xff0) >> 4);

		if (i & 0x80)
			ctabentry = 0x80 | ((i & 0x0c) << 2) | (color_prom[i >> 4] & 0x0f);
		else
			ctabentry = 0x80 | ((i & 0x03) << 4) | (color_prom[i >> 4] & 0x0f);

		colortable_entry_set_value(machine.colortable, 0x180 + i_swapped, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *BGROM = machine.region("gfx4")->base();
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO(
			1,
			code,
			(attr & 0x78) >> 3,		/* seems correct */
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	galivan_state *state = machine.driver_data<galivan_state>();
	int attr = state->m_videoram[tile_index + 0x400];
	int code = state->m_videoram[tile_index] | ((attr & 0x01) << 8);
	SET_TILE_INFO(
			0,
			code,
			(attr & 0xe0) >> 5,		/* not sure */
			0);
	tileinfo.category = attr & 8 ? 0 : 1;	/* seems correct */
}

static TILE_GET_INFO( ninjemak_get_bg_tile_info )
{
	UINT8 *BGROM = machine.region("gfx4")->base();
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO(
			1,
			code,
			((attr & 0x60) >> 3) | ((attr & 0x0c) >> 2),	/* seems correct */
			0);
}

static TILE_GET_INFO( ninjemak_get_tx_tile_info )
{
	galivan_state *state = machine.driver_data<galivan_state>();
	int attr = state->m_videoram[tile_index + 0x400];
	int code = state->m_videoram[tile_index] | ((attr & 0x03) << 8);

	if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
		code = attr = 0x01;

	SET_TILE_INFO(
			0,
			code,
			(attr & 0x1c) >> 2,		/* seems correct ? */
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( galivan )
{
	galivan_state *state = machine.driver_data<galivan_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 128, 128);
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_cols, 8, 8, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(15);
}

VIDEO_START( ninjemak )
{
	galivan_state *state = machine.driver_data<galivan_state>();

	state->m_bg_tilemap = tilemap_create(machine, ninjemak_get_bg_tile_info, tilemap_scan_cols, 16, 16, 512, 32);
	state->m_tx_tilemap = tilemap_create(machine, ninjemak_get_tx_tile_info, tilemap_scan_cols, 8, 8, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(galivan_state::galivan_videoram_w)
{
	m_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

/* Written through port 40 */
WRITE8_MEMBER(galivan_state::galivan_gfxbank_w)
{

	/* bits 0 and 1 coin counters */
	coin_counter_w(machine(), 0,data & 1);
	coin_counter_w(machine(), 1,data & 2);

	/* bit 2 flip screen */
	m_flipscreen = data & 0x04;
	m_bg_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	/* bit 7 selects one of two ROM banks for c000-dfff */
	memory_set_bank(machine(), "bank1", (data & 0x80) >> 7);

	/*  logerror("Address: %04X - port 40 = %02x\n", cpu_get_pc(&space.device()), data); */
}

WRITE8_MEMBER(galivan_state::ninjemak_gfxbank_w)
{

	/* bits 0 and 1 coin counters */
	coin_counter_w(machine(), 0,data & 1);
	coin_counter_w(machine(), 1,data & 2);

	/* bit 2 flip screen */
	m_flipscreen = data & 0x04;
	m_bg_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	/* bit 3 unknown */

	/* bit 4 background disable flag */
	m_ninjemak_dispdisable = data & 0x10;

	/* bit 5 sprite flag ??? */

	/* bit 6, 7 ROM bank select */
	memory_set_bank(machine(), "bank1", (data & 0xc0) >> 6);

#if 0
	{
		char mess[80];
		int btz[8];
		int offs;

		for (offs = 0; offs < 8; offs++) btz[offs] = (((data >> offs) & 0x01) ? 1 : 0);

		sprintf(mess, "BK:%01X%01X S:%01X B:%01X T:%01X FF:%01X C2:%01X C1:%01X", btz[7], btz[6], btz[5], btz[4], btz[3], btz[2], btz[1], btz[0]);
		popmessage(mess);
	}
#endif
}



/* Written through port 41-42 */
WRITE8_MEMBER(galivan_state::galivan_scrollx_w)
{
	if (offset == 1)
	{
		if (data & 0x80)
			m_write_layers = 1;
		else if (m_write_layers)
		{
			m_layers = data & 0x60;
			m_write_layers = 0;
		}
	}
	m_galivan_scrollx[offset] = data;
}

/* Written through port 43-44 */
WRITE8_MEMBER(galivan_state::galivan_scrolly_w)
{
	m_galivan_scrolly[offset] = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	galivan_state *state = machine.driver_data<galivan_state>();
	const UINT8 *spritepalettebank = machine.region("user1")->base();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int code;
		int attr = spriteram[offs + 2];
		int color = (attr & 0x3c) >> 2;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx, sy;

		sx = (spriteram[offs + 3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - spriteram[offs];
		if (state->m_flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

//      code = spriteram[offs + 1] + ((attr & 0x02) << 7);
		code = spriteram[offs + 1] + ((attr & 0x06) << 7);	// for ninjemak, not sure ?

		drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
				code,
				color + 16 * (spritepalettebank[code >> 2] & 0x0f),
				flipx,flipy,
				sx,sy,15);
	}
}


SCREEN_UPDATE_IND16( galivan )
{
	galivan_state *state = screen.machine().driver_data<galivan_state>();
	state->m_bg_tilemap->set_scrollx(0, state->m_galivan_scrollx[0] + 256 * (state->m_galivan_scrollx[1] & 0x07));
	state->m_bg_tilemap->set_scrolly(0, state->m_galivan_scrolly[0] + 256 * (state->m_galivan_scrolly[1] & 0x07));

	if (state->m_layers & 0x40)
		bitmap.fill(0, cliprect);
	else
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	if (state->m_layers & 0x20)
	{
		state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
		state->m_tx_tilemap->draw(bitmap, cliprect, 1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
	}
	else
	{
		draw_sprites(screen.machine(), bitmap, cliprect);
		state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
		state->m_tx_tilemap->draw(bitmap, cliprect, 1, 0);
	}

	return 0;
}

SCREEN_UPDATE_IND16( ninjemak )
{
	galivan_state *state = screen.machine().driver_data<galivan_state>();

	/* (scrollx[1] & 0x40) does something */
	state->m_bg_tilemap->set_scrollx(0, state->m_scrollx);
	state->m_bg_tilemap->set_scrolly(0, state->m_scrolly);

	if (state->m_ninjemak_dispdisable)
		bitmap.fill(0, cliprect);
	else
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
