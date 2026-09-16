// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Aaron Giles,Jonathan Gevaryahu,hap,Robbbert
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

  8080bw_v.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "8080bw.h"


MACHINE_START_MEMBER(_8080bw_state,extra_8080bw_vh)
{
	save_item(NAME(m_color_map));
	save_item(NAME(m_screen_red));

	// These two only belong to schaser, but for simplicity's sake let's waste
	// two bytes in other drivers' .sta files.
	save_item(NAME(m_schaser_background_disable));
	save_item(NAME(m_schaser_background_select));
}


void rollingc_state::rollingc_palette(palette_device &palette) const
{
	// palette is 3bpp + intensity
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, pal1bit(i >> 2) >> 1, pal1bit(i >> 1) >> 1, pal1bit(i >> 0) >> 1);
		palette.set_pen_color(i | 8, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}

	// but according to photos, pen 6 is clearly orange instead of dark-yellow, and pen 5 is less dark as well
	// pens 1, 2 and 4 are good though. Maybe we're missing a color prom?
	palette.set_pen_color(0x05, 0xff, 0x00, 0x80); // pink
	palette.set_pen_color(0x06, 0xff, 0x80, 0x00); // orange
}

void cosmo_state::palette_init(palette_device &palette) const
{
	// character palette is 1bpp
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}

	uint8_t starmap[4];

	// Stars need 64 entries (2 bits per colour)
	starmap[0] = 0;
	starmap[1] = 80;
	starmap[2] = 80;
	starmap[3] = 255;

	uint8_t bit0, bit1, r, g, b;

	for (int i = 0; i < 64; i++)
	{
		// bit 5 = red @ 10k Ohm, bit 4 = red @ 10k Ohm
		bit0 = BIT(i, 1);
		bit1 = BIT(i, 4);
		r = starmap[(bit1 << 1) | bit0];

		// bit 3 = green @ 10k Ohm, bit 2 = green @ 10k Ohm
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 5);
		g = starmap[(bit1 << 1) | bit0];

		// bit 1 = blue @ 10k Ohm, bit 0 = blue @ 10k Ohm
		bit0 = BIT(i, 2);
		bit1 = BIT(i, 3);
		b = starmap[(bit1 << 1) | bit0];

		// set the RGB color
		palette.set_pen_color(i + 8, r, g, b);
	}
}

void _8080bw_state::sflush_palette(palette_device &palette) const
{
	// standard 3-bit rbg palette, but background color is bright blue
	palette.set_pen_color(0, 0x80, 0x80, 0xff);
	for (int i = 1; i < 8; i++)
		palette.set_pen_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1)));
}


inline void invaders_clone_palette_state::set_pixel(bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, int color)
{
	if (y >= MW8080BW_VCOUNTER_START_NO_VBLANK)
	{
		if (m_flip_screen)
			bitmap.pix(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = m_palette->pen_color(color);
		else
			bitmap.pix(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = m_palette->pen_color(color);
	}
}


inline void invaders_clone_palette_state::set_8_pixels(bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, uint8_t data, int fore_color, int back_color)
{
	for (int i = 0; i < 8; i++)
	{
		set_pixel(bitmap, y, x, BIT(data, 0) ? fore_color : back_color);

		x += 1;
		data >>= 1;
	}
}


// this is needed as this driver doesn't emulate the shift register like mw8080bw does
void invaders_clone_palette_state::clear_extra_columns(bitmap_rgb32 &bitmap, int color)
{
	for (uint8_t x = 0; x < 4; x++)
	{
		for (uint8_t y = MW8080BW_VCOUNTER_START_NO_VBLANK; y != 0; y++)
		{
			if (m_flip_screen)
				bitmap.pix(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + x)) =  m_palette->pen_color(color);
			else
				bitmap.pix(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + x) = m_palette->pen_color(color);
		}
	}
}


uint32_t _8080bw_state::screen_update_invadpt2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const prom = memregion("proms")->base();
	uint8_t const *const color_map_base = m_color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t const y = offs >> 5;
		uint8_t const x = offs << 3;

		offs_t const color_address = (offs >> 8 << 5) | (offs & 0x1f);

		uint8_t const data = m_main_ram[offs];
		uint8_t const fore_color = m_screen_red ? 1 : color_map_base[color_address] & 0x07;

		set_8_pixels(bitmap, y, x, data, fore_color, 0);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}


