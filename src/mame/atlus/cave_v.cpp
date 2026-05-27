// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Cave Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing:

        X/C/V/B/Z  with  Q   shows layer 0 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  W   shows layer 1 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  E   shows layer 2 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  R   shows layer 3 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  A   shows sprites (tiles with priority 0/1/2/3/All)

        Keys can be used together!

    [ 1 Layer per chip (games use as many as 4 chips) ]

        Layer Size:             512 x 512
        Tiles:                  8 x 8 & 16 x 16.

        There are 2 tilemaps in memory, one per tiles dimension.
        A bit decides which one gets displayed.
        The tiles depth varies with games, from 16 to 256 colors.

        A per layer row-scroll / row-select effect can be enabled:

        a different scroll value is fetched (from tile RAM) for each
        scan line, and a different tilemap line for each scan line

    [ 1024 Zooming Sprites ]

        There are 2 or 4 0x4000 Sprite RAM Areas. A hardware register's
        bit selects an area to display (sprites double buffering).

        The sprites are NOT tile based: the "tile" size and start address
        is selectable for each sprite with a 16 pixel granularity.

        Also note that the zoom is of a peculiar type: pixels are never
        drawn more than once. So shrinking works as usual (some pixels are
        just not drawn) while enlarging adds some transparent pixels to
        the image, uniformly, to reach the final size.

**************************************************************************/

#include "emu.h"
#include "crsshair.h"
#include "cave.h"


/* Sailormn: the lower 2 Megabytes of tiles banked */

void cave_z80_state::sailormn_tilebank_w(int bank)
{
	if (m_sailormn_tilebank != bank)
	{
		m_sailormn_tilebank = bank;
		m_tilemap[2]->mark_all_dirty();
	}
}

void cave_z80_state::sailormn_get_banked_code(bool tiledim, u32 &color, u32 &pri, u32 &code)
{
	if (!tiledim)
	{
		if ((code < 0x10000) && (m_sailormn_tilebank))
			code += 0x40000;
	}
}


/***************************************************************************

                            Video Init Routines

    Depending on the game, there can be from 1 to 4 layers and the
    tile sizes can be 8x8 or 16x16.

***************************************************************************/

void cave_state::video_start()
{
	m_layers_offs_x = 0x13;
	m_layers_offs_y = -0x12;

	m_row_effect_offs_n = -1;
	m_row_effect_offs_f = 1;

	m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() +
							(m_gfxdecode[0]->gfx(0)->colors() - 1) *
							m_gfxdecode[0]->gfx(0)->granularity();

	switch (m_kludge)
	{
		case 1: /* sailormn */
			m_row_effect_offs_n = -1;
			m_row_effect_offs_f = -1;
			break;
		case 2: /* uopoko dfeveron */
			m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() - m_gfxdecode[0]->gfx(0)->granularity();
			break;
		case 4: /* pwrinst2 */
			m_background_pen[0] = m_gfxdecode[0]->gfx(0)->colorbase() - m_gfxdecode[0]->gfx(0)->granularity();
			m_layers_offs_y++;
			break;
	}
}

// ppsatan (3 screen)
void ppsatan_state::video_start()
{
	cave_state::video_start();
	for (int chip = 1; chip < 3; chip++)
	{
		m_background_pen[chip] = m_gfxdecode[chip]->gfx(0)->colorbase() +
							(m_gfxdecode[chip]->gfx(0)->colors() - 1) *
							m_gfxdecode[chip]->gfx(0)->granularity();

		switch (m_kludge)
		{
			case 2: /* uopoko dfeveron */
			case 4: /* pwrinst2 */
				m_background_pen[chip] = m_gfxdecode[chip]->gfx(0)->colorbase() - m_gfxdecode[chip]->gfx(0)->granularity();
				break;
		}
	}
}

bool cave_state::colpri_cb(u8 &dstpri, u32 &colpri)
{
	const u8 pri = (colpri >> 6) & 3;
	if (dstpri <= pri)
	{
		colpri &= 0x3f;
		dstpri = 0xff;
		return true;
	}
	return false;
}

