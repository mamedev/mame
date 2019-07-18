// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*  Namco System NA1 / 2 Video Hardware */

/*
Notes:
- fa/fghtatck: Global screen window effect cut one line from top/bottom of screen, especially noticeable with credit display.
  It's a btanb according to a PCB video I've seen -AS.

TODO:
- background color currently follows what xday2 tries to set up, it's not sure if this is correct for everything else.
  Maybe there's a setting that actually switches between that and black pen?
- non-shadow pixels for sprites flagged to enable shadows have bad colors
- xday2: after choosing life (right) mode and set up date of birth, a joystick screen shows up.
  Bottom yellow frame doesn't show up correctly.
- xday2: at the end of life event, there's an unemulated screen shutdown event (might be posirq?)

*/

#include "emu.h"
#include "includes/namcona1.h"


void namcona1_state::tilemap_get_info(
	tile_data &tileinfo,
	int tile_index,
	const u16 *tilemap_videoram,
	bool use_4bpp_gfx)
{
	const u16 data = tilemap_videoram[tile_index];
	const u32 tile = data & 0xfff;
	const u8 gfx = use_4bpp_gfx ? 1 : 0;
	const u32 color = use_4bpp_gfx ? (data & 0x7000) >> 12 : 0;

	if (data & 0x8000)
	{
		SET_TILE_INFO_MEMBER(gfx, tile, color, TILE_FORCE_LAYER0);
	}
	else
	{
		SET_TILE_INFO_MEMBER(gfx, tile, color, 0);
		tileinfo.mask_data = &m_shaperam[tile << 3];
	}
} /* tilemap_get_info */

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info0)
{
	tilemap_get_info(tileinfo, tile_index, 0 * 0x1000 + m_videoram, m_vreg[0xbc / 2] & 0x1);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info1)
{
	tilemap_get_info(tileinfo, tile_index, 1 * 0x1000 + m_videoram, m_vreg[0xbc / 2] & 0x2);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info2)
{
	tilemap_get_info(tileinfo, tile_index, 2 * 0x1000 + m_videoram, m_vreg[0xbc / 2] & 0x4);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info3)
{
	tilemap_get_info(tileinfo, tile_index, 3 * 0x1000 + m_videoram, m_vreg[0xbc / 2] & 0x8);
}

TILE_GET_INFO_MEMBER(namcona1_state::roz_get_info)
{
	/* each logical tile is constructed from 4*4 normal tiles */
	const bool use_4bpp_gfx = m_vreg[0xbc / 2] & 0x10; /* ? */
	const u16 c = tile_index & 0x3f;
	const u16 r = tile_index >> 6;
	const u16 data = m_videoram[0x8000 / 2 + ((r >> 2) << 6) + (c >> 2)];
	const u32 tile = ((data & 0xfbf) + (c & 3) + ((r & 3) << 6)) & 0xfff; /* mask out bit 0x40 - patch for Emeraldia Japan */
	const u8 gfx = use_4bpp_gfx ? 1 : 0;
	const u32 color = use_4bpp_gfx ? (data & 0x7000) >> 12 : 0;

	if (data & 0x8000)
	{
		SET_TILE_INFO_MEMBER(gfx, tile, color, TILE_FORCE_LAYER0);
	}
	else
	{
		SET_TILE_INFO_MEMBER(gfx, tile, color, 0);
		tileinfo.mask_data = &m_shaperam[tile << 3];
	}
} /* roz_get_info */

/*************************************************************************/

void namcona1_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	if (offset < 0x8000 / 2)
	{
		m_bg_tilemap[offset >> 12]->mark_tile_dirty(offset & 0xfff);
	}
	else if (offset < 0x8800 / 2)
	{
		if (offset & ~0x30)
		{
			for (int i = 0; i < 4; i++)
			{
				m_bg_tilemap[4]->mark_tile_dirty(((offset & 0x3cf) << 2) + i);
				m_bg_tilemap[4]->mark_tile_dirty(((offset & 0x3cf) << 2) + 0x40 + i);
				m_bg_tilemap[4]->mark_tile_dirty(((offset & 0x3cf) << 2) + 0x80 + i);
				m_bg_tilemap[4]->mark_tile_dirty(((offset & 0x3cf) << 2) + 0xc0 + i);
			}
		}
	}
} /* videoram_w */

