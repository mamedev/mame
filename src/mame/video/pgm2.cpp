// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "includes/pgm2.h"

inline void pgm2_state::draw_sprite_pixel(const rectangle &cliprect, int palette_offset, int realx, int realy, int pal)
{
	if (cliprect.contains(realx, realy))
	{
		uint16_t pix = m_sprites_colour[palette_offset] & 0x3f; // there are some stray 0xff bytes in some roms, so mask
		uint16_t pendat = pix + (pal * 0x40);
		uint16_t* dstptr_bitmap = &m_sprite_bitmap.pix16(realy);
		dstptr_bitmap[realx] = pendat;
	}
}

inline void pgm2_state::draw_sprite_chunk(const rectangle &cliprect, int &palette_offset, int x, int realy, int sizex, int xdraw, int pal, uint32_t maskdata, uint32_t zoomx_bits, int repeats, int &realxdraw, int realdraw_inc, int palette_inc)
{
	for (int xchunk = 0; xchunk < 32; xchunk++)
	{
		int pix, xzoombit;
		if (palette_inc == -1)
		{
			pix = (maskdata >> xchunk) & 1;
			xzoombit = (zoomx_bits >> xchunk) & 1;
		}
		else
		{
			pix = (maskdata >> (31 - xchunk)) & 1;
			xzoombit = (zoomx_bits >> (31 - xchunk)) & 1;
		}

		if (pix)
		{
			if (repeats != 0) // grow
			{
				// draw it the base number of times
				for (int i = 0; i < repeats; i++)
				{
					draw_sprite_pixel(cliprect, palette_offset, x + realxdraw, realy, pal);
					realxdraw += realdraw_inc;
				}

				// draw it again if zoom bit is set
				if (xzoombit)
				{
					draw_sprite_pixel(cliprect, palette_offset, x + realxdraw, realy, pal);
					realxdraw += realdraw_inc;
				}

				palette_offset += palette_inc;
				palette_offset &= m_sprites_colour_mask;
			}
			else // shrink
			{
				if (xzoombit) draw_sprite_pixel(cliprect, palette_offset, x + realxdraw, realy, pal);

				palette_offset += palette_inc;
				palette_offset &= m_sprites_colour_mask;

				if (xzoombit) realxdraw += realdraw_inc;

			}
		}
		else
		{
			if (repeats != 0) // grow
			{
				for (int i = 0; i < repeats; i++)
				{
					realxdraw += realdraw_inc;
				}

				if (xzoombit)
				{
					realxdraw += realdraw_inc;
				}
			}
			else // shrink
			{
				if (xzoombit) realxdraw += realdraw_inc;
			}
		}
	}

}

inline void pgm2_state::skip_sprite_chunk(int &palette_offset, uint32_t maskdata, int reverse)
{
	int bits = population_count_32(maskdata);

	if (!reverse)
	{
		palette_offset+=bits;
	}
	else
	{
		palette_offset-=bits;
	}

	palette_offset &= m_sprites_colour_mask;

}

inline void pgm2_state::draw_sprite_line(const rectangle &cliprect, int &mask_offset, int &palette_offset, int x, int realy, int flipx, int reverse, int sizex, int pal, int zoomybit, int zoomx_bits, int xrepeats)
{
	int realxdraw = 0;

	if (flipx ^ reverse)
		realxdraw = (population_count_32(zoomx_bits) * sizex) - 1;

	for (int xdraw = 0; xdraw < sizex; xdraw++)
	{
		uint32_t maskdata = m_sprites_mask[mask_offset + 0] << 24;
		maskdata |= m_sprites_mask[mask_offset + 1] << 16;
		maskdata |= m_sprites_mask[mask_offset + 2] << 8;
		maskdata |= m_sprites_mask[mask_offset + 3] << 0;

		maskdata ^= m_realspritekey;

		if (reverse)
		{
			mask_offset -= 4;
		}
		else if (!reverse)
		{
			mask_offset += 4;
		}


		mask_offset &= m_sprites_mask_mask;

		if (zoomybit)
		{
			if (!flipx)
			{
				if (!reverse) draw_sprite_chunk(cliprect, palette_offset, x, realy, sizex, xdraw, pal, maskdata, zoomx_bits, xrepeats, realxdraw, 1, 1);
				else draw_sprite_chunk(cliprect, palette_offset, x, realy, sizex, xdraw, pal, maskdata, zoomx_bits, xrepeats, realxdraw, -1, -1);
			}
			else
			{
				if (!reverse) draw_sprite_chunk(cliprect, palette_offset, x, realy, sizex, xdraw, pal, maskdata, zoomx_bits, xrepeats, realxdraw, -1, 1);
				else draw_sprite_chunk(cliprect, palette_offset, x, realy, sizex, xdraw, pal, maskdata, zoomx_bits, xrepeats, realxdraw, 1, -1);

			}
		}
		else skip_sprite_chunk(palette_offset, maskdata, reverse);
	}
}

