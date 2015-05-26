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

TILE_GET_INFO_MEMBER(psikyo_state::get_tile_info_0)
{
	UINT16 code = ((UINT16 *)m_vram_0.target())[BYTE_XOR_BE(tile_index)];
	SET_TILE_INFO_MEMBER(1,
			(code & 0x1fff) + 0x2000 * m_tilemap_0_bank,
			(code >> 13) & 7,
			0);
}

TILE_GET_INFO_MEMBER(psikyo_state::get_tile_info_1)
{
	UINT16 code = ((UINT16 *)m_vram_1.target())[BYTE_XOR_BE(tile_index)];
	SET_TILE_INFO_MEMBER(1,
			(code & 0x1fff) + 0x2000 * m_tilemap_1_bank,
			((code >> 13) & 7) + 0x40, // So we only have to decode the gfx once.
			0);
}


WRITE32_MEMBER(psikyo_state::psikyo_vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	if (ACCESSING_BITS_16_31)
	{
		m_tilemap_0_size0->mark_tile_dirty(offset * 2);
		m_tilemap_0_size1->mark_tile_dirty(offset * 2);
		m_tilemap_0_size2->mark_tile_dirty(offset * 2);
		m_tilemap_0_size3->mark_tile_dirty(offset * 2);
	}

	if (ACCESSING_BITS_0_15)
	{
		m_tilemap_0_size0->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_0_size1->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_0_size2->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_0_size3->mark_tile_dirty(offset * 2 + 1);
	}
}

WRITE32_MEMBER(psikyo_state::psikyo_vram_1_w)
{
	COMBINE_DATA(&m_vram_1[offset]);
	if (ACCESSING_BITS_16_31)
	{
		m_tilemap_1_size0->mark_tile_dirty(offset * 2);
		m_tilemap_1_size1->mark_tile_dirty(offset * 2);
		m_tilemap_1_size2->mark_tile_dirty(offset * 2);
		m_tilemap_1_size3->mark_tile_dirty(offset * 2);
	}

	if (ACCESSING_BITS_0_15)
	{
		m_tilemap_1_size0->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_1_size1->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_1_size2->mark_tile_dirty(offset * 2 + 1);
		m_tilemap_1_size3->mark_tile_dirty(offset * 2 + 1);
	}
}

void psikyo_state::psikyo_switch_banks( int tmap, int bank )
{
	if ((tmap == 0) && (bank != m_tilemap_0_bank))
	{
		m_tilemap_0_bank = bank;
		m_tilemap_0_size0->mark_all_dirty();
		m_tilemap_0_size1->mark_all_dirty();
		m_tilemap_0_size2->mark_all_dirty();
		m_tilemap_0_size3->mark_all_dirty();
	}
	else if ((tmap == 1) && (bank != m_tilemap_1_bank))
	{
		m_tilemap_1_bank = bank;
		m_tilemap_1_size0->mark_all_dirty();
		m_tilemap_1_size1->mark_all_dirty();
		m_tilemap_1_size2->mark_all_dirty();
		m_tilemap_1_size3->mark_all_dirty();
	}
}


