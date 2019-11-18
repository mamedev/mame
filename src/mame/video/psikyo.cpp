// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/***************************************************************************

                            -= Psikyo Games =-

                driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                            [ 2 Scrolling Layers ]

        - Dynamic Size
        - Line Scroll

        Layer Sizes:             512 x 2048 ( $20 x $80 tiles)
                                1024 x 1048 ( $40 x $40 tiles)
                                2048 x  512 ( $80 x $20 tiles)
                                4096 x  256 ($100 x $10 tiles)

        Tengai uses all four above

        Tiles:                  16x16x4
        Color Codes:            8


                    [ ~ $300 Multi-Tile Sprites With Zoom ]


        Each sprite is made of 16x16 tiles, up to 8x8 tiles.

        There are $300 sprites, followed by a list of the indexes
        of the sprites to actually display ($400 max). The list is
        terminated by the special index value FFFF.

        The tile code specified for a sprite is actually fed to a
        ROM holding a look-up table with the real tile code to display.

        Sprites can be shrinked up to ~50% following a linear curve of
        sizes.


        Since the tilemaps can change size its safest to allocate all
        the possible sizes at startup, as opposed to during the emulation

        By doing it this way theres no chance of a memory allocation
        failing during gameplay and crashing MAME

**************************************************************************/

#include "emu.h"
#include "includes/psikyo.h"


/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset:

0000.w          fed- ---- ---- ----     Color
                ---c ba98 7654 3210     Code

***************************************************************************/

template<int Layer>
TILE_GET_INFO_MEMBER(psikyo_state::get_tile_info)
{
	u16 code = ((u16 *)m_vram[Layer].target())[BYTE_XOR_BE(tile_index)];
	SET_TILE_INFO_MEMBER(1,
			(code & 0x1fff) + 0x2000 * m_tilemap_bank[Layer],
			((code >> 13) & 7) + (Layer * 0x40),
			0);
}

void psikyo_state::switch_bgbanks(u8 tmap, u8 bank)
{
	if (bank != m_tilemap_bank[tmap])
	{
		m_tilemap_bank[tmap] = bank;
		for (int size = 0; size < 4; size++)
			m_tilemap[tmap][size]->mark_all_dirty();
	}
}


VIDEO_START_MEMBER(psikyo_state,psikyo)
{
	m_spritelist = std::make_unique<sprite_t[]>(0x800/2*8*8);
	m_sprite_ptr_pre = m_spritelist.get();
	/* The Hardware is Capable of Changing the Dimensions of the Tilemaps, its safer to create
	   the various sized tilemaps now as opposed to later */

	for (int size = 0; size < 4; size++)
	{
		m_tilemap[0][size] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(psikyo_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 0x20 << size, 0x80 >> size);
		m_tilemap[1][size] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(psikyo_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 0x20 << size, 0x80 >> size);

		for (int layer = 0; layer < 2; layer++)
		{
			m_tilemap[layer][size]->set_scroll_rows(1);
			m_tilemap[layer][size]->set_scroll_cols(1);
		}
	}
	save_item(NAME(m_old_linescroll));
	save_item(NAME(m_sprite_ctrl));
}

VIDEO_START_MEMBER(psikyo_state,sngkace)
{
	VIDEO_START_CALL_MEMBER( psikyo );

	switch_bgbanks(0, 0); // sngkace / samuraia don't use banking
	switch_bgbanks(1, 1); // They share "gfx2" to save memory on other boards
}



/***************************************************************************

                                Sprites Drawing

Offset:         Value:

0000/2.w        Y/X + Y/X Size

                    fedc ---- ---- ----     Zoom Y/X ???
                    ---- ba9- ---- ----     Tiles along Y/X
                    ---- ---8 7654 3210     Position


0004.w          Color + Flags

                    f--- ---- ---- ----     Flip Y
                    -e-- ---- ---- ----     Flip X
                    --d- ---- ---- ----     ? USED
                    ---c ba98 ---- ----     Color
                    ---- ---- 76-- ----     Priority
                    ---- ---- --54 321-     -
                    ---- ---- ---- ---0     Code High Bit


0006.w                                      Code Low Bits

                (Code goes into a LUT in ROM where
                 the real tile code is.)


Note:   Not all sprites are displayed: in the top part of spriteram
        (e.g. 401800-401fff) there's the list of sprites indexes to
        actually display, terminated by FFFF.

        The last entry (e.g. 401ffe) is special and holds some flags:

            fedc ba98 7654 ----
            ---- ---- ---- 32--     Transparent Pen select? 10 for 0xf, 01 for 0x0.
            ---- ---- ---- --1-
            ---- ---- ---- ---0     Sprites Disable


***************************************************************************/

