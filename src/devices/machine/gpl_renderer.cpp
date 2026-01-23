// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

// GPL162xx (and related) chips have a rendering engine that is similar in some ways to SPG2xx
// but different enough to keep as a separate implementation

#include "emu.h"
#include "gpl_renderer.h"

DEFINE_DEVICE_TYPE(GPL_RENDERER, gpl_renderer_device, "gpl_renderer", "GeneralPlus video rendering")

gpl_renderer_device::gpl_renderer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_space_read_cb(*this, 0),
	m_cpuspace(*this, finder_base::DUMMY_TAG, -1),
	m_cs_space(*this, finder_base::DUMMY_TAG, -1)
{
}

gpl_renderer_device::gpl_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	gpl_renderer_device(mconfig, GPL_RENDERER, tag, owner, clock)
{
}

void gpl_renderer_device::device_start()
{
	m_rgb555_to_rgb888 = std::make_unique<uint32_t []>(0x8000);
	m_rgb555_to_rgb888_current = std::make_unique<uint32_t []>(0x8000);

	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);

		m_rgb555_to_rgb888_current[i] = 0x0000;
	}

	save_item(NAME(m_video_regs_1c));
	save_item(NAME(m_video_regs_1d));
	save_item(NAME(m_video_regs_1e));

	save_item(NAME(m_video_regs_2a));

	save_item(NAME(m_video_regs_30));
	save_item(NAME(m_video_regs_3c));

	save_item(NAME(m_video_regs_42));

	save_item(NAME(m_ycmp_table));

	save_pointer(NAME(m_rgb555_to_rgb888_current), 0x8000);
	save_item(NAME(m_brightness_or_saturation_dirty));

	// new GPL features
	save_item(NAME(m_video_regs_7f));
}

void gpl_renderer_device::device_reset()
{
	m_video_regs_1c = 0x0000;
	m_video_regs_1d = 0x0000;
	m_video_regs_1e = 0x0000;

	m_video_regs_2a = 0x0000;

	m_video_regs_30 = 0x0000;
	m_video_regs_3c = 0x0020;

	m_video_regs_42 = 0x0001;

	for (int i = 0; i < 480; i++)
	{
		m_ycmp_table[i] = 0xffffffff;
	}

	m_brightness_or_saturation_dirty = true;

	// new GPL features
	m_video_regs_7f = 0x0000;
}


// this builds up a line table for the vcmp effect, this is not correct when step is used
// untested on GPL renderer, rarely used even on SPG
void gpl_renderer_device::update_vcmp_table()
{
	int currentline = 0;

	int step = m_video_regs_1e & 0xff;
	if (step & 0x80)
		step = step - 0x100;

	int current_inc_value = (m_video_regs_1c<<4);

	int counter = 0;

	for (int i = 0; i < 480; i++)
	{
		if (i < m_video_regs_1d)
		{
			m_ycmp_table[i] = 0xffffffff;
		}
		else
		{
			if ((currentline >= 0) && (currentline < 256))
			{
				m_ycmp_table[i] = currentline;
			}

			counter += current_inc_value;

			while (counter >= (0x20<<4))
			{
				currentline++;
				current_inc_value += step;

				counter -= (0x20<<4);
			}
		}
	}
}

// Perform a lerp between a and b
inline uint8_t gpl_renderer_device::mix_channel(uint8_t bottom, uint8_t top, uint8_t alpha)
{
	return ((0x20 - alpha) * bottom + alpha * top) >> 5;
}

