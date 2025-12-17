// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation

    The VT video is based on the ppu2c0x but with enhanced capabilities such
    as 16 colour sprites.

******************************************************************************/

#include "emu.h"
#include "ppu2c0x_vt.h"

#include "screen.h"

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */

// devices
DEFINE_DEVICE_TYPE(PPU_VT03, ppu_vt03_device, "ppu_vt03", "VT03 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_VT03PAL, ppu_vt03pal_device, "ppu_vt03pal", "VT03 PPU (PAL)")

DEFINE_DEVICE_TYPE(PPU_VT32, ppu_vt32_device, "ppu_vt32", "VT32 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_VT32PAL, ppu_vt32pal_device, "ppu_vt32pal", "VT32 PPU (PAL)")

DEFINE_DEVICE_TYPE(PPU_VT3XX, ppu_vt3xx_device, "ppu_vt3xx", "VT3XX PPU (NTSC)")

ppu_vt03_device::ppu_vt03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock),
	m_is_pal(false),
	m_is_50hz(false),
	m_read_bg(*this, 0),
	m_read_sp(*this, 0),
	m_read_onespace(*this, 0),
	m_read_onespace_with_relative(*this, 0)
{
}

ppu_vt03_device::ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu_vt03_device(mconfig, PPU_VT03, tag, owner, clock)
{
}


ppu_vt03pal_device::ppu_vt03pal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu_vt03_device(mconfig, PPU_VT03PAL, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
	m_is_pal = true;
	m_is_50hz = true;
}

ppu_vt32_device::ppu_vt32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	ppu_vt03_device(mconfig, type, tag, owner, clock)
{
}

ppu_vt32_device::ppu_vt32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu_vt32_device(mconfig, PPU_VT32, tag, owner, clock)
{
}

ppu_vt32pal_device::ppu_vt32pal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu_vt32_device(mconfig, PPU_VT32PAL, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
	m_is_pal = true;
	m_is_50hz = true;
}

u8 ppu_vt03_device::palette_read(offs_t offset)
{
	if (offset < 0x20)
		return ppu2c0x_device::palette_read(offset);
	else
		return m_palette_ram[offset];
}

void ppu_vt03_device::palette_write(offs_t offset, u8 data)
{
	if (offset < 0x20)
	{
		ppu2c0x_device::palette_write(offset, data);
	}
	else
	{
		m_palette_ram[offset] = data;
	}
}

u8 ppu_vt03_device::extended_modes_enable_r() { return m_extended_modes_enable; }
u8 ppu_vt03_device::extended_modes2_enable_r() { return m_extended_modes2_enable; }
u8 ppu_vt03_device::videobank0_0_r() { return m_videobank0[0x0]; }
u8 ppu_vt03_device::videobank0_1_r() { return m_videobank0[0x1]; }
u8 ppu_vt03_device::videobank0_2_r() { return m_videobank0[0x2]; }
u8 ppu_vt03_device::videobank0_3_r() { return m_videobank0[0x3]; }
u8 ppu_vt03_device::videobank0_4_r() { return m_videobank0[0x4]; }
u8 ppu_vt03_device::videobank0_5_r() { return m_videobank0[0x5]; }
u8 ppu_vt03_device::videobank1_r() { return m_videobank1; }
u8 ppu_vt03_device::unk_2019_r() { return 0x00; } // unused?
u8 ppu_vt03_device::videobank0_extra_r() { return m_videobank0_extra; }
u8 ppu_vt03_device::unk_201b_r() { return 0x00; } // unused?
u8 ppu_vt03_device::gun_x_r() { return 0x00; }
u8 ppu_vt03_device::gun_y_r() { return 0x00; }
u8 ppu_vt03_device::gun2_x_r() { return 0x00; }
u8 ppu_vt03_device::gun2_y_r() { return 0x00; }


void ppu_vt03_device::init_vtxx_rgb555_palette_tables()
{
	int entry = 0;
	for (int emp = 0; emp < 8; emp++)
	{
		for (int palval = 0; palval < 0x8000; palval++)
		{
			//u16 rgbval = (m_palette_ram[i & 0x7f] & 0xff) | ((m_palette_ram[(i & 0x7f) + 0x80] & 0xff) << 8);
			const u8 blue = (palval & 0x001f) << 3;
			const u8 green = (palval & 0x3e0) >> 2;
			const u8 red = (palval & 0x7C00) >> 7;

			// TODO: apply emphasis values if they work in this mode
			m_vtpens_rgb555[entry] = rgb_t(red, green, blue);
			entry++;
		}
	}
}

void ppu_vt03_device::init_vtxx_rgb444_palette_tables()
{
	int entry = 0;
	for (int emp = 0; emp < 8; emp++)
	{
		for (int palval = 0; palval < 0x1000; palval++)
		{
			//u16 rgbval = (m_palette_ram[i & 0x7f] & 0x3f) | ((m_palette_ram[(i & 0x7f) + 0x80] & 0x3f) << 6);
			const u8 red = (palval & 0x000f) << 4;
			const u8 green = (palval & 0x0f0);
			const u8 blue = (palval & 0xf00) >> 4;

			// TODO: apply emphasis values if they work in this mode
			m_vtpens_rgb444[entry] = rgb_t(red, green, blue);
			entry++;
		}
	}
}
// what cases are palmode 1 anyway?
void ppu_vt03_device::init_vt03_palette_tables(int palmode)
{
	// the 12-bit VT HSV format, Credit to NewRisingSun
	int entry = 0;
	for (int color_emphasis = 0; color_emphasis < 8; color_emphasis++)
	{
		for (int palval = 0; palval < 0x1000; palval++)
		{
			int nPhase = (palval >> 0) & 0xF;
			int nLuma = (palval >> 4) & 0xF;
			int nChroma = (palval >> 8) & 0xF;
			float phaseOffset = -11.0;
			//bool inverted = false;
			if ((nLuma < (nChroma + 1) >> 1 || nLuma > 15 - (nChroma >> 1)) && (palmode != 1))
			{
				//inverted = true;
				// Strange color number wrap-around. Is this for protection reasons, or a bug of the original hardware?
				// The VT03 data sheet advises programmers that 4 <= nLuma*2 +nChroma <= 0x1F, which does not correspond exactly to this condition.
				static const unsigned char altPhases[16] = { 13,  7,  8,  9, 10, 11, 12,  1,  2,  3,  4,  5,  6,  0, 14, 15 };
				static const float    altPhaseOffset[16] = { -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,  0, -5, -5, -5 }; // Slight tweak in phase 6 for Z-Dog
				phaseOffset += altPhaseOffset[nPhase]; // These "alternative" colors seem to be slightly shifted in addition to being wrapped-around, at least in EmuVT.
				nPhase = altPhases[nPhase];
				nChroma = 16 - nChroma;
				nLuma = (nLuma - 8) & 0xF;
			}

			float fLuma = (nLuma - 4) / 9.625;     // Value determined from matching saturation =0 phases 1-12
			float fChroma = nChroma / 18.975;      // Value determined from matching phases 0 and 13 across all luminance and saturation levels
			const float fPhase = ((nPhase - 2) * 30.0 + phaseOffset) * M_PI / 180.0;

			if (palmode == 1)
			{
				if (fPhase > 0 && fPhase < 13)
				{
					fLuma /= 1.5;
					fChroma /= 2;
				}
			}

			float Y = fLuma;
			float C = fChroma;
			if (nPhase == 0 || nPhase > 12) C = 0.0;// Phases 0 and 13-15 are grays
			if (nPhase == 0) Y += fChroma;        // Phase 0 is the upper bound of the waveform
			if (nPhase == 13) Y -= fChroma;        // Phase 13 is the lower bound of the waveform
			if (nPhase >= 14) Y = 0.0;             // Phases 14 and 15 always black

			const float V = sin(fPhase) * C * 1.05; // 1.05 needed to get closer to EmuVT palette's color levels in phases 1-12
			const float U = cos(fPhase) * C * 1.05;
			float R = Y + 1.1400 * V + 0.0000 * U;
			float G = Y - 0.5807 * V - 0.3940 * U;
			float B = Y - 0.0000 * V + 2.0290 * U;
			if (R < 0.0) R = 0.0;
			if (R > 1.0) R = 1.0;
			if (G < 0.0) G = 0.0;
			if (G > 1.0) G = 1.0;
			if (B < 0.0) B = 0.0;
			if (B > 1.0) B = 1.0;
			const int RV = R * 255.0;
			const int GV = G * 255.0;
			const int BV = B * 255.0;

			// does this really apply to the VT palette?
			//bool is_pal = m_scanlines_per_frame != NTSC_SCANLINES_PER_FRAME;
			//apply_color_emphasis_and_clamp(is_pal, color_emphasis, R, G, B);

			set_pen_color(YUV444_COLOR + entry, rgb_t(RV, GV, BV));
			entry++;
		}
	}
}

void ppu_vt03_device::device_start()
{
	start_nopalram();

	m_palette_ram.resize(0x100);

	for (int i = 0; i < 0x100; i++)
		m_palette_ram[i] = 0x00;

	save_item(NAME(m_palette_ram));
	save_item(NAME(m_read_bg4_bg3));
	save_item(NAME(m_extra_sprite_bits));
	save_item(NAME(m_videobank0));
	save_item(NAME(m_videobank1));
	save_item(NAME(m_extended_modes_enable));
	save_item(NAME(m_extended_modes2_enable));
	save_item(NAME(m_videobank0_extra));
	save_item(NAME(m_vt3xx_palette));

	init_vt03_palette_tables(0);
	init_vtxx_rgb555_palette_tables();
	init_vtxx_rgb444_palette_tables();

	// for VT3xx / VT32 (may have different meanings on each though)
	save_item(NAME(m_newvid_1b));
	save_item(NAME(m_newvid_1c));
	save_item(NAME(m_newvid_1d));
	save_item(NAME(m_newvid_1e));
	save_item(NAME(m_tilebases_2x));
}

void ppu_vt03_device::device_reset()
{
	ppu2c0x_device::device_reset();

	for (int i = 0; i < 0xff; i++)
		m_palette_ram[i] = 0x0;

	for (int i = 0; i < 0x400; i++)
		m_vt3xx_palette[i] = 0x00;

	// todo: what are the actual defaults for these?
	m_extended_modes_enable = 0x00;
	m_extended_modes2_enable = 0x00;

	for (int i = 0; i < 6; i++)
		m_videobank0[i] = 0;

	m_videobank0_extra = 0;
	m_videobank1 = 0;

	m_read_bg4_bg3 = 0;

	// for VT3xx
	m_newvid_1b = 0x00;
	m_newvid_1c = 0x00;
	m_newvid_1d = 0x00;
	m_newvid_1e = 0x00;

	for (int i = 0; i < 4; i++)
		m_tilebases_2x[i] = 0x00;
}

u8 ppu_vt03_device::get_m_read_bg4_bg3()
{
	return m_read_bg4_bg3;
}

void ppu_vt03_device::read_sprite_plane_data(int address)
{
	m_planebuf[0] = m_read_sp((address + 0) & 0x1fff);
	m_planebuf[1] = m_read_sp((address + 8) & 0x1fff);

	const bool is4bpp = BIT(m_extended_modes_enable, 2);

	if (is4bpp)
	{
		m_planebuf[2] = m_read_sp(((address + 0) & 0x1fff)|0x2000);
		m_planebuf[3] = m_read_sp(((address + 8) & 0x1fff)|0x2000);
	}
}

void ppu_vt03_device::make_sprite_pixel_data(u8 &pixel_data, bool flipx)
{
	ppu2c0x_device::make_sprite_pixel_data(pixel_data, flipx);

	const bool is4bpp = BIT(m_extended_modes_enable, 2);
	const bool is16pix = BIT(m_extended_modes_enable, 0);

	if (is4bpp)
	{
		if (flipx)
		{
			// yes, shift by 5 and 6 because of the way the palette is arranged in RAM
			pixel_data |= (((m_planebuf[2] & 1) << 5) | ((m_planebuf[3] & 1) << 6));
			m_planebuf[2] = m_planebuf[2] >> 1;
			m_planebuf[3] = m_planebuf[3] >> 1;

			if (is16pix)
			{
				const u8 pix0 = pixel_data & 0x03;
				const u8 pix1 = (pixel_data >> 5) & 0x03;
				pixel_data = pix1 | (pix0 << 5);
			}
		}
		else
		{
			pixel_data |= (((m_planebuf[2] >> 7) & 1) << 5) | (((m_planebuf[3] >> 7) & 1) << 6);
			m_planebuf[2] = m_planebuf[2] << 1;
			m_planebuf[3] = m_planebuf[3] << 1;
		}
	}
}

void ppu_vt03_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap)
{
	const bool is4bpp = BIT(m_extended_modes_enable, 2);
	const bool is16pix = BIT(m_extended_modes_enable, 0);

	if (is4bpp)
	{
		if (!is16pix)
		{
			const u8 pen = pixel_data + (4 * color);
			draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
		}
		else
		{
			/* this mode makes use of the extra planes to increase sprite width instead
			    we probably need to split them out again and draw them at xpos+8 with a
			    cliprect - not seen used yet */
			if ((pixel_data & 0x03) != 0)
			{
				const u8 pen = (pixel_data & 0x03) + (4 * color);
				draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
			}

			if (((pixel_data >> 5) & 0x03) != 0)
			{
				const u8 pen = ((pixel_data >> 5) & 0x03) + (4 * color);
				draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel + 8));
			}
		}
	}
	else
	{
		ppu2c0x_device::draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data, bitmap);
	}
}

