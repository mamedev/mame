// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (Video)

**********************************************************************/

#include "emu.h"
#include "spg2xx_video.h"

DEFINE_DEVICE_TYPE(SPG24X_VIDEO, spg24x_video_device, "spg24x_video", "SPG240-series System-on-a-Chip (Video)")

#define LOG_IRQS            (1U << 4)
#define LOG_VLINES          (1U << 5)
#define LOG_DMA             (1U << 9)
#define LOG_PPU_READS       (1U << 22)
#define LOG_PPU_WRITES      (1U << 23)
#define LOG_UNKNOWN_PPU     (1U << 24)
#define LOG_PPU             (LOG_PPU_READS | LOG_PPU_WRITES | LOG_UNKNOWN_PPU)
#define LOG_ALL             (LOG_IRQS | LOG_PPU | LOG_VLINES | LOG_DMA )

#define VERBOSE             (0)
#include "logmacro.h"

#define VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VIDEO_IRQ_STATUS    m_video_regs[0x63]

spg2xx_video_device::spg2xx_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_guny_in(*this),
	m_gunx_in(*this),
	m_sprlimit_read_cb(*this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_scrollram(*this, "scrollram"),
	m_hcompram(*this, "hcompram"),
	m_paletteram(*this, "paletteram"),
	m_spriteram(*this, "spriteram"),
	m_video_irq_cb(*this)
{
}

spg24x_video_device::spg24x_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg2xx_video_device(mconfig, SPG24X_VIDEO, tag, owner, clock)
{
}

void spg2xx_video_device::device_start()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);
	}

	m_guny_in.resolve_safe(0);
	m_gunx_in.resolve_safe(0);

	m_screenpos_timer = timer_alloc(TIMER_SCREENPOS);
	m_screenpos_timer->adjust(attotime::never);

	save_item(NAME(m_video_regs));

	m_sprlimit_read_cb.resolve_safe(0);

	m_video_irq_cb.resolve();
}

void spg2xx_video_device::device_reset()
{
	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;
	m_video_regs[0x3c] = 0x0020;
	m_video_regs[0x42] = 0x0001;

	for (int i = 0; i < 480; i++)
	{
		m_ycmp_table[i] = -1;
	}
}

/*************************
*     Video Hardware     *
*************************/


void spg2xx_video_device::draw_linemap(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t* regs)
{
	if ((scanline < 0) || (scanline >= 240))
		return;

	address_space &space = m_cpu->space(AS_PROGRAM);

	uint32_t tilemap = regs[4];
	uint32_t palette_map = regs[5];

	//printf("draw bitmap bases %04x %04x\n", tilemap, palette_map);

	//uint32_t xscroll = regs[0];
	uint32_t yscroll = regs[1];

	int realline = (scanline + yscroll) & 0xff;


	uint16_t tile = space.read_word(tilemap + realline);
	uint16_t palette = 0;

	//if (!tile)
	//  continue;

	palette = space.read_word(palette_map + realline / 2);
	if (scanline & 1)
		palette >>= 8;
	else
		palette &= 0x00ff;

	//const int linewidth = 320 / 2;
	int sourcebase = tile | (palette << 16);

	uint32_t ctrl = regs[3];

	if (ctrl & 0x80) // HiColor mode (rad_digi)
	{
		for (int i = 0; i < 320; i++)
		{
			const uint16_t data = space.read_word(sourcebase + i);

			if (!(data & 0x8000))
			{
				dst[i] = m_rgb555_to_rgb888[data & 0x7fff];
			}
		}
	}
	else
	{
		for (int i = 0; i < 320 / 2; i++)
		{
			uint8_t palette_entry;
			uint16_t color;
			const uint16_t data = space.read_word(sourcebase + i);

			palette_entry = (data & 0x00ff);
			color = m_paletteram[palette_entry];

			if (!(color & 0x8000))
			{
				dst[(i * 2) + 0] = m_rgb555_to_rgb888[color & 0x7fff];
			}

			palette_entry = (data & 0xff00) >> 8;
			color = m_paletteram[palette_entry];

			if (!(color & 0x8000))
			{
				dst[(i * 2) + 1] = m_rgb555_to_rgb888[color & 0x7fff];
			}
		}
	}
}