bool cave_z80_state::pwrinst2_colpri_cb(u8 &dstpri, u32 &colpri)
{
	const u8 pri = ((colpri >> 6) & 1) + 2;
	if (dstpri <= pri)
	{
		colpri = (colpri & 0x3f) | ((colpri & 0x80) >> 1);
		dstpri = 0xff;
		return true;
	}
	return false;
}

/***************************************************************************

                                Screen Drawing


                  Layers Control Registers (vctrl_0..3)


        Offset:     Bits:                   Value:

        0.w         f--- ---- ---- ----     0 = Layer Flip X
                    -e-- ---- ---- ----     Activate Row-scroll
                    --d- ---- ---- ----
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll X

        2.w         f--- ---- ---- ----     0 = Layer Flip Y
                    -e-- ---- ---- ----     Activate Row-select
                    --d- ---- ---- ----     0 = 8x8 tiles, 1 = 16x16 tiles
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll Y

        4.w         fedc ba98 765- ----
                    ---- ---- ---4 ----     Layer Disable
                    ---- ---- ---- 32--
                    ---- ---- ---- --10     Layer Priority (decides the order
                                            of the layers for tiles with the
                                            same tile priority)


        Row-scroll / row-select data is fetched from tile RAM + $1000.

        Row-select:     a tilemap line is specified for each scan line.
        Row-scroll:     a different scroll value is specified for each scan line.


                      Sprites Registers (videoregs)


    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Sprites Flip X
                -edc ba98 7654 3210     Sprites Offset X

        2.w     f--- ---- ---- ----     Sprites Flip Y
                -edc ba98 7654 3210     Sprites Offset Y

        ..

        8.w     fedc ba98 7654 32--
                ---- ---- ---- --10     Sprite RAM Bank

        There are more!

***************************************************************************/

inline void cave_state::tilemap_draw(int chip,
	screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
	u32 flags, u32 priority, u32 priority2, int layer)
{
	tilemap038_device *tilemap = m_tilemap[layer];

	/* Bail out if ... */

	if (!tilemap)                                              /* no tilemap; */
		return;

	if (((tilemap->external() & 0x0003) != priority2) ||       /* tilemap's global priority not the requested one; */
			((!tilemap->enable())))                            /* tilemap's disabled. */
		return;

	const bool flipx = tilemap->flipx();
	const bool flipy = tilemap->flipy();
	tilemap->set_flip((flipx ? TILEMAP_FLIPX : 0) | (flipy ? TILEMAP_FLIPY : 0));

	int offs_x         = m_layers_offs_x;
	const int offs_y   = m_layers_offs_y;

	const int offs_row = flipy ? m_row_effect_offs_f : m_row_effect_offs_n;

	/* An additional 8 pixel offset for layers with 8x8 tiles. Plus
	   Layer 0 is displaced by 1 pixel wrt Layer 1, so is Layer 2 wrt
	   Layer 1 */
	if      (tilemap == m_tilemap[0])    offs_x -= (tilemap->tile_is_16x16() ? 1 : (1 + 8));
	else if (tilemap == m_tilemap[1])    offs_x -= (tilemap->tile_is_16x16() ? 2 : (2 + 8));
	else if (tilemap == m_tilemap[2])    offs_x -= (tilemap->tile_is_16x16() ? 3 : (3 + 8));
	else if (tilemap == m_tilemap[3])    offs_x -= (tilemap->tile_is_16x16() ? 4 : (4 + 8));

	const int sx = tilemap->scrollx() - m_spritegen[chip]->videoregs_r(0) + (flipx ? (offs_x + 2) : -offs_x);
	const int sy = tilemap->scrolly() - m_spritegen[chip]->videoregs_r(1) + (flipy ? (offs_y + 2) : -offs_y);

	if (tilemap->rowselect_en())  // row-select
	{
		rectangle clip;
		int endline, vramdata0, vramdata1;

		/*
		    Row-select:

		    A tilemap line is specified for each scan line. This is handled
		    using many horizontal clipping regions (slices) and calling
		    tilemap_draw multiple times.
		*/

		clip.min_x = cliprect.min_x;
		clip.max_x = cliprect.max_x;

		for (int startline = cliprect.min_y; startline <= cliprect.max_y;)
		{
			/* Find the largest slice */
			vramdata0 = (vramdata1 = tilemap->rowselect(sy + offs_row + startline));
			for (endline = startline + 1; endline <= cliprect.max_y; endline++)
				if ((++vramdata1) != tilemap->rowselect(sy + offs_row + endline)) break;

			tilemap->set_scrolly(0, vramdata0 - startline);

			if (tilemap->rowscroll_en())  // row-scroll, row-select
			{
				/*
				    Row-scroll:

				    A different scroll value is specified for each scan line.
				    This is handled using tilemap->set_scroll_rows and calling
				    tilemap->draw just once.
				*/

				tilemap->set_scroll_rows(512);
				for (int line = startline; line < endline; line++)
					tilemap->set_scrollx((vramdata0 - startline + line) & 511,
										sx + tilemap->rowscroll(sy + offs_row + line));
			}
			else                    // no row-scroll, row-select
			{
				tilemap->set_scroll_rows(1);
				tilemap->set_scrollx(0, sx);
			}

			if (flipy)
			{
				clip.min_y = cliprect.max_y - (endline - 1 - cliprect.min_y);
				clip.max_y = cliprect.max_y - (startline - cliprect.min_y);
			}
			else
			{
				clip.min_y = startline;
				clip.max_y = endline - 1;
			}

			tilemap->draw(screen, bitmap, clip, flags, priority, 0);

			startline = endline;
		}
	}
	else if (tilemap->rowscroll_en()) // row-scroll, no row-select
	{
		tilemap->set_scroll_rows(512);
		for (int line = cliprect.min_y; line <= cliprect.max_y; line++)
			tilemap->set_scrollx((line + sy) & 511,
							sx + tilemap->rowscroll(sy + offs_row + line));
		tilemap->set_scrolly(0, sy);
		tilemap->draw(screen, bitmap, cliprect, flags, priority, 0);
	}
	else
	{
		/* Normal scrolling */
		tilemap->set_scroll_rows(1);
		tilemap->set_scroll_cols(1);
		tilemap->set_scrollx(0, sx);
		tilemap->set_scrolly(0, sy);
		tilemap->draw(screen, bitmap, cliprect, flags, priority, 0);
	}
}


