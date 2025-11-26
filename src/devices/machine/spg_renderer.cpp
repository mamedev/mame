// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

#include "emu.h"
#include "spg_renderer.h"

DEFINE_DEVICE_TYPE(SPG_RENDERER, spg_renderer_device, "spg_renderer", "SunPlus SPG2xx video rendering")

spg_renderer_device::spg_renderer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
{
}

spg_renderer_device::spg_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg_renderer_device(mconfig, SPG_RENDERER, tag, owner, clock)
{
}

void spg_renderer_device::device_start()
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
}

void spg_renderer_device::device_reset()
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
}


// Perform a lerp between a and b
inline uint8_t spg_renderer_device::mix_channel(uint8_t bottom, uint8_t top, uint8_t alpha)
{
	return ((0x20 - alpha) * bottom + alpha * top) >> 5;
}

template<bool Blend, bool FlipX>
void spg_renderer_device::draw_tilestrip(uint32_t screenwidth, uint32_t drawwidthmask, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *paletteram, uint8_t blendlevel)
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
			uint16_t b = spc.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
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


void spg_renderer_device::draw_linemap(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram)
{
	if ((scanline < 0) || (scanline >= 240))
		return;

	uint32_t tilemap = tilemapregs[2];
	uint32_t palette_map = tilemapregs[3];

	//if (scanline == 128)
	//  popmessage("draw draw_linemap reg0 %04x reg1 %04x bases %04x %04x\n", tilemapregs[0], tilemapregs[1], tilemap, palette_map);

	//uint32_t xscroll = scrollregs[0];
	uint32_t yscroll = scrollregs[1];

	int realline = (scanline + yscroll) & 0xff;


	uint32_t tile = spc.read_word(tilemap + realline);
	uint16_t palette = 0;

	//if (!tile)
	//  continue;

	palette = spc.read_word(palette_map + realline / 2);
	if (scanline & 1)
		palette >>= 8;
	else
		palette &= 0x00ff;

	//const int linewidth = 320 / 2;
	int sourcebase = tile | (palette << 16);

	uint32_t ctrl = tilemapregs[1];

	if (ctrl & 0x80) // HiColor mode (rad_digi)
	{
		for (int i = 0; i < 320; i++)
		{
			const uint16_t data = spc.read_word(sourcebase + i);

			if (!(data & 0x8000))
			{
				m_linebuf[i] = data & 0x7fff;
			}
		}
	}
	else
	{
		const uint32_t attr = tilemapregs[0];
		const uint8_t bpp = attr & 0x0003;
		const uint32_t nc_bpp = ((bpp)+1) << 1;
		uint32_t palette_offset = (attr & 0x0f00) >> 4;
		palette_offset >>= nc_bpp;
		palette_offset <<= nc_bpp;

		uint32_t bits = 0;
		uint32_t nbits = 0;

		for (int i = 0; i < 320; i++)
		{
			bits <<= nc_bpp;
			if (nbits < nc_bpp)
			{
				uint16_t b = spc.read_word(sourcebase++ & 0x3fffff);
				b = (b << 8) | (b >> 8);
				bits |= b << (nc_bpp - nbits);
				nbits += 16;
			}
			nbits -= nc_bpp;

			uint32_t pal = palette_offset + (bits >> 16);
			bits &= 0xffff;

			uint16_t rgb = paletteram[pal];

			if (!(rgb & 0x8000))
			{
				m_linebuf[i] = rgb;
			}
		}
	}
}

// this builds up a line table for the vcmp effect, this is not correct when step is used
void spg_renderer_device::update_vcmp_table()
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