template<bool Blend, bool FlipX>
void gpl_renderer_device::draw_tilestrip(bool read_from_csspace, uint32_t screenwidth, uint32_t drawwidthmask, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *paletteram, uint8_t blendlevel)
{
	const uint32_t yflipmask = flip_y ? tile_h - 1 : 0;
	uint32_t m = tilegfxdata_addr + words_per_tile * tile + bits_per_row * (tile_scanline ^ yflipmask);
	uint32_t bits = 0;
	uint32_t nbits = 0;

	for (int32_t x = FlipX ? (tile_w - 1) : 0; FlipX ? x >= 0 : x < tile_w; FlipX ? x-- : x++)
	{
		int realdrawpos = (drawx + x) & drawwidthmask;

		bits <<= nc_bpp;

		if (nbits < nc_bpp)
		{
			if (!read_from_csspace)
			{
				uint16_t b = spc.read_word(m++ & 0x3fffff);
				b = (b << 8) | (b >> 8);
				bits |= b << (nc_bpp - nbits);
				nbits += 16;
			}
			else
			{
				uint16_t b;
				const int addr = m & 0x7ffffff;
				if (addr < m_csbase)
				{
					b = m_cpuspace->read_word(addr);
				}
				else
				{
					b = m_cs_space->read_word(addr - m_csbase);
				}
				m++;
				b = (b << 8) | (b >> 8);
				bits |= b << (nc_bpp - nbits);
				nbits += 16;
			}
		}
		nbits -= nc_bpp;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;


		if (realdrawpos >= 0 && realdrawpos < screenwidth)
		{
			uint16_t rgb = paletteram[pal];

			if (!(rgb & 0x8000))
			{
				if (Blend && !(m_linebuf[realdrawpos] & 0x8000))
				{

					m_linebuf[realdrawpos] = (mix_channel((uint8_t)(m_linebuf[realdrawpos] >> 10) & 0x1f,  (rgb >> 10) & 0x1f, blendlevel) << 10) |
											 (mix_channel((uint8_t)(m_linebuf[realdrawpos] >> 5)  & 0x1f,  (rgb >> 5)  & 0x1f, blendlevel) << 5) |
											 (mix_channel((uint8_t)(m_linebuf[realdrawpos] >> 0)  & 0x1f,  (rgb >> 0)  & 0x1f, blendlevel) << 0);
				}
				else
				{
					m_linebuf[realdrawpos]  = rgb;
				}
			}
		}
	}
}