uint32_t _8080bw_state::screen_update_ballbomb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const prom = memregion("proms")->base();
	uint8_t const *const color_map_base = m_color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t const y = offs >> 5;
		uint8_t const x = offs << 3;

		offs_t const color_address = (offs >> 8 << 5) | (offs & 0x1f);

		uint8_t const data = m_main_ram[offs];
		uint8_t const fore_color = m_screen_red ? 1 : color_map_base[color_address] & 0x07;

		// blue background
		set_8_pixels(bitmap, y, x, data, fore_color, 2);
	}

	clear_extra_columns(bitmap, 2);

	return 0;
}


uint32_t _8080bw_state::screen_update_schaser(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *background_map_base = memregion("proms")->base();

	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t back_color = 0;

		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		if (!m_schaser_background_disable)
		{
			offs_t back_address = (offs >> 8 << 5) | (offs & 0x1f);

			uint8_t back_data = background_map_base[back_address];

			/* the equations derived from the schematics don't appear to produce
			   the right colors, but this one does, at least for this PROM */
			back_color = (((back_data & 0x0c) == 0x0c) && m_schaser_background_select) ? 4 : 2;
		}

		set_8_pixels(bitmap, y, x, data, fore_color, back_color);
	}

	clear_extra_columns(bitmap, m_schaser_background_disable ? 0 : 2);

	return 0;
}


uint32_t _8080bw_state::screen_update_schasercv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		// blue background
		set_8_pixels(bitmap, y, x, data, fore_color, 2);
	}

	clear_extra_columns(bitmap, 2);

	return 0;
}


uint32_t rollingc_state::screen_update_rollingc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f00) >> 3)] & 0x0f;
		uint8_t back_color = m_scattered_colorram2[(offs & 0x1f) | ((offs & 0x1f00) >> 3)] & 0x0f;

		set_8_pixels(bitmap, y, x, data, fore_color, back_color);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}


uint32_t _8080bw_state::screen_update_polaris(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *color_map_base = memregion("proms")->base();
	uint8_t *cloud_gfx = memregion("user1")->base();

	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		/* for the background color, bit 0 of the map PROM is connected to green gun.
		   red is 0 and blue is 1, giving cyan and blue for the background.  This
		   is different from what the schematics shows, but it's supported
		   by screenshots.  Bit 3 is connected to cloud enable, while
		   bits 1 and 2 are marked 'not use' (sic) */

		uint8_t back_color = (color_map_base[color_address] & 0x01) ? 6 : 2;
		uint8_t fore_color = ~m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		uint8_t cloud_y = y - m_polaris_cloud_pos;

		if ((color_map_base[color_address] & 0x08) || (cloud_y >= 64))
		{
			set_8_pixels(bitmap, y, x, data, fore_color, back_color);
		}
		else
		{
			/* cloud appears in this part of the screen */
			int i;

			for (i = 0; i < 8; i++)
			{
				uint8_t color;

				if (data & 0x01)
				{
					color = fore_color;
				}
				else
				{
					int bit = 1 << (~x & 0x03);
					offs_t cloud_gfx_offs = ((x >> 2) & 0x03) | ((~cloud_y & 0x3f) << 2);

					color = (cloud_gfx[cloud_gfx_offs] & bit) ? 7 : back_color;
				}

				set_pixel(bitmap, y, x, color);

				x = x + 1;
				data = data >> 1;
			}
		}
	}

	clear_extra_columns(bitmap, 6);

	return 0;
}


uint32_t _8080bw_state::screen_update_lupin3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = ~m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		set_8_pixels(bitmap, y, x, data, fore_color, 0);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}


