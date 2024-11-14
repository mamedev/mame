// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "wgp.h"
#include "screen.h"


// reference : https://www.youtube.com/watch?v=Sb3I3eQQvcU
/*******************************************************************/

template<unsigned Offset>
TILE_GET_INFO_MEMBER(wgp_state::get_piv_tile_info)
{
	const u16 tilenum = m_pivram[tile_index + Offset];    /* 3 blocks of $2000 */
	const u16 attr = m_pivram[tile_index + Offset + 0x8000];  /* 3 blocks of $2000 */

	tileinfo.set(1,
			tilenum & 0x3fff,
			(attr & 0x3f),  /* attr & 0x1 ?? */
			TILE_FLIPYX((attr & 0xc0) >> 6));
}


void wgp_state::core_vh_start(int piv_xoffs, int piv_yoffs)
{
	m_piv_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x0000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_piv_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x1000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_piv_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x2000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_piv_xoffs = piv_xoffs;
	m_piv_yoffs = piv_yoffs;

	m_piv_tilemap[0]->set_transparent_pen(0);
	m_piv_tilemap[1]->set_transparent_pen(0);
	m_piv_tilemap[2]->set_transparent_pen(0);

	/* flipscreen n/a */
	m_piv_tilemap[0]->set_scrolldx(-piv_xoffs, 0);
	m_piv_tilemap[1]->set_scrolldx(-piv_xoffs, 0);
	m_piv_tilemap[2]->set_scrolldx(-piv_xoffs, 0);
	m_piv_tilemap[0]->set_scrolldy(-piv_yoffs, 0);
	m_piv_tilemap[1]->set_scrolldy(-piv_yoffs, 0);
	m_piv_tilemap[2]->set_scrolldy(-piv_yoffs, 0);

	/* We don't need tilemap_set_scroll_rows, as the custom draw routine applies rowscroll manually */
	m_tc0100scn->set_colbanks(0x80, 0xc0, 0x40);

	save_item(NAME(m_piv_ctrl_reg));
	save_item(NAME(m_rotate_ctrl));
	save_item(NAME(m_piv_zoom));
	save_item(NAME(m_piv_scrollx));
	save_item(NAME(m_piv_scrolly));
}

void wgp_state::video_start()
{
	core_vh_start(32, 16);
}

VIDEO_START_MEMBER(wgp_state,wgp2)
{
	core_vh_start(32, 16);
}


/******************************************************************
                 PIV TILEMAP READ AND WRITE HANDLERS

Piv Tilemaps
------------

(The unused gaps look as though Taito considered making their
custom chip capable of four rather than three tilemaps.)

500000 - 501fff : unknown/unused
502000 - 507fff : piv tilemaps 0-2 [tile numbers only]

508000 - 50ffff : this area relates to pixel rows in each piv tilemap.
    Includes rowscroll for the piv tilemaps, 1-2 of which act as a
    simple road. To curve, it has rowscroll applied to each row.

508000 - 5087ff unknown/unused

508800  piv0 row color bank (low byte = row horizontal zoom)
509000  piv1 row color bank (low byte = row horizontal zoom)
509800  piv2 row color bank (low byte = row horizontal zoom)

    Usual low byte is 0x7f, the default row horizontal zoom.

    The high byte is the color offset per pixel row. Controlling
    color bank per scanline is rare in Taito games. Top Speed may
    have a similar system to make its road 'move'.

    In-game the high bytes are set to various values (seen 0 - 0x2b).

50a000  piv0 rowscroll [sky]  (not used, but the code supports this)
50c000  piv1 rowscroll [road] (values 0xfd00 - 0x400)
50e000  piv2 rowscroll [road or scenery] (values 0xfd00 - 0x403)

    [It seems strange that unnecessarily large space allocations were
    made for rowscroll. Perhaps the raster color/zoom effects were an
    afterthought, and 508000-9fff was originally slated as rowscroll
    for 'missing' 4th piv layer. Certainly the layout is illogical.]

510000 - 511fff : unknown/unused
512000 - 517fff : piv tilemaps 0-2 [just tile colors ??]

*******************************************************************/