VIDEO_START_MEMBER(psikyo_state,psikyo)
{
	/* The Hardware is Capable of Changing the Dimensions of the Tilemaps, its safer to create
	   the various sized tilemaps now as opposed to later */

	m_tilemap_0_size0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x80);
	m_tilemap_0_size1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x40);
	m_tilemap_0_size2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 0x80, 0x20);
	m_tilemap_0_size3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 0x100, 0x10);

	m_tilemap_1_size0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x80);
	m_tilemap_1_size1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x40);
	m_tilemap_1_size2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 0x80, 0x20);
	m_tilemap_1_size3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psikyo_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 0x100, 0x10);

	m_spritebuf1 = auto_alloc_array(machine(), UINT32, 0x2000 / 4);
	m_spritebuf2 = auto_alloc_array(machine(), UINT32, 0x2000 / 4);

	m_tilemap_0_size0->set_scroll_rows(0x80 * 16);  // line scrolling
	m_tilemap_0_size0->set_scroll_cols(1);

	m_tilemap_0_size1->set_scroll_rows(0x40 * 16);  // line scrolling
	m_tilemap_0_size1->set_scroll_cols(1);

	m_tilemap_0_size2->set_scroll_rows(0x20 * 16);  // line scrolling
	m_tilemap_0_size2->set_scroll_cols(1);

	m_tilemap_0_size3->set_scroll_rows(0x10 * 16);  // line scrolling
	m_tilemap_0_size3->set_scroll_cols(1);

	m_tilemap_1_size0->set_scroll_rows(0x80 * 16);  // line scrolling
	m_tilemap_1_size0->set_scroll_cols(1);

	m_tilemap_1_size1->set_scroll_rows(0x40 * 16);  // line scrolling
	m_tilemap_1_size1->set_scroll_cols(1);

	m_tilemap_1_size2->set_scroll_rows(0x20 * 16);  // line scrolling
	m_tilemap_1_size2->set_scroll_cols(1);

	m_tilemap_1_size3->set_scroll_rows(0x10 * 16);  // line scrolling
	m_tilemap_1_size3->set_scroll_cols(1);

	save_pointer(NAME(m_spritebuf1), 0x2000 / 4);
	save_pointer(NAME(m_spritebuf2), 0x2000 / 4);
}

VIDEO_START_MEMBER(psikyo_state,sngkace)
{
	VIDEO_START_CALL_MEMBER( psikyo );

	psikyo_switch_banks(0, 0); // sngkace / samuraia don't use banking
	psikyo_switch_banks(1, 1); // They share "gfx2" to save memory on other boards
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

void psikyo_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen )
{
	/* tile layers 0 & 1 have priorities 1 & 2 */
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	int offs;
	UINT16 *spritelist = (UINT16 *)(m_spritebuf2 + 0x1800 / 4);
	UINT8 *TILES = memregion("spritelut")->base();    // Sprites LUT
	int TILES_LEN = memregion("spritelut")->bytes();

	int width = m_screen->width();
	int height = m_screen->height();

	/* Exit if sprites are disabled */
	if (spritelist[BYTE_XOR_BE((0x800 - 2) / 2)] & 1)   return;

	/* Look for "end of sprites" marker in the sprites list */
	for (offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)   // skip last "sprite"
	{
		UINT16 sprite = spritelist[BYTE_XOR_BE(offs)];
		if (sprite == 0xffff)
			break;
	}

	offs -= 2/2;

	//  fprintf(stderr, "\n");
	for ( ; offs >= 0/2 ; offs -= 2/2)
	{
		UINT32 *source;
		int sprite;

		int x, y, attr, code, flipx, flipy, nx, ny, zoomx, zoomy;
		int dx, dy, xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		sprite = spritelist[BYTE_XOR_BE(offs)];

		sprite %= 0x300;
		source = &m_spritebuf2[sprite * 8 / 4];

		/* Draw this sprite */

		y   =   source[0 / 4] >> 16;
		x   =   source[0 / 4] & 0xffff;
		attr    =   source[4 / 4] >> 16;
		code    =   source[4 / 4] & 0x1ffff;

		flipx   =   attr & 0x4000;
		flipy   =   attr & 0x8000;

		zoomx   =   ((x & 0xf000) >> 12);
		zoomy   =   ((y & 0xf000) >> 12);
		nx  =   ((x & 0x0e00) >> 9) + 1;
		ny  =   ((y & 0x0e00) >> 9) + 1;
		x   =   ((x & 0x01ff));
		y   =   ((y & 0x00ff)) - (y & 0x100);

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

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc)
			{
				int addr = (code * 2) & (TILES_LEN - 1);

				if (zoomx == 32 && zoomy == 32)
					m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
							TILES[addr+1] * 256 + TILES[addr],
							attr >> 8,
							flipx, flipy,
							x + dx * 16, y + dy * 16,
							screen.priority(),
							pri[(attr & 0xc0) >> 6],trans_pen);
				else
					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
								TILES[addr+1] * 256 + TILES[addr],
								attr >> 8,
								flipx, flipy,
								x + (dx * zoomx) / 2, y + (dy * zoomy) / 2,
								zoomx << 11,zoomy << 11,
								screen.priority(),pri[(attr & 0xc0) >> 6],trans_pen);

				code++;
			}
		}
	}
}