// To draw everything 4 times the horizontal size

void cosmo_state::set_pixel(bitmap_rgb32 &bitmap, uint8_t y, uint32_t x, int color)
{
	if (y >= MW8080BW_VCOUNTER_START_NO_VBLANK)
	{
		if (m_flip_screen)
			bitmap.pix(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), (MW8080BW_HPIXCOUNT * 4) - 1 - x) = m_palette->pen_color(color);
		else
			bitmap.pix(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = m_palette->pen_color(color);
	}
}

void cosmo_state::set_8_pixels(bitmap_rgb32 &bitmap, uint8_t y, uint32_t x, uint8_t data, int fore_color, int back_color)
{
	for (int i = 0; i < 8; i++)
	{
		set_pixel(bitmap, y, x, (data & 0x01) ? fore_color : back_color);
		set_pixel(bitmap, y, x + 1, (data & 0x01) ? fore_color : back_color);
		set_pixel(bitmap, y, x + 2, (data & 0x01) ? fore_color : back_color);
		set_pixel(bitmap, y, x + 3, (data & 0x01) ? fore_color : back_color);

		x = x + 4;
		data = data >> 1;
	}
}

uint32_t cosmo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_colorram[color_address] & 0x07;

		set_8_pixels(bitmap, y, (x * 4), data, fore_color, 0);
	}

	// Stars
	stars_update_origin();

	// render stars


	// iterate over scanlines
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t star_offs = m_star_rng_origin + y * 1024;
		star_offs %= STAR_RNG_PERIOD;
		stars_draw_row(bitmap, 1024, y, star_offs, 0xff);
	}

	// clear_extra_columns(bitmap, 0);

	return 0;
}

void cosmo_state::stars_init()
{
	m_stars_sidescroll = 0;
	m_star_speed = 3;
	m_bright_star = 0;
	m_rng_offs = 0;

	uint32_t shiftreg = 0;

	// precalculate the RNG
	for (int i = 0; i < STAR_RNG_PERIOD; i++)
	{
		// stars are enabled if the upper 7 bits are 1 (ignoring highest bit)
		int const enabled = ((shiftreg & 0x0fe00) == 0x0fe00); // was 1fe01 * 1fe00

		// color comes from the next 6 bits
		int const color = (~shiftreg & 0x1f8) >> 3;

		// store the color value in the low 6 bits and the enable flag in the upper bit
		m_stars[i] = color | (enabled << 7);

		// the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0
		shiftreg = (shiftreg >> 1) | ((((shiftreg >> 12) ^ ~shiftreg) & 1) << 16);
	}
}

void cosmo_state::stars_update_origin()
{
	int const curframe = m_screen->frame_number();

	// only update on a different frame
	if (curframe != m_star_rng_origin_frame)
	{

		int per_frame_delta = 2 - m_star_speed; // (speed horizontal)

		if (m_stars_sidescroll) per_frame_delta += 4096;

		int total_delta = per_frame_delta * (curframe - m_star_rng_origin_frame);

		// we can't just use % here because mod of a negative number is undefined
		while (total_delta < 0)
			total_delta += STAR_RNG_PERIOD;

		// now that everything is positive, do the mod
		m_star_rng_origin = (m_star_rng_origin + total_delta) % STAR_RNG_PERIOD;
		m_star_rng_origin_frame = curframe;
	}
}

void cosmo_state::stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs, uint8_t starmask)
{
	// ensure our star offset is valid
	star_offs %= STAR_RNG_PERIOD;

	// iterate over the specified number of 6MHz pixels
	for (int x = 0; x < maxx; x++)
	{
		// Brightness, toggled every star but forced low on every second block of 4 pixels group
		if (y & 0x01)
		{
			if ((x % 32) > 15) m_bright_star = 0;
		}
		else
		{
			if ((x % 32) < 16) m_bright_star = 0;
		}

		uint8_t const star = m_stars[star_offs++];
		star_offs %= STAR_RNG_PERIOD;
		if ((star & 0x80) != 0 && (star & starmask) != 0)
		{
			if (bitmap.pix(y, x) == m_palette->pen_color(0))
			{
				if (m_bright_star)
					bitmap.pix(y, x) = m_palette->pen_color((star & 0x3f) + 8);
				else
					bitmap.pix(y, x) = m_palette->pen_color((star & 0x0f) + 8);

			}
		}
		else
		{
			m_bright_star ^= 1;  // only toggles if no star?
		}
	}
}