/*************************************************************************/

void namcona1_state::UpdatePalette(int offset)
{
	const u16 data = m_paletteram[offset]; /* -RRRRRGG GGGBBBBB */
	/**
	 * sprites can be configured to use an alternate interpretation of palette ram
	 * (used in-game in Emeraldia)
	 *
	 * RRRGGGBB RRRGGGBB
	 */
	int r = (((data & 0x00e0) >> 5) + ((data & 0xe000) >> 13) * 2) * 0xff / (0x7 * 3);
	int g = (((data & 0x001c) >> 2) + ((data & 0x1c00) >> 10) * 2) * 0xff / (0x7 * 3);
	int b = (((data & 0x0003) >> 0) + ((data & 0x0300) >> 8) * 2) * 0xff / (0x3 * 3);
	m_palette->set_pen_color(offset + 0x1000, r, g, b);

	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

void namcona1_state::paletteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	if (m_vreg[0x8e / 2])
	{ /* graphics enabled; update palette immediately */
		UpdatePalette(offset);
	}
	else
	{
		m_palette_is_dirty = 1;
	}
}


u16 namcona1_state::gfxram_r(offs_t offset)
{
	const u16 type = m_vreg[0x0c / 2];
	if (type == 0x03)
	{
		if (offset < 0x4000)
		{
			offset *= 2;
			return (m_shaperam[offset] << 8) | m_shaperam[offset+1];
		}
	}
	else if (type == 0x02)
	{
		return m_cgram[offset];
	}
	return 0x0000;
} /* gfxram_r */

void namcona1_state::gfxram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 type = m_vreg[0x0c / 2];

	if (type == 0x03)
	{
		if (offset < 0x4000)
		{
			offset *= 2;
			if (ACCESSING_BITS_8_15)
				m_shaperam[offset] = data >> 8;
			if (ACCESSING_BITS_0_7)
				m_shaperam[offset + 1] = data;
			m_gfxdecode->gfx(2)->mark_dirty(offset >> 3);
		}
	}
	else if (type == 0x02)
	{
		const u16 old_word = m_cgram[offset];
		COMBINE_DATA(&m_cgram[offset]);
		if (m_cgram[offset] != old_word)
		{
			m_gfxdecode->gfx(0)->mark_dirty(offset >> 5);
			m_gfxdecode->gfx(1)->mark_dirty(offset >> 5);
		}
	}
} /* gfxram_w */

void namcona1_state::video_start()
{
	// normal tilemaps
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info0),this), TILEMAP_SCAN_ROWS, 8,8,64,64);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info1),this), TILEMAP_SCAN_ROWS, 8,8,64,64);
	m_bg_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info2),this), TILEMAP_SCAN_ROWS, 8,8,64,64);
	m_bg_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info3),this), TILEMAP_SCAN_ROWS, 8,8,64,64);

	// roz tilemap
	m_bg_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::roz_get_info),this), TILEMAP_SCAN_ROWS, 8,8,64,64);

	m_shaperam.resize(0x8000);

	m_gfxdecode->gfx(2)->set_source(&m_shaperam[0]);

	save_item(NAME(m_shaperam));
	save_item(NAME(m_palette_is_dirty));
} /* video_start */

void namcona1_state::device_post_load()
{
	for (int i = 0; i < 3; i++)
		m_gfxdecode->gfx(i)->mark_all_dirty();
}


/*************************************************************************/