void ppu_vt03_device::read_tile_plane_data(int address, int color)
{
	const bool is4bpp = BIT(m_extended_modes_enable, 1);
	m_whichpixel = 0;

	if (m_extended_modes_enable & 0x10) // extended mode
		m_read_bg4_bg3 = color;
	else
		m_read_bg4_bg3 = 0;

	if (is4bpp)
	{
		m_planebuf[0] = m_read_bg( (address + 0) & 0x1fff );
		m_planebuf[1] = m_read_bg( (address + 8) & 0x1fff );
		m_planebuf[2] = m_read_bg( ((address + 0) & 0x1fff) | 0x2000 );
		m_planebuf[3] = m_read_bg( ((address + 8) & 0x1fff) | 0x2000 );
	}
	else
	{
		m_planebuf[0] = m_read_bg((address & 0x1fff));
		m_planebuf[1] = m_read_bg((address + 8) & 0x1fff);
	}
}

void ppu_vt03_device::shift_tile_plane_data(u8 &pix)
{
	const bool is4bpp = BIT(m_extended_modes_enable, 1);

	pix = 0;

	if (is4bpp)
	{
		pix = (BIT(m_planebuf[0], ~m_whichpixel & 0x07) << 0) |
				(BIT(m_planebuf[1], ~m_whichpixel & 0x07) << 1) |
				(BIT(m_planebuf[2], ~m_whichpixel & 0x07) << 5) |
				(BIT(m_planebuf[3], ~m_whichpixel & 0x07) << 6);
	}
	else
	{
		pix = (BIT(m_planebuf[0], ~m_whichpixel & 0x07) << 0) |
				(BIT(m_planebuf[1], ~m_whichpixel & 0x07) << 1);
	}

	m_whichpixel++;
}