void pgm2_state::draw_sprites(screen_device &screen, const rectangle &cliprect, uint32_t* spriteram)
{
	m_sprite_bitmap.fill(0x8000, cliprect);

	int endoflist = -1;

	//printf("frame\n");

	for (int i = 0; i < 0x2000 / 4; i += 4)
	{
		if (spriteram[i + 2] & 0x80000000)
		{
			endoflist = i;
			break;
		}
	}

	if (endoflist != -1)
	{
		for (int i = 0; i < endoflist - 2; i += 4)
		{
			//printf("sprite with %08x %08x %08x %08x\n", spriteram[i + 0], spriteram[i + 1], spriteram[i + 2], spriteram[i + 3]);

			int x = (spriteram[i + 0] & 0x000007ff) >> 0;
			int y = (spriteram[i + 0] & 0x003ff800) >> 11;
			int pal = (spriteram[i + 0] & 0x0fc00000) >> 22;
			int pri = (spriteram[i + 0] & 0x80000000) >> 31;

			int unk0 = (spriteram[i + 0] & 0x30000000) >> 0;

			// kov3 uses this in places (eg. horsemen special move heads) and shops when it can only possibly mean 'disable'
			// it is also used in places on kov2nl and orleg2, often the 'power up' effects surrounding your character to create an on/off flicker each frame
			int disable = (spriteram[i + 0] & 0x40000000) >> 30;
			if (disable) continue;

			int sizex = (spriteram[i + 1] & 0x0000003f) >> 0;
			int sizey = (spriteram[i + 1] & 0x00007fc0) >> 6;
			int flipx = (spriteram[i + 1] & 0x00800000) >> 23;
			int reverse = (spriteram[i + 1] & 0x80000000) >> 31; // more of a 'reverse entire drawing' flag than y-flip, but used for that purpose
			int zoomx = (spriteram[i + 1] & 0x007f0000) >> 16;
			int zoomy = (spriteram[i + 1] & 0x7f000000) >> 24;
			int unk1 = (spriteram[i + 1] & 0x00008000) >> 0;

			if (unk0 || unk1)
			{
				//popmessage("sprite rendering unused bits set unk0 %08x unk1 %08x\n", unk0, unk1);
			}

			int mask_offset = (spriteram[i + 2] << 1);
			int palette_offset = (spriteram[i + 3]);

			// use all the bits of zoom to lookup, probably why the table is copied 4x in RAM
			uint32_t zoomy_bits = m_sp_zoom[zoomy];
			uint32_t zoomx_bits = m_sp_zoom[zoomx];

			// but use these bits as the scale factor
			int xrepeats = (zoomx & 0x60)>>5;
			int yrepeats = (zoomy & 0x60)>>5;

			if (x & 0x400) x -= 0x800;
			if (y & 0x400) y -= 0x800;

			if (reverse)
				mask_offset -= 2;

			mask_offset &= m_sprites_mask_mask;
			palette_offset &= m_sprites_colour_mask;

			pal |= (pri << 6); // encode priority with the palette for manual mixing later

			int realy = y;

			int sourceline = 0;
			for (int ydraw = 0; ydraw < sizey; sourceline++)
			{
				int zoomy_bit = (zoomy_bits >> (sourceline & 0x1f)) & 1;

				// store these for when we need to draw a line twice
				uint32_t pre_palette_offset = palette_offset;
				uint32_t pre_mask_offset = mask_offset;

				if (yrepeats != 0) // grow
				{
					for (int i = 0; i < yrepeats; i++)
					{
						// draw it the base number of times
						palette_offset = pre_palette_offset;
						mask_offset = pre_mask_offset;
						draw_sprite_line(cliprect, mask_offset, palette_offset, x, realy, flipx, reverse, sizex, pal, 1, zoomx_bits, xrepeats);
						realy++;
					}

					if (zoomy_bit) // draw it again if zoom bit is set
					{
						palette_offset = pre_palette_offset;
						mask_offset = pre_mask_offset;
						draw_sprite_line(cliprect, mask_offset, palette_offset, x, realy, flipx, reverse, sizex, pal, 1, zoomx_bits, xrepeats);
						realy++;
					}

					ydraw++;
				}
				else // shrink
				{
					draw_sprite_line(cliprect, mask_offset, palette_offset, x, realy, flipx, reverse, sizex, pal, 1, zoomx_bits, xrepeats);

					if (zoomy_bit)
					{
						realy++;
					}
					ydraw++;
				}
			}
		}
	}
}