void namcona1_state::pdraw_tile(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,
		const rectangle &clip,
		u32 code,
		u32 color,
		int sx, int sy,
		bool flipx, bool flipy,
		u8 priority,
		bool bShadow,
		bool bOpaque,
		u8 gfx_region)
{
	gfx_element *gfx = m_gfxdecode->gfx(gfx_region);
	gfx_element *mask = m_gfxdecode->gfx(2);

	const u16 pal_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const u16 *source_base = gfx->get_data((code % gfx->elements()));
	const u16 *mask_base = mask->get_data((code % mask->elements()));

	/* compute sprite increment per screen pixel */
	int dx, dy;

	int ex = sx + gfx->width();
	int ey = sy + gfx->height();

	int x_index_base;
	int y_index;

	if (flipx)
	{
		x_index_base = gfx->width() - 1;
		dx = -1;
	}
	else
	{
		x_index_base = 0;
		dx = 1;
	}

	if (flipy)
	{
		y_index = gfx->height() - 1;
		dy = -1;
	}
	else
	{
		y_index = 0;
		dy = 1;
	}

	if (sx < clip.min_x)
	{ /* clip left */
		int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += pixels * dx;
	}
	if (sy < clip.min_y)
	{ /* clip top */
		int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += pixels * dy;
	}
	/* NS 980211 - fixed incorrect clipping */
	if (ex > clip.max_x + 1)
	{ /* clip right */
		int pixels = ex - clip.max_x - 1;
		ex -= pixels;
	}
	if (ey > clip.max_y + 1)
	{ /* clip bottom */
		int pixels = ey - clip.max_y - 1;
		ey -= pixels;
	}

	if (ex > sx)
	{ /* skip if inner loop doesn't draw anything */
		for (int y = sy; y < ey; y++)
		{
			const u16 *source = source_base + y_index * gfx->rowbytes();
			const u16 *mask_addr = mask_base + y_index * mask->rowbytes();
			u16 *dest = &dest_bmp.pix16(y);
			u8 *pri = &screen.priority().pix8(y);

			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				if (bOpaque)
				{
					if (pri[x] <= priority)
					{
						const u16 c = source[x_index];
						dest[x] = pal_base + c;
					}
					pri[x] = 0xff;
				}
				else
				{
					/* sprite pixel is opaque */
					if (mask_addr[x_index] != 0)
					{
						if (pri[x] <= priority)
						{
							const u16 c = source[x_index];

							/* render a shadow only if the sprites color is $F (8bpp) or $FF (4bpp) */
							if (bShadow)
							{
								if ((gfx_region == 0 && color == 0x0f) ||
									(gfx_region == 1 && color == 0xff))
								{
									pen_t *palette_shadow_table = m_palette->shadow_table();
									dest[x] = palette_shadow_table[dest[x]];
								}
								else
								{
									dest[x] = pal_base + c + 0x1000;
								}
							}
							else
							{
								dest[x] = pal_base + c;
							}
						}
						pri[x] = 0xff;
					}
				}
				x_index += dx;
			}
			y_index += dy;
		}
	}
} /* pdraw_tile */

void namcona1_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 *source = m_spriteram;

	const u16 sprite_control = m_vreg[0x22 / 2];
	if (sprite_control & 1) source += 0x400; /* alternate spriteram bank */

	for (int which = 0; which < 0x100; which++)
	{ /* max 256 sprites */
		int bpp4,palbase;
		const u16 ypos    = source[0];    /* FHHH---Y YYYYYYYY    flipy, height, ypos */
		const u16 tile    = source[1];    /* O???TTTT TTTTTTTT    opaque tile */
		const u16 color   = source[2];    /* FSWWOOOO CCCCBPPP    flipx, shadow, width, color offset for 4bpp, color, 4bbp - 8bpp mode, pri*/
		const u16 xpos    = source[3];    /* -------X XXXXXXXX    xpos */

		const u8 priority = color & 0x7;
		const u16 width = ((color >> 12) & 0x3) + 1;
		const u16 height = ((ypos >> 12) & 0x7) + 1;
		bool flipy = ypos & 0x8000;
		bool flipx = color & 0x8000;

		if (color & 8)
		{
			palbase = (color & 0xf0) | ((color & 0xf00) >> 8);
			bpp4 = 1;
		}
		else
		{
			palbase = (color & 0xf0) >> 4;
			bpp4 = 0;
		}

		for (int row = 0; row < height; row++)
		{
			int sy = (ypos & 0x1ff) - 30 + 32;
			if (flipy)
			{
				sy += (height - 1 - row) << 3;
			}
			else
			{
				sy += row << 3;
			}
			sy = ((sy + 8) & 0x1ff) - 8;

			for (int col = 0; col < width; col++)
			{
				int sx = (xpos & 0x1ff) - 10;
				if (flipx)
				{
					sx += (width - 1 - col) << 3;
				}
				else
				{
					sx += col << 3;
				}
				sx = ((sx + 16) & 0x1ff) - 8;

				pdraw_tile(screen,
					bitmap,
					cliprect,
					(tile & 0xfff) + (row << 6) + col,
					palbase,
					sx,sy,flipx,flipy,
					priority,
					color & 0x4000, /* shadow */
					tile & 0x8000, /* opaque */
					bpp4);

			} /* next col */
		} /* next row */
		source += 4;
	}
} /* draw_sprites */