// for now this is the same as the above function
// until I work out why it makes a partial copy of the sprite list, and how best to apply it
// sprite placement of the explosion graphic seems incorrect compared to the original sets? (no / different zoom support?)
// it might be a problem with the actual bootleg
void psikyo_state::draw_sprites_bootleg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen )
{
	/* tile layers 0 & 1 have priorities 1 & 2 */
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	int offs;
	UINT16 *spritelist = (UINT16 *)(m_spritebuf2 + 0x1800 / 4);
	UINT8 *TILES = memregion("spritelut")->base();    // Sprites LUT
	int TILES_LEN = memregion("spritelut")->bytes();

	int width = m_screen->width();
	int height = m_screen->height();

	/* Exit if sprites are disabled */
	if (spritelist[BYTE_XOR_BE((0x800 - 2) / 2)] & 1)
		return;

	/* Look for "end of sprites" marker in the sprites list */
	for (offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)   // skip last "sprite"
	{
		UINT16 sprite = spritelist[BYTE_XOR_BE(offs)];
		if (sprite == 0xffff)
			break;
	}

	offs -= 2/2;

	//  fprintf(stderr, "\n");
	for ( ; offs >= 0/2 ; offs -= 2/2)
	{
		UINT32 *source;
		int sprite;

		int x, y, attr, code, flipx, flipy, nx, ny, zoomx, zoomy;
		int dx, dy, xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		sprite = spritelist[BYTE_XOR_BE(offs)];

		sprite %= 0x300;
		source = &m_spritebuf2[sprite * 8 / 4];

		/* Draw this sprite */

		y   =   source[0] >> 16;
		x   =   source[0] & 0xffff;
		attr    =   source[1] >> 16;
		code    =   source[1] & 0x1ffff;

		flipx   =   attr & 0x4000;
		flipy   =   attr & 0x8000;

		zoomx   =   ((x & 0xf000) >> 12);
		zoomy   =   ((y & 0xf000) >> 12);
		nx  =   ((x & 0x0e00) >> 9) + 1;
		ny  =   ((y & 0x0e00) >> 9) + 1;
		x   =   ((x & 0x01ff));
		y   =   ((y & 0x00ff)) - (y & 0x100);

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

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc)
			{
				int addr = (code * 2) & (TILES_LEN-1);

				if (zoomx == 32 && zoomy == 32)
					m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
							TILES[addr+1] * 256 + TILES[addr],
							attr >> 8,
							flipx, flipy,
							x + dx * 16, y + dy * 16,
							screen.priority(),
							pri[(attr & 0xc0) >> 6],trans_pen);
				else
					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
								TILES[addr+1] * 256 + TILES[addr],
								attr >> 8,
								flipx, flipy,
								x + (dx * zoomx) / 2, y + (dy * zoomy) / 2,
								zoomx << 11,zoomy << 11,
								screen.priority(),pri[(attr & 0xc0) >> 6],trans_pen);

				code++;
			}
		}
	}
}





/***************************************************************************

                                Screen Drawing

***************************************************************************/

int psikyo_state::tilemap_width( int size )
{
	if (size == 0)
		return 0x80 * 16;
	else if(size == 1)
		return 0x40 * 16;
	else if(size == 2)
		return 0x20 * 16;
	else
		return 0x10 * 16;
}