void psikyo_state::get_sprites()
{
	/* tile layers 0 & 1 have priorities 1 & 2 */
	m_sprite_ctrl = m_spriteram->buffer()[0x1ffe / 4] & 0xffff;
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	const u16 *spritelist = (u16 *)(m_spriteram->buffer() + 0x1800 / 4);
	const u8 *TILES = m_spritelut->base();    // Sprites LUT
	u32 const TILES_LEN = m_spritelut->bytes();

	u16 const width = m_screen->width();
	u16 const height = m_screen->height();

	/* Exit if sprites are disabled */
	m_sprite_ptr_pre = m_spritelist.get();
	if (m_sprite_ctrl & 1)
		return;

	/* Look for "end of sprites" marker in the sprites list */
	for (int offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)   // skip last "sprite"
	{
		int xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		u16 sprite = spritelist[BYTE_XOR_BE(offs)];

		if (sprite == 0xffff)
			break;

		sprite %= 0x300;
		const u32 *source = &m_spriteram->buffer()[sprite * 8 / 4];

		/* Draw this sprite */

		s32 y          =   source[0 / 4] >> 16;
		s32 x          =   source[0 / 4] & 0xffff;
		u16 const attr =   source[4 / 4] >> 16;
		u32 code       =   source[4 / 4] & 0x1ffff;

		int flipx      =   attr & 0x4000;
		int flipy      =   attr & 0x8000;

		u32 zoomx      =   ((x & 0xf000) >> 12);
		u32 zoomy      =   ((y & 0xf000) >> 12);
		u8 const nx    =   ((x & 0x0e00) >> 9) + 1;
		u8 const ny    =   ((y & 0x0e00) >> 9) + 1;
		x              =   ((x & 0x01ff));
		y              =   ((y & 0x00ff)) - (y & 0x100);

		/* 180-1ff are negative coordinates. Note that $80 pixels is
		   the maximum extent of a sprite, which can therefore be moved
		   out of screen without problems */
		if (x >= 0x180)
			x -= 0x200;

		x += (nx * zoomx + 2) / 4;
		y += (ny * zoomy + 2) / 4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (flip_screen())
		{
			x = width  - x - (nx * zoomx) / 2;
			y = height - y - (ny * zoomy) / 2;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)  { xstart = nx - 1;  xend = -1;  xinc = -1; }
		else        { xstart = 0;       xend = nx;  xinc = +1; }

		if (flipy)  { ystart = ny - 1;  yend = -1;   yinc = -1; }
		else        { ystart = 0;       yend = ny;   yinc = +1; }

		for (int dy = ystart; dy != yend; dy += yinc)
		{
			for (int dx = xstart; dx != xend; dx += xinc)
			{
				u32 const addr = (code * 2) & (TILES_LEN - 1);

				m_sprite_ptr_pre->gfx = 0;
				m_sprite_ptr_pre->code = TILES[addr+1] * 256 + TILES[addr];
				m_sprite_ptr_pre->color = attr >> 8;
				m_sprite_ptr_pre->flipx = flipx;
				m_sprite_ptr_pre->flipy = flipy;
				if (zoomx == 32 && zoomy == 32)
				{
					m_sprite_ptr_pre->x = x + dx * 16;
					m_sprite_ptr_pre->y = y + dy * 16;
				}
				else
				{
					m_sprite_ptr_pre->x = x + (dx * zoomx) / 2;
					m_sprite_ptr_pre->y = y + (dy * zoomy) / 2;
				}
				m_sprite_ptr_pre->zoomx = zoomx << 11;
				m_sprite_ptr_pre->zoomy = zoomy << 11;
				m_sprite_ptr_pre->primask = pri[(attr & 0xc0) >> 6];

				m_sprite_ptr_pre++;
				code++;
			}
		}
	}
}