void spg_renderer_device::draw_tilestrip(uint32_t screenwidth, uint32_t drawwidthmask, bool blend, bool flip_x, const rectangle &cliprect, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint32_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t *paletteram, uint8_t blendlevel)
{
	if (blend)
	{
		if (flip_x)
		{
			draw_tilestrip<true, true>(screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		else
		{
			draw_tilestrip<true, false>(screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
	else
	{
		if (flip_x)
		{
			draw_tilestrip<false, true>(screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		else
		{
			draw_tilestrip<false, false>(screenwidth, drawwidthmask, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
}

void spg_renderer_device::draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint16_t tilegfxdata_addr, uint16_t *scrollregs, uint16_t *tilemapregs, address_space &spc, uint16_t *paletteram, uint16_t *scrollram, uint32_t which)
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
	uint32_t tilegfxdata_addr_full = tilegfxdata_addr * 0x40;

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

	const uint32_t total_width = 512;
	uint32_t y_mask = 0x100;
	const uint32_t screenwidth = 320;

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
	const uint8_t blendlevel = s_blend_levels[m_video_regs_2a & 3];

	const uint32_t words_per_tile = bits_per_row * tile_h;

	int realxscroll = xscroll;

	// Tennis in My Wireless Sports confirms the need to add the scroll value here rather than rowscroll being screen-aligned
	// this is different to GPL?
	if (row_scroll)
		realxscroll += (int16_t)scrollram[(logical_scanline + yscroll) & 0xff];

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

		if (!tile)
			continue;

		uint32_t tileattr = attr;
		uint32_t tilectrl = ctrl;

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

		blend = (tilectrl & 0x0100) ? true : false;
		flip_x = (tileattr & 0x0004) ? true : false;
		flip_y = (tileattr & 0x0008) ? true : false;

		palette_offset = (tileattr & 0x0f00) >> 4;
		// got tile info

		palette_offset >>= nc_bpp;
		palette_offset <<= nc_bpp;

		const int drawx = (x0 * tile_w) - (realxscroll & (tile_w - 1));
		draw_tilestrip(screenwidth, drawwidthmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr_full, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
	}
}

void spg_renderer_device::draw_sprite(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, uint32_t base_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram)
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

	const uint32_t screenwidth = 320;
	const uint32_t screenheight  = 256;
	const uint32_t xmask = 0x1ff;
	const uint32_t ymask = 0x1ff;

	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << ((attr & 0x0030) >> 4);

	// TODO: not all SPG2xx models support this, check which ones do
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

	uint32_t palette_offset = (attr & 0x0f00) >> 4;

	// the Circuit Racing game in PDC100 needs this or some graphics have bad colours at the edges when turning as it leaves stray lower bits set
	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;

	if (firstline < lastline)
	{
		int scanx = scanline - firstline;

		if ((scanx >= 0) && (scanline <= lastline))
		{
			draw_tilestrip(screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
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
			draw_tilestrip(screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
		// clipped against the bottom
		tempfirstline = firstline;
		templastline = lastline + (screenheight<<1);
		scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			draw_tilestrip(screenwidth, xmask, blend, flip_x, cliprect, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram, blendlevel);
		}
	}
}

void spg_renderer_device::draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t spritegfxdata_addr, address_space &spc, uint16_t *paletteram, uint16_t *spriteram, int sprlimit)
{
	if (!(m_video_regs_42 & 0x0001))
		return;

	for (uint32_t n = 0; n < sprlimit; n++)
	{
		draw_sprite(cliprect, scanline, priority, spritegfxdata_addr, 4 * n, spc, paletteram, spriteram);
	}
}

void spg_renderer_device::new_line(const rectangle &cliprect)
{
	update_palette_lookup();

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		m_linebuf[x] = 0x8000; // transparent
	}
}

void spg_renderer_device::update_palette_lookup()
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

void spg_renderer_device::apply_saturation_and_fade(bitmap_rgb32 &bitmap, const rectangle &cliprect, int scanline)
{
	uint32_t *src = &bitmap.pix(scanline, cliprect.min_x);

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		uint16_t px = (m_linebuf[x] & 0x8000) ? 0x0 : m_linebuf[x];
		*src = m_rgb555_to_rgb888_current[px];
		src++;
	}
}