void ppu_vt03_device::draw_back_pen(u32 *dst, int back_pen)
{
	if (m_extended_modes_enable & 0x80)
	{
		// is the back_pen always just pen 0 in VT modes? (using last data written to a transparent pen as per NES logic doesn't work as writes are split across 2 bytes)
		draw_tile_pixel_inner(0, dst);
	}
	else
	{
		// in normal modes we still have the data from the palette writes as the 'backpen' so treat it as before
		*dst = pen_color(back_pen & 0x1ff);
	}
}


void ppu_vt03_device::draw_tile_pixel_inner(u8 pen, u32 *dest)
{
	if (is_v3xx_extended_mode())
	{
		// correct for lxcmcysp, lxcmc250
		u16 pal0 = readbyte(((pen & 0xff) * 2) + 0x3c00);
		pal0 |= readbyte(((pen & 0xff) * 2) + 0x3c01) << 8;

		const int palb = (pal0 >> 0) & 0x1f;
		const int palg = (pal0 >> 5) & 0x1f;
		const int palr = (pal0 >> 10) & 0x1f;

		*dest = rgb_t(palr << 3, palg << 3, palb << 3);
	}
	else
	{
		if (BIT(m_extended_modes_enable, 7))
		{
			if (m_pal_mode == PAL_MODE_NEW_RGB) // unknown newer VT mode
			{
				u32 palval = (m_palette_ram[pen & 0x7f] & 0xff) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x7f) << 8);

				// does grayscale mode exist here? (we haven't calculated any colours for it)
				//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				//  palval &= 0x30;

				// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
				palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 10);

				*dest = m_vtpens_rgb555[palval & 0x3ffff];
			}
			else if (m_pal_mode == PAL_MODE_NEW_RGB12) // unknown newer VT mode
			{
				u32 palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

				// does grayscale mode exist here? (we haven't calculated any colours for it)
				//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				//  palval &= 0x30;

				// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
				palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

				*dest = m_vtpens_rgb444[palval & 0x7fff];
			}
			else // VT03 mode
			{
				u32 palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

				// does grayscale mode exist here? (we haven't calculated any colours for it)
				//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				//  palval &= 0x30;

				// apply colour emphasis (does it really exist here?) (we calculate values for it when building the palette lookup)
				palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

				*dest = pen_color(YUV444_COLOR + (palval & 0x7fff));
			}
		}
		else // old colour compatible mode
		{
			u16 palval = (m_palette_ram[pen & 0x7f] & 0x3f);

			if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				palval &= 0x30;

			// apply colour emphasis
			palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 1);

			*dest = pen_color(palval & 0x1ff);
		}
	}
}
void ppu_vt03_device::draw_tile_pixel(u8 pix, int color, u32 back_pen, u32 *&dest)
{
	if (is_v3xx_extended_mode())
	{
		draw_tile_pixel_inner(pix, dest);
	}
	else
	{
		const bool is4bpp = BIT(m_extended_modes_enable, 1);

		if (!is4bpp)
		{
			ppu2c0x_device::draw_tile_pixel(pix, color, back_pen, dest);
		}
		else
		{
			int basepen;
			int pen;

			if (m_extended_modes_enable & 0x10) // extended mode
			{
				basepen = 0;
			}
			else
			{
				basepen = 4 * color; // for use in the palette decoding
			}

			if (pix)
			{
				pen = pix + basepen;
			}
			else
			{
				pen = 0; // back_pen; // fixme backpen logic probably differs on vt03 due to extra colours
			}

			draw_tile_pixel_inner(pen, dest);
		}
	}
}