UINT32 psikyo_state::screen_update_psikyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, layers_ctrl = -1;

	UINT32 tm0size, tm1size;

	UINT32 layer0_scrollx, layer0_scrolly;
	UINT32 layer1_scrollx, layer1_scrolly;
	UINT32 layer0_ctrl = m_vregs[0x412 / 4];
	UINT32 layer1_ctrl = m_vregs[0x416 / 4];
	UINT32 spr_ctrl = m_spritebuf2[0x1ffe / 4];

	tilemap_t *tmptilemap0, *tmptilemap1;

	flip_screen_set(~ioport("DSW")->read() & 0x00010000);       // hardwired to a DSW bit

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

	/* For gfx banking for s1945jn/gunbird/btlkroad */
	if (m_ka302c_banking)
	{
		psikyo_switch_banks(0, (layer0_ctrl & 0x400) >> 10);
		psikyo_switch_banks(1, (layer1_ctrl & 0x400) >> 10);
	}

	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0: tm0size = 1;    break;
	case 1: tm0size = 2;    break;
	case 2: tm0size = 3;    break;
	default:    tm0size = 0;    break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0: tm1size = 1;    break;
	case 1: tm1size = 2;    break;
	case 2: tm1size = 3;    break;
	default:    tm1size = 0;    break;
	}

	if (tm0size == 0)
		tmptilemap0 = m_tilemap_0_size0;
	else if (tm0size == 1)
		tmptilemap0 = m_tilemap_0_size1;
	else if (tm0size == 2)
		tmptilemap0 = m_tilemap_0_size2;
	else
		tmptilemap0 = m_tilemap_0_size3;

	if (tm1size == 0)
		tmptilemap1 = m_tilemap_1_size0;
	else if (tm1size == 1)
		tmptilemap1 = m_tilemap_1_size1;
	else if (tm1size == 2)
		tmptilemap1 = m_tilemap_1_size2;
	else
		tmptilemap1 = m_tilemap_1_size3;

	tmptilemap0->enable(~layer0_ctrl & 1);
	tmptilemap1->enable(~layer1_ctrl & 1);

	/* Layers scrolling */

	layer0_scrolly = m_vregs[0x402 / 4];
	layer0_scrollx = m_vregs[0x406 / 4];
	layer1_scrolly = m_vregs[0x40a / 4];
	layer1_scrollx = m_vregs[0x40e / 4];

	tmptilemap0->set_scrolly(0, layer0_scrolly);

	tmptilemap1->set_scrolly(0, layer1_scrolly);

	for (i = 0; i < 256; i++)   /* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		/* layer 0 */
		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x000/2 + i/16)];
			else
				/* per-line rowscroll */
				x0 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x000/2 + i)];
		}


			tmptilemap0->set_scrollx(
			(i + layer0_scrolly) % tilemap_width(tm0size),
			layer0_scrollx + x0 );


		/* layer 1 */
		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x200/2 + i/16)];
			else
				/* per-line rowscroll */
				x1 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x200/2 + i)];
		}


			tmptilemap1->set_scrollx(
			(i + layer1_scrolly) % tilemap_width(tm1size),
			layer1_scrollx + x1 );
	}

	m_tilemap_0_size0->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size1->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size2->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size3->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));

	m_tilemap_1_size0->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size1->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size2->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size3->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));

	bitmap.fill(m_palette->black_pen(), cliprect);

	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)
		tmptilemap0->draw(screen, bitmap, cliprect, layer0_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tmptilemap1->draw(screen, bitmap, cliprect, layer1_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites(screen, bitmap, cliprect, (spr_ctrl & 4 ? 0 : 15));

	return 0;
}

/* todo: work out why the sprites flicker,
  if it misses a frame due to slowdown it wipes both the list
  and the extra buffer the bootleg has

  layer offsets should also differ?

*/

UINT32 psikyo_state::screen_update_psikyo_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, layers_ctrl = -1;

	UINT32 tm0size, tm1size;

	UINT32 layer0_scrollx, layer0_scrolly;
	UINT32 layer1_scrollx, layer1_scrolly;
	UINT32 layer0_ctrl = m_vregs[0x412 / 4];
	UINT32 layer1_ctrl = m_vregs[0x416 / 4];
	UINT32 spr_ctrl = m_spritebuf2[0x1ffe / 4];

	tilemap_t *tmptilemap0, *tmptilemap1;

	flip_screen_set(~ioport("DSW")->read() & 0x00010000);       // hardwired to a DSW bit

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

	/* For gfx banking for s1945jn/gunbird/btlkroad */
	if (m_ka302c_banking)
	{
		psikyo_switch_banks(0, (layer0_ctrl & 0x400) >> 10);
		psikyo_switch_banks(1, (layer1_ctrl & 0x400) >> 10);
	}

	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0: tm0size = 1;    break;
	case 1: tm0size = 2;    break;
	case 2: tm0size = 3;    break;
	default:    tm0size = 0;    break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0: tm1size = 1;    break;
	case 1: tm1size = 2;    break;
	case 2: tm1size = 3;    break;
	default:    tm1size = 0;    break;
	}

	if (tm0size == 0)
		tmptilemap0 = m_tilemap_0_size0;
	else if (tm0size == 1)
		tmptilemap0 = m_tilemap_0_size1;
	else if (tm0size == 2)
		tmptilemap0 = m_tilemap_0_size2;
	else
		tmptilemap0 = m_tilemap_0_size3;

	if (tm1size == 0)
		tmptilemap1 = m_tilemap_1_size0;
	else if (tm1size == 1)
		tmptilemap1 = m_tilemap_1_size1;
	else if (tm1size == 2)
		tmptilemap1 = m_tilemap_1_size2;
	else
		tmptilemap1 = m_tilemap_1_size3;

	tmptilemap0->enable(~layer0_ctrl & 1);
	tmptilemap1->enable(~layer1_ctrl & 1);

	/* Layers scrolling */

	layer0_scrolly = m_vregs[0x402 / 4];
	layer0_scrollx = m_vregs[0x406 / 4];
	layer1_scrolly = m_vregs[0x40a / 4];
	layer1_scrollx = m_vregs[0x40e / 4];

	tmptilemap0->set_scrolly(0, layer0_scrolly);

	tmptilemap1->set_scrolly(0, layer1_scrolly);

	for (i = 0; i < 256; i++)   /* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		/* layer 0 */
		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x000/2 + i/16)];
			else
				/* per-line rowscroll */
				x0 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x000/2 + i)];
		}


			tmptilemap0->set_scrollx(
			(i + layer0_scrolly) % tilemap_width(tm0size),
			layer0_scrollx + x0 );


		/* layer 1 */
		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x200/2 + i/16)];
			else
				/* per-line rowscroll */
				x1 = ((UINT16 *)m_vregs.target())[BYTE_XOR_BE(0x200/2 + i)];
		}


			tmptilemap1->set_scrollx(
			(i + layer1_scrolly) % tilemap_width(tm1size),
			layer1_scrollx + x1 );
	}

	m_tilemap_0_size0->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size1->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size2->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));
	m_tilemap_0_size3->set_transparent_pen((layer0_ctrl & 8 ? 0 : 15));

	m_tilemap_1_size0->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size1->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size2->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));
	m_tilemap_1_size3->set_transparent_pen((layer1_ctrl & 8 ? 0 : 15));

	bitmap.fill(m_palette->black_pen(), cliprect);

	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)
		tmptilemap0->draw(screen, bitmap, cliprect, layer0_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tmptilemap1->draw(screen, bitmap, cliprect, layer1_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites_bootleg(screen, bitmap, cliprect, (spr_ctrl & 4 ? 0 : 15));

	return 0;
}


void psikyo_state::screen_eof_psikyo(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_spritebuf2, m_spritebuf1, 0x2000);
		memcpy(m_spritebuf1, m_spriteram, 0x2000);
	}
}
