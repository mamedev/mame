// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
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

PALETTE_INIT_MEMBER(galivan_state, galivan)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0-0x7f */
	for (i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i, i);

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

		palette.set_pen_indirect(0x80 + i, ctabentry);
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

		palette.set_pen_indirect(0x180 + i_swapped, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(galivan_state::get_bg_tile_info)
{
	UINT8 *BGROM = memregion("gfx4")->base();
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO_MEMBER(1,
			code,
			(attr & 0x78) >> 3,     /* seems correct */
			0);
}

TILE_GET_INFO_MEMBER(galivan_state::get_tx_tile_info)
{
	int attr = m_videoram[tile_index + 0x400];
	int code = m_videoram[tile_index] | ((attr & 0x01) << 8);
	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xe0) >> 5,     /* not sure */
			0);
	tileinfo.category = attr & 8 ? 0 : 1;   /* seems correct */
}

TILE_GET_INFO_MEMBER(galivan_state::ninjemak_get_bg_tile_info)
{
	UINT8 *BGROM = memregion("gfx4")->base();
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO_MEMBER(1,
			code,
			((attr & 0x60) >> 3) | ((attr & 0x0c) >> 2),    /* seems correct */
			0);
}

TILE_GET_INFO_MEMBER(galivan_state::ninjemak_get_tx_tile_info)
{
	int attr = m_videoram[tile_index + 0x400];
	int code = m_videoram[tile_index] | ((attr & 0x03) << 8);

	if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
		code = attr = 0x01;

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0x1c) >> 2,     /* seems correct ? */
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(galivan_state,galivan)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galivan_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galivan_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);
}

VIDEO_START_MEMBER(galivan_state,ninjemak)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galivan_state::ninjemak_get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 512, 32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galivan_state::ninjemak_get_tx_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);
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
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_counter_w(1,data & 2);

	/* bit 2 flip screen */
	flip_screen_set(data & 0x04);

	/* bit 7 selects one of two ROM banks for c000-dfff */
	membank("bank1")->set_entry((data & 0x80) >> 7);

	/*  logerror("Address: %04X - port 40 = %02x\n", space.device().safe_pc(), data); */
}

WRITE8_MEMBER(galivan_state::ninjemak_gfxbank_w)
{
	/* bits 0 and 1 coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_counter_w(1,data & 2);

	/* bit 2 flip screen */
	flip_screen_set(data & 0x04);

	/* bit 3 unknown */

	/* bit 4 background disable flag */
	m_ninjemak_dispdisable = data & 0x10;

	/* bit 5 sprite flag ??? */

	/* bit 6, 7 ROM bank select */
	membank("bank1")->set_entry((data & 0xc0) >> 6);

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

void galivan_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 *spritepalettebank = memregion("user1")->base();
	UINT8 *buffered_spriteram = m_spriteram->buffer();
	int length = m_spriteram->bytes();
	int flip = flip_screen();
	gfx_element *gfx = m_gfxdecode->gfx(2);

	/* draw the sprites */
	for (int offs = 0; offs < length; offs += 4)
	{
		int code;
		int attr = buffered_spriteram[offs + 2];
		int color = (attr & 0x3c) >> 2;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx, sy;

		sx = (buffered_spriteram[offs + 3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - buffered_spriteram[offs];
		if (flip)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

//      code = buffered_spriteram[offs + 1] + ((attr & 0x02) << 7);
		code = buffered_spriteram[offs + 1] + ((attr & 0x06) << 7);  // for ninjemak, not sure ?

		gfx->transpen(bitmap,cliprect,
				code,
				color + 16 * (spritepalettebank[code >> 2] & 0x0f),
				flipx,flipy,
				sx,sy,15);
	}
}


UINT32 galivan_state::screen_update_galivan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_galivan_scrollx[0] + 256 * (m_galivan_scrollx[1] & 0x07));
	m_bg_tilemap->set_scrolly(0, m_galivan_scrolly[0] + 256 * (m_galivan_scrolly[1] & 0x07));

	if (m_layers & 0x40)
		bitmap.fill(0, cliprect);
	else
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_layers & 0x20)
	{
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 1, 0);
		draw_sprites(bitmap, cliprect);
	}
	else
	{
		draw_sprites(bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	}

	return 0;
}

UINT32 galivan_state::screen_update_ninjemak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* (scrollx[1] & 0x40) does something */
	m_bg_tilemap->set_scrollx(0, m_scrollx);
	m_bg_tilemap->set_scrolly(0, m_scrolly);

	if (m_ninjemak_dispdisable)
		bitmap.fill(0, cliprect);
	else
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