void ppu_vt03_device::read_extra_sprite_bits(int sprite_index)
{
	m_extra_sprite_bits = (m_spriteram[sprite_index + 2] & 0x1c) >> 2;
}

u8 ppu_vt03_device::get_speva2_speva0()
{
	return m_extra_sprite_bits;
}

void ppu_vt03_device::extended_modes_enable_w(u8 data)
{
	/*  7   : COLCOMP
	    6   : UNUSED (8bpp enable on VT09?)
	    5   : UNUSED
	    4   : BKEXTEN
	    3   : SPEXTEN
	    2   : SP16EN
	    1   : BK16EN
	    0   : PIX16EN */

	m_extended_modes_enable = data;
}

void ppu_vt03_device::extended_modes2_enable_w(u8 data) { m_extended_modes2_enable = data; }
void ppu_vt03_device::videobank0_0_w(u8 data) { m_videobank0[0x0] = data; }
void ppu_vt03_device::videobank0_1_w(u8 data) { m_videobank0[0x1] = data; }
void ppu_vt03_device::videobank0_2_w(u8 data) { m_videobank0[0x2] = data; }
void ppu_vt03_device::videobank0_3_w(u8 data) { m_videobank0[0x3] = data; }
void ppu_vt03_device::videobank0_4_w(u8 data) { m_videobank0[0x4] = data; }
void ppu_vt03_device::videobank0_5_w(u8 data) { m_videobank0[0x5] = data; }
void ppu_vt03_device::videobank1_w(u8 data) { m_videobank1 = data; }
void ppu_vt03_device::gun_reset_w(u8 data) { logerror("%s: gun_reset_w %02x\n", machine().describe_context(), data); }
void ppu_vt03_device::videobank0_extra_w(u8 data) { m_videobank0_extra = data; }
/* 201b unused */
/* 201c read gun read x (older VT chipsets) */
/* 201d read gun read y (older VT chipsets) */
/* 201e read gun 2 read x (older VT chipsets) */
/* 201f read gun 2 read y (older VT chipsets) */

void ppu_vt32_device::m_newvid_1b_w(u8 data) { logerror("%s: m_newvid_1b_w %02x\n", machine().describe_context(), data); m_newvid_1b = data; }
void ppu_vt32_device::m_newvid_1c_w(u8 data) { logerror("%s: m_newvid_1c_w %02x\n", machine().describe_context(), data); m_newvid_1c = data; }
void ppu_vt32_device::m_newvid_1d_w(u8 data) { logerror("%s: m_newvid_1d_w %02x\n", machine().describe_context(), data); m_newvid_1d = data; }

void ppu_vt32_device::draw_background(u8 *line_priority)
{
	// fingerd also uses 6d for the first song, which doesn't work with this code
	// maybe changes how extended attribute is used?
	if ((get_newvid_1c() == 0x2e) || (get_newvid_1c() == 0x0a) || (get_newvid_1c() == 0x6e))
	{
		// strange custom mode, feels more like a vt369 mode
		// tiles use 16x16x8 packed data

		// determine where in the nametable to start drawing from
		// based on the current scanline and scroll regs
		const u8  scroll_x_coarse = m_refresh_data & 0x001f;
		const u8  scroll_y_coarse = (m_refresh_data & 0x03c0) >> 5;
		// m_refresh_data & 0x0020 in this case would be the top/bottom of the tile

		const u16 nametable = (m_refresh_data & 0x0c00) >> 1;
		const u8  scroll_y_fine = (m_refresh_data & 0x7000) >> 12;

		int x = scroll_x_coarse >> 1;// &~1;

		int tile_index = (nametable | 0x2000) + scroll_y_coarse * 16;

		int start_x = ((((scroll_x_coarse & 1) << 3) + m_x_fine) ^ 0x0f) - 0xf;
		u32 *dest = &m_bitmap.pix(m_scanline, start_x);

		m_tilecount = 0;

		// draw the 15 or 16 tiles that make up a line
		while (m_tilecount < 17)
		{
			const int index1 = tile_index + (x * 2);
			int page2 = readbyte(index1);
			page2 |= (readbyte(index1 + 1) & 0x0f) << 8; // index+1 is colour data? and extra tile bits

			if (start_x < VISIBLE_SCREEN_WIDTH)
			{
				int gfx_address = page2 * 0x100;
				// this should probably go through the standard video banking?
				gfx_address += m_videobank0[0x5] * 0x800;
				gfx_address += m_videobank1 * 0x8000;

				gfx_address += scroll_y_fine * 16;
				gfx_address += ((m_refresh_data & 0x0020) >> 5) * 0x80;

				for (int i = 0; i < 16; i++)
				{
					u8 pix = m_read_onespace(gfx_address + i);
					if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
					{
						u32 palval;

						if (pix & 0x80)
							palval = (m_vt3xx_palette[pix & 0x7f] & 0x3f) | ((m_vt3xx_palette[(pix & 0x7f) + 0x80] & 0x3f) << 6);
						else
							palval = (m_vt3xx_palette[(pix & 0x7f) + 0x100] & 0x3f) | ((m_vt3xx_palette[(pix & 0x7f) + 0x180] & 0x3f) << 6);

						// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
						palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

						*dest = m_vtpens_rgb444[palval & 0x7fff];
						if (pix)
							line_priority[start_x + i] |= 0x02;
					}
					dest++;
				}
				start_x += 16;
				x++;
				if (x > 15)
				{
					x = 0;
					tile_index ^= 0x200;
				}

			}
			m_tilecount++;
		}
	}
	else
	{
		ppu2c0x_device::draw_background(line_priority);
	}
}