void wgp_state::pivram_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pivram[offset]);

	if (offset < 0x3000)
	{
		m_piv_tilemap[(offset / 0x1000)]->mark_tile_dirty((offset % 0x1000));
	}
	else if ((offset >= 0x3400) && (offset < 0x4000))
	{
		/* do nothing, custom draw routine takes care of raster effects */
	}
	else if ((offset >= 0x8000) && (offset < 0xb000))
	{
		m_piv_tilemap[((offset - 0x8000)/ 0x1000)]->mark_tile_dirty((offset % 0x1000));
	}
}

void wgp_state::piv_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 a, b;

	COMBINE_DATA(&m_piv_ctrlram[offset]);
	data = m_piv_ctrlram[offset];

	switch (offset)
	{
		case 0x00:
			a = -data;
			b = (a & 0xffe0) >> 1;  /* kill bit 4 */
			m_piv_scrollx[0] = (a & 0xf) | b;
			break;

		case 0x01:
			a = -data;
			b = (a & 0xffe0) >> 1;
			m_piv_scrollx[1] = (a & 0xf) | b;
			break;

		case 0x02:
			a = -data;
			b = (a & 0xffe0) >> 1;
			m_piv_scrollx[2] = (a & 0xf) | b;
			break;

		case 0x03:
			m_piv_scrolly[0] = data;
			break;

		case 0x04:
			m_piv_scrolly[1] = data;
			break;

		case 0x05:
			m_piv_scrolly[2] = data;
			break;

		case 0x06:
			/* Overall control reg (?)
			   0x39  %00111001   normal
			   0x2d  %00101101   piv2 layer goes under piv1
			         seen on Wgp stages 4,5,7 in which piv 2 used
			         for cloud or scenery wandering up screen */

			m_piv_ctrl_reg = data;
			break;

		case 0x08:
			/* piv 0 y zoom (0x7f = normal, not seen others) */
			m_piv_zoom[0] = data;
			break;

		case 0x09:
			/* piv 1 y zoom (0x7f = normal, values 0 &
			      0xff7f-ffbc in Wgp2) */
			m_piv_zoom[1] = data;
			break;

		case 0x0a:
			/* piv 2 y zoom (0x7f = normal, values 0 &
			      0xff7f-ffbc in Wgp2, 0-0x98 in Wgp round 4/5) */
			m_piv_zoom[2] = data;
			break;
	}
}




/****************************************************************
                     SPRITE DRAW ROUTINES

TODO
====

Implement rotation/zoom properly.

Sprite/piv priority: sprites always over?

Wgp round 1 had some junky brown mud bank sprites in-game.
They are indexed 0xe720-e790. 0x2720*4 => +0x9c80-9e80 in
the spritemap area. They should be 2x2 not 4x4 tiles. We
kludge this. Round 2 +0x9d40-9f40 contains the 2x2 sprites.
What REALLY controls number of tiles in a sprite?

Sprite colors: dust after crash in Wgp2 is odd; some
black/grey barrels on late Wgp circuit also look strange -
possibly the same wrong color.


Memory Map
----------

400000 - 40bfff : Sprite tile mapping area

    Tile numbers (0-0x3fff) alternate with word containing tile
    color/unknown bits. I'm _not_ 100% sure that only Wgp2 uses
    the unknown bits.

    xxxxxxxx x.......  unused ??
    ........ .x......  unknown (Wgp2 only: Taito tyre bridge on default course)
    ........ ..x.....  unknown (Wgp2 only)
    ........ ...x....  unknown (Wgp2 only: Direction signs just before hill # 1)
    ........ ....cccc  color (0-15)

    Tile map for each standard big sprite is 64 bytes (16 tiles).
    (standard big sprite comprises 4x4 16x16 tiles)

    Tile map for each small sprite only uses 16 of the 64 bytes.
      The remaining 48 bytes are garbage and should be ignored.
    (small sprite comprises 2x2 16x16 tiles)

40c000 - 40dbff : Sprite Table

    Every 16 bytes contains one sprite entry. First entry is
    ignored [as 0 in active sprites list means no sprite].

    (0x1c0 [no.of entries] * 0x40 [bytes for big sprite] = 0x6fff
    of sprite tile mapping area can be addressed at any one time.)

    Sprite entry     (preliminary)
    ------------

    +0x00  x pos (signed)
    +0x02  y pos (signed)
    +0x04  index to tile mapping area [2 msbs always set]

           (400000 + (index & 0x3fff) << 2) points to relevant part of
           sprite tile mapping area. Index >0x2fff would be invalid.

    +0x06  zoom size (pixels) [typical range 0x1-5f, 0x3f = standard]
           Looked up from a logarithm table in the data rom indexed
           by the z coordinate. Max size prog allows before it blanks
           the sprite is 0x140.

    +0x08  incxx ?? (usually stuck at 0xf800)
    +0x0a  incyy ?? (usually stuck at 0xf800)

    +0x0c  z coordinate i.e. how far away the sprite is from you
           going into the screen. Max distance prog allows before it
           blanks the sprite is 0x3fff. 0x1400 is about the farthest
           away that the code creates sprites. 0x400 = standard
           distance corresponding to 0x3f zoom.  <0x400 = close to

    +0x0e  non-zero only during rotation.

    NB: +0x0c and +0x0e are paired. Equivalent of incyx and incxy ??

    (No longer used entries typically have 0xfff6 in +0x06 and +0x08.)

    Only 2 rotation examples (i) at 0x40c000 when Taito
    logo displayed (Wgp only). (ii) stage 5 (rain).
    Other in-game sprites are simply using +0x06 and +0x0c,

    So the sprite rotation in Wgp screenshots must be a *blanket*
    rotate effect, identical to the one applied to piv layers.
    This explains why sprite/piv positions are basically okay
    despite failure to implement rotation.

40dc00 - 40dfff: Active Sprites list

    Each word is a sprite number, 0x0 through 0x1bf. If !=0
    a word makes active the 0x10 bytes of sprite data at
    (40c000 + sprite_num * 0x10). (Wgp2 fills this in reverse).

40fff0: Unknown (sprite control word?)

    Wgp alternates 0x8000 and 0. Wgp2 only pokes 0.
    Could this be some frame buffer control that would help to
    reduce the sprite timing glitches in Wgp?

****************************************************************/

