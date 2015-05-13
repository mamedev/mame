// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Lemmings video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the vram format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

#include "emu.h"
#include "includes/lemmings.h"


/******************************************************************************/

TILE_GET_INFO_MEMBER(lemmings_state::get_tile_info)
{
	UINT16 tile = m_vram_data[tile_index];

	SET_TILE_INFO_MEMBER(2,
			tile&0x7ff,
			(tile>>12)&0xf,
			0);
}

void lemmings_state::video_start()
{
	m_vram_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lemmings_state::get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_vram_tilemap->set_transparent_pen(0);
	m_bitmap0.fill(0x100);

	m_gfxdecode->gfx(2)->set_source(m_vram_buffer);

	m_sprgen->alloc_sprite_bitmap();
	m_sprgen2->alloc_sprite_bitmap();

	save_item(NAME(m_bitmap0));
	save_item(NAME(m_vram_buffer));
	save_item(NAME(m_sprite_triple_buffer_0));
	save_item(NAME(m_sprite_triple_buffer_1));
}

void lemmings_state::screen_eof_lemmings(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_sprite_triple_buffer_0, m_spriteram->buffer(), 0x800);
		memcpy(m_sprite_triple_buffer_1, m_spriteram2->buffer(), 0x800);
	}
}

/******************************************************************************/

// RAM based
WRITE16_MEMBER(lemmings_state::lemmings_pixel_0_w)
{
	int sx, sy, src, old;

	old = m_pixel_0_data[offset];
	COMBINE_DATA(&m_pixel_0_data[offset]);
	src = m_pixel_0_data[offset];
	if (old == src)
		return;

	sy = (offset << 1) / 0x800;
	sx = (offset << 1) & 0x7ff;

	if (sx > 2047 || sy > 255)
		return;

	m_bitmap0.pix16(sy, sx + 0) = ((src >> 8) & 0xf) | 0x100;
	m_bitmap0.pix16(sy, sx + 1) = ((src >> 0) & 0xf) | 0x100;
}

// RAM based tiles for the FG tilemap
WRITE16_MEMBER(lemmings_state::lemmings_pixel_1_w)
{
	int sx, sy, src, tile;

	COMBINE_DATA(&m_pixel_1_data[offset]);
	src = m_pixel_1_data[offset];

	sy = ((offset << 1) / 0x200);
	sx = ((offset << 1) & 0x1ff);

	/* Copy pixel to buffer for easier decoding later */
	tile = ((sx / 8) * 32) + (sy / 8);
	m_gfxdecode->gfx(2)->mark_dirty(tile);
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 8) & 0xf;

	sx += 1; /* Update both pixels in the word */
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 0) & 0xf;
}

WRITE16_MEMBER(lemmings_state::lemmings_vram_w)
{
	COMBINE_DATA(&m_vram_data[offset]);
	m_vram_tilemap->mark_tile_dirty(offset);
}


void lemmings_state::lemmings_copy_bitmap(bitmap_rgb32& bitmap, bitmap_ind16& srcbitmap, int* xscroll, int* yscroll, const rectangle& cliprect)
{
	int y,x;
	const pen_t *paldata = m_palette->pens();

	for (y=cliprect.min_y; y<cliprect.max_y;y++)
	{
		UINT32* dst = &bitmap.pix32(y,0);

		for (x=cliprect.min_x; x<cliprect.max_x;x++)
		{
			UINT16 src = srcbitmap.pix16((y-*yscroll)&0xff,(x-*xscroll)&0x7ff);

			if (src!=0x100)
				dst[x] = paldata[src];
		}
	}
}

UINT32 lemmings_state::screen_update_lemmings(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x1 = -m_control_data[0];
	int x0 = -m_control_data[2];
	int y = 0;
	rectangle rect;
	rect.max_y = cliprect.max_y;
	rect.min_y = cliprect.min_y;

	m_sprgen->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer_1, 0x400, true);
	m_sprgen2->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer_0, 0x400, true);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x300, 0xff);

	/* Pixel layer can be windowed in hardware (two player mode) */
	if ((m_control_data[6] & 2) == 0)
	{
		lemmings_copy_bitmap(bitmap, m_bitmap0, &x1, &y, cliprect);
	}
	else
	{
		rect.max_x = 159;
		rect.min_x = 0;
		lemmings_copy_bitmap(bitmap, m_bitmap0, &x0, &y, rect);

		rect.max_x = 319;
		rect.min_x = 160;
		lemmings_copy_bitmap(bitmap, m_bitmap0, &x1, &y, rect);
	}

	m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0xff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x300, 0xff);
	m_vram_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0xff);
	return 0;
}
