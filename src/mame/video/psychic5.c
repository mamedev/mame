// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************
  Psychic 5
  Bombs Away

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/psychic5.h"


#define BG_PAL_INTENSITY_RG 0x1fe
#define BG_PAL_INTENSITY_BU 0x1ff


/***************************************************************************
  Palette color
***************************************************************************/

void psychic5_state::change_palette(int offset, UINT8* palram, int palbase)
{
	UINT8 lo = palram[(offset) & ~1];
	UINT8 hi = palram[(offset) | 1];

	int color = offset >> 1;

	if (m_blend)
		m_blend->set(palbase + color, hi & 0x0f);

	m_palette->set_pen_color(palbase + color, pal4bit(lo >> 4), pal4bit(lo), pal4bit(hi >> 4));
}

void psychic5_state::change_bg_palette(int color, int lo_offs, int hi_offs)
{
	UINT8 r,g,b,lo,hi,ir,ig,ib,ix;
	rgb_t irgb;

	/* red,green,blue intensities */
	ir = pal4bit(m_palette_intensity >> 12);
	ig = pal4bit(m_palette_intensity >>  8);
	ib = pal4bit(m_palette_intensity >>  4);
	ix = m_palette_intensity & 0x0f;

	irgb = rgb_t(ir,ig,ib);

	lo = m_ps5_palette_ram_bg[lo_offs];
	hi = m_ps5_palette_ram_bg[hi_offs];

	/* red,green,blue component */
	r = pal4bit(lo >> 4);
	g = pal4bit(lo);
	b = pal4bit(hi >> 4);

	/* Grey background enable */
	if (m_bg_control[4] & 2)
	{
		UINT8 val = (r + g + b) / 3;        /* Grey */
		/* Just leave plain grey */
		m_palette->set_pen_color(color,m_blend->func(rgb_t(val,val,val),irgb,ix));
	}
	else
	{
		/* Seems fishy, but the title screen would be black otherwise... */
		if (!(m_title_screen & 1))
		{
			/* Leave the world as-is */
			m_palette->set_pen_color(color,m_blend->func(rgb_t(r,g,b),irgb,ix));
		}
	}
}

void psychic5_state::set_background_palette_intensity()
{
	int i;
	m_palette_intensity = m_ps5_palette_ram_sp[BG_PAL_INTENSITY_BU] |
						(m_ps5_palette_ram_sp[BG_PAL_INTENSITY_RG]<<8);

	/* for all of the background palette */
	for (i = 0; i < 0x100; i++)
		change_bg_palette(i+0x100,i*2,i*2+1);
}


/***************************************************************************
  Memory handler
***************************************************************************/

READ8_MEMBER(psychic5_state::vram_page_select_r)
{
	return m_ps5_vram_page;
}

WRITE8_MEMBER(psychic5_state::vram_page_select_w)
{
	m_ps5_vram_page = data & 1;
	m_vrambank->set_bank(data);
}

WRITE8_MEMBER(psychic5_state::psychic5_title_screen_w)
{
	m_title_screen = data;
}



WRITE8_MEMBER(psychic5_state::sprite_col_w)
{
	m_ps5_palette_ram_sp[offset] = data;
	change_palette(offset,m_ps5_palette_ram_sp, 0x000);
}

WRITE8_MEMBER(psychic5_state::bg_col_w)
{
	m_ps5_palette_ram_bg[offset] = data;
	change_palette(offset,m_ps5_palette_ram_bg, 0x100);
}

WRITE8_MEMBER(psychic5_state::tx_col_w)
{
	m_ps5_palette_ram_tx[offset] = data;
	change_palette(offset,m_ps5_palette_ram_tx, 0x200);
}


WRITE8_MEMBER(psychic5_state::fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER( psychic5_state::bg_videoram_w )
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}



WRITE8_MEMBER(psychic5_state::bombsa_unknown_w)
{
	m_bombsa_unknown = data;
}


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

TILE_GET_INFO_MEMBER(psychic5_state::get_bg_tile_info)
{
	int offs = tile_index << 1;
	int attr = m_bg_videoram[offs + 1];
	int code = m_bg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(psychic5_state::get_fg_tile_info)
{
	int offs = tile_index << 1;
	int attr = m_fg_videoram[offs + 1];
	int code = m_fg_videoram[offs] | ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(2, code, color, flags);
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

void psychic5_state::video_start()
{
	save_item(NAME(m_ps5_vram_page));
	save_item(NAME(m_bg_clip_mode));
	save_item(NAME(m_palette_intensity));
	save_item(NAME(m_sx1));
	save_item(NAME(m_sy1));
	save_item(NAME(m_sy2));
}


VIDEO_START_MEMBER(psychic5_state,psychic5)
{
	video_start();

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psychic5_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psychic5_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS,  8,  8, 32, 32);
	m_fg_tilemap->set_transparent_pen(15);

	save_item(NAME(m_title_screen));

}

VIDEO_START_MEMBER(psychic5_state,bombsa)
{
	video_start();

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psychic5_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 128, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(psychic5_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS,  8,  8,  32, 32);
	m_fg_tilemap->set_transparent_pen(15);

	save_item(NAME(m_bombsa_unknown));
}

VIDEO_RESET_MEMBER(psychic5_state,psychic5)
{
	m_bg_clip_mode = 0;
	m_ps5_vram_page = 0;
	m_title_screen = 0;
	m_palette_intensity = 0;
}



/***************************************************************************
  Screen refresh
***************************************************************************/

#define DRAW_SPRITE(code, sx, sy) \
	if (m_blend) \
		m_blend->drawgfx(m_palette, bitmap, cliprect, m_gfxdecode->gfx(0), code, color, flipx, flipy, sx, sy, 15); \
	else \
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 15);