/* Sprite tilemapping area doesn't have a straightforward
   structure for each big sprite: the hardware is probably
   constructing each 4x4 sprite from 4 2x2 sprites... */

static const u8 xlookup[16] =
	{ 0, 1, 0, 1,
		2, 3, 2, 3,
		0, 1, 0, 1,
		2, 3, 2, 3 };

static const u8 ylookup[16] =
	{ 0, 0, 1, 1,
		0, 0, 1, 1,
		2, 2, 3, 3,
		2, 2, 3, 3 };

void wgp_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int i, j, k;
//  u16 rotate = 0;
	const u32 tile_mask = (m_gfxdecode->gfx(0)->elements()) - 1;
	static const u32 primasks[2] = {0x0, 0xfffc};   /* fff0 => under rhs of road only */

	for (int offs = 0x1ff; offs >= 0; offs--)
	{
		const int code = (m_spriteram[0xe00 + offs]);

		if (code)   /* do we have an active sprite ? */
		{
			i = (code << 3) & 0xfff;    /* yes, so we look up its sprite entry */

			int x = m_spriteram[i];
			int y = m_spriteram[i + 1];
			const int bigsprite = m_spriteram[i + 2] & 0x3fff;

			/* The last five words [i + 3 through 7] must be zoom/rotation
			   control: for time being we kludge zoom using 1 word.
			   Timing problems are causing many glitches. */

			if ((m_spriteram[i + 4] == 0xfff6) && (m_spriteram[i + 5] == 0))
				continue;

//          if (((m_spriteram[i + 4] != 0xf800) && (m_spriteram[i + 4] != 0xfff6))
//              || ((m_spriteram[i + 5] != 0xf800) && (m_spriteram[i + 5] != 0))
//              || m_spriteram[i + 7] != 0)
//              rotate = i << 1;

			/***** Begin zoom kludge ******/

			const int zoomx = (m_spriteram[i + 3] & 0x1ff) + 1;
			const int zoomy = (m_spriteram[i + 3] & 0x1ff) + 1;

			y -= 4;
			// distant sprites were some 16 pixels too far down //
			y -= ((0x40 - zoomy)/4);

			/****** end zoom kludge *******/

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			const int map_index = bigsprite << 1; /* now we access sprite tilemap */

			/* don't know what selects 2x2 sprites: we use a nasty kludge which seems to work */

			i = m_spritemap[map_index + 0xa];
			j = m_spritemap[map_index + 0xc];
			const bool small_sprite = ((i > 0) & (i <= 8) & (j > 0) & (j <= 8));

			if (small_sprite)
			{
				for (i = 0; i < 4; i++)
				{
					const u32 tile = m_spritemap[(map_index + (i << 1))] & tile_mask;
					const u32 col  = m_spritemap[(map_index + (i << 1) + 1)] & 0xf;

					/* not known what controls priority */
					const int priority = (m_spritemap[(map_index + (i << 1) + 1)] & 0x70) >> 4;

					int flipx = 0;  // no flip xy?
					int flipy = 0;

					k = xlookup[i]; // assumes no xflip
					j = ylookup[i]; // assumes no yflip

					const int curx = x + ((k * zoomx) / 2);
					const int cury = y + ((j * zoomy) / 2);

					const int zx = x + (((k + 1) * zoomx) / 2) - curx;
					const int zy = y + (((j + 1) * zoomy) / 2) - cury;

					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
							tile,
							col,
							flipx, flipy,
							curx,cury,
							zx << 12, zy << 12,
							screen.priority(),primasks[((priority >> 1) &1)],0);  /* maybe >> 2 or 0...? */
				}
			}
			else
			{
				for (i = 0; i < 16; i++)
				{
					const u32 tile = m_spritemap[(map_index + (i << 1))] & tile_mask;
					const u32 col  = m_spritemap[(map_index + (i << 1) + 1)] & 0xf;

					/* not known what controls priority */
					const int priority = (m_spritemap[(map_index + (i << 1) + 1)] & 0x70) >> 4;

					int flipx = 0;  // no flip xy?
					int flipy = 0;

					k = xlookup[i]; // assumes no xflip
					j = ylookup[i]; // assumes no yflip

					const int curx = x + ((k * zoomx) / 4);
					const int cury = y + ((j * zoomy) / 4);

					const int zx = x + (((k + 1) * zoomx) / 4) - curx;
					const int zy = y + (((j + 1) * zoomy) / 4) - cury;

					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
							tile,
							col,
							flipx, flipy,
							curx,cury,
							zx << 12, zy << 12,
							screen.priority(),primasks[((priority >> 1) &1)],0);  /* maybe >> 2 or 0...? */
				}
			}
		}

	}