// for now this is the same as the above function
// until I work out why it makes a partial copy of the sprite list, and how best to apply it
// sprite placement of the explosion graphic seems incorrect compared to the original sets? (no / different zoom support?)
// it might be a problem with the actual bootleg
void psikyo_state::get_sprites_bootleg()
{
	/* tile layers 0 & 1 have priorities 1 & 2 */
	m_sprite_ctrl = m_spriteram->buffer()[0x1ffe / 4] & 0xffff;
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	const u16 *spritelist = (u16 *)(m_spriteram->buffer() + 0x1800 / 4);
	const u8 *TILES = m_spritelut->base();    // Sprites LUT
	u32 const TILES_LEN = m_spritelut->bytes();

	u16 const width = m_screen->width();
	u16 const height = m_screen->height();

	/* Exit if sprites are disabled */
	m_sprite_ptr_pre = m_spritelist.get();
	if (m_sprite_ctrl & 1)
		return;

	/* Look for "end of sprites" marker in the sprites list */
	for (int offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)   // skip last "sprite"
	{
		int xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		u16 sprite = spritelist[BYTE_XOR_BE(offs)];

		if (sprite == 0xffff)
			break;

		sprite %= 0x300;
		const u32 *source = &m_spriteram->buffer()[sprite * 8 / 4];

		/* Draw this sprite */

		s32 y          =   source[0 / 4] >> 16;
		s32 x          =   source[0 / 4] & 0xffff;
		u16 const attr =   source[4 / 4] >> 16;
		u32 code       =   source[4 / 4] & 0x1ffff;

		int flipx      =   attr & 0x4000;
		int flipy      =   attr & 0x8000;

		u32 zoomx      =   ((x & 0xf000) >> 12);
		u32 zoomy      =   ((y & 0xf000) >> 12);
		u8 const nx    =   ((x & 0x0e00) >> 9) + 1;
		u8 const ny    =   ((y & 0x0e00) >> 9) + 1;
		x              =   ((x & 0x01ff));
		y              =   ((y & 0x00ff)) - (y & 0x100);

		/* 180-1ff are negative coordinates. Note that $80 pixels is
		   the maximum extent of a sprite, which can therefore be moved
		   out of screen without problems */
		if (x >= 0x180)
			x -= 0x200;

		x += (nx * zoomx + 2) / 4;
		y += (ny * zoomy + 2) / 4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;


		if (flip_screen())
		{
			x = width  - x - (nx * zoomx) / 2;
			y = height - y - (ny * zoomy) / 2;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)  { xstart = nx - 1;  xend = -1;  xinc = -1; }
		else        { xstart = 0;       xend = nx;  xinc = +1; }

		if (flipy)  { ystart = ny - 1;  yend = -1;   yinc = -1; }
		else        { ystart = 0;       yend = ny;   yinc = +1; }

		for (int dy = ystart; dy != yend; dy += yinc)
		{
			for (int dx = xstart; dx != xend; dx += xinc)
			{
				u32 const addr = (code * 2) & (TILES_LEN-1);

				m_sprite_ptr_pre->gfx = 0;
				m_sprite_ptr_pre->code = TILES[addr+1] * 256 + TILES[addr];
				m_sprite_ptr_pre->color = attr >> 8;
				m_sprite_ptr_pre->flipx = flipx;
				m_sprite_ptr_pre->flipy = flipy;
				if (zoomx == 32 && zoomy == 32)
				{
					m_sprite_ptr_pre->x = x + dx * 16;
					m_sprite_ptr_pre->y = y + dy * 16;
				}
				else
				{
					m_sprite_ptr_pre->x = x + (dx * zoomx) / 2;
					m_sprite_ptr_pre->y = y + (dy * zoomy) / 2;
				}
				m_sprite_ptr_pre->zoomx = zoomx << 11;
				m_sprite_ptr_pre->zoomy = zoomy << 11;
				m_sprite_ptr_pre->primask = pri[(attr & 0xc0) >> 6];

				m_sprite_ptr_pre++;
				code++;
			}
		}
	}
}

void psikyo_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	struct sprite_t *sprite_ptr = m_sprite_ptr_pre;
	u32 transmask = 0;
	if (m_sprite_ctrl & 4) transmask |= (1 << 0);
	if (m_sprite_ctrl & 8) transmask |= (1 << 15);

	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		if (sprite_ptr->zoomx == 0x10000 && sprite_ptr->zoomy == 0x10000)
		{
			m_gfxdecode->gfx(sprite_ptr->gfx)->prio_transmask(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				screen.priority(),sprite_ptr->primask,transmask);
		}
		else
		{
			m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transmask(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),sprite_ptr->primask,transmask);
		}
	}
}

/***************************************************************************

                                Screen Drawing

***************************************************************************/

u16 psikyo_state::tilemap_width(u8 size)
{
	return (0x80 * 16) >> size;
}

