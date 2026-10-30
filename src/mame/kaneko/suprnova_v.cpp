// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert
/* Super Kaneko Nova System video */

#include "emu.h"
#include "suprnova.h"

#include <algorithm>

/* draws ROZ with linescroll OR columnscroll to 16-bit indexed bitmap */
void skns_state::draw_roz(bitmap_ind16 &bitmap, bitmap_ind8 &bitmapflags, const rectangle &cliprect, tilemap_t *tmap, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, u32 *scrollram)
{
	//bitmap_ind16 *destbitmap = bitmap;
	const bitmap_ind16 &srcbitmap = tmap->pixmap();
	const bitmap_ind8 &srcbitmapflags = tmap->flagsmap();
	const int xmask = srcbitmap.width() - 1;
	const int ymask = srcbitmap.height() - 1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	//u8 *pri;
	//const u16 *src;
	//const u8 *maskptr;
	//int destadvance = destbitmap->bpp / 8;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + cliprect.min_y * incyx;
	starty += cliprect.min_x * incxy + cliprect.min_y * incyy;

	/* extract start/end points */
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	/* loop over rows */
	while (sy <= ey)
	{
		/* initialize X counters */
		int x = sx;
		u32 cx = startx;
		u32 cy = starty;

		/* get dest and priority pointers */
		u16 *const dest = &bitmap.pix(sy);
		u8 *const destflags = &bitmapflags.pix(sy);

		/* loop over columns */
		while (x <= ex)
		{
			if (wraparound || (cx < widthshifted && cy < heightshifted)) // not sure how this will cope with no wraparound, but row/col scroll..
			{
				const u32 srcx = cx >> 16;
				const u32 srcy = cy >> 16;
				if (columnscroll)
				{
					dest[x] = srcbitmap.pix((srcy - scrollram[srcx & 0x3ff]) & ymask, srcx & xmask);
					destflags[x] = srcbitmapflags.pix((srcy - scrollram[srcx & 0x3ff]) & ymask, srcx & xmask);
				}
				else
				{
					dest[x] = srcbitmap.pix(srcy & ymask, (srcx - scrollram[srcy & 0x3ff]) & xmask);
					destflags[x] = srcbitmapflags.pix(srcy & ymask, (srcx - scrollram[srcy & 0x3ff]) & xmask);
				}
			}

			/* advance in X */
			cx += incxx;
			cy += incxy;
			x++;
			//pri++;
		}

		/* advance in Y */
		startx += incyx;
		starty += incyy;
		sy++;
	}
}


void skns_state::pal_regs_w(offs_t offset, u32 data, u32 mem_mask)
{
	data = COMBINE_DATA(&m_pal_regs[offset]);
	m_palette_updated = true;

	switch (offset)
	{
		/* RWRA regs are for SPRITES */
		case (0x00 / 4): // RWRA0
			if (m_use_spc_bright != BIT(data, 0))
			{
				m_use_spc_bright = BIT(data, 0);
				m_spc_changed = true;
			}
			m_alt_enable_sprites = BIT(data, 8);
			break;
		case (0x04 / 4): // RWRA1
			if (m_bright_spc_g != (data & 0xff))
			{
				m_bright_spc_g = data & 0xff;
				m_spc_changed = true;
			}
			m_bright_spc_g_trans = (data >> 8) & 0xff;
			break;
		case (0x08 / 4): // RWRA2
			if (m_bright_spc_r != (data & 0xff))
			{
				m_bright_spc_r = data & 0xff;
				m_spc_changed = true;
			}
			m_bright_spc_r_trans = (data >> 8) & 0xff;
			break;
		case (0x0c / 4): // RWRA3
			if (m_bright_spc_b != (data & 0xff))
			{
				m_bright_spc_b = data & 0xff;
				m_spc_changed = true;
			}
			m_bright_spc_b_trans = (data >> 8) & 0xff;
			break;

		/* RWRB regs are for BACKGROUND */
		case (0x10 / 4): // RWRB0
			if (m_use_v3_bright != BIT(data, 0))
			{
				m_use_v3_bright = BIT(data, 0);
				m_v3_changed = true;
			}
			m_alt_enable_background = BIT(data, 8);
			break;
		case (0x14 / 4): // RWRB1
			if (m_bright_v3_g != (data & 0xff))
			{
				m_bright_v3_g = data & 0xff;
				m_v3_changed = true;
			}
			m_bright_v3_g_trans = (data >> 8) & 0xff;
			break;
		case (0x18 / 4): // RWRB2
			if (m_bright_v3_r != (data & 0xff))
			{
				m_bright_v3_r = data & 0xff;
				m_v3_changed = true;
			}
			m_bright_v3_r_trans = (data >> 8) & 0xff;
			break;
		case (0x1c / 4): // RWRB3
			if (m_bright_v3_b != (data & 0xff))
			{
				m_bright_v3_b = data & 0xff;
				m_v3_changed = true;
			}
			m_bright_v3_b_trans = (data >> 8) & 0xff;
			break;
	}
}