void pgm2_state::copy_sprites_from_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	pri <<= 12;

	const pen_t *paldata = m_sp_palette->pens();
	uint16_t* srcptr_bitmap;
	uint32_t* dstptr_bitmap;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		srcptr_bitmap = &m_sprite_bitmap.pix16(y);
		dstptr_bitmap = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t pix = srcptr_bitmap[x];

			if (pix != 0x8000)
			{
				if ((pix&0x1000) == pri)
					dstptr_bitmap[x] = paldata[pix & 0xfff];
			}
		}

	}
}

uint32_t pgm2_state::screen_update_pgm2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int mode = m_vidmode[0] & 0x00030000; // other bits not used?

	switch (mode>>16)
	{
		default:
		case 0x00: m_screen->set_visible_area(0, 320 - 1, 0, 240 - 1); break;
		case 0x01: m_screen->set_visible_area(0, 448 - 1, 0, 224 - 1); break;
		case 0x02: m_screen->set_visible_area(0, 512 - 1, 0, 240 - 1); break;
	}

	m_fg_tilemap->set_scrollx(0, m_fgscroll[0] & 0xffff);
	m_fg_tilemap->set_scrolly(0, m_fgscroll[0] >> 16);
	m_bg_tilemap->set_scrolly(0, (m_bgscroll[0x0/4] & 0xffff0000)>>16 );

	for (int y = 0; y <= cliprect.max_y; y++)
	{
		uint16_t linescroll = (y & 1) ? ((m_lineram[(y >> 1)] & 0xffff0000) >> 16) : (m_lineram[(y >> 1)] & 0x0000ffff);
		m_bg_tilemap->set_scrollx((y + ((m_bgscroll[0x0 / 4] & 0xffff0000) >> 16)) & 0x3ff, ((m_bgscroll[0x0 / 4] & 0x0000ffff) >> 0) + linescroll);
	}

	const pen_t *paldata = m_bg_palette->pens();

	bitmap.fill(paldata[0], cliprect); // are there any places bg pen is showing so we know what it should be?

	draw_sprites(screen, cliprect, m_spritebufferram.get());

	copy_sprites_from_bitmap(screen, bitmap, cliprect, 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	copy_sprites_from_bitmap(screen, bitmap, cliprect, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE_LINE_MEMBER(pgm2_state::screen_vblank_pgm2)
{
	// rising edge
	if (state)
	{
		memcpy(m_spritebufferram.get(), m_sp_videoram, 0x2000);
	}
}

WRITE32_MEMBER(pgm2_state::fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pgm2_state::get_fg_tile_info)
{
	int tileno = (m_fg_videoram[tile_index] & 0x0003ffff) >> 0;
	int colour = (m_fg_videoram[tile_index] & 0x007c0000) >> 18; // 5 bits
	int flipxy = (m_fg_videoram[tile_index] & 0x01800000) >> 23;

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPXY(flipxy));
}

WRITE32_MEMBER(pgm2_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pgm2_state::get_bg_tile_info)
{
	int tileno = (m_bg_videoram[tile_index] & 0x0003ffff) >> 0;
	int colour = (m_bg_videoram[tile_index] & 0x003c0000) >> 18; // 4 bits
	int flipxy = (m_bg_videoram[tile_index] & 0x01800000) >> 23;

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPXY(flipxy));
}

void pgm2_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode2, tilemap_get_info_delegate(FUNC(pgm2_state::get_fg_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 96, 64); // 0x6000 bytes
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode3, tilemap_get_info_delegate(FUNC(pgm2_state::get_bg_tile_info), this), TILEMAP_SCAN_ROWS, 32, 32, 64, 32); // 0x2000 bytes
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_rows(32 * 32);

	m_spritebufferram = make_unique_clear<uint32_t[]>(0x2000 / 4);

	m_screen->register_screen_bitmap(m_sprite_bitmap);

	save_pointer(NAME(m_spritebufferram.get()), 0x2000 / 4);

	m_sprites_mask_mask = memregion("sprites_mask")->bytes() - 1;
	m_sprites_colour_mask = memregion("sprites_colour")->bytes() - 1;
}