u32 psikyo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 bgpen = 0;
	int i, layers_ctrl = -1;

	u32 tmsize[2];

	u32 scrollx[2]{ m_vregs[0x406 / 4], m_vregs[0x40e / 4] }, scrolly[2]{ m_vregs[0x402 / 4], m_vregs[0x40a / 4] };
	u32 layer_ctrl[2]{ m_vregs[0x412 / 4], m_vregs[0x416 / 4] };

	tilemap_t *tmptilemap[2];

	flip_screen_set(~m_in_dsw->read() & 0x00010000);       // hardwired to a DSW bit

	/* Layers enable (not quite right) */

	/* bit  0   : layer enable
	        1   : opaque tiles (used in Gunbird attract mode)
	        2   : ?
	        3   : transparent colour (0 or 15)
	        4- 5: ?
	        6- 7: tilemap size
	        8   : per-line rowscroll
	        9   : per-tile rowscroll
	       10   : tilebank (btlkroad/gunbird/s1945jn only)
	       11-15: ? */

/*
    gunbird:    L:00d0-04d0 S:0008 (00e1 04e1 0009 or 00e2 04e2 000a, for a blink, on scene transitions)
    sngkace:    L:00d0-00d0 S:0008 (00d1 00d1 0009, for a blink, on scene transitions)
    s1945:      L:00d0-04d0 S:0008
    btlkrodj:   L:0120-0510 S:0008 (0121 0511 0009, for a blink, on scene transitions)
    tengai: L:0178-0508 S:0004 <-- Transpen is 0 as opposed to 15.

    tengai:
        L:01f8-05c8, 1 needs size 0, 2 needs size 0 Title
        L:00f8-05c8, 1 needs size 0, 2 needs size 0 No RowScroll on layer 0
        L:01b8-05c8, 1 needs size 3, 2 needs size 0
        L:0178-0508, 1 needs size ?, 2 needs size 1 Psikyo logo
        L:0178-0508, 1 needs size 2, 2 needs size 1 Intro
        L:0178-0548, 1 needs size 2, 2 needs size ? Test
        L:0178-0588,                 2 needs size 3 More Intro
*/
	for (int layer = 0; layer < 2; layer++)
	{
		/* For gfx banking for s1945jn/gunbird/btlkroad */
		if (m_ka302c_banking)
		{
			switch_bgbanks(layer, (layer_ctrl[layer] & 0x400) >> 10);
		}

		switch ((layer_ctrl[layer] & 0x00c0) >> 6)
		{
		case 0:  tmsize[layer] = 1;    break;
		case 1:  tmsize[layer] = 2;    break;
		case 2:  tmsize[layer] = 3;    break;
		default: tmsize[layer] = 0;    break;
		}

		tmptilemap[layer] = m_tilemap[layer][tmsize[layer]];

		tmptilemap[layer]->enable(~layer_ctrl[layer] & 1);

		/* Layers scrolling */
		tmptilemap[layer]->set_scrolly(0, scrolly[layer]);
		if (layer_ctrl[layer] & 0x0300) /* per-line rowscroll */
		{
			int tile_rowscroll = (layer_ctrl[layer] & 0x0200) >> 7; /* per-tile rowscroll */
			assert(tile_rowscroll == 0 || tile_rowscroll == 4);
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 4; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(tilemap_width(i));
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}
			for (i = 0; i < 256; i++)   /* 256 screen lines */
			{
				int x0 = ((u16 *)m_vregs.target())[BYTE_XOR_BE((layer * 0x200)/2 + (i >> tile_rowscroll))];
				tmptilemap[layer]->set_scrollx(
				(i + scrolly[layer]) % (tilemap_width(tmsize[layer])),
				scrollx[layer] + x0 );
			}
		}
		else
		{
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 4; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(1);
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}
			tmptilemap[layer]->set_scrollx(0, scrollx[layer]);
		}
		tmptilemap[layer]->set_transparent_pen((layer_ctrl[layer] & 8 ? 0 : 15));
	}

	// TODO : is this correct?
	if (layers_ctrl & 1)
		bgpen = ((layer_ctrl[0] & 8) ? 0x800 : 0x80f);
	else if (layers_ctrl & 2)
		bgpen = ((layer_ctrl[1] & 8) ? 0xc00 : 0xc0f);
	else
		bgpen = m_palette->black_pen(); // TODO

	bitmap.fill(bgpen, cliprect);

	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)
		tmptilemap[0]->draw(screen, bitmap, cliprect, layer_ctrl[0] & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tmptilemap[1]->draw(screen, bitmap, cliprect, layer_ctrl[1] & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}

/* todo: work out why the sprites flicker,
  if it misses a frame due to slowdown it wipes both the list
  and the extra buffer the bootleg has

  layer offsets should also differ?

*/

u32 psikyo_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 bgpen = 0;
	int i, layers_ctrl = -1;

	u32 tmsize[2];

	u32 scrollx[2]{ m_vregs[0x406 / 4], m_vregs[0x40e / 4] }, scrolly[2]{ m_vregs[0x402 / 4], m_vregs[0x40a / 4] };
	u32 layer_ctrl[2]{ m_vregs[0x412 / 4], m_vregs[0x416 / 4] };

	tilemap_t *tmptilemap[2];

	flip_screen_set(~m_in_dsw->read() & 0x00010000);       // hardwired to a DSW bit

	/* Layers enable (not quite right) */

	/* bit  0   : layer enable
	        1   : opaque tiles (used in Gunbird attract mode)
	        2   : ?
	        3   : transparent colour (0 or 15)
	        4- 5: ?
	        6- 7: tilemap size
	        8   : per-line rowscroll
	        9   : per-tile rowscroll
	       10   : tilebank (btlkroad/gunbird/s1945jn only)
	       11-15: ? */

/*
    gunbird:    L:00d0-04d0 S:0008 (00e1 04e1 0009 or 00e2 04e2 000a, for a blink, on scene transitions)
    sngkace:    L:00d0-00d0 S:0008 (00d1 00d1 0009, for a blink, on scene transitions)
    s1945:      L:00d0-04d0 S:0008
    btlkrodj:   L:0120-0510 S:0008 (0121 0511 0009, for a blink, on scene transitions)
    tengai: L:0178-0508 S:0004 <-- Transpen is 0 as opposed to 15.

    tengai:
        L:01f8-05c8, 1 needs size 0, 2 needs size 0 Title
        L:00f8-05c8, 1 needs size 0, 2 needs size 0 No RowScroll on layer 0
        L:01b8-05c8, 1 needs size 3, 2 needs size 0
        L:0178-0508, 1 needs size ?, 2 needs size 1 Psikyo logo
        L:0178-0508, 1 needs size 2, 2 needs size 1 Intro
        L:0178-0548, 1 needs size 2, 2 needs size ? Test
        L:0178-0588,                 2 needs size 3 More Intro
*/
	for (int layer = 0; layer < 2; layer++)
	{
		/* For gfx banking for s1945jn/gunbird/btlkroad */
		if (m_ka302c_banking)
		{
			switch_bgbanks(layer, (layer_ctrl[layer] & 0x400) >> 10);
		}

		switch ((layer_ctrl[layer] & 0x00c0) >> 6)
		{
		case 0:  tmsize[layer] = 1;    break;
		case 1:  tmsize[layer] = 2;    break;
		case 2:  tmsize[layer] = 3;    break;
		default: tmsize[layer] = 0;    break;
		}

		tmptilemap[layer] = m_tilemap[layer][tmsize[layer]];

		tmptilemap[layer]->enable(~layer_ctrl[layer] & 1);

		/* Layers scrolling */
		tmptilemap[layer]->set_scrolly(0, scrolly[layer]);

		if (layer_ctrl[layer] & 0x0300) /* per-line rowscroll */
		{
			int tile_rowscroll = (layer_ctrl[layer] & 0x0200) >> 7; /* per-tile rowscroll */
			assert(tile_rowscroll == 0 || tile_rowscroll == 4);
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 4; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(tilemap_width(i));
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}
			for (i = 0; i < 256; i++)   /* 256 screen lines */
			{
				int x0 = ((u16 *)m_vregs.target())[BYTE_XOR_BE((layer * 0x200)/2 + (i >> tile_rowscroll))];
				tmptilemap[layer]->set_scrollx(
				(i + scrolly[layer]) % (tilemap_width(tmsize[layer])),
				scrollx[layer] + x0 );
			}
		}
		else
		{
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 4; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(1);
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}
			tmptilemap[layer]->set_scrollx(0, scrollx[layer]);
		}
		tmptilemap[layer]->set_transparent_pen((layer_ctrl[layer] & 8 ? 0 : 15));
	}

	// TODO : is this correct?
	if (layers_ctrl & 1)
		bgpen = ((layer_ctrl[0] & 8) ? 0x800 : 0x80f);
	else if (layers_ctrl & 2)
		bgpen = ((layer_ctrl[1] & 8) ? 0xc00 : 0xc0f);
	else
		bgpen = m_palette->black_pen(); // TODO

	bitmap.fill(bgpen, cliprect);

	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)
		tmptilemap[0]->draw(screen, bitmap, cliprect, layer_ctrl[0] & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tmptilemap[1]->draw(screen, bitmap, cliprect, layer_ctrl[1] & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}


WRITE_LINE_MEMBER(psikyo_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		get_sprites();
		m_spriteram->copy();
	}
}

WRITE_LINE_MEMBER(psikyo_state::screen_vblank_bootleg)
{
	// rising edge
	if (state)
	{
		get_sprites_bootleg(); // TODO : it's seems differ?
		m_spriteram->copy();
	}
}
