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

void ppu_vt03_device::set_201x_descramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5)
{
	m_2012_2017_descramble[0] = reg0; // TOOD: name regs
	m_2012_2017_descramble[1] = reg1;
	m_2012_2017_descramble[2] = reg2;
	m_2012_2017_descramble[3] = reg3;
	m_2012_2017_descramble[4] = reg4;
	m_2012_2017_descramble[5] = reg5;
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


uint8_t ppu_vt03_device::read_extended(offs_t offset)
{
	offset += 0x10;
	logerror("%s: read from extended PPU reg %02x\n", machine().describe_context(), offset);

	switch (offset)
	{
	case 0x10:
		return m_201x_regs[0x0];

	case 0x11:
		return m_201x_regs[0x1];

	case 0x12:
		return m_201x_regs[m_2012_2017_descramble[0]];

	case 0x13:
		return m_201x_regs[m_2012_2017_descramble[1]];

	case 0x14:
		return m_201x_regs[m_2012_2017_descramble[2]];

	case 0x15:
		return m_201x_regs[m_2012_2017_descramble[3]];

	case 0x16:
		return m_201x_regs[m_2012_2017_descramble[4]];

	case 0x17:
		return m_201x_regs[m_2012_2017_descramble[5]];

	case 0x18:
		return m_201x_regs[0x8];

	case 0x19:
		return 0x00;

	case 0x1a:
		return m_201x_regs[0xa];

	case 0x1b:
		return 0x00;

	case 0x1c:
		return 0x00;

	case 0x1d:
		return 0x00;

	case 0x1e:
		return 0x00;

	case 0x1f:
		return 0x00;
	}

	return 0x00;
}



void ppu_vt03_device::init_vtxx_rgb555_palette_tables()
{
	int entry = 0;
	for (int emp = 0; emp < 8; emp++)
	{
		for (int palval = 0; palval < 0x8000; palval++)
		{
		//  uint16_t rgbval = (m_palette_ram[i & 0x7f] & 0xff) | ((m_palette_ram[(i & 0x7f) + 0x80] & 0xff) << 8);
			uint8_t blue = (palval & 0x001f) << 3;
			uint8_t green = (palval & 0x3e0) >> 2;
			uint8_t red = (palval & 0x7C00) >> 7;

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
			uint8_t red = (palval & 0x000f) << 4;
			uint8_t green = (palval & 0x0f0);
			uint8_t blue = (palval & 0xf00) >> 4;

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
			float fPhase = ((nPhase - 2) * 30.0 + phaseOffset) * M_PI / 180.0;

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

			float V = sin(fPhase) * C * 1.05; // 1.05 needed to get closer to EmuVT palette's color levels in phases 1-12
			float U = cos(fPhase) * C * 1.05;
			float R = Y + 1.1400 * V + 0.0000 * U;
			float G = Y - 0.5807 * V - 0.3940 * U;
			float B = Y - 0.0000 * V + 2.0290 * U;
			if (R < 0.0) R = 0.0;
			if (R > 1.0) R = 1.0;
			if (G < 0.0) G = 0.0;
			if (G > 1.0) G = 1.0;
			if (B < 0.0) B = 0.0;
			if (B > 1.0) B = 1.0;
			int RV = R * 255.0;
			int GV = G * 255.0;
			int BV = B * 255.0;

			// does this really apply to the VT palette?
			//bool is_pal = m_scanlines_per_frame != NTSC_SCANLINES_PER_FRAME;
			//apply_color_emphasis_and_clamp(is_pal, color_emphasis, R, G, B);

			m_vtpens[entry] = rgb_t(RV, GV, BV);
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
	save_item(NAME(m_201x_regs));

	init_vt03_palette_tables(0);
	init_vtxx_rgb555_palette_tables();
	init_vtxx_rgb444_palette_tables();
}

uint8_t ppu_vt03_device::get_201x_reg(int reg)
{
	//logerror(" getting reg %d is %02x ", reg, m_201x_regs[reg]);

	return m_201x_regs[reg];
}

void ppu_vt03_device::set_201x_reg(int reg, uint8_t data)
{
	m_201x_regs[reg] = data;
}

void ppu_vt03_device::device_reset()
{
	ppu2c0x_device::device_reset();

	for (int i = 0; i < 0xff; i++)
		m_palette_ram[i] = 0x0;

	// todo: what are the actual defaults for these?
	for (int i = 0; i < 0x20; i++)
		set_201x_reg(i, 0x00);

	m_read_bg4_bg3 = 0;
	m_va34 = 0;
}


uint8_t ppu_vt03_device::get_m_read_bg4_bg3()
{
	return m_read_bg4_bg3;
}

uint8_t ppu_vt03_device::get_va34()
{
	return m_va34;
}


void ppu_vt03_device::read_sprite_plane_data(int address)
{
	m_va34 = 0;
	m_planebuf[0] = m_read_sp((address + 0) & 0x1fff);
	m_planebuf[1] = m_read_sp((address + 8) & 0x1fff);

	int is4bpp = get_201x_reg(0x0) & 0x04;

	if (is4bpp)
	{
		m_va34 = 1;
		m_extplanebuf[0] = m_read_sp((address + 0) & 0x1fff);
		m_extplanebuf[1] = m_read_sp((address + 8) & 0x1fff);
	}
}

void ppu_vt03_device::make_sprite_pixel_data(uint8_t& pixel_data, int flipx)
{
	ppu2c0x_device::make_sprite_pixel_data(pixel_data, flipx);

	int is4bpp = get_201x_reg(0x0) & 0x04;
	int is16pix = get_201x_reg(0x0) & 0x01;

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
				uint8_t pix0 = pixel_data & 0x03;
				uint8_t pix1 = (pixel_data >> 5) & 0x03;
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
	int is4bpp = get_201x_reg(0x0) & 0x04;
	int is16pix = get_201x_reg(0x0) & 0x01;

	if (is4bpp)
	{
		if (!is16pix)
		{
			uint8_t pen = pixel_data + (4 * color);
			draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
		}
		else
		{
			/* this mode makes use of the extra planes to increase sprite width instead
			    we probably need to split them out again and draw them at xpos+8 with a
			    cliprect - not seen used yet */
			if ((pixel_data & 0x03) != 0)
			{
				uint8_t pen = (pixel_data & 0x03) + (4 * color);
				draw_tile_pixel_inner(pen, &bitmap.pix(m_scanline, sprite_xpos + pixel));
			}

			if (((pixel_data >> 5) & 0x03) != 0)
			{
				uint8_t pen = ((pixel_data >> 5) & 0x03) + (4 * color);
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
	int is4bpp = get_201x_reg(0x0) & 0x02;

	if (m_201x_regs[0] & 0x10) // extended mode
		m_read_bg4_bg3 = color;
	else
		m_read_bg4_bg3 = 0;

	if (is4bpp)
	{
		m_va34 = 0;
		m_planebuf[0] = m_read_bg((address & 0x1fff));
		m_planebuf[1] = m_read_bg((address + 8) & 0x1fff);
		m_va34 = 1;
		m_extplanebuf[0] = m_read_bg((address & 0x1fff));
		m_extplanebuf[1] = m_read_bg((address + 8) & 0x1fff);
	}
	else
	{
		m_va34 = 0;
		m_planebuf[0] = m_read_bg((address & 0x1fff));
		m_planebuf[1] = m_read_bg((address + 8) & 0x1fff);
	}
}

void ppu_vt03_device::shift_tile_plane_data(uint8_t& pix)
{
	int is4bpp = get_201x_reg(0x0) & 0x02;

	ppu2c0x_device::shift_tile_plane_data(pix);

	if (is4bpp)
	{
		pix |= (((m_extplanebuf[0] >> 7) & 1) << 5) | (((m_extplanebuf[1] >> 7) & 1) << 6); // yes, shift by 5 and 6 because of the way the palette is arranged in RAM
		m_extplanebuf[0] = m_extplanebuf[0] << 1;
		m_extplanebuf[1] = m_extplanebuf[1] << 1;
	}
}


void ppu_vt03_device::draw_back_pen(uint32_t* dst, int back_pen)
{
	if (m_201x_regs[0] & 0x80)
	{
		// is the back_pen always just pen 0 in VT modes? (using last data written to a transparent pen as per NES logic doesn't work as writes are split across 2 bytes)
		draw_tile_pixel_inner(0, dst);
	}
	else
	{
		// in normal modes we still have the data from the palette writes as the 'backpen' so treat it as before
		uint32_t pix;
		pix = m_nespens[back_pen & 0x1ff];
		*dst = pix;
	}
}


void ppu_vt03_device::draw_tile_pixel_inner(uint8_t pen, uint32_t *dest)
{
	if (m_201x_regs[0] & 0x80)
	{
		if (m_pal_mode == PAL_MODE_NEW_RGB) // unknown newer VT mode
		{
			uint32_t palval;
			palval = (m_palette_ram[pen & 0x7f] & 0xff) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x7f) << 8);

			// does grayscale mode exist here? (we haven't calculated any colours for it)
			//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			//  palval &= 0x30;

			// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
			palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 10);

			uint32_t pix;
			pix = m_vtpens_rgb555[palval & 0x3ffff];
			*dest = pix;
		}
		else if (m_pal_mode == PAL_MODE_NEW_RGB12) // unknown newer VT mode
		{
			uint32_t palval;
			palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

			// does grayscale mode exist here? (we haven't calculated any colours for it)
			//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			//  palval &= 0x30;

			// apply colour emphasis (does it really exist here?) (we haven't calculated any colours for it, so ths has no effect)
			palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

			uint32_t pix;
			pix = m_vtpens_rgb444[palval & 0x7fff];
			*dest = pix;
		}
		else // VT03 mode
		{
			uint32_t palval;
			palval = (m_palette_ram[pen & 0x7f] & 0x3f) | ((m_palette_ram[(pen & 0x7f) + 0x80] & 0x3f) << 6);

			// does grayscale mode exist here? (we haven't calculated any colours for it)
			//if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			//  palval &= 0x30;

			// apply colour emphasis (does it really exist here?) (we calculate values for it when building the palette lookup)
			palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 7);

			uint32_t pix;
			pix = m_vtpens[palval  & 0x7fff];
			*dest = pix;
		}
	}
	else // old colour compatible mode
	{
		uint16_t palval;
		palval = (m_palette_ram[pen & 0x7f] & 0x3f);

		if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			palval &= 0x30;

		// apply colour emphasis
		palval |= ((m_regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) << 1);

		uint32_t pix;
		pix = m_nespens[palval & 0x1ff];
		*dest = pix;
	}
}
void ppu_vt03_device::draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t*& dest)
{
	int is4bpp = get_201x_reg(0x0) & 0x02;

	if (!is4bpp)
	{
		ppu2c0x_device::draw_tile_pixel(pix, color, back_pen, dest);
	}
	else
	{
		int basepen;
		int pen;

		if (m_201x_regs[0] & 0x10) // extended mode
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

void ppu_vt03_device::read_extra_sprite_bits(int sprite_index)
{
	m_extra_sprite_bits = (m_spriteram[sprite_index + 2] & 0x1c) >> 2;
}

uint8_t ppu_vt03_device::get_speva2_speva0()
{
	return m_extra_sprite_bits;
}

void ppu_vt03_device::set_2010_reg(uint8_t data)
{
	/*  7   : COLCOMP
	    6   : UNUSED (8bpp enable on VT09?)
	    5   : UNUSED
	    4   : BKEXTEN
	    3   : SPEXTEN
	    2   : SP16EN
	    1   : BK16EN
	    0   : PIX16EN */

	m_201x_regs[0x0] = data;
}

void ppu_vt03_device::write_extended(offs_t offset, uint8_t data)
{
	offset += 0x10;
	logerror("%s: write to extended PPU reg 0x20%02x %02x\n", machine().describe_context(), offset, data);
	switch (offset)
	{
	case 0x10:
		set_2010_reg(data);
		break;

	case 0x11:
		m_201x_regs[0x1] = data;
		break;

	case 0x12:
		m_201x_regs[m_2012_2017_descramble[0]] = data;
		break;

	case 0x13:
		m_201x_regs[m_2012_2017_descramble[1]] = data;
		break;

	case 0x14:
		m_201x_regs[m_2012_2017_descramble[2]] = data;
		break;

	case 0x15:
		m_201x_regs[m_2012_2017_descramble[3]] = data;
		break;

	case 0x16:
		m_201x_regs[m_2012_2017_descramble[4]] = data;
		break;

	case 0x17:
		logerror("set reg 7 %02x\n", data);
		m_201x_regs[m_2012_2017_descramble[5]] = data;
		break;

	case 0x18:
		logerror("set reg 8 %02x\n", data);
		m_201x_regs[0x8] = data;
		break;

	case 0x19:
		// reset gun port (value doesn't matter)
		break;

	case 0x1a:
		m_201x_regs[0xa] = data;
		break;

	case 0x1b:
		// unused
		break;

	case 0x1c:
		// (READ) x-coordinate of gun
		break;

	case 0x1d:
		// (READ) y-coordinate of gun
		break;

	case 0x1e:
		// (READ) x-coordinate of gun 2
		break;

	case 0x1f:
		// (READ) y-coordinate of gun 2
		break;
	}
}