bool spg2xx_video_device::get_tile_info(uint32_t tilemap_rambase, uint32_t palettemap_rambase, uint32_t x0, uint32_t y0, uint32_t tile_count_x, uint32_t ctrl, uint32_t attr, uint16_t& tile, bool& blend, bool& flip_x, bool& flip_y, uint32_t& palette_offset)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	uint32_t tile_address = x0 + (tile_count_x * y0);

	tile = (ctrl & 0x0004) ? space.read_word(tilemap_rambase) : space.read_word(tilemap_rambase + tile_address);

	if (!tile)
		return false;

	uint32_t tileattr = attr;
	uint32_t tilectrl = ctrl;
	if ((ctrl & 2) == 0)
	{   // -(1) bld(1) flip(2) pal(4)

		uint16_t palette = (ctrl & 0x0004) ? space.read_word(palettemap_rambase) : space.read_word(palettemap_rambase + tile_address / 2);
		if (x0 & 1)
			palette >>= 8;
		else
			palette &= 0x00ff;


		tileattr &= ~0x000c;
		tileattr |= (palette >> 2) & 0x000c;    // flip

		tileattr &= ~0x0f00;
		tileattr |= (palette << 8) & 0x0f00;    // palette

		tilectrl &= ~0x0100;
		tilectrl |= (palette << 2) & 0x0100;    // blend
	}

	blend = (tileattr & 0x4000 || tilectrl & 0x0100);
	flip_x = (tileattr & 0x0004);
	flip_y= (tileattr & 0x0008);

	palette_offset = (tileattr & 0x0f00) >> 4;


	return true;
}


void spg2xx_video_device::update_vcmp_table()
{
	for (int i = 0; i < 480; i++)
	{
		int currentline = 0;

		if (i < m_video_regs[0x1d])
		{
			m_ycmp_table[i] = -1;
		}
		else
		{
			if (currentline < 240)
			{
				m_ycmp_table[i] = currentline;
				currentline += m_video_regs[0x1c];
			}
			else
			{
				m_ycmp_table[i] = -1;
			}
		}
	}
}

// Perform a lerp between a and b
inline uint8_t spg2xx_video_device::mix_channel(uint8_t bottom, uint8_t top)
{
	uint8_t alpha = (m_video_regs[0x2a] & 3) << 6;
	return ((256 - alpha) * bottom + alpha * top) >> 8;
}

template<spg2xx_video_device::blend_enable_t Blend, spg2xx_video_device::flipx_t FlipX>
void spg2xx_video_device::draw_tilestrip(const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	const uint32_t yflipmask = flip_y ? tile_h - 1 : 0;
	uint32_t m = tilegfxdata_addr + words_per_tile * tile + bits_per_row * (tile_scanline ^ yflipmask);
	uint32_t bits = 0;
	uint32_t nbits = 0;

	for (int32_t x = FlipX ? (tile_w - 1) : 0; FlipX ? x >= 0 : x < tile_w; FlipX ? x-- : x++)
	{
		int realdrawpos = (drawx + x) & 0x1ff;

		bits <<= nc_bpp;

		if (nbits < nc_bpp)
		{
			uint16_t b = space.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
		}
		nbits -= nc_bpp;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;

		if (realdrawpos >= 0 && realdrawpos < 320)
		{
			uint16_t rgb = m_paletteram[pal];

			if (!(rgb & 0x8000))
			{
				if (Blend)
				{
					dst[realdrawpos] = (mix_channel((uint8_t)(dst[realdrawpos] >> 16), m_rgb5_to_rgb8[(rgb >> 10) & 0x1f]) << 16) |
						(mix_channel((uint8_t)(dst[realdrawpos] >> 8), m_rgb5_to_rgb8[(rgb >> 5) & 0x1f]) << 8) |
						(mix_channel((uint8_t)(dst[realdrawpos] >> 0), m_rgb5_to_rgb8[rgb & 0x1f]));
				}
				else
				{
					dst[realdrawpos] = m_rgb555_to_rgb888[rgb];
				}
			}
		}
	}
}