void namcona1_state::draw_pixel_line(const rectangle &cliprect, u16 *pDest, u8 *pPri, u16 *pSource, const pen_t *paldata)
{
	for (int x = 0; x < 38 << 3; x += 2)
	{
		u16 data = *pSource++;
		pPri[x + 0] = 0xff;
		pPri[x + 1] = 0xff;
		if (x >= cliprect.min_x && x <= cliprect.max_x)
			pDest[x+0] = paldata[data>>8];
		if (x+1 >= cliprect.min_x && x+1 <= cliprect.max_x)
			pDest[x+1] = paldata[data&0xff];
	} /* next x */
} /* draw_pixel_line */

void namcona1_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask)
{
	if (which == 4)
	{
		/* draw the roz tilemap all at once */
		int incxx = ((s16)m_vreg[0xc0 / 2])<<8;
		int incxy = ((s16)m_vreg[0xc2 / 2])<<8;
		int incyx = ((s16)m_vreg[0xc4 / 2])<<8;
		int incyy = ((s16)m_vreg[0xc6 / 2])<<8;
		s16 xoffset = m_vreg[0xc8 / 2];
		s16 yoffset = m_vreg[0xca / 2];
		int dx = 46; /* horizontal adjust */
		int dy = -8; /* vertical adjust */
		u32 startx = (xoffset<<12) + incxx * dx + incyx * dy;
		u32 starty = (yoffset<<12) + incxy * dx + incyy * dy;
		m_bg_tilemap[4]->draw_roz(screen, bitmap, cliprect,
			startx, starty, incxx, incxy, incyx, incyy, 0, 0, primask, 0);
	}
	else
	{
		/* draw one scanline at a time */
		/*          scrollx lineselect
		*  tmap0   ffe000  ffe200
		*  tmap1   ffe400  ffe600
		*  tmap2   ffe800  ffea00
		*  tmap3   ffec00  ffee00
		*/
		const u16 *scroll = &m_scroll[which * 0x400 / 2];
		rectangle clip = cliprect;
		int xadjust = 0x3a - which*2;
		int scrollx = xadjust;
		int scrolly = 0;

		for (int line = 0; line < 256; line++)
		{
			clip.min_y = line;
			clip.max_y = line;
			int xdata = scroll[line];
			int ydata = scroll[line + 0x200 / 2];

			if (xdata)
			{
				/* screenwise linescroll */
				if (xdata & 0x4000) // resets current xscroll value (knuckle head)
					scrollx = xadjust + xdata;
				else
					scrollx += xdata & 0x1ff;
			}

			if (ydata & 0x4000)
			{
				/* line select: dword offset from 0xff000 or tilemap source line */
				scrolly = (ydata - line) & 0x1ff;
			}

			if (line >= cliprect.min_y && line <= cliprect.max_y)
			{
				// TODO: not convinced about this trigger
				if (xdata == 0xc001)
				{
				   /* This is a simplification, but produces the correct behavior for the only game that uses this
				    * feature, Numan Athletics.
				    */
					// TODO: with this it breaks colors in VS Express event, likely pal bank is somewhere else in this mode, assuming it has one anyway?
					//const pen_t *paldata = &m_palette->pen(m_bg_tilemap[which]->palette_offset());
					const pen_t *paldata = &m_palette->pen(0);

					draw_pixel_line(cliprect, &bitmap.pix16(line),
								&screen.priority().pix8(line),
								m_videoram + ydata + 25,
								paldata);
				}
				else
				{
					m_bg_tilemap[which]->set_scrollx(0, scrollx);
					m_bg_tilemap[which]->set_scrolly(0, scrolly);
					m_bg_tilemap[which]->draw(screen, bitmap, clip, 0, primask, 0);
				}
			}
		}
	}
} /* draw_background */

