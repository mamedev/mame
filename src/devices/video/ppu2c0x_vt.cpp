// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation

    The VT video is based on the ppu2c0x but with enhanced capabilities such
    as 16 colour sprites.

******************************************************************************/

#include "emu.h"
#include "ppu2c0x_vt.h"


/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */

// devices
DEFINE_DEVICE_TYPE(PPU_VT03, ppu_vt03_device, "ppu_vt03", "VT03 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_VT03PAL, ppu_vt03pal_device, "ppu_vt03pal", "VT03 PPU (PAL)")

DEFINE_DEVICE_TYPE(PPU_VT3XX, ppu_vt3xx_device, "ppu_vt3xx", "VT3XX PPU (NTSC)")

ppu_vt03_device::ppu_vt03_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock),
	m_is_pal(false),
	m_is_50hz(false),
	m_read_bg(*this, 0),
	m_read_sp(*this, 0)
{
}

ppu_vt03_device::ppu_vt03_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_vt03_device(mconfig, PPU_VT03, tag, owner, clock)
{
}


ppu_vt03pal_device::ppu_vt03pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_vt03_device(mconfig, PPU_VT03PAL, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
	m_is_pal = true;
	m_is_50hz = true;
}


uint8_t ppu_vt03_device::palette_read(offs_t offset)
{
	if (offset < 0x20)
		return ppu2c0x_device::palette_read(offset);
	else
		return m_palette_ram[offset];
}

void ppu_vt03_device::palette_write(offs_t offset, uint8_t data)
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

uint8_t ppu_vt03_device::extended_modes_enable_r(offs_t offset) { return m_extended_modes_enable; }
uint8_t ppu_vt03_device::extended_modes2_enable_r(offs_t offset) { return m_extended_modes2_enable; }
uint8_t ppu_vt03_device::videobank0_0_r(offs_t offset) { return m_videobank0[0x0]; }
uint8_t ppu_vt03_device::videobank0_1_r(offs_t offset) { return m_videobank0[0x1]; }
uint8_t ppu_vt03_device::videobank0_2_r(offs_t offset) { return m_videobank0[0x2]; }
uint8_t ppu_vt03_device::videobank0_3_r(offs_t offset) { return m_videobank0[0x3]; }
uint8_t ppu_vt03_device::videobank0_4_r(offs_t offset) { return m_videobank0[0x4]; }
uint8_t ppu_vt03_device::videobank0_5_r(offs_t offset) { return m_videobank0[0x5]; }
uint8_t ppu_vt03_device::videobank1_r(offs_t offset) { return m_videobank1; }
uint8_t ppu_vt03_device::read_2019(offs_t offset) { return 0x00; } // unused?
uint8_t ppu_vt03_device::videobank0_extra_r(offs_t offset) { return m_videobank0_extra; }
uint8_t ppu_vt03_device::read_201b(offs_t offset) { return 0x00; } // unused?
uint8_t ppu_vt03_device::gun_x_r(offs_t offset) { return 0x00; }
uint8_t ppu_vt03_device::gun_y_r(offs_t offset) { return 0x00; }
uint8_t ppu_vt03_device::gun2_x_r(offs_t offset) { return 0x00; }
uint8_t ppu_vt03_device::gun2_y_r(offs_t offset) { return 0x00; }