void spg2xx_video_device::draw_page(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t *regs)
{
	const uint32_t attr = regs[2];
	const uint32_t ctrl = regs[3];

	if (!(ctrl & 0x0008))
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	if (ctrl & 0x0001) // Bitmap / Linemap mode! (basically screen width tile mode)
	{
		draw_linemap(cliprect, dst, scanline, priority, tilegfxdata_addr, regs);
		return;
	}

	const uint32_t xscroll = regs[0];
	const uint32_t yscroll = regs[1];
	const uint32_t tilemap_rambase = regs[4];
	const uint32_t palettemap_rambase = regs[5];
	const int tile_width = (attr & 0x0030) >> 4;
	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << (tile_width);
	const uint32_t tile_count_x = 512 / tile_w; // all tilemaps are 512 pixels wide
	const uint32_t bitmap_y = (scanline + yscroll) & 0xff; // all tilemaps are 256 pixels high
	const uint32_t y0 = bitmap_y / tile_h;
	const uint32_t tile_scanline = bitmap_y % tile_h;
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;
	const uint32_t words_per_tile = bits_per_row * tile_h;
	const bool row_scroll = (ctrl & 0x0010);

	int realxscroll = xscroll;
	if (row_scroll)
	{
		// Tennis in My Wireless Sports confirms the need to add the scroll value here rather than rowscroll being screen-aligned
		realxscroll += (int16_t)m_scrollram[(scanline+yscroll) & 0xff];
	}

	for (uint32_t x0 = 0; x0 < (320+tile_w)/tile_w; x0++)
	{

		bool blend, flip_x, flip_y;
		uint16_t tile;
		uint32_t palette_offset;

		if (!get_tile_info(tilemap_rambase, palettemap_rambase, (x0 + (realxscroll >> (tile_width+3))) & (tile_count_x-1) , y0, tile_count_x, ctrl, attr, tile, blend, flip_x, flip_y, palette_offset))
			continue;

		palette_offset >>= nc_bpp;
		palette_offset <<= nc_bpp;

		int drawx = (x0 * tile_w);
		drawx = drawx - (realxscroll & (tile_w-1));

		if (blend)
		{
			if (flip_x)
			{
				draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
			}
			else
			{
				draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
			}
		}
		else
		{
			if (flip_x)
			{
				draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
			}
			else
			{
				draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
			}
		}
	}
}

void spg2xx_video_device::draw_sprite(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t base_addr)
{
	uint32_t tilegfxdata_addr = 0x40 * m_video_regs[0x22];
	uint16_t tile = m_spriteram[base_addr + 0];
	int16_t x = m_spriteram[base_addr + 1];
	int16_t y = m_spriteram[base_addr + 2];
	uint16_t attr = m_spriteram[base_addr + 3];

	if (!tile)
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << ((attr & 0x0030) >> 4);

	if (!(m_video_regs[0x42] & 0x0002))
	{
		x = (160 + x) - tile_w / 2;
		y = (120 - y) - (tile_h / 2) + 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

	int firstline = y;
	int lastline = y + (tile_h - 1);
	lastline &= 0x1ff;

	bool blend = (attr & 0x4000);
	bool flip_x = (attr & 0x0004);
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;
	const uint32_t words_per_tile = bits_per_row * tile_h;

	bool flip_y = (attr & 0x0008);
	uint32_t palette_offset = (attr & 0x0f00) >> 4;
	
	// the Circuit Racing game in PDC100 needs this or some graphics have bad colours at the edges when turning as it leaves stray lower bits set
	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;


	if (firstline < lastline)
	{
		int scanx = scanline - firstline;

		if ((scanx >= 0) && (scanline <= lastline))
		{
			if (blend)
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
		}
	}
	else
	{
		// clipped from top
		int tempfirstline = firstline - 0x200;
		int templastline = lastline;
		int scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			if (blend)
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
		}
		// clipped against the bottom
		tempfirstline = firstline;
		templastline = lastline + 0x200;
		scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			if (blend)
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile);
				}
			}
		}
	}
}

void spg2xx_video_device::draw_sprites(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority)
{
	if (!(m_video_regs[0x42] & 0x0001))
	{
		return;
	}

	for (uint32_t n = 0; n < m_sprlimit_read_cb(); n++)
	{
		draw_sprite(cliprect, dst, scanline, priority, 4 * n);
	}
}