#if 0
	if (rotate)
		popmessage("sprite rotate offs %04x ?", rotate);
#endif
}


/*********************************************************
                       CUSTOM DRAW
*********************************************************/

static inline void bryan2_drawscanline(bitmap_ind16 &bitmap, int x, int y, int length,
		const u16 *src, bool transparent, u32 orient, bitmap_ind8 &priority, u8 pri, u8 primask = 0xff)
{
	u16 *dsti = &bitmap.pix(y, x);
	u8 *dstp = &priority.pix(y, x);

	if (transparent)
	{
		while (length--)
		{
			const u32 spixel = *src++;
			if (spixel < 0x7fff)
			{
				*dsti = spixel;
				*dstp = (*dstp & primask) | pri;
			}
			dsti++;
			dstp++;
		}
	}
	else  /* Not transparent case */
	{
		while (length--)
		{
			*dsti++ = *src++;
			*dstp = (*dstp & primask) | pri;
			dstp++;
		}
	}
}



void wgp_state::piv_layer_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u32 priority)
{
	bitmap_ind16 &srcbitmap = m_piv_tilemap[layer]->pixmap();
	bitmap_ind8 &flagsbitmap = m_piv_tilemap[layer]->flagsmap();

	int y_index;

	/* I have a fairly strong feeling these should be u32's, x_index is
	   falling through from max +ve to max -ve quite a lot in this routine */
	int sx;

	u16 scanline[512];
	int flipscreen = 0; /* n/a */

	const int screen_width = cliprect.width();
	const int min_y = cliprect.min_y;
	const int max_y = cliprect.max_y;

	const int width_mask = 0x3ff;

	const u32 zoomx = 0x10000;    /* No overall X zoom, unlike TC0480SCP */

	/* Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max ???
	   In WGP see:  stage 4 (big spectator stand)
	                stage 5 (cloud layer)
	                stage 7 (two bits of background scenery)
	                stage 8 (unknown - surely something should be appearing here...)
	   In WGP2 see: road at big hill (default course) */

	/* This calculation may be wrong, the y_index one too */
	const u32 zoomy = 0x10000 - (((m_piv_ctrlram[0x08 + layer] & 0xff) - 0x7f) * 512);

	if (!flipscreen)
	{
		sx = ((m_piv_scrollx[layer]) << 16);
		sx += (m_piv_xoffs) * zoomx;     /* may be imperfect */

		y_index = (m_piv_scrolly[layer] << 16);
		y_index += (m_piv_yoffs + min_y) * zoomy;        /* may be imperfect */
	}
	else    /* piv tiles flipscreen n/a */
	{
		sx = 0;
		y_index = 0;
	}

	for (int y = min_y; y <= max_y; y++)
	{
		int a;

		const int src_y_index = (y_index >> 16) & 0x3ff;
		const int row_index = src_y_index;

		const int row_zoom = m_pivram[row_index + layer * 0x400 + 0x3400] & 0xff;

		u16 row_colbank = m_pivram[row_index + layer * 0x400 + 0x3400] >> 8;
		a = (row_colbank & 0xe0);   /* kill bit 4 */
		row_colbank = (((row_colbank & 0xf) << 1) | a) << 4;

		u16 row_scroll = m_pivram[row_index + layer * 0x1000 + 0x4000];
		a = (row_scroll & 0xffe0) >> 1; /* kill bit 4 */
		row_scroll = ((row_scroll & 0xf) | a) & width_mask;

		int x_index = sx - (row_scroll << 16);

		int x_step = zoomx;
		if (row_zoom > 0x7f)    /* zoom in: reduce x_step */
		{
			x_step -= (((row_zoom - 0x7f) << 8) & 0xffff);
		}
		else if (row_zoom < 0x7f)   /* zoom out: increase x_step */
		{
			x_step += (((0x7f - row_zoom) << 8) & 0xffff);
		}

		const u16 *const src16 = &srcbitmap.pix(src_y_index);
		const u8 *const  tsrc  = &flagsbitmap.pix(src_y_index);
		u16 *dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (int i = 0; i < screen_width; i++)
			{
				*dst16++ = src16[(x_index >> 16) & width_mask] + row_colbank;
				x_index += x_step;
			}
		}
		else
		{
			for (int i = 0; i < screen_width; i++)
			{
				if (tsrc[(x_index >> 16) & width_mask])
					*dst16++ = src16[(x_index >> 16) & width_mask] + row_colbank;
				else
					*dst16++ = 0x8000;
				x_index += x_step;
			}
		}

		bryan2_drawscanline(bitmap, 0, y, screen_width, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? false : true, ROT0, screen.priority(), priority);

		y_index += zoomy;
	}
}