ppu_vt3xx_device::ppu_vt3xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	ppu_vt03_device(mconfig, PPU_VT3XX, tag, owner, clock)
{
	m_spriteramsize = 0x200;
}

void ppu_vt3xx_device::device_start()
{
	ppu_vt03_device::device_start();

	save_item(NAME(m_204x_screenregs));
	save_item(NAME(m_2008_spritehigh));
}

void ppu_vt3xx_device::device_reset()
{
	ppu_vt03_device::device_reset();

	for (int i = 0; i < 0xa; i++)
		m_204x_screenregs[i] = 0x00;

	m_2008_spritehigh = 0;
}

u8 ppu_vt3xx_device::extvidreg_201c_r(offs_t offset) { return m_newvid_1c; }
u8 ppu_vt3xx_device::extvidreg_201d_r(offs_t offset) { return m_newvid_1d; }
u8 ppu_vt3xx_device::extvidreg_201e_r(offs_t offset) { return m_newvid_1e; }
u8 ppu_vt3xx_device::tilebases_202x_r(offs_t offset) { return m_tilebases_2x[offset]; }

void ppu_vt3xx_device::extvidreg_201c_w(offs_t offset, u8 data) { m_newvid_1c = data; logerror("%s: extvidreg_201c_w %02x\n", machine().describe_context(), data); }
void ppu_vt3xx_device::extvidreg_201d_w(offs_t offset, u8 data) { m_newvid_1d = data; logerror("%s: extvidreg_201d_w %02x\n", machine().describe_context(), data); }

void ppu_vt3xx_device::extvidreg_201e_w(offs_t offset, u8 data)
{
	/*
	 extended mode feature enables
	 ---- -s--
	 s = old/new sprite mode
	*/
	m_newvid_1e = data;
	logerror("%s: extvidreg_201e_w %02x\n", machine().describe_context(), data);
}

void ppu_vt3xx_device::tilebases_202x_w(offs_t offset, u8 data)
{
	if (data != m_tilebases_2x[offset])
		logerror("%s: NEW VALUE tilebases_202x_w %d %02x\n", machine().describe_context(), offset, data);

	m_tilebases_2x[offset] = data;
}

// move this to ppu_vt03_device? as it seems like even some of the VT32 games write here
void ppu_vt3xx_device::lcdc_regs_w(offs_t offset, u8 data)
{
	// these seem somehow related to the screen dimensions, but could
	// be specific to the type of LCD being used (scale against the actual screen)
	// so for now we just use a table lookup
	//
	// of note lxcmcysp (which has a vertical screen squashed to horizontal) writes different
	// config values here compared to the natively horizontal versions
	//
	// the real devices scale the higher res images to the lower LCD, dropping pixels
	logerror("%s: ppu_vt3xx_device::lcdc_regs_w %d %02x\n", machine().describe_context(), offset, data);
	m_204x_screenregs[offset] = data;

	struct vid_mode
	{
		int min_x;
		int max_x;
		int min_y;
		int max_y;
		u8 regvals[0xa];
	};

	static const vid_mode mode_table[] = {
		// configurations used for lower resolution output
		{ 0, 159, 0, 127, { 0xa0, 0xff, 0x00, 0x40, 0xff, 0x04, 0x00, 0xa8, 0x04, 0x0f }, },
		{ 0, 199, 0, 199, { 0xdc, 0xff, 0x00, 0x58, 0xff, 0x04, 0x10, 0xa8, 0x04, 0x00 }, }, // hkb502 menu, uncertain dimensions
		{ 0, 127, 0, 159, { 0x80, 0x80, 0x00, 0x50, 0xff, 0x04, 0x00, 0xaa, 0x08, 0x00 }, }, // lexi30 menu
		{ 0, 127, 0, 159, { 0x80, 0xff, 0x00, 0x50, 0xff, 0x04, 0x00, 0xa6, 0x04, 0x00 }, }, // jl1810gr
		{ 0, 127, 0, 159, { 0x80, 0x3f, 0x00, 0x50, 0xff, 0x69, 0x00, 0x54, 0x08, 0x00 }, }, // gcs2mgp

		// lxcypkdp uses this on the menus, they must rotate the rendering somehow as this is vertical and the games are horizontal!
		{ 0, 127, 0, 159, { 0x80, 0xfe, 0x00, 0x50, 0xff, 0x04, 0x00, 0xa8, 0x04, 0x00 }, },

		// configurations used for 'regular' output
		{ 0, 255, 0, 239, { 0xa0, 0x57, 0x09, 0x40, 0x93, 0x04, 0x00, 0x83, 0x08, 0x00 }, }, // full mode for the 0, 159, 0, 127 config

		{ 0, 255, 0, 239, { 0x40, 0xa1, 0x00, 0x78, 0xff, 0x69, 0x0a, 0x69, 0x26, 0x00 }, }, // denv150
		{ 0, 255, 0, 239, { 0x40, 0xa1, 0x00, 0x78, 0xff, 0x04, 0x0a, 0xd4, 0x0a, 0x00 }, }, // myarccn
		{ 0, 255, 0, 239, { 0xdc, 0xe1, 0x00, 0x58, 0xbf, 0x04, 0x10, 0x93, 0x04, 0x00 }, }, // hkb502 normal games

		{ -1, -1, -1, -1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, },
	};

	if (offset == 0x09)
	{
		// default to these (standard resolution) if no entry is found
		int new_min_x = 0;
		int new_max_x = 255;
		int new_min_y = 0;
		int new_max_y = 239;

		rectangle curvisarea = screen().visible_area();
		logerror("current screen dimensions are %d %d %d %d\n", curvisarea.min_x, curvisarea.max_x, curvisarea.min_y, curvisarea.max_y);

		int tablenum = 0;
		do
		{
			bool found = true;
			for (int entry = 0; entry < 0xa; entry++)
			{
				if (mode_table[tablenum].regvals[entry] != m_204x_screenregs[entry])
					found = false;
			}

			if (found)
			{
				new_min_x = mode_table[tablenum].min_x;
				new_max_x = mode_table[tablenum].max_x;
				new_min_y = mode_table[tablenum].min_y;
				new_max_y = mode_table[tablenum].max_y;

				logerror("new screen dimensions are %d %d %d %d\n", new_min_x, new_max_x, new_min_y, new_max_y);

			}

			tablenum++;
		} while (mode_table[tablenum].min_x != -1);

		screen().set_visible_area(new_min_x, new_max_x, new_min_y, new_max_y);

	}
}