void ppu_vt03_device::init_vtxx_rgb555_palette_tables()
{
	int entry = 0;
	for (int emp = 0; emp < 8; emp++)
	{
		for (int palval = 0; palval < 0x8000; palval++)
		{
			//uint16_t rgbval = (m_palette_ram[i & 0x7f] & 0xff) | ((m_palette_ram[(i & 0x7f) + 0x80] & 0xff) << 8);
			const uint8_t blue = (palval & 0x001f) << 3;
			const uint8_t green = (palval & 0x3e0) >> 2;
			const uint8_t red = (palval & 0x7C00) >> 7;

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
			//uint16_t rgbval = (m_palette_ram[i & 0x7f] & 0x3f) | ((m_palette_ram[(i & 0x7f) + 0x80] & 0x3f) << 6);
			const uint8_t red = (palval & 0x000f) << 4;
			const uint8_t green = (palval & 0x0f0);
			const uint8_t blue = (palval & 0xf00) >> 4;

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
	save_item(NAME(m_extplanebuf));
	save_item(NAME(m_extra_sprite_bits));
	save_item(NAME(m_videobank0));
	save_item(NAME(m_videobank1));
	save_item(NAME(m_extended_modes_enable));
	save_item(NAME(m_extended_modes2_enable));
	save_item(NAME(m_videobank0_extra));

	init_vt03_palette_tables(0);
	init_vtxx_rgb555_palette_tables();
	init_vtxx_rgb444_palette_tables();

	// for VT3xx
	save_item(NAME(m_newvid_1c));
	save_item(NAME(m_newvid_1d));
	save_item(NAME(m_newvid_1e));
}

void ppu_vt03_device::device_reset()
{
	ppu2c0x_device::device_reset();

	for (int i = 0; i < 0xff; i++)
		m_palette_ram[i] = 0x0;

	// todo: what are the actual defaults for these?
	m_extended_modes_enable = 0x00;
	m_extended_modes2_enable = 0x00;

	for (int i = 0; i < 6; i++)
		m_videobank0[i] = 0;

	m_videobank0_extra = 0;
	m_videobank1 = 0;

	m_read_bg4_bg3 = 0;

	// for VT3xx
	m_newvid_1c = 0x00;
	m_newvid_1d = 0x00;
	m_newvid_1e = 0x00;
}


uint8_t ppu_vt03_device::get_m_read_bg4_bg3()
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
		m_extplanebuf[0] = m_read_sp(((address + 0) & 0x1fff)|0x2000);
		m_extplanebuf[1] = m_read_sp(((address + 8) & 0x1fff)|0x2000);
	}
}

void ppu_vt03_device::make_sprite_pixel_data(uint8_t& pixel_data, bool flipx)
{
	ppu2c0x_device::make_sprite_pixel_data(pixel_data, flipx);

	const bool is4bpp = BIT(m_extended_modes_enable, 2);
	const bool is16pix = BIT(m_extended_modes_enable, 0);

	if (is4bpp)
	{
		if (flipx)
		{
			// yes, shift by 5 and 6 because of the way the palette is arranged in RAM
			pixel_data |= (((m_extplanebuf[0] & 1) << 5) | ((m_extplanebuf[1] & 1) << 6));
			m_extplanebuf[0] = m_extplanebuf[0] >> 1;
			m_extplanebuf[1] = m_extplanebuf[1] >> 1;

			if (is16pix)
			{
				const uint8_t pix0 = pixel_data & 0x03;
				const uint8_t pix1 = (pixel_data >> 5) & 0x03;
				pixel_data = pix1 | (pix0 << 5);
			}
		}
		else
		{
			pixel_data |= (((m_extplanebuf[0] >> 7) & 1) << 5) | (((m_extplanebuf[1] >> 7) & 1) << 6);
			m_extplanebuf[0] = m_extplanebuf[0] << 1;
			m_extplanebuf[1] = m_extplanebuf[1] << 1;
		}
	}
}

void ppu_vt03_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap)
{
	const bool is4bpp = BIT(m_extended_modes_enable, 2);
	const bool is16pix = BIT(m_extended_modes_enable, 0);

	if (is4bpp)
	{
		if (!is16pix)
		{
			const uint8_t pen = pixel_data + (4 * color);
			draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
		}
		else
		{
			/* this mode makes use of the extra planes to increase sprite width instead
			    we probably need to split them out again and draw them at xpos+8 with a
			    cliprect - not seen used yet */
			if ((pixel_data & 0x03) != 0)
			{
				const uint8_t pen = (pixel_data & 0x03) + (4 * color);
				draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
			}

			if (((pixel_data >> 5) & 0x03) != 0)
			{
				const uint8_t pen = ((pixel_data >> 5) & 0x03) + (4 * color);
				draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel + 8));
			}
			//ppu2c0x_device::draw_sprite_pixel(sprite_xpos, color, pixel, pixel_data & 0x03, bitmap);
			//ppu2c0x_device::draw_sprite_pixel(sprite_xpos, color, pixel + 8, (pixel_data >> 5) & 0x03, bitmap);
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
		m_extplanebuf[0] = m_read_bg( ((address + 0) & 0x1fff) | 0x2000 );
		m_extplanebuf[1] = m_read_bg( ((address + 8) & 0x1fff) | 0x2000 );
	}
	else
	{
		m_planebuf[0] = m_read_bg((address & 0x1fff));
		m_planebuf[1] = m_read_bg((address + 8) & 0x1fff);
	}
}