void gpl_renderer_device::draw_tilestrip(bool read_from_csspace, uint32_t screenwidth, uint32_t drawwidthmask, bool blend, bool flip_x, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *paletteram, uint8_t blendlevel)
{
	if (blend)
	{
		if (flip_x)
		{
			draw_tilestrip<true, true>(read_from_csspace, screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		else
		{
			draw_tilestrip<true, false>(read_from_csspace, screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
	else
	{
		if (flip_x)
		{
			draw_tilestrip<false, true>(read_from_csspace, screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		else
		{
			draw_tilestrip<false, false>(read_from_csspace, screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
}

void gpl_renderer_device::draw_linemap(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram)
{
	uint32_t ctrl = tilemapregs[1];

	if (0)
	{
		if (ctrl & 0x0010)
			popmessage("bitmap mode %08x with rowscroll\n", tilegfxdata_addr);
		else
			popmessage("bitmap mode %08x\n", tilegfxdata_addr);
	}

	// note, in interlace modes it appears every other line is unused? (480 entry table, but with blank values)
	// and furthermore the rowscroll and rowzoom tables only have 240 entries, not enough for every line
	// the end of the rowscroll table (entries 240-255) contain something else, maybe garbage data as it's offscreen, maybe not
	uint32_t tilemap = tilemapregs[2];
	uint32_t palette_map = tilemapregs[3];

	uint32_t linebase = spc.read_word(tilemap + scanline); // every other word is unused, but there are only enough entries for 240 lines then, sometimes to do with interlace mode?
	uint16_t palette = spc.read_word(palette_map + (scanline / 2));

	if (scanline & 1)
		palette >>= 8;
	else
		palette &= 0xff;

	if (!linebase)
		return;

	linebase = linebase | (palette << 16);

	int upperpalselect = 0;
	if (tilegfxdata_addr & 0x80000000)
		upperpalselect = 1;

	tilegfxdata_addr &= 0x7ffffff;

	// this logic works for jak_s500 and the test modes to get the correct base, doesn't seem to work for jak_car2 ingame, maybe data is copied to wrong place?
	int gfxbase = (tilegfxdata_addr & 0x7ffffff) + (linebase & 0x7ffffff);

	for (int i = 0; i < 160; i++) // will have to be 320 for jak_car2 ingame, jak_s500 lines are wider than screen, and zoomed
	{
		uint16_t pix;
		const int addr = gfxbase & 0x7ffffff;
		if (addr < m_csbase)
		{
			pix = m_cpuspace->read_word(addr);
		}
		else
		{
			pix = m_cs_space->read_word(addr - m_csbase);
		}
		gfxbase++;

		int xx;
		uint16_t pal;

		if ((scanline >= 0) && (scanline < 480))
		{
			xx = i * 2;

			pal = (pix & 0xff) | 0x100;

			if (upperpalselect)
				pal |= 0x200;

			if (xx >= 0 && xx <= cliprect.max_x)
			{
				uint16_t rgb = paletteram[pal];

				if (!(rgb & 0x8000))
				{
					m_linebuf[xx] = rgb;
				}
			}

			xx = (i * 2) + 1;
			pal = (pix >> 8) | 0x100;

			if (upperpalselect)
				pal |= 0x200;

			if (xx >= 0 && xx <= cliprect.max_x)
			{
				uint16_t rgb = paletteram[pal];

				if (!(rgb & 0x8000))
				{
					m_linebuf[xx] = rgb;
				}
			}
		}
	}
}

void gpl_renderer_device::draw_sprite(bool read_from_csspace, int extended_sprites_mode, uint32_t palbank, bool highres, const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, uint32_t base_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram)
{
	uint32_t tilegfxdata_addr = spritegfxdata_addr;
	uint32_t tile = spriteram[base_addr + 0];
	int16_t x = spriteram[base_addr + 1];
	int16_t y = spriteram[base_addr + 2];
	uint16_t attr = spriteram[base_addr + 3];

	if (!tile)
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	uint32_t screenwidth;
	uint32_t screenheight;
	uint32_t xmask;
	uint32_t ymask;

	screenwidth = 320;
	screenheight = 256;
	xmask = 0x1ff;
	ymask = 0x1ff;

	// TODO: higher mask values might apply all the time on GPL
	if (highres)
	{
		screenwidth = 640;
		screenheight = 512;
		xmask = 0x3ff;
		ymask = 0x3ff;
	}

	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << ((attr & 0x0030) >> 4);

	// TODO: only applies in QVGA mode, not VGA mode
	if (!(m_video_regs_42 & 0x0002))
	{
		x = ((screenwidth/2) + x) - tile_w / 2;
		y = ((screenheight/2) - y) - (tile_h / 2);
	}

	x &= xmask;
	y &= ymask;

	int firstline = y;
	int lastline = y + (tile_h - 1);
	lastline &= ymask;

	const bool blend = (attr & 0x4000) ? true : false;
	bool flip_x = (attr & 0x0004) ? true : false;
	bool flip_y = (attr & 0x0008) ? true : false;
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;

	// Max blend level (3) should result in 100% opacity on the sprite, per docs
	// Min blend level (0) should result in 25% opacity on the sprite, per docs
	static const uint8_t s_blend_levels[4] = { 0x08, 0x10, 0x18, 0x20 };
	uint8_t blendlevel = s_blend_levels[m_video_regs_2a & 3];

	uint32_t words_per_tile = bits_per_row * tile_h;

	// 7400 format on GPL162xx is
	//
	// 7400 - NNNN NNNN NNNN NNNN (N = sprite tile number/address)
	// 7401 - AAAA AAXX XXXX XXXX (A = Angle or Y1[5:0], X = Xpos/X0[9:0])
	// 7402 - ZZZZ ZZYY YYYY YYYY (Z = Zoom, or Y2[5:0], Y = Ypos/Y0[9:0])
	// 7403 - pbDD PPPP VVHH FFCC (p = Palette Bank, b = blend, D = depth, P = palette, V = vertical size, H = horizontal size, F = flip, C = colour)
	if (m_video_regs_7f & 0x0200) // 'virtual 3D' sprite mode (GPAC800 / GPL16250 only) has 4 extra entries per sprite
	{
		// 2nd sprite bank is...
		//
		// 7400 - MMBB BBBB NNNN NNNN - M = Mosaic, B = blend level, N = sprite/tile number/adddress)    Attribute 1 of sprite 0
		// 7401 - YYYY YYXX XXXX XXXX - Y = Y3[5:0]             X = X1[9:0]                              X1 of sprite 0
		// 7402 - YYyy yyXX XXXX XXXX - Y = Y3[7:6] y = Y1[9:6] X = X2[9:0]                              X2 of sprite 0
		// 7403 - YYyy yyXX XXXX XXXX - Y = Y3[9:8] y = Y2[9:6] X = X3[9:0]                              X3 of sprite 0
		// 7404 - Attribute 1 of sprite 1
		// ....
		//
		// Normally Zoom/Rotate functions are disabled in this mode, as the attributes are use for co-ordinate data
		// but setting Flip to 0x3 causes them to be used (ignoring flip) instead of the extra co-ordinates
		flip_x = false;
		flip_y = false;

		tile |= (spriteram[(base_addr)+0x400] & 0x00ff) << 16;
		blendlevel = ((spriteram[(base_addr)+0x400] & 0x3f00) >> 8);
	}
	else // regular extended mode, just 1 extra entry per sprite
	{
		// 2nd sprite bank is...
		// 7400 - MMBB BBBB NNNN NNNN - M = Mosaic, B = blend level, N = sprite/tile number/adddress)    Attribute 1 of sprite 0
		// ....

		// before or after the 0 tile check?
		tile |= (spriteram[(base_addr / 4) + 0x400] & 0x00ff) << 16;
		blendlevel = ((spriteram[(base_addr / 4) + 0x400] & 0x3f00) >> 8);
	}

	blendlevel >>= 1; // hack, drawing code expects 5 bits, not 6

	// good for gormiti, smartfp, wrlshunt, paccon, jak_totm, jak_s500, jak_gtg
	if (m_video_regs_42 & 0x0010) // direct addressing mode
	{
		// paccon and smartfp use this mode
		words_per_tile = 8;
	}
	else
	{
		// extended address bits only used in direct mode, jak_prr and other GPAC500 games rely on this
		tile &= 0xffff;
	}

	uint32_t palette_offset = (attr & 0x0f00) >> 4;

	// TODO: tkmag220 / myac220 don't set this bit and expect all sprite palettes to be from the same bank as background palettes
	// beijuehh (extended_sprites_mode == 2) appears to disagree with that logic, it has this set, but expects palettes and sprites
	// from the first bank but also needs the attr & 0x8000 check below for the 'pause' graphics so isn't ignoring the 'extended'
	// capabilities entirely.
	if ((palbank & 1) && (extended_sprites_mode != 2))
		palette_offset |= 0x100;

	// many other gpl16250 sets have this bit set when they want the upper 256 colours on a per-sprite basis, seems like an extended feature
	if (attr & 0x8000)
		palette_offset |= 0x200;

	// the Circuit Racing game in PDC100 needs this or some graphics have bad colours at the edges when turning as it leaves stray lower bits set
	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;

	if (firstline < lastline)
	{
		int scanx = scanline - firstline;

		if ((scanx >= 0) && (scanline <= lastline))
		{
			draw_tilestrip(read_from_csspace, screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
	else
	{
		// clipped from top
		int tempfirstline = firstline - (screenheight<<1);
		int templastline = lastline;
		int scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			draw_tilestrip(read_from_csspace, screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		// clipped against the bottom
		tempfirstline = firstline;
		templastline = lastline + (screenheight<<1);
		scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			draw_tilestrip(read_from_csspace, screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
}

void gpl_renderer_device::draw_sprites(bool read_from_csspace, int extended_sprites_mode, uint32_t palbank, bool highres, const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram)
{
	if (!(m_video_regs_42 & 0x0001))
		return;

	// sprite count / limit appears to be a GPL only feature
	int sprlimit = (m_video_regs_42 & 0xff00) >> 8;
	if (sprlimit == 0)
		sprlimit = 0x100;

	for (uint32_t n = 0; n < sprlimit; n++)
	{
		draw_sprite(read_from_csspace, extended_sprites_mode, palbank, highres, cliprect, scanline, priority, spritegfxdata_addr, 4 * n, spc, paletteram, spriteram);
	}
}

void gpl_renderer_device::new_line(const rectangle &cliprect)
{
	update_palette_lookup();

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		m_linebuf[x] = 0x0000; // non-transparent (paccon blends against the back colour at least)
	}
}

void gpl_renderer_device::draw_page(bool read_from_csspace, uint32_t palbank, const rectangle &cliprect, uint32_t scanline, int priority, uint16_t tilegfxdata_addr_msb, uint16_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram, uint16_t *scrollram, uint32_t which)
{
	const uint32_t attr = tilemapregs[0];
	const uint32_t ctrl = tilemapregs[1];

	if (!(ctrl & 0x0008))
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	// graphic data segments/bases
	uint32_t tilegfxdata_addr_full;

	if (m_video_regs_7f & 0x0040) // FREE == 1
	{
		tilegfxdata_addr_full = ((tilegfxdata_addr_msb & 0x07ff) << 16) | tilegfxdata_addr;
	}
	else // FREE == 0 (default / legacy)
	{
		tilegfxdata_addr_full = tilegfxdata_addr * 0x40;
	}

	if (ctrl & 0x0001) // Bitmap / Linemap mode! (basically screen width tile mode)
	{
		draw_linemap(cliprect, scanline, priority, tilegfxdata_addr_full, scrollregs, tilemapregs, spc, paletteram);
		return;
	}

	uint32_t logical_scanline = scanline;

	if (ctrl & 0x0040) // 'vertical compression feature' (later models only?)
	{
		// used by senspeed
		//if (m_video_regs_1e != 0x0000)
		//  popmessage("vertical compression mode with non-0 step amount %04x offset %04x step %04x\n", m_video_regs_1c, m_video_regs_1d, m_video_regs_1e);

		logical_scanline = m_ycmp_table[scanline];
		if (logical_scanline == 0xffffffff)
			return;
	}

	uint32_t total_width, y_mask, screenwidth;

	// TODO: both bits control overall tilemap size (NOT tied to screen resolution!)
	if (attr & 0x8000)
	{
		total_width = 1024;
		y_mask = 0x200;
		screenwidth = 640;
	}
	else
	{
		total_width = 512;
		y_mask = 0x100;
		screenwidth = 320;
	}

	if (attr & 0x4000)
	{
		y_mask <<= 1; // double height tilemap?
	}

	const uint32_t drawwidthmask = total_width - 1;
	y_mask--; // turn into actual mask

	const uint32_t xscroll = scrollregs[0];
	const uint32_t yscroll = scrollregs[1];
	const uint32_t tilemap_rambase = tilemapregs[2];
	const uint32_t exattributemap_rambase = tilemapregs[3];
	const int tile_width = (attr & 0x0030) >> 4;
	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << (tile_width);
	const uint32_t tile_count_x = total_width / tile_w; // tilemaps are 512 or 1024 wide depending on screen mode?
	const uint32_t bitmap_y = (logical_scanline + yscroll) & y_mask; // tilemaps are 256 or 512 high depending on screen mode?
	const uint32_t y0 = bitmap_y / tile_h;
	const uint32_t tile_scanline = bitmap_y % tile_h;
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;
	const bool row_scroll = (ctrl & 0x0010);

	// Max blend level (3) should result in 100% opacity, per docs
	// Min blend level (0) should result in 25% opacity, per docs
	static const uint8_t s_blend_levels[4] = { 0x08, 0x10, 0x18, 0x20 };
	uint8_t blendlevel = s_blend_levels[m_video_regs_2a & 3];

	// good for gormiti, smartfp, wrlshunt, paccon, jak_totm, jak_s500, jak_gtg
	uint32_t words_per_tile;

	if (m_video_regs_7f & 0x0004) // TX_DIRECT
		words_per_tile = 8;
	else
		words_per_tile = bits_per_row * tile_h;

	int realxscroll = xscroll;

	// the logic seems to be different on GPL16250 compared to SPG2xx
	// see Galaxian in paccon and Crazy Moto in myac220, is this mode be selected or did behavior just change?
	if (row_scroll)
		realxscroll += (int16_t)scrollram[logical_scanline & 0xff];

	const int upperscrollbits = (realxscroll >> (tile_width + 3));
	const int endpos = (screenwidth + tile_w) / tile_w;

	for (uint32_t x0 = 0; x0 < endpos; x0++)
	{
		bool blend;
		bool flip_x;
		bool flip_y;
		uint32_t tile;
		uint32_t palette_offset;

		// get tile info
		const int realx0 = (x0 + upperscrollbits) & (tile_count_x - 1);
		uint32_t tile_address = realx0 + (tile_count_x * y0);

		tile = (ctrl & 0x0004) ? spc.read_word(tilemap_rambase) : spc.read_word(tilemap_rambase + tile_address);

		// TODO: no skipping in direct modes?
		if (!tile)
		{
			if (m_video_regs_7f & 0x0002)
			{
				// Galaga in paccon won't render '0' characters in the scoring table if you skip empty tiles, so maybe GPL16250 doesn't skip? - extra tile bits from extended read make no difference

				// probably not based on register m_video_regs_7f, but paccon galaga needs no skip, jak_gtg and jak_hmhsm needs to skip
				//49 0100 1001  no skip (paccon galaga)
				//4b 0100 1011  skip    (paccon pacman)
				//53 0101 0011  skip    (jak_gtg, jak_hmhsm)
				continue;
			}
		}

		uint32_t tileattr = attr;
		uint32_t tilectrl = ctrl;

		if (m_video_regs_7f & 0x0004) // TX_DIRECT
		{
			uint16_t exattribute = (tilectrl & 0x0004) ? spc.read_word(exattributemap_rambase) : spc.read_word(exattributemap_rambase + tile_address / 2);
			if (realx0 & 1)
				exattribute >>= 8;
			else
				exattribute &= 0x00ff;

			// when TX_DIRECT is used the attributes become extra addressing bits (smartfp)
			tile |= (exattribute & 0xff) << 16;
			//blendlevel = 0x1f; // hack
		}
		else
		{
			if ((tilectrl & 2) == 0)
			{
				// -(1) bld(1) flip(2) pal(4)
				uint16_t exattribute = (tilectrl & 0x0004) ? spc.read_word(exattributemap_rambase) : spc.read_word(exattributemap_rambase + tile_address / 2);
				if (realx0 & 1)
					exattribute >>= 8;
				else
					exattribute &= 0x00ff;

				tileattr &= ~0x000c;
				tileattr |= (exattribute >> 2) & 0x000c;    // flip

				tileattr &= ~0x0f00;
				tileattr |= (exattribute << 8) & 0x0f00;    // palette

				tilectrl &= ~0x0100;
				tilectrl |= (exattribute << 2) & 0x0100;    // blend
			}
		}

		blend = (tilectrl & 0x0100) ? true : false;
		flip_x = (tileattr & 0x0004) ? true : false;
		flip_y = (tileattr & 0x0008) ? true : false;

		palette_offset = (tileattr & 0x0f00) >> 4;
		// got tile info

		// TODO, some GPL models use 2 bits here, others only use this
		if (tilegfxdata_addr_msb & 0x8000)
			palette_offset |= 0x200;

		palette_offset >>= nc_bpp;
		palette_offset <<= nc_bpp;

		const int drawx = (x0 * tile_w) - (realxscroll & (tile_w - 1));
		draw_tilestrip(read_from_csspace, screenwidth, drawwidthmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr_full, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
	}
}

void gpl_renderer_device::apply_saturation_and_fade(bitmap_rgb32 &bitmap, const rectangle &cliprect, int scanline)
{
	uint32_t* src = &bitmap.pix(scanline, cliprect.min_x);

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		uint16_t px = (m_linebuf[x] & 0x8000) ? 0x0 : m_linebuf[x];
		*src = m_rgb555_to_rgb888_current[px];
		src++;
	}
}


void gpl_renderer_device::update_palette_lookup()
{
	if (!m_brightness_or_saturation_dirty)
		return;

	static const float s_u8_to_float = 1.0f / 255.0f;
	static const float s_gray_r = 0.299f;
	static const float s_gray_g = 0.587f;
	static const float s_gray_b = 0.114f;
	const float sat_adjust = (0xff - (m_video_regs_3c & 0x00ff)) / (float)(0xff - 0x20);

	const uint16_t fade_offset = m_video_regs_30;

	for (uint16_t i = 0; i < 0x8000; i++)
	{
		uint32_t src = m_rgb555_to_rgb888[i];

		if ((m_video_regs_3c & 0x00ff) != 0x0020) // apply saturation
		{
			const uint32_t src_rgb = src;
			const float src_r = (uint8_t)(src_rgb >> 16) * s_u8_to_float;
			const float src_g = (uint8_t)(src_rgb >> 8) * s_u8_to_float;
			const float src_b = (uint8_t)(src_rgb >> 0) * s_u8_to_float;
			const float luma = src_r * s_gray_r + src_g * s_gray_g + src_b * s_gray_b;
			const float adjusted_r = luma + (src_r - luma) * sat_adjust;
			const float adjusted_g = luma + (src_g - luma) * sat_adjust;
			const float adjusted_b = luma + (src_b - luma) * sat_adjust;
			const int integer_r = (int)floor(adjusted_r * 255.0f);
			const int integer_g = (int)floor(adjusted_g * 255.0f);
			const int integer_b = (int)floor(adjusted_b * 255.0f);
			src = (integer_r > 255 ? 0xff0000 : (integer_r < 0 ? 0 : ((uint8_t)integer_r << 16))) |
				(integer_g > 255 ? 0x00ff00 : (integer_g < 0 ? 0 : ((uint8_t)integer_g << 8))) |
				(integer_b > 255 ? 0x0000ff : (integer_b < 0 ? 0 : (uint8_t)integer_b));
		}

		if (fade_offset != 0) // apply fade
		{
			const uint32_t src_rgb = src;
			const uint8_t src_r = (src_rgb >> 16) & 0xff;
			const uint8_t src_g = (src_rgb >> 8) & 0xff;
			const uint8_t src_b = (src_rgb >> 0) & 0xff;
			const uint8_t r = src_r - fade_offset;
			const uint8_t g = src_g - fade_offset;
			const uint8_t b = src_b - fade_offset;
			src = (r > src_r ? 0 : (r << 16)) |
				(g > src_g ? 0 : (g << 8)) |
				(b > src_b ? 0 : (b << 0));
		}

		m_rgb555_to_rgb888_current[i] = src;
	}

	m_brightness_or_saturation_dirty = false;
}