uint32_t _8080bw_state::screen_update_indianbt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *prom = memregion("proms")->base();
	uint8_t *color_map_base = m_color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = color_map_base[color_address] & 0x07;

		set_8_pixels(bitmap, y, x, data, fore_color, 0);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}


uint32_t _8080bw_state::screen_update_sflush(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		set_8_pixels(bitmap, y, x, data, fore_color, 0);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}


uint32_t shuttlei_state::screen_update_shuttlei(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t const y = offs >> 5;
		uint8_t const x = offs << 3;

		uint8_t data = m_main_ram[offs];

		for (int i = 0; i < 8; i++)
		{
			if (m_flip_screen)
				bitmap.pix(191-y, 255-(x|i)) = m_palette->pen_color(BIT(data, 7));
			else
				bitmap.pix(y, x|i) = m_palette->pen_color(BIT(data, 7));
			data <<= 1;
		}
	}

	return 0;
}


uint32_t spacecom_state::screen_update_spacecom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x1c00; offs++)
	{
		uint8_t const y = offs >> 5;
		uint8_t const x = offs << 3;
		uint8_t const flipx = m_flip_screen ? 7 : 0;

		uint8_t data = m_main_ram[offs+0x400];

		for (int i = 0; i < 8; i++)
		{
			bitmap.pix(y, x | (i ^ flipx)) = m_palette->pen_color(BIT(data, 0));
			data >>= 1;
		}
	}

	return 0;
}

uint32_t vortex_state::screen_update_vortex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t video_data = 0;
	rgb_t col = rgb_t::black();

	for (int y = MW8080BW_VCOUNTER_START_NO_VBLANK; y < 0x100; y++)
	{
		for (int x = 0; x < 0x100; x++)
		{
			// the video RAM is read at every 8 pixels starting with pixel 4
			if ((x & 0x07) == 0x04)
			{
				offs_t const offs = (offs_t(y) << 5) | (x >> 3);
				video_data = m_main_ram[offs];
				uint8_t const pix = BIT(m_main_ram[(offs + 1) & 0x1fff], 0);
				col = rgb_t(pix ? 0 : 255, pix ? 255 : 0, BIT(x, 5) ? 255 : 0);
			}

			// plot the current pixel
			pen_t const pen = BIT(video_data, 0) ? col : rgb_t::black();

			if (m_flip_screen)
				bitmap.pix(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = pen;
			else
				bitmap.pix(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

			// next pixel
			video_data >>= 1;
		}

		// end of line, flush out the shift register
		for (int i = 0; i < 4; i++)
		{
			pen_t const pen = BIT(video_data, 0) ? col : rgb_t::black();

			if (m_flip_screen)
				bitmap.pix(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + i)) = pen;
			else
				bitmap.pix(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

			video_data >>= 1;
		}

		// at next row, video_data is now 0, so the next line will start with 4 blank pixels
	}

	return 0;
}

/*******************************************************/
/*                                                     */
/* Model Racing "Orbite"                               */
/*                                                     */
/*******************************************************/
uint32_t orbite_state::screen_update_orbite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_main_ram.bytes(); offs++)
	{
		uint8_t back_color = 0;

		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data = m_main_ram[offs];
		uint8_t fore_color = m_scattered_colorram[(offs & 0x1f) | ((offs & 0x1f80) >> 2)] & 0x07;

		set_8_pixels(bitmap, y, x, data, fore_color, back_color);
	}

	clear_extra_columns(bitmap, 0);

	return 0;
}