u32 cave_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	for (int layer = 0; layer < 4; layer++)
	{
		if (m_tilemap[layer])
			m_tilemap[layer]->prepare();
	}

#ifdef MAME_DEBUG
{
	if (machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) || machine().input().code_pressed(KEYCODE_C) ||
			machine().input().code_pressed(KEYCODE_V) || machine().input().code_pressed(KEYCODE_B))
	{
		int msk = 0, val = 0;

		if (machine().input().code_pressed(KEYCODE_X))  val = 1;    // priority 0 only
		if (machine().input().code_pressed(KEYCODE_C))  val = 2;    // ""       1
		if (machine().input().code_pressed(KEYCODE_V))  val = 4;    // ""       2
		if (machine().input().code_pressed(KEYCODE_B))  val = 8;    // ""       3
		if (machine().input().code_pressed(KEYCODE_Z))  val = 1|2|4|8;  // All of the above priorities

		if (machine().input().code_pressed(KEYCODE_Q))  msk |= val <<  0;   // for layer 0
		if (machine().input().code_pressed(KEYCODE_W))  msk |= val <<  4;   // for layer 1
		if (machine().input().code_pressed(KEYCODE_E))  msk |= val <<  8;   // for layer 2
		if (machine().input().code_pressed(KEYCODE_R))  msk |= val << 12;   // for layer 3
		if (machine().input().code_pressed(KEYCODE_A))  msk |= val << 16;   // for sprites
		if (msk != 0) layers_ctrl &= msk;

		/* Show the scroll / flags registers of the selected layer */
		if ((m_tilemap[0]) && (msk & 0x000f))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[0]->vregs(0),m_tilemap[0]->vregs(1),m_tilemap[0]->vregs(2));
		if ((m_tilemap[1]) && (msk & 0x00f0))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[1]->vregs(0),m_tilemap[1]->vregs(1),m_tilemap[1]->vregs(2));
		if ((m_tilemap[2]) && (msk & 0x0f00))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[2]->vregs(0),m_tilemap[2]->vregs(1),m_tilemap[2]->vregs(2));
		if ((m_tilemap[3]) && (msk & 0xf000))   popmessage("x:%04X y:%04X f:%04X", m_tilemap[3]->vregs(0),m_tilemap[3]->vregs(1),m_tilemap[3]->vregs(2));
	}

	/* Show the row / "column" scroll enable flags, when they change state */
	m_rasflag = 0;
	for (int layer = 0; layer < 4; layer++)
	{
		if (m_tilemap[layer])
		{
			m_rasflag |= (m_tilemap[layer]->vregs(0) & 0x4000) ? 0x0001 << (4*layer) : 0;
			m_rasflag |= (m_tilemap[layer]->vregs(1) & 0x4000) ? 0x0002 << (4*layer) : 0;
		}
	}

	if (m_rasflag != m_old_rasflag)
	{
		popmessage("Line Effect: 0:%c%c 1:%c%c 2:%c%c 3:%c%c",
			(m_rasflag & 0x0001) ? 'x' : ' ', (m_rasflag & 0x0002) ? 'y' : ' ',
			(m_rasflag & 0x0010) ? 'x' : ' ', (m_rasflag & 0x0020) ? 'y' : ' ',
			(m_rasflag & 0x0100) ? 'x' : ' ', (m_rasflag & 0x0200) ? 'y' : ' ',
			(m_rasflag & 0x1000) ? 'x' : ' ', (m_rasflag & 0x2000) ? 'y' : ' ');
		m_old_rasflag = m_rasflag;
	}
}
#endif

	bitmap.fill(m_palette[0]->pen_color(m_background_pen[0]), cliprect);
	screen.priority().fill(0, cliprect);

	/*
	    Tiles and sprites are ordered by priority (0 back, 3 front) with
	    sprites going below tiles of their same priority.

	    Sprites with the same priority are ordered by their place in
	    sprite RAM (last sprite is the frontmost).

	    Tiles with the same priority are ordered by the priority of their layer.

	    Tiles with the same priority *and* the same priority of their layer
	    are ordered by layer (0 back, 2 front)
	*/
	for (int pri = 0; pri <= 3; pri++)  // tile / sprite priority
	{
		for (int pri2 = 0; pri2 <= 3; pri2++)   // priority of the whole layer
		{
			if (layers_ctrl & (1 << (pri +  0)))    tilemap_draw(0, screen, bitmap, cliprect, pri, pri + 1, pri2, 0);
			if (layers_ctrl & (1 << (pri +  4)))    tilemap_draw(0, screen, bitmap, cliprect, pri, pri + 1, pri2, 1);
			if (layers_ctrl & (1 << (pri +  8)))    tilemap_draw(0, screen, bitmap, cliprect, pri, pri + 1, pri2, 2);
			if (layers_ctrl & (1 << (pri + 12)))    tilemap_draw(0, screen, bitmap, cliprect, pri, pri + 1, pri2, 3);
		}
	}
	m_spritegen[0]->draw(screen, bitmap, cliprect);
	return 0;
}



/**************************************************************/

// Poka Poka Satan: 3 screens * (1 Sprite chip + 1 Tilemap chip)

u32 ppsatan_state::screen_update_ppsatan_core(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip)
{
	m_tilemap[chip]->prepare();

	bitmap.fill(m_palette[chip]->pen_color(m_background_pen[chip]), cliprect);
	screen.priority().fill(0, cliprect);

	for (int pri = 0; pri <= 3; pri++)  // tile / sprite priority
	{
		for (int pri2 = 0; pri2 <= 3; pri2++)   // priority of the whole layer
			tilemap_draw(chip, screen, bitmap, cliprect, pri, pri + 1, pri2, chip);
	}
	m_spritegen[chip]->draw(screen, bitmap, cliprect);

	return 0;
}

u32 ppsatan_state::screen_update_ppsatan_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 0);
}
u32 ppsatan_state::screen_update_ppsatan_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	machine().crosshair().get_crosshair(1).set_screen(&screen);
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 1);
}
u32 ppsatan_state::screen_update_ppsatan_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	machine().crosshair().get_crosshair(0).set_screen(&screen);
	return screen_update_ppsatan_core(screen, bitmap, cliprect, 2);
}