void ppu_vt03_device::shift_tile_plane_data(uint8_t& pix)
{
	const bool is4bpp = BIT(m_extended_modes_enable, 1);

	pix = 0;

	if (is4bpp)
	{
		switch (m_whichpixel)
		{
		case 0: pix = (BIT(m_planebuf[0], 7) << 0) | (BIT(m_planebuf[1], 7) << 1) | (BIT(m_extplanebuf[0], 7) << 5) | (BIT(m_extplanebuf[1], 7) << 6); break;
		case 1: pix = (BIT(m_planebuf[0], 6) << 0) | (BIT(m_planebuf[1], 6) << 1) | (BIT(m_extplanebuf[0], 6) << 5) | (BIT(m_extplanebuf[1], 6) << 6); break;
		case 2: pix = (BIT(m_planebuf[0], 5) << 0) | (BIT(m_planebuf[1], 5) << 1) | (BIT(m_extplanebuf[0], 5) << 5) | (BIT(m_extplanebuf[1], 5) << 6); break;
		case 3: pix = (BIT(m_planebuf[0], 4) << 0) | (BIT(m_planebuf[1], 4) << 1) | (BIT(m_extplanebuf[0], 4) << 5) | (BIT(m_extplanebuf[1], 4) << 6); break;
		case 4: pix = (BIT(m_planebuf[0], 3) << 0) | (BIT(m_planebuf[1], 3) << 1) | (BIT(m_extplanebuf[0], 3) << 5) | (BIT(m_extplanebuf[1], 3) << 6); break;;
		case 5: pix = (BIT(m_planebuf[0], 2) << 0) | (BIT(m_planebuf[1], 2) << 1) | (BIT(m_extplanebuf[0], 2) << 5) | (BIT(m_extplanebuf[1], 2) << 6); break;
		case 6: pix = (BIT(m_planebuf[0], 1) << 0) | (BIT(m_planebuf[1], 1) << 1) | (BIT(m_extplanebuf[0], 1) << 5) | (BIT(m_extplanebuf[1], 1) << 6); break;
		case 7: pix = (BIT(m_planebuf[0], 0) << 0) | (BIT(m_planebuf[1], 0) << 1) | (BIT(m_extplanebuf[0], 0) << 5) | (BIT(m_extplanebuf[1], 0) << 6); break;
		}
	}
	else
	{
		switch (m_whichpixel)
		{
		case 0: pix = (BIT(m_planebuf[0], 7) << 0) | (BIT(m_planebuf[1], 7) << 1); break;
		case 1: pix = (BIT(m_planebuf[0], 6) << 0) | (BIT(m_planebuf[1], 6) << 1); break;
		case 2: pix = (BIT(m_planebuf[0], 5) << 0) | (BIT(m_planebuf[1], 5) << 1); break;
		case 3: pix = (BIT(m_planebuf[0], 4) << 0) | (BIT(m_planebuf[1], 4) << 1); break;
		case 4: pix = (BIT(m_planebuf[0], 3) << 0) | (BIT(m_planebuf[1], 3) << 1); break;
		case 5: pix = (BIT(m_planebuf[0], 2) << 0) | (BIT(m_planebuf[1], 2) << 1); break;
		case 6: pix = (BIT(m_planebuf[0], 1) << 0) | (BIT(m_planebuf[1], 1) << 1); break;
		case 7: pix = (BIT(m_planebuf[0], 0) << 0) | (BIT(m_planebuf[1], 0) << 1); break;
		}
	}

	m_whichpixel++;
}


void ppu_vt03_device::draw_back_pen(uint32_t* dst, int back_pen)
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