void psychic5_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Draw the sprites */
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 16)
	{
		int attr  = m_spriteram[offs + 13];
		int code  = m_spriteram[offs + 14] | ((attr & 0xc0) << 2);
		int color = m_spriteram[offs + 15] & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = m_spriteram[offs + 12];
		int sy = m_spriteram[offs + 11];
		int size = (attr & 0x08) ? 32:16;

		if (attr & 0x01) sx -= 256;
		if (attr & 0x04) sy -= 256;

		if (flip_screen())
		{
			sx = 224 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (size == 32)
		{
			int x0,x1,y0,y1;

			if (flipx) { x0 = 2; x1 = 0; }
			else { x0 = 0; x1 = 2; }

			if (flipy) { y0 = 1; y1 = 0; }
			else { y0 = 0; y1 = 1; }

			DRAW_SPRITE(code + x0 + y0, sx, sy)
			DRAW_SPRITE(code + x0 + y1, sx, sy + 16)
			DRAW_SPRITE(code + x1 + y0, sx + 16, sy)
			DRAW_SPRITE(code + x1 + y1, sx + 16, sy + 16)
		}
		else
		{
			if (flip_screen())
				DRAW_SPRITE(code, sx + 16, sy + 16)
			else
				DRAW_SPRITE(code, sx, sy)
		}
	}
}

void psychic5_state::draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;

	set_background_palette_intensity();

	if (!(m_title_screen & 1))
	{
		m_bg_clip_mode = 0;
		m_sx1 = m_sy1 = m_sy2 = 0;
	}
	else
	{
		int sy1_old = m_sy1;
		int sx1_old = m_sx1;
		int sy2_old = m_sy2;

		m_sy1 = m_spriteram[11];       /* sprite 0 */
		m_sx1 = m_spriteram[12];
		m_sy2 = m_spriteram[11+128];   /* sprite 8 */

		switch (m_bg_clip_mode)
		{
		case  0: case  4: if (sy1_old != m_sy1) m_bg_clip_mode++; break;
		case  2: case  6: if (sy2_old != m_sy2) m_bg_clip_mode++; break;
		case  8: case 10:
		case 12: case 14: if (sx1_old != m_sx1) m_bg_clip_mode++; break;
		case  1: case  5: if (m_sy1 == 0xf0) m_bg_clip_mode++; break;
		case  3: case  7: if (m_sy2 == 0xf0) m_bg_clip_mode++; break;
		case  9: case 11: if (m_sx1 == 0xf0) m_bg_clip_mode++; break;
		case 13: case 15: if (sx1_old == 0xf0) m_bg_clip_mode++;
		case 16: if (m_sy1 != 0x00) m_bg_clip_mode = 0; break;
		}

		switch (m_bg_clip_mode)
		{
		case  0: case  4: case  8: case 12: case 16:
			clip.set(0, 0, 0, 0);
			break;
		case  1: clip.min_y = m_sy1; break;
		case  3: clip.max_y = m_sy2; break;
		case  5: clip.max_y = m_sy1; break;
		case  7: clip.min_y = m_sy2; break;
		case  9: case 15: clip.min_x = m_sx1; break;
		case 11: case 13: clip.max_x = m_sx1; break;
		}

		if (flip_screen())
			clip.set(255 - clip.max_x, 255 - clip.min_x, 255 - clip.max_y, 255 - clip.min_y);
	}

	m_bg_tilemap->draw(screen, bitmap, clip, 0, 0);
}

UINT32 psychic5_state::screen_update_psychic5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16 bg_scrollx = m_bg_control[0] | (m_bg_control[1] << 8);
	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	UINT16 bg_scrolly = m_bg_control[2] | (m_bg_control[3] << 8);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (m_bg_control[4] & 1)    /* Backgound enable */
		draw_background(screen, bitmap, cliprect);
	if (!(m_title_screen & 1))
		draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 psychic5_state::screen_update_bombsa(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16 bg_scrollx = m_bg_control[0] | (m_bg_control[1] << 8);
	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	UINT16 bg_scrolly = m_bg_control[2] | (m_bg_control[3] << 8);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_bg_control[4] & 1)    /* Backgound enable */
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->pen(0x0ff), cliprect);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