// vt3xx tile modes are no longer planar, but the tile code provides ROM offsets that
// would be, this converts them to offsets that give us the data we want.
offs_t ppu_vt3xx_device::recalculate_offsets_8x8x4packed_tile(int address, int va34)
{
	int finaladdr = get_newmode_tilebase() * 0x2000;
	int tileline = address & 0x0007;
	int tileplane = address & 0x0008;
	int tilenum = address & 0x0ff0;
	int colorbits = get_m_read_bg4_bg3();
	int finaloffset = (tilenum << 1) | (tileline << 2) | (tileplane >> 2) | va34;
	finaloffset += colorbits * 0x2000;
	return finaladdr + finaloffset;
}

offs_t ppu_vt3xx_device::recalculate_offsets_8x8x8packed_tile(int address, int va34)
{
	int finaladdr = get_newmode_tilebase() * 0x2000;
	int colorbits = get_m_read_bg4_bg3();
	int tileline = address & 0x0007;
	int tileplane = address & 0x0008;
	int tilenum = address & 0x0ff0;
	int finaloffset = (tilenum << 2) | (tileline << 3) | (tileplane >> 1) | va34;
	finaloffset += colorbits * 0x4000;
	return finaladdr + finaloffset;
}

offs_t ppu_vt3xx_device::recalculate_offsets_16x16x8packed_hires_tile(int address, int va34)
{
	int finaladdr = get_newmode_tilebase() * 0x2000;
	int colorbits = get_m_read_bg4_bg3();
	int tileline = address & 0x0007;
	int tileplane = address & 0x0008;
	int tilenum = (address & 0x0ff0) >> 4;
	int finaloffset = tilenum * 0x100;
	finaloffset += tileline * 0x20; // 0x10 are the odd lines, we currently only fetch even as we're pretending these are 8x8
	finaloffset += va34; // 3 bits
	finaloffset += tileplane; // 1 bit
	finaloffset += colorbits * 0x10000;
	return finaladdr + finaloffset;
}

void ppu_vt3xx_device::read_tile_plane_data(int address, int color)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::read_tile_plane_data(address, color);
	}
	else
	{
		m_read_bg4_bg3 = color;
		m_whichpixel = 0;

		// used by the rtvgc300 / rtvgc300fz menus, and also 'image match' in lxcmcysp
		if (m_newvid_1c & 0x04) // high resolution mode
		{
			m_planebuf[0] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 0));
			m_planebuf[1] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 0));
			m_planebuf[2] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 1));
			m_planebuf[3] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 1));
			m_planebuf[4] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 2));
			m_planebuf[5] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 2));
			m_planebuf[6] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 3));
			m_planebuf[7] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 3));
			m_planebuf[8] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 4));
			m_planebuf[9] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 4));
			m_planebuf[10] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 5));
			m_planebuf[11] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 5));
			m_planebuf[12] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 6));
			m_planebuf[13] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 6));
			m_planebuf[14] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 0) & 0x1fff, 7));
			m_planebuf[15] = m_read_onespace_with_relative(recalculate_offsets_16x16x8packed_hires_tile((address + 8) & 0x1fff, 7));
		}
		else
		{
			if ((m_newvid_1c & 0x03) == 0x02)
			{
				m_planebuf[0] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 0) & 0x1fff, 0));
				m_planebuf[1] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 8) & 0x1fff, 0));
				m_planebuf[2] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 0) & 0x1fff, 1));
				m_planebuf[3] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 8) & 0x1fff, 1));
				m_planebuf[4] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 0) & 0x1fff, 2));
				m_planebuf[5] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 8) & 0x1fff, 2));
				m_planebuf[6] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 0) & 0x1fff, 3));
				m_planebuf[7] = m_read_onespace_with_relative(recalculate_offsets_8x8x8packed_tile((address + 8) & 0x1fff, 3));
			}
			else
			{
				m_planebuf[0] = m_read_onespace_with_relative(recalculate_offsets_8x8x4packed_tile((address + 0) & 0x1fff, 0));
				m_planebuf[1] = m_read_onespace_with_relative(recalculate_offsets_8x8x4packed_tile((address + 8) & 0x1fff, 0));
				m_planebuf[2] = m_read_onespace_with_relative(recalculate_offsets_8x8x4packed_tile((address + 0) & 0x1fff, 1));
				m_planebuf[3] = m_read_onespace_with_relative(recalculate_offsets_8x8x4packed_tile((address + 8) & 0x1fff, 1));
			}
		}
	}
}

void ppu_vt3xx_device::shift_tile_plane_data(u8 &pix)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::shift_tile_plane_data(pix);
	}
	else
	{
		if (m_newvid_1c & 0x04) // high resolution mode
		{
			// we currently pretend this is 8x8, not 16x16
			pix = m_planebuf[((m_whichpixel & 0x03) << 2) | BIT(m_whichpixel, 2)];
		}
		else
		{
			if ((m_newvid_1c & 0x03) == 0x02)
			{
				// 8x8x8 non-planar mode
				pix = m_planebuf[bitswap<3>(m_whichpixel, 1, 0, 2)];
			}
			else
			{
				// extended modes
				// 8x8x4 non-planar mode
				pix = (m_planebuf[bitswap<2>(m_whichpixel, 1, 2)] >> (BIT(m_whichpixel, 0) << 2)) & 0x0f;
			}
		}
		m_whichpixel++;
	}
}