void skns_state::palette_set_brightness(offs_t offset)
{
	int b = pal5bit(m_palette_ram[offset] >> 0);
	int g = pal5bit(m_palette_ram[offset] >> 5);
	int r = pal5bit(m_palette_ram[offset] >> 10);
	const int a = pal1bit(m_palette_ram[offset] >> 15);

	bool use_bright;
	int brightness_b, brightness_g, brightness_r;
	if (offset < (0x40 * 256)) // 1st half is for Sprites
	{
		use_bright = m_use_spc_bright;
		brightness_b = m_bright_spc_b;
		brightness_g = m_bright_spc_g;
		brightness_r = m_bright_spc_r;
	}
	else // V3 bg's
	{
		use_bright = m_use_v3_bright;
		brightness_b = m_bright_v3_b;
		brightness_g = m_bright_v3_g;
		brightness_r = m_bright_v3_r;
	}

	if (use_bright)
	{
		if (brightness_b) b = (b * (brightness_b + 1)) >> 8;
		else b = 0;
		if (brightness_g) g = (g * (brightness_g + 1)) >> 8;
		else g = 0;
		if (brightness_r) r = (r * (brightness_r + 1)) >> 8;
		else r = 0;
	}

	m_palette->set_pen_color(offset, rgb_t(a, r, g, b));
}

void skns_state::palette_ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_palette_ram[offset]);

	palette_set_brightness(offset);
}


void skns_state::palette_update()
{
	if (m_palette_updated)
	{
		if (m_spc_changed)
		{
			for (int i = 0; i < (0x40 * 256); i++)
			{
				palette_set_brightness(i);
			}
			m_spc_changed = false;
		}
		if (m_v3_changed)
		{
			for (int i = (0x40 * 256); i < (0x80 * 256); i++)
			{
				palette_set_brightness(i);
			}
			m_v3_changed = false;
		}
		m_palette_updated = false;
	}
}


template <unsigned Which>
TILE_GET_INFO_MEMBER(skns_state::get_tile_info)
{
	const u32 code = ((m_tilemap_ram[Which][tile_index] & 0x001fffff) >> 0);
	const u32 colr = ((m_tilemap_ram[Which][tile_index] & 0x3f000000) >> 24);
	const u8 pri  = ((m_tilemap_ram[Which][tile_index] & 0x00e00000) >> 21);
	const u8 depth = BIT(m_v3_regs[0x0c / 4], Which << 3);
	const u32 flags = TILE_FLIPXY(m_tilemap_ram[Which][tile_index] >> 30);

	tileinfo.set((Which << 1) + depth,
			code,
			colr,
			flags);
	tileinfo.category = pri;

	//if (pri) popmessage("pri A!! %02x\n", pri);
}

void skns_state::v3_regs_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old = m_v3_regs[offset];
	data = COMBINE_DATA(&m_v3_regs[offset]);

	/* if the depth changes we need to dirty the tilemap */
	if (offset == 0x0c / 4)
	{
		if (BIT(old ^ data, 0))
			m_tilemap[0]->mark_all_dirty();
		if (BIT(old ^ data, 8))
			m_tilemap[1]->mark_all_dirty();
	}
}