void ppu_vt03_device::draw_tile_pixel_inner(uint8_t pen, uint32_t *dest)
{
	if (is_v3xx_extended_mode())
	{
		// correct for lxcmcysp, lxcmc250
		uint16_t pal0 = readbyte(((pen & 0xff)*2)+0x2400);
		         pal0 |= readbyte(((pen & 0xff)*2)+0x2401) << 8;

		int palb = (pal0 >> 0) & 0x1f;
		int palg = (pal0 >> 5) & 0x1f;
		int palr = (pal0 >> 10) & 0x1f;

		*dest = rgb_t(palr<<3, palg<<3, palb<<3);
	}
	else
	{
		if (BIT(m_extended_modes_enable, 7))
		{
			if (m_pal_mode == PAL_MODE_NEW_RGB) // unknown newer VT mode
			{
				uint32_t palval = (m_palette_ram[pen & 0x7f] & 0xff) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x7f) << 8);

				// does grayscale mode exist here? (we haven't calculated any colours for it)
				//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				//  palval &= 0x30;

				// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
				palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 10);

				*dest = m_vtpens_rgb555[palval & 0x3ffff];
			}
			else if (m_pal_mode == PAL_MODE_NEW_RGB12) // unknown newer VT mode
			{
				uint32_t palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

				// does grayscale mode exist here? (we haven't calculated any colours for it)
				//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				//  palval &= 0x30;

				// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
				palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

				*dest = m_vtpens_rgb444[palval & 0x7fff];
			}
			else // VT03 mode
			{
				uint32_t palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

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
			uint16_t palval = (m_palette_ram[pen & 0x7f] & 0x3f);

			if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				palval &= 0x30;

			// apply colour emphasis
			palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 1);

			*dest = pen_color(palval & 0x1ff);
		}
	}
}
void ppu_vt03_device::draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t*& dest)
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

uint8_t ppu_vt03_device::get_speva2_speva0()
{
	return m_extra_sprite_bits;
}


void ppu_vt03_device::extended_modes_enable_w(offs_t offset, uint8_t data)
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

void ppu_vt03_device::extended_modes2_enable_w(offs_t offset, uint8_t data) { m_extended_modes2_enable = data; }
void ppu_vt03_device::videobank0_0_w(offs_t offset, uint8_t data) { m_videobank0[0x0] = data; }
void ppu_vt03_device::videobank0_1_w(offs_t offset, uint8_t data) { m_videobank0[0x1] = data; }
void ppu_vt03_device::videobank0_2_w(offs_t offset, uint8_t data) { m_videobank0[0x2] = data; }
void ppu_vt03_device::videobank0_3_w(offs_t offset, uint8_t data) { m_videobank0[0x3] = data; }
void ppu_vt03_device::videobank0_4_w(offs_t offset, uint8_t data) { m_videobank0[0x4] = data; }
void ppu_vt03_device::videobank0_5_w(offs_t offset, uint8_t data) { m_videobank0[0x5] = data; }
void ppu_vt03_device::videobank1_w(offs_t offset, uint8_t data) { m_videobank1 = data; }
void ppu_vt03_device::gun_reset_w(offs_t offset, uint8_t data) { logerror("%s: gun_reset_w %02x\n", machine().describe_context(), data); }
void ppu_vt03_device::videobank0_extra_w(offs_t offset, uint8_t data) { m_videobank0_extra = data; }
/* 201b unused */
/* 201c read gun read x (older VT chipsets) */
/* 201d read gun read y (older VT chipsets) */
/* 201e read gun 2 read x (older VT chipsets) */
/* 201f read gun 2 read y (older VT chipsets) */


ppu_vt3xx_device::ppu_vt3xx_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_vt03_device(mconfig, PPU_VT3XX, tag, owner, clock)
{
}

void ppu_vt3xx_device::device_start()
{
	ppu_vt03_device::device_start();
	// TODO: move VT3xx specifics here
}

void ppu_vt3xx_device::device_reset()
{
	ppu_vt03_device::device_reset();
	// TODO: move VT3xx specifics here
}

uint8_t ppu_vt3xx_device::read_201c_newvid(offs_t offset) { return m_newvid_1c; }
uint8_t ppu_vt3xx_device::read_201d_newvid(offs_t offset) { return m_newvid_1d; }
uint8_t ppu_vt3xx_device::read_201e_newvid(offs_t offset) { return m_newvid_1e; }

void ppu_vt3xx_device::write_201c_newvid(offs_t offset, uint8_t data) { m_newvid_1c = data; logerror("%s: write_201c_newvid %02x\n", machine().describe_context(), data); }
void ppu_vt3xx_device::write_201d_newvid(offs_t offset, uint8_t data) { m_newvid_1d = data; logerror("%s: write_201d_newvid %02x\n", machine().describe_context(), data); }

