// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Funny Bubble Video hardware

todo - convert to tilemap

 */


#include "emu.h"
#include "funybubl.h"

rgb_t funybubl_state::funybubl_R6B6G6(uint32_t raw)
{
	return rgb_t(pal6bit(raw >> 12), pal6bit(raw >>  0), pal6bit(raw >>  6));
}

void funybubl_state::tilemap_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_tilemapram[offset]);
	m_tilemap->mark_tile_dirty(offset>>1);
}

TILE_GET_INFO_MEMBER(funybubl_state::get_tile_info)
{
	uint16_t const code = m_tilemapram[tile_index << 1] | (m_tilemapram[(tile_index << 1) | 1] << 8);
	tileinfo.set(0, code & 0x7fff, BIT(code, 15), 0);
}

void funybubl_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funybubl_state::get_tile_info)),TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap->set_transparent_pen(0);
}

void funybubl_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *source = &m_spriteram[0x1000 - 0x20];
	uint8_t *finish = m_spriteram;

	while (source >= finish)
	{
		int xpos, ypos, tile;

		/* the sprites are in the sprite list twice
		 the first format (in comments) appears to be a buffer, if you use
		 this list you get garbage sprites in 2 player mode
		 the second format (used) seems correct

		 */
/*
        ypos = 0xff - source[1 + 0x10];
        xpos = source[2 + 0x10];
        tile = source[0 + 0x10] | ( (source[3 + 0x10] & 0x0f) <<8);
        if (source[3 + 0x10] & 0x80) tile += 0x1000;
        if (source[3 + 0x10] & 0x20) xpos += 0x100;
        // bits 0x40 (not used?) and 0x10 (just set during transition period of x co-ord 0xff and 0x00) ...
        xpos -= 8;
        ypos -= 14;

*/
		ypos = source[2];
		xpos = source[3];
		tile = source[0] | ( (source[1] & 0x0f) << 8);
		if (source[1] & 0x80) tile += 0x1000;
		if (source[1] & 0x20)
		{
			if (xpos < 0xe0)
				xpos += 0x100;
		}

		// bits 0x40 and 0x10 not used?...

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, tile, 0, 0, 0, xpos, ypos, 255);
		source -= 0x20;
	}
}


uint32_t funybubl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

#if 0
	if ( machine().input().code_pressed_once(KEYCODE_W) )
	{
		FILE *fp;

		fp = fopen("funnybubsprites", "w+b");
		if (fp)
		{
			fwrite(&m_spriteram[0], 0x1000, 1, fp);
			fclose(fp);
		}
	}
#endif
	return 0;
}