// CRTC safety checks
bool namcona1_state::screen_enabled(const rectangle &cliprect)
{
	if (cliprect.min_x < 0)
		return false;

	if (cliprect.max_x < 0)
		return false;

	if (cliprect.min_x > cliprect.max_x)
		return false;

	if (cliprect.min_y > cliprect.max_y)
		return false;

	return true;
}

u32 namcona1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// CRTC visible area parameters
	// (used mostly by Numan Athletics for global screen window effects, cfr. start of events/title screen to demo mode transitions)
	rectangle display_rect;
	display_rect.min_x = m_vreg[0x80 / 2]-0x48;
	display_rect.max_x = m_vreg[0x82 / 2]-0x48-1;
	display_rect.min_y = std::max((int)m_vreg[0x84 / 2],   cliprect.min_y);
	display_rect.max_y = std::min((int)m_vreg[0x86 / 2] - 1, cliprect.max_y);

	/* int flipscreen = m_vreg[0x98 / 2]; (TBA) */

	screen.priority().fill(0, cliprect);

	// guess for X-Day 2 (flames in attract), seems wrong for Emeraldia but unsure
//  bitmap.fill(0xff, cliprect); /* background color? */
	bitmap.fill((m_vreg[0xba / 2] & 0xf) << 8, cliprect);

	if (m_vreg[0x8e / 2] && screen_enabled(display_rect) == true)
	{
		/* gfx enabled */
		if (m_palette_is_dirty)
		{
			/* palette updates are delayed when graphics are disabled */
			for (int which = 0; which < 0x1000; which++)
			{
				UpdatePalette(which);
			}
			m_palette_is_dirty = 0;
		}

		for (int priority = 0; priority < 8; priority++)
		{
			for (int which = 4; which >= 0; which--)
			{
				int pri;
				if (which == 4)
				{
					pri = m_vreg[0xa0 / 2 + 5] & 0x7;
				}
				else
				{
					pri = m_vreg[0xa0 / 2 + which] & 0x7;
				}
				if (pri == priority)
				{
					draw_background(screen, bitmap, display_rect, which, priority);
				}
			} /* next tilemap */
		} /* next priority level */

		draw_sprites(screen, bitmap, display_rect);
	} /* gfx enabled */
	return 0;
}

/*
roz bad in emeraldaj - blit param?

$efff20: sprite control: 0x3a,0x3e,0x3f
            bit 0x01 selects spriteram bank

               0    2    4    6    8    a    c    e
$efff00:    src0 src1 src2 dst0 dst1 dst2 BANK [src
$efff10:    src] [dst dst] #BYT BLIT eINT 001f 0001
$efff20:    003f 003f IACK ---- ---- ---- ---- ----
...
$efff80:    0050 0170 0020 0100 0170 POS? 0000 GFXE
$efff90:    0000 0001 0002 0003 FLIP ---- ---- ----
$efffa0:    PRI  PRI  PRI  PRI  ???? PRI  00c0 ----     priority (0..7)
$efffb0:    CLR  CLR  CLR  CLR  0001 CLR  BPP  ----     color (0..f), bpp flag per layer
$efffc0:    RZXX RZXY RZYX RZYY RZX0 RZY0 0044 ----     ROZ


Emeralda:   0048 0177 0020 0100 0000 00f0 0000 0001     in-game
            0050 0170 0020 0100 0000 00f0 0000 0001     self test

NumanAth:   0050 0170 0020 0100 0170 00d8 0000 0001     in-game


*/