void skns_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skns_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[0]->set_transparent_pen(0);

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skns_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap_bitmap_lower.allocate(512, 512);
	m_tilemap_bitmapflags_lower.allocate(512, 512);

	m_tilemap_bitmap_higher.allocate(512, 512);
	m_tilemap_bitmapflags_higher.allocate(512, 512);

	m_gfxdecode->gfx(1)->set_granularity(256);
	m_gfxdecode->gfx(3)->set_granularity(256);

	save_item(NAME(m_use_spc_bright));
	save_item(NAME(m_use_v3_bright));
	save_item(NAME(m_bright_spc_b));
	save_item(NAME(m_bright_spc_g));
	save_item(NAME(m_bright_spc_r));
	save_item(NAME(m_bright_spc_b_trans));
	save_item(NAME(m_bright_spc_g_trans));
	save_item(NAME(m_bright_spc_r_trans));
	save_item(NAME(m_bright_v3_b));
	save_item(NAME(m_bright_v3_g));
	save_item(NAME(m_bright_v3_r));
	save_item(NAME(m_bright_v3_b_trans));
	save_item(NAME(m_bright_v3_g_trans));
	save_item(NAME(m_bright_v3_r_trans));
	save_item(NAME(m_spc_changed));
	save_item(NAME(m_v3_changed));
	save_item(NAME(m_palette_updated));
	save_item(NAME(m_alt_enable_background));
	save_item(NAME(m_alt_enable_sprites));
}

void skns_state::video_reset()
{
	m_use_spc_bright = m_use_v3_bright = true;
	m_bright_spc_b= m_bright_spc_g = m_bright_spc_r = 0x00;
	m_bright_spc_b_trans = m_bright_spc_g_trans = m_bright_spc_r_trans = 0x00;
	m_bright_v3_b = m_bright_v3_g = m_bright_v3_r = 0x00;
	m_bright_v3_b_trans = m_bright_v3_g_trans = m_bright_v3_r_trans = 0x00;

	m_spc_changed = m_v3_changed = m_palette_updated = false;
	m_alt_enable_background = m_alt_enable_sprites = true;
}

void skns_state::draw_tilemap(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int which)
{
	const u32 reg_offs = (which ? 0x34 : 0x10) / 4;
	if (!BIT(m_v3_regs[reg_offs], 0))
		return;

	const bool nowrap = BIT(m_v3_regs[reg_offs], 2);

	const u32 startx = m_v3_regs[reg_offs + (0x0c / 4)];
	const u32 starty = m_v3_regs[reg_offs + (0x10 / 4)];
	const int incxx  = util::sext(m_v3_regs[reg_offs + (0x14 / 4)] & 0x7ffff, 19);
	const int incxy  = m_v3_regs[reg_offs + (0x18 / 4)];
	const int incyx  = m_v3_regs[reg_offs + (0x1c / 4)];
	const int incyy  = util::sext(m_v3_regs[reg_offs + (0x20 / 4)] & 0x7ffff, 19); // level 3 boss in sengekis

	const bool columnscroll = BIT(m_v3_regs[0x0c / 4], which ? 9 : 1); // selects column scroll or rowscroll

	draw_roz(bitmap, bitmap_flags,
			cliprect, m_tilemap[which],
			startx << 8, starty << 8,
			incxx << 8, incxy << 8, incyx << 8, incyy << 8,
			!nowrap, columnscroll,
			&m_v3slc_ram[which << 10]);
}