void spg2xx_video_device::apply_saturation_and_fade(bitmap_rgb32& bitmap, const rectangle& cliprect, int scanline)
{
	static const float s_u8_to_float = 1.0f / 255.0f;
	static const float s_gray_r = 0.299f;
	static const float s_gray_g = 0.587f;
	static const float s_gray_b = 0.114f;
	const float sat_adjust = (0xff - (m_video_regs[0x3c] & 0x00ff)) / (float)(0xff - 0x20);

	const uint16_t fade_offset = m_video_regs[0x30];

	uint32_t* src = &bitmap.pix32(scanline, cliprect.min_x);

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		if ((m_video_regs[0x3c] & 0x00ff) != 0x0020) // apply saturation
		{
			const uint32_t src_rgb = *src;
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
			*src = (integer_r > 255 ? 0xff0000 : (integer_r < 0 ? 0 : ((uint8_t)integer_r << 16))) |
				(integer_g > 255 ? 0x00ff00 : (integer_g < 0 ? 0 : ((uint8_t)integer_g << 8))) |
				(integer_b > 255 ? 0x0000ff : (integer_b < 0 ? 0 : (uint8_t)integer_b));

		}

		if (fade_offset != 0) // apply fade
		{
			const uint32_t src_rgb = *src;
			const uint8_t src_r = (src_rgb >> 16) & 0xff;
			const uint8_t src_g = (src_rgb >> 8) & 0xff;
			const uint8_t src_b = (src_rgb >> 0) & 0xff;
			const uint8_t r = src_r - fade_offset;
			const uint8_t g = src_g - fade_offset;
			const uint8_t b = src_b - fade_offset;
			*src = (r > src_r ? 0 : (r << 16)) |
				(g > src_g ? 0 : (g << 8)) |
				(b > src_b ? 0 : (b << 0));
		}

		src++;
	}


}


uint32_t spg2xx_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (0)
	{
		uint16_t attr1 = m_video_regs[0x12];
		uint16_t ctrl1 = m_video_regs[0x13];
		uint16_t attr2 = m_video_regs[0x18];
		uint16_t ctrl2 = m_video_regs[0x19];

		// attr   --zz pppp ssss FFbb
		// ctrl  ---- ---b hZzR ewrb

		popmessage("Pg1 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n"
			"Pg2 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n"
			"Pg1 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n"
			"Pg2 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			attr1, (attr1 & 0xc000) >> 14, (attr1 >> 12) & 3, (attr1 >> 8) & 15, 8 << ((attr1 >> 6) & 3), 8 << ((attr1 >> 4) & 3), BIT(attr1, 3), BIT(attr1, 2), 2 * ((attr1 & 3) + 1),
			attr2, (attr2 & 0xc000) >> 14, (attr2 >> 12) & 3, (attr2 >> 8) & 15, 8 << ((attr2 >> 6) & 3), 8 << ((attr2 >> 4) & 3), BIT(attr2, 3), BIT(attr2, 2), 2 * ((attr2 & 3) + 1),
			ctrl1, (ctrl1 & 0xfe00), BIT(ctrl1, 8), BIT(ctrl1, 7), BIT(ctrl1, 6), BIT(ctrl1, 5), BIT(ctrl1, 4), BIT(ctrl1, 3), BIT(ctrl1, 2), BIT(ctrl1, 1), BIT(ctrl1, 0),
			ctrl2, (ctrl2 & 0xfe00), BIT(ctrl2, 8), BIT(ctrl2, 7), BIT(ctrl2, 6), BIT(ctrl2, 5), BIT(ctrl2, 4), BIT(ctrl2, 3), BIT(ctrl2, 2), BIT(ctrl2, 1), BIT(ctrl2, 0));

	}


	const uint32_t page1_addr = 0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_video_regs[0x21];
	uint16_t *page1_regs = m_video_regs + 0x10;
	uint16_t *page2_regs = m_video_regs + 0x16;

	bitmap.fill(0, cliprect);

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		uint32_t* dst = &bitmap.pix32(scanline, cliprect.min_x);

		for (int i = 0; i < 4; i++)
		{
			draw_page(cliprect, dst, scanline, i, page1_addr, page1_regs);
			draw_page(cliprect, dst, scanline, i, page2_addr, page2_regs);
			draw_sprites(cliprect, dst, scanline, i);
		}

		apply_saturation_and_fade(bitmap, cliprect, scanline);
	}

	return 0;
}

