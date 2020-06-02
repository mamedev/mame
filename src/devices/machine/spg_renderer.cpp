// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

#include "emu.h"
#include "spg_renderer.h"

DEFINE_DEVICE_TYPE(SPG_RENDERER, spg_renderer_device, "spg_renderer", "SunPlus / GeneralPlus video rendering")

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
}

void spg_renderer_device::device_reset()
{
	m_video_regs_2a = 0x0000;
	m_video_regs_22 = 0x0000;
	m_video_regs_42 = 0x0000;
}


// Perform a lerp between a and b
inline uint8_t spg_renderer_device::mix_channel(uint8_t bottom, uint8_t top)
{
	uint8_t alpha = (m_video_regs_2a & 3) << 6;
	return ((256 - alpha) * bottom + alpha * top) >> 8;
}

template<spg_renderer_device::blend_enable_t Blend, spg_renderer_device::flipx_t FlipX>
void spg_renderer_device::draw_tilestrip(const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t* palette)
{
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
			uint16_t b = spc.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
		}
		nbits -= nc_bpp;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;

		if (realdrawpos >= 0 && realdrawpos < 320)
		{
			uint16_t rgb = palette[pal];

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




void spg_renderer_device::draw_sprite(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t base_addr, address_space &spc, uint16_t* palette, uint16_t* spriteram)
{
	uint32_t tilegfxdata_addr = 0x40 * m_video_regs_22;
	uint16_t tile = spriteram[base_addr + 0];
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

	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << ((attr & 0x0030) >> 4);

	if (!(m_video_regs_42 & 0x0002))
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
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
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
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
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
					draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
			}
			else
			{
				if (flip_x)
				{
					draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
				else
				{
					draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, palette);
				}
			}
		}
	}
}

void spg_renderer_device::draw_sprites(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority, address_space &spc, uint16_t* palette, uint16_t* spriteram, int sprlimit)
{
	if (!(m_video_regs_42 & 0x0001))
	{
		return;
	}

	for (uint32_t n = 0; n < sprlimit; n++)
	{
		draw_sprite(cliprect, dst, scanline, priority, 4 * n, spc, palette, spriteram);
	}
}