inline rgb_t ppu_vt3xx_device::get_pen_value(int pixel_data, int bpp, int pal)
{
	u8 pen;
	if (bpp == 4)
		pen = pixel_data | pal << 4;
	else
		pen = pixel_data; // does pal have another meaning in 8bpp mode?

	u16 pal0 = readbyte(((pen & 0xff) * 2) + 0x3e00);
	pal0 |= readbyte(((pen & 0xff) * 2) + 0x3e01) << 8;

	const int palb = (pal0 >> 0) & 0x1f;
	const int palg = (pal0 >> 5) & 0x1f;
	const int palr = (pal0 >> 10) & 0x1f;

	return rgb_t(palr << 3, palg << 3, palb << 3);
}

inline void ppu_vt3xx_device::draw_extended_sprite_pixel_low(bitmap_rgb32 &bitmap, int pixel_data, int pixel, int xpos, int pal, int bpp, u8 *line_priority)
{
	if (pixel_data) // opaque check
	{
		if ((xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			// has another sprite been drawn here?/
			if (!line_priority[xpos + pixel])
			{
				const rgb_t palval = get_pen_value(pixel_data, bpp, pal);
				m_bitmap.pix(m_scanline, xpos + pixel) = palval;
				// indicate that a sprite was drawn at this location, even if it's not seen
				line_priority[xpos + pixel] |= 0x01;
			}
		}
	}
}

inline void ppu_vt3xx_device::draw_extended_sprite_pixel_high(bitmap_rgb32 &bitmap, int pixel_data, int pixel, int xpos, int pal, int bpp, u8 *line_priority)
{
	if (pixel_data) // opaque check
	{
		if ((xpos + pixel) < VISIBLE_SCREEN_WIDTH)
		{
			// has another sprite been drawn here?
			if (BIT(~line_priority[xpos + pixel], 0))
			{
				const rgb_t palval = get_pen_value(pixel_data, bpp, pal);
				m_bitmap.pix(m_scanline, xpos + pixel) = palval;
				// indicate that a sprite was drawn at this location, even if it's not seen
				line_priority[xpos + pixel] |= 0x01;
			}
		}
	}
}

inline u8 ppu_vt3xx_device::get_pixel_data(u8 *spritepatternbuf, int bpp, int pixel)
{
	u8 pixel_data;
	if (bpp == 4)
	{
		pixel_data = spritepatternbuf[pixel >> 1];
		if (pixel & 1)
			pixel_data >>= 4;
		else
			pixel_data &= 0xf;
	}
	else
	{
		pixel_data = spritepatternbuf[pixel];
	}
	return pixel_data;
}

void ppu_vt3xx_device::draw_sprites_high_res(u8 *line_priority)
{
	// high res sprite mode uses an entirely different format (and possibly different spriteram)
	for (int spritenum = 0x00; spritenum < 0x40; spritenum++)
	{
		int ypos = m_spriteram[(spritenum * 8) + 0];
		int tilenum = m_spriteram[(spritenum * 8) + 1];
		tilenum |= m_spriteram[(spritenum * 8) + 2] << 8;
		int bpp = m_spriteram[(spritenum * 8) + 3] & 0x80;
		int width = m_spriteram[(spritenum * 8) + 3] & 0x30;
		int height = m_spriteram[(spritenum * 8) + 3] & 0x0c;
		int pal = m_spriteram[(spritenum * 8) + 6] & 0x3f;
		//int flipx = m_spriteram[(spritenum * 8) + 6] & 0x40;
		//int flipy = m_spriteram[(spritenum * 8) + 6] & 0x80;
		int xpos = m_spriteram[(spritenum * 8) + 7];

		if (bpp)
			bpp = 8;
		else
			bpp = 4;

		//flipx = flipx >> 6;
		//flipy = flipy >> 7;

		width = width >> 4;
		width = 4 << width;
		height = height >> 2;
		height = 4 << height;

		//if (m_scanline == 128)
		//  logerror("high res sprite %d xpos %02x ypos %02x tile %04x pal %02x xsize %d ysize %d bpp %d flipx %d flipy %d\n", spritenum, xpos, ypos, tilenum, pal, width, height, bpp, flipx, flipy);

		// if the sprite isn't visible, skip it
		if ((ypos + height <= m_scanline) || (ypos > m_scanline))
			continue;

		// compute the character's line to draw
		const int sprite_line = m_scanline - ypos;

		int pattern_offset;
		if (bpp == 4)
		{
			pattern_offset = tilenum * (2 * height * width);
			pattern_offset += sprite_line * (2 * width);
		}
		else
		{
			pattern_offset = tilenum * (4 * height * width);
			pattern_offset += sprite_line * (4 * width);
		}

		pattern_offset += get_newmode_spritebase() * 0x2000;

		for (int pixel = 0; pixel < width; pixel++)
		{
			u8 pixel_data;

			if (bpp == 4)
			{
				/*
				pixel_data = m_read_onespace_with_relative(pattern_offset + (pixel >> 1));
				if (pixel & 1)
				    pixel_data >>= 4;
				else
				    pixel_data &= 0xf;
				*/
				// we're pretending this isn't high-res so skipping pixels
				pixel_data = m_read_onespace_with_relative(pattern_offset + pixel);
				pixel_data &= 0xf;
			}
			else
			{
				//pixel_data = m_read_onespace_with_relative(pattern_offset + pixel);
				// we're pretending this isn't high-res so skipping pixels
				pixel_data = m_read_onespace_with_relative(pattern_offset + (pixel * 2));
			}

			if (xpos + pixel >= 0)
			{
				if (pixel_data) // opaque check
				{
					if ((xpos + pixel) < VISIBLE_SCREEN_WIDTH)
					{
						const rgb_t palval = get_pen_value(pixel_data, bpp, pal);
						m_bitmap.pix(m_scanline, xpos + pixel) = palval;
					}
				}
			}
		}

	}
}

void ppu_vt3xx_device::draw_sprites_standard_res(u8 *line_priority)
{
	/*

	+ 0x000    yyyy yyyy   y = ypos
	+ 0x080    tttt tttt   t = tile number

	for new format 0  (m_newvid_1d & 0x08 set)
	+ 0x100    YXpT TTpp   Y = negative Y pos    X = negative X pos    T = high tile number    p = palette

	for new format 1  (m_newvid_1d & 0x08 not set)
	+ 0x100    fFzT TTpp   f = yflip    F = xflip    T = high tile number    p = palette    z = priority

	+ 0x180    xxxx xxxx   x = xpos

	*/

	// new style sprites
	for (int spritenum = 0x00; spritenum < 0x80; spritenum++)
	{
		const bool is_new_format = m_newvid_1e & 0x04;

		// old packed spriteram format
		int ypos_table = 0x000;
		int xpos_table = 0x003;
		int tilenum_table = 0x001;
		int extra_table = 0x002;
		int table_step = 4;

		// new expanded spriteram format
		if (m_newvid_1e & 0x04)
		{
			ypos_table = 0x000;
			xpos_table = 0x180;
			tilenum_table = 0x080;
			extra_table = 0x100;
			table_step = 1;
		}

		const int sprite_table_offset = spritenum * table_step;
		int pri = 0;
		int ypos = m_spriteram[ypos_table + sprite_table_offset];
		int xpos = m_spriteram[xpos_table + sprite_table_offset];
		int tilenum = m_spriteram[tilenum_table + sprite_table_offset];
		tilenum |= (m_spriteram[extra_table + sprite_table_offset] & 0x1c) << 6;

		int pal = m_spriteram[extra_table + sprite_table_offset] & 0x03;

		if (m_newvid_1d & 0x08) // format 0
		{
			pal |= (m_spriteram[extra_table + sprite_table_offset] & 0x20) >> 3;
			if (m_spriteram[extra_table + sprite_table_offset] & 0x40)
			{
				xpos = -0x100 + xpos; // allows for partially offscreen sprites?
			}

			// TODO: verify
			if (m_spriteram[extra_table + sprite_table_offset] & 0x80)
			{
				ypos = -0x100 + ypos;
			}
		}
		else // format 1
		{
			pri = (m_spriteram[extra_table + sprite_table_offset] & 0x20) >> 5;
		}

		int height, width, bpp, alt_16_handling;

		if (is_new_format)
		{
			height = 16;
			width = 8;
			bpp = 8;
			alt_16_handling = false;

			if (m_newvid_1d & 0x02)
			{
				width = 16;
				bpp = 4;
			}

			// testing with lxcmcysp later games in the list
			// 12 09 0f -- 'alt_16_handling'
			// 22 09 0f -- some games, works
			// 12 0f 0f -- menu, works
			// 12 0b 0f -- hercules in red5mam, still broken
			if ((!(m_newvid_1d & 0x04)) && (m_newvid_1c & 0x10))
			{
				alt_16_handling = true;
				bpp = 4;
			}
		}
		else
		{
			// use the old size register in this mode? monster jump in lxcmcysp at least sets it
			height = (m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;
			width = 8;
			bpp = 8;
			alt_16_handling = false;

			// 12 0f 0b -- tetrtin
			if ((m_newvid_1c == 0x12) && (m_newvid_1d == 0x0f) && (m_newvid_1e == 0x0b))
			{
				// this seems to disagree with only using the old height register in this mode
				bpp = 4;
				width = 16;
				height = 16;
			}
		}

		// if the sprite isn't visible, skip it
		if ((ypos + height <= m_scanline) || (ypos > m_scanline))
			continue;

		// compute the character's line to draw
		const int sprite_line = m_scanline - ypos;

		// a 16 pixel wide sprite (packed format), at 4bpp, requires 8 bytes for a single line
		// at 16 pixels high it requires 128 bytes for a whole tile

		// sprites can be 8 pixels wide and 8bpp, or 16 pixels wide and 4bpp?
		int index1;

		if (bpp == 4)
		{
			if (alt_16_handling)
				index1 = tilenum * 32;
			else
				index1 = tilenum * 128;
		}
		else
		{
			index1 = tilenum * 64; // why? a 16 wide 4bpp sprite takes up the same number of bytes as an 8 wide 8bpp sprite
		}

		int pattern_offset;

		if (alt_16_handling)
			pattern_offset = index1 + sprite_line * 4;
		else
			pattern_offset = index1 + sprite_line * 8;

		pattern_offset += get_newmode_spritebase() * 0x2000;

		u8 spritepatternbuf[8];
		for (int i = 0; i < 8; i++)
			spritepatternbuf[i] = m_read_onespace_with_relative(pattern_offset + i);

		if (pri)
		{
			for (int pixel = 0; pixel < width; pixel++)
			{
				u8 pixel_data = get_pixel_data(spritepatternbuf, bpp, pixel);
				if (xpos + pixel >= 0)
					draw_extended_sprite_pixel_low(m_bitmap, pixel_data, pixel, xpos, pal, bpp, line_priority);
			}
		}
		else
		{
			for (int pixel = 0; pixel < width; pixel++)
			{
				u8 pixel_data = get_pixel_data(spritepatternbuf, bpp, pixel);
				if (xpos + pixel >= 0)
					draw_extended_sprite_pixel_high(m_bitmap, pixel_data, pixel, xpos, pal, bpp, line_priority);
			}

		}
	}
}

void ppu_vt3xx_device::draw_sprites(u8 *line_priority)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::draw_sprites(line_priority);
	}
	else
	{
		if (m_newvid_1c & 0x04) // high resolution mode
		{
			draw_sprites_high_res(line_priority);
		}
		else
		{
			draw_sprites_standard_res(line_priority);
		}
	}
}

void ppu_vt3xx_device::write_to_spriteram_with_increment(u8 data)
{
	if (!m_newvid_1e) // might be the CPU speed control bit instead
	{
		ppu_vt03_device::write_to_spriteram_with_increment(data);
	}
	else
	{
		m_spriteram[m_regs[PPU_SPRITE_ADDRESS] | (m_2008_spritehigh & 0x1) << 8] = data;
		m_regs[PPU_SPRITE_ADDRESS] = (m_regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
		if (m_regs[PPU_SPRITE_ADDRESS] == 0x00)
			m_2008_spritehigh ^= 0x1;
	}
}