void ppu_vt3xx_device::write_201e_newvid(offs_t offset, uint8_t data)
{
	/*
	 extended mode feature enables
	 ---- -s--
	 s = old/new sprite mode
	*/
	m_newvid_1e = data;
	logerror("%s: write_201e_newvid %02x\n", machine().describe_context(), data);
}

void ppu_vt3xx_device::read_tile_plane_data(int address, int color)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::read_tile_plane_data(address, color);
	}
	else
	{
		// format is no longer planar

		// old format 8 bits (1 byte) = 8 pixels of 1 plane (one line) of tile
		// +1 bytes = next row
		// +8 bytes = next plane
		// +16 byte = next tile (or +32 bytes in ROM in 4bpp mode, but we signal this by setting 0x2000)

		// new format
		// one byte = 4 planes, 2 pixels

		m_read_bg4_bg3 = color;

		m_whichpixel = 0;
		m_planebuf[0] = m_read_bg((address & 0x1fff));
		m_planebuf[1] = m_read_bg((address + 8) & 0x1fff);
		m_extplanebuf[0] = m_read_bg( ((address + 0) & 0x1fff) | 0x2000 );
		m_extplanebuf[1] = m_read_bg( ((address + 8) & 0x1fff) | 0x2000 );
		m_extplanebuf_vt3xx_0[0] = m_read_bg( ((address + 0) & 0x1fff) | 0x4000 );
		m_extplanebuf_vt3xx_0[1] = m_read_bg( ((address + 8) & 0x1fff) | 0x4000 );
		m_extplanebuf_vt3xx_1[0] = m_read_bg( ((address + 0) & 0x1fff) | 0x6000 );
		m_extplanebuf_vt3xx_1[1] = m_read_bg( ((address + 8) & 0x1fff) | 0x6000 );
	}
}

void ppu_vt3xx_device::draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t*& dest)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::draw_tile_pixel(pix, color, back_pen, dest);
	}
	else
	{
		// extended modes
		ppu_vt03_device::draw_tile_pixel(pix, color, back_pen, dest);
	}
}

void ppu_vt3xx_device::shift_tile_plane_data(uint8_t& pix)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::shift_tile_plane_data(pix);
	}
	else
	{
		// extended modes
		// 8x8x4 non-planar mode
		/*
		switch (m_whichpixel)
		{
		case 0: pix = (m_planebuf[0] >> 0) & 0xf; break;
		case 1: pix = (m_planebuf[0] >> 4) & 0xf; break;
		case 2: pix = (m_planebuf[1] >> 0) & 0xf; break;
		case 3: pix = (m_planebuf[1] >> 4) & 0xf; break;
		case 4: pix = (m_extplanebuf[0] >> 0) & 0xf; break;
		case 5: pix = (m_extplanebuf[0] >> 4) & 0xf; break;
		case 6: pix = (m_extplanebuf[1] >> 0) & 0xf; break;
		case 7: pix = (m_extplanebuf[1] >> 4) & 0xf; break;
		}
		*/
		// 8x8x8 non-planar mode
		switch (m_whichpixel)
		{
		case 0: pix = m_planebuf[0]; break;
		case 4: pix = m_planebuf[1]; break;
		case 1: pix = m_extplanebuf[0]; break;
		case 5: pix = m_extplanebuf[1]; break;
		case 2: pix = m_extplanebuf_vt3xx_0[0]; break;
		case 6: pix = m_extplanebuf_vt3xx_0[1]; break;
		case 3: pix = m_extplanebuf_vt3xx_1[0]; break;
		case 7: pix = m_extplanebuf_vt3xx_1[1]; break;
		}
		//pix = bitswap<8>(pix,  3, 2, 5, 4, 1,0, 7, 6);

		m_whichpixel++;
	}
}


void ppu_vt3xx_device::draw_back_pen(uint32_t* dst, int back_pen)
{
	if (!m_newvid_1e)
	{
		ppu_vt03_device::draw_back_pen(dst, back_pen);
	}
	else
	{
		// extended modes
		ppu_vt03_device::draw_back_pen(dst, back_pen);
	}
}