void spg2xx_video_device::do_sprite_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = m_video_regs[0x70] & 0x3fff;
	uint32_t dst = m_video_regs[0x71];

	for (uint32_t j = 0; j < len; j++)
	{
		m_spriteram[(dst + j) & 0x3ff] = mem.read_word(src + j);
	}

	m_video_regs[0x72] = 0;
	if (VIDEO_IRQ_ENABLE & 4)
	{
		const uint16_t old = VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS |= 4;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
	}
}

READ16_MEMBER(spg2xx_video_device::video_r)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_r: Page 1 X Scroll\n");
		return m_video_regs[offset];

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_r: Page 1 Y Scroll\n");
		return m_video_regs[offset];


	case 0x38: // Current Line
		LOGMASKED(LOG_VLINES, "video_r: Current Line: %04x\n", m_screen->vpos());
		return m_screen->vpos();

	case 0x3e: // Light Pen Y Position
		LOGMASKED(LOG_PPU_READS, "video_r: Light Pen Y / Lightgun Y\n");
		return m_guny_in();

	case 0x3f: // Light Pen X Position
		LOGMASKED(LOG_PPU_READS, "video_r: Light Pen X / Lightgun X\n");
		return m_gunx_in();

	case 0x62: // Video IRQ Enable
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Enable: %04x\n", VIDEO_IRQ_ENABLE);
		return VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Status: %04x\n", VIDEO_IRQ_STATUS);
		return VIDEO_IRQ_STATUS;

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER(spg2xx_video_device::video_w)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Y Scroll = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x12: // Page 1 Attributes
	{
		uint16_t attr1 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg1 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",
			attr1, (attr1 & 0xc000) >> 14, (attr1 >> 12) & 3, (attr1 >> 8) & 15, 8 << ((attr1 >> 6) & 3), 8 << ((attr1 >> 4) & 3), BIT(attr1, 3), BIT(attr1, 2), 2 * ((attr1 & 3) + 1));
		m_video_regs[offset] = data;
		break;
	}

	case 0x13: // Page 1 Control
	{
		uint16_t ctrl1 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w Pg1 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			ctrl1, (ctrl1 & 0xfe00), BIT(ctrl1, 8), BIT(ctrl1, 7), BIT(ctrl1, 6), BIT(ctrl1, 5), BIT(ctrl1, 4), BIT(ctrl1, 3), BIT(ctrl1, 2), BIT(ctrl1, 1), BIT(ctrl1, 0));
		m_video_regs[offset] = data;
		break;
	}

	case 0x14: // Page 1 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x15: // Page 1 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x16: // Page 2 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x17: // Page 2 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x18: // Page 2 Attributes
	{
		uint16_t attr2 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg2 Attr = %04x (unused: %01x, Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",
			attr2, (attr2 & 0xc000) >> 14, (attr2 >> 12) & 3, (attr2 >> 8) & 15, 8 << ((attr2 >> 6) & 3), 8 << ((attr2 >> 4) & 3), BIT(attr2, 3), BIT(attr2, 2), 2 * ((attr2 & 3) + 1));
		m_video_regs[offset] = data;
		break;
	}

	case 0x19: // Page 2 Control
	{
		uint16_t ctrl2 = data;
		LOGMASKED(LOG_PPU_WRITES, "video_w: Pg2 Ctrl = %04x (unused: %04x, Blend:%d, HiColor:%d, Ycmp:%d, Hcmp:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",
			ctrl2, (ctrl2 & 0xfe00), BIT(ctrl2, 8), BIT(ctrl2, 7), BIT(ctrl2, 6), BIT(ctrl2, 5), BIT(ctrl2, 4), BIT(ctrl2, 3), BIT(ctrl2, 2), BIT(ctrl2, 1), BIT(ctrl2, 0));
		m_video_regs[offset] = data;
		break;
	}

	case 0x1a: // Page 2 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Tile Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x1b: // Page 2 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attribute Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x1c: // vertical compression, amount, 0x20 = no scale? (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Value = %04x\n", data);
		m_video_regs[offset] = data;
		update_vcmp_table();
		break;

	case 0x1d: // (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Y_Offset = %04x\n", data);
		m_video_regs[offset] = data;
		update_vcmp_table();
		break;

	case 0x1e: // (not on spg288?)
		LOGMASKED(LOG_PPU_WRITES, "video_w: Ycmp_Step = %04x\n", data);
		m_video_regs[offset] = data;
		update_vcmp_table();
		break;

	case 0x20: // Page 1 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x21: // Page 2 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x22: // Sprite Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x2a: // Blend Level Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Blend Level Control = %04x\n", data & 0x0003);
		m_video_regs[offset] = data & 0x0003;
		break;

	case 0x30: // Fade Effect Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Fade Effect Control = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x36: // IRQ pos V
	case 0x37: // IRQ pos H
		m_video_regs[offset] = data & 0x01ff;
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Position: %04x,%04x (%04x)\n", m_video_regs[0x37], m_video_regs[0x36], 0x2800 | offset);
		if (m_video_regs[0x37] < 160 && m_video_regs[0x36] < 240)
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
		else
			m_screenpos_timer->adjust(attotime::never);
		break;

	case 0x39: // Latch 1st Line Pen Pulse
		LOGMASKED(LOG_PPU_WRITES, "video_w: Latch 1st Line Pen Pulse = %04x\n", data & 0x0001);
		m_video_regs[offset] = data & 0x0001;
		break;

	case 0x3c: // TV Control 1
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 1 = %04x (Hue:%02x, Saturation:%02x)\n", data, data >> 8, data & 0x00ff);
		m_video_regs[offset] = data;
		break;

	case 0x3d: // TV Control 2
	{
		static const char* const s_lpf_mode[4] = { "LPF1", "LPF2", "All", "Edge" };
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 2 = %04x (LPFMode:%s, Enable:%d, Interlace:%d)\n", data & 0x000f
			, s_lpf_mode[(data >> 2) & 3], BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data & 0x000f;
		break;
	}

	case 0x3e: // Light Pen Y Position
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen Y / Lightgun Y (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x3f: // Light Pen X Position
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen X / Lightgun X (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x42: // Sprite Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Control = %04x (TopLeft:%d, Enable:%d)\n", data & 0x0003, BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data & 0x0003;
		break;

	case 0x62: // Video IRQ Enable
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Enable = %04x (DMA:%d, Timing:%d, Blanking:%d)\n", data, BIT(data, 2), BIT(data, 1), BIT(data, 0));
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_ENABLE = data & 0x0007;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x63: // Video IRQ Acknowledge
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Acknowledge = %04x\n", data);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS &= ~data;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x70: // Sprite DMA Source
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Source = %04x\n", data & 0x3fff);
		m_video_regs[offset] = data & 0x3fff;
		break;

	case 0x71: // Sprite DMA Dest
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Dest = %04x\n", data & 0x03ff);
		m_video_regs[offset] = data & 0x03ff;
		break;

	case 0x72: // Sprite DMA Length
	{
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Length = %04x\n", data & 0x03ff);
		uint16_t length = data & 0x3ff;
		do_sprite_dma(length ? length : 0x400);
		break;
	}

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_w: Unknown register %04x = %04x\n", 0x2800 + offset, data);
		m_video_regs[offset] = data;
		break;
	}
}

WRITE_LINE_MEMBER(spg2xx_video_device::vblank)
{
	if (!state)
	{
		VIDEO_IRQ_STATUS &= ~1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
		return;
	}

	if (VIDEO_IRQ_ENABLE & 1)
	{
		VIDEO_IRQ_STATUS |= 1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
	}
}

void spg2xx_video_device::check_video_irq()
{
	LOGMASKED(LOG_IRQS, "%ssserting IRQ0 (%04x, %04x)\n", (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? "A" : "Dea", VIDEO_IRQ_STATUS, VIDEO_IRQ_ENABLE);
	m_video_irq_cb((VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
}

void spg2xx_video_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SCREENPOS:
		{
			if (VIDEO_IRQ_ENABLE & 2)
			{
				VIDEO_IRQ_STATUS |= 2;
				check_video_irq();
			}
			m_screen->update_partial(m_screen->vpos());

			// fire again, jak_dbz pinball needs this
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
			break;
		}
	}
}