u32 skns_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	palette_update();

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_tilemap_bitmap_lower.fill(0);
	m_tilemap_bitmapflags_lower.fill(0);
	m_tilemap_bitmap_higher.fill(0);
	m_tilemap_bitmapflags_higher.fill(0);

	{
		const int tilemap_pri_a = BIT(m_v3_regs[0x10 / 4], 1);
		const int tilemap_pri_b = BIT(m_v3_regs[0x34 / 4], 1);

		//popmessage("pri %d %d\n", tilemap_pri_a, tilemap_pri_b);

		/*if (!tilemap_pri_b) { */
		if (m_alt_enable_background)
		{
			draw_tilemap(m_tilemap_bitmap_lower,  m_tilemap_bitmapflags_lower,  cliprect, 1);
			draw_tilemap(m_tilemap_bitmap_higher, m_tilemap_bitmapflags_higher, cliprect, 0);
		}

		{
			pen_t const *const clut = &m_palette->pen(0);

			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				u16 const *const src = &m_tilemap_bitmap_lower.pix(y);
				u8 const *const srcflags = &m_tilemap_bitmapflags_lower.pix(y);

				u16 const *const src2 = &m_tilemap_bitmap_higher.pix(y);
				u8 const *const src2flags = &m_tilemap_bitmapflags_higher.pix(y);

				u16 const *const src3 = &m_spritegen->bitmap().pix(y);

				u32 *const dst = &bitmap.pix(y);

				for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					u16 pendata  = src[x] & 0x7fff;
					u16 pendata2 = src2[x] & 0x7fff;
					u16 pendata3 = m_alt_enable_sprites ? src3[x] & 0x3fff : 0;

					u16 pri  = ((srcflags[x] & 0x07) << 1) | tilemap_pri_b;
					u16 pri2 = ((src2flags[x] & 0x07) << 1) | tilemap_pri_a;
					u16 pri3 = ((src3[x] & 0xc000) >> 12) + 3;

					const bool pendraw = pendata & 0xff;
					const bool pen2draw = pendata2 & 0xff;
					const bool pen3draw = pendata3 & 0xff;

					// work out which layers bg pixel has the higher priority
					//  note, can the bg layers be blended?? sarukani uses an alpha pen for
					//        some of them.. and registers indicate it might be possible..

					// this priority mixing is almost certainly still incorrect
					// bg colour / prioirty handling is now wrong

					u16 bgpendata;
					u16 bgpri;
					if (pri <= pri2) // <= is good for last level of cyvern.. < seem better for galpanis kaneko logo
					{
						if (pen2draw)
						{
							bgpendata = pendata2 & 0x7fff;
							bgpri = pri2;
						}
						else if (pendraw)
						{
							bgpendata = pendata & 0x7fff;
							bgpri = pri;
						}
						else
						{
							bgpendata = pendata2 & 0x7fff;
							bgpri = 0;
						}
					}
					else
					{
						if (pendraw)
						{
							bgpendata = pendata & 0x7fff;
							bgpri = pri;
						}
						else if (pen2draw)
						{
							bgpendata = pendata2 & 0x7fff;
							bgpri = pri2;
						}
						else
						{
							bgpendata = 0;
							bgpri = 0;
						}
					}

					// if the sprites are higher than the bg pixel
					u32 coldat = 0;
					if (pri3 > bgpri)
					{
						if (pen3draw)
						{
							coldat = clut[pendata3];

							if (BIT(coldat, 31))
							{
								const u32 srccolour = clut[bgpendata & 0x7fff];
								const u32 dstcolour = clut[pendata3 & 0x3fff];

								int r = (srccolour & 0x000000ff) >> 0;
								int g = (srccolour & 0x0000ff00) >> 8;
								int b = (srccolour & 0x00ff0000) >> 16;

								int r2 = (dstcolour & 0x000000ff) >> 0;
								int g2 = (dstcolour & 0x0000ff00) >> 8;
								int b2 = (dstcolour & 0x00ff0000) >> 16;

								r2 = (r2 * m_bright_spc_r_trans) >> 8;
								g2 = (g2 * m_bright_spc_g_trans) >> 8;
								b2 = (b2 * m_bright_spc_b_trans) >> 8;

								r = std::min(r + r2, 255);
								g = std::min(g + g2, 255);
								b = std::min(b + b2, 255);

								coldat = (r << 0) | (g << 8) | (b << 16);
							}
							else
							{
								coldat = clut[pendata3];
							}
						}
						else
						{
							coldat = clut[bgpendata];
						}
					}
					else
					{
						coldat = clut[bgpendata];
					}
					dst[x] = coldat | (0xffu << 24u);
				}
			}
		}
	}

	return 0;
}

void skns_state::screen_vblank(int state)
{
	if (state)
	{
		m_spritegen->draw_sprites(m_screen->visible_area(), m_spriteram, m_spriteram.bytes(), m_spc_regs); // TODO: not all 0x4000 of the sprite RAM area can be displayed on real hardware
	}
}