/**************************************************************
                        SCREEN REFRESH
**************************************************************/

u32 wgp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("piv0: %01x",m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("piv1: %01x",m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("piv2: %01x",m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[3] ^= 1;
		popmessage("TC0100SCN top bg layer: %01x",m_dislayer[3]);
	}
#endif

	for (int i = 0; i < 3; i++)
	{
		m_piv_tilemap[i]->set_scrollx(0, m_piv_scrollx[i]);
		m_piv_tilemap[i]->set_scrolly(0, m_piv_scrolly[i]);
	}

	m_tc0100scn->tilemap_update();

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	if (m_piv_ctrl_reg == 0x2d)
	{
		layer[1] = 2;
		layer[2] = 1;
	}

/* We should draw the following on a 1024x1024 bitmap... */

#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[1], 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	draw_sprites(screen, bitmap, cliprect, 16);

/* ... then here we should apply rotation from m_rotate_ctrl[] to the bitmap before we draw the TC0100SCN layers on it */
	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 0);

#ifdef MAME_DEBUG
	if (m_dislayer[3] == 0)
#endif
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 0);

#if 0
	popmessage("piv_ctrl_reg: %04x y zoom: %04x %04x %04x",
			m_piv_ctrl_reg,
			m_piv_zoom[0], m_piv_zoom[1], m_piv_zoom[2]);
#endif

/* Enable this to watch the rotation control words */
#if 0
	{
		char buf[80];
		int i;

		for (int i = 0; i < 8; i += 1)
		{
			sprintf (buf, "%02x: %04x", i, rotate_ctrl[i]);
			ui_draw_text (buf, 0, i*8);
		}
	}
#endif
	return 0;
}
