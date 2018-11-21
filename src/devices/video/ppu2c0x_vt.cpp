// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation

    The VT video is based on the ppu2c0x but with enhanced capabilities such
    as 16 colour sprites.

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x_vt.h"

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */

// devices
DEFINE_DEVICE_TYPE(PPU_VT03, ppu_vt03_device, "ppu_vt03", "VT03 PPU")


ppu_vt03_device::ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppu2c0x_device(mconfig, PPU_VT03, tag, owner, clock),
	m_read_bg(*this),
	m_read_sp(*this)
{
	for(int i = 0; i < 6; i++)
		m_2012_2017_descramble[i] = 2 + i;
}

READ8_MEMBER(ppu_vt03_device::palette_read)
{
	if (m_201x_regs[0] & 0x80)
	{
		return m_newpal[offset];
	}
	else
	{
		return ppu2c0x_device::palette_read(space, offset);
	}
}

void ppu_vt03_device::set_new_pen(int i)
{
	if((i < 0x20) && ((i & 0x3) == 0)) {
		set_pen_color(i & 0x7f, rgb_t(0, 0, 0));
	} else {
		if(m_pal_mode == PAL_MODE_NEW_RGB) {
			uint16_t rgbval = (m_newpal[i&0x7f] & 0xff) | ((m_newpal[(i&0x7f)+0x80] & 0xff)<<8);
			uint8_t blue = (rgbval & 0x001f) << 3;
			uint8_t green = (rgbval & 0x3e0) >> 2;
			uint8_t red  = (rgbval & 0x7C00) >> 7;
			set_pen_color(i & 0x7f, rgb_t(red, green, blue));
		} else if(m_pal_mode == PAL_MODE_NEW_RGB12) {
			uint16_t rgbval = (m_newpal[i&0x7f] & 0x3f) | ((m_newpal[(i&0x7f)+0x80] & 0x3f)<<6);
			uint8_t red = (rgbval & 0x000f) << 4;
			uint8_t green = (rgbval & 0x0f0);
			uint8_t blue  = (rgbval & 0xf00) >> 4;
			set_pen_color(i & 0x7f, rgb_t(red, green, blue));
		} else {
			// Credit to NewRisingSun
			uint16_t palval = (m_newpal[i&0x7f] & 0x3f) | ((m_newpal[(i&0x7f)+0x80] & 0x3f)<<6);
			int nPhase  = (palval >>0) &0xF;
			int nLuma   = (palval >>4) &0xF;
			int nChroma = (palval >>8) &0xF;
			float phaseOffset = -11.0;
			//bool inverted = false;
			if ((nLuma < (nChroma+1) >>1 || nLuma > 15 -(nChroma >>1)) && (m_pal_mode != PAL_MODE_NEW_VG)) {
				//inverted = true;
				// Strange color number wrap-around. Is this for protection reasons, or a bug of the original hardware?
				// The VT03 data sheet advises programmers that 4 <= nLuma*2 +nChroma <= 0x1F, which does not correspond exactly to this condition.
				static const unsigned char altPhases[16] = {  13,  7,  8,  9, 10, 11, 12,  1,  2,  3,  4,  5,  6,  0, 14, 15};
				static const float    altPhaseOffset[16] = {  -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,  0, -5, -5, -5}; // Slight tweak in phase 6 for Z-Dog
				phaseOffset += altPhaseOffset[nPhase]; // These "alternative" colors seem to be slightly shifted in addition to being wrapped-around, at least in EmuVT.
				nPhase = altPhases[nPhase];
				nChroma = 16 -nChroma;
				nLuma = (nLuma -8) &0xF;
			}
			float fLuma   = (nLuma-4) /9.625;     // Value determined from matching saturation =0 phases 1-12
			float fChroma = nChroma /18.975;      // Value determined from matching phases 0 and 13 across all luminance and saturation levels
			float fPhase  = ((nPhase-2) *30.0 +phaseOffset) *M_PI /180.0;
			if(m_pal_mode == PAL_MODE_NEW_VG) {
				if(fPhase > 0 && fPhase < 13) {
					fLuma /= 1.5;
					fChroma /= 2;
				}
			}
			float Y = fLuma;
			float C = fChroma;
			if (nPhase == 0 || nPhase >12) C =0.0;// Phases 0 and 13-15 are grays
			if (nPhase == 0) Y += fChroma;        // Phase 0 is the upper bound of the waveform
			if (nPhase ==13) Y -= fChroma;        // Phase 13 is the lower bound of the waveform
			if (nPhase >=14) Y = 0.0;             // Phases 14 and 15 always black

			float V = sin(fPhase) *C *1.05; // 1.05 needed to get closer to EmuVT palette's color levels in phases 1-12
			float U = cos(fPhase) *C *1.05;
			float R = Y + 1.1400*V + 0.0000*U;
			float G = Y - 0.5807*V - 0.3940*U;
			float B = Y - 0.0000*V + 2.0290*U;
			if (R <0.0) R =0.0;
			if (R >1.0) R =1.0;
			if (G <0.0) G =0.0;
			if (G >1.0) G =1.0;
			if (B <0.0) B =0.0;
			if (B >1.0) B =1.0;
			int RV = R *255.0;
			int GV = G *255.0;
			int BV = B *255.0;

			set_pen_color(i & 0x7f, rgb_t(RV, GV ,BV));
		}
	}


}

WRITE8_MEMBER(ppu_vt03_device::palette_write)
{
	//logerror("pal write %d %02x\n", offset, data);
	// why is the check pal_mask = (m_pal_mode == PAL_MODE_NEW_VG) ? 0x08 : 0x80 in set_2010_reg and 0x04 : 0x80 here?
	uint8_t pal_mask = (m_pal_mode == PAL_MODE_NEW_VG) ? 0x04 : 0x80;

	if (m_201x_regs[0] & pal_mask)
	{
		m_newpal[offset&0xff] = data;
		set_new_pen(offset);
	}
	else
	{
		//if(m_pal_mode == PAL_MODE_NEW_VG) // ddrdismx writes the palette before setting the register but doesn't use 'PAL_MODE_NEW_VG', Konami logo is missing if you don't allow writes to be stored for when we switch
		m_newpal[offset&0xff] = data;
		ppu2c0x_device::palette_write(space, offset, data);
	}
}

READ8_MEMBER( ppu_vt03_device::read )
{
	 if(offset <= 0xf) {
		 return ppu2c0x_device::read(space,offset);
	 } else if(offset <= 0x1f) {
		 return get_201x_reg(offset & 0x0f);
	 } else {
		 return 0;
	 }
}

void ppu_vt03_device::init_palette()
{
	// todo, work out the format of the 12 palette bits instead of just calling the main init
	ppu2c0x_device::init_palette(true);
}

void ppu_vt03_device::device_start()
{
	ppu2c0x_device::device_start();

	m_newpal = std::make_unique<uint8_t[]>(0x100);
	save_pointer(NAME(m_newpal), 0x100);

	save_item(NAME(m_201x_regs));
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

	m_read_bg.resolve_safe(0);
	m_read_sp.resolve_safe(0);
	for (int i = 0;i < 0xff;i++)
		m_newpal[i] = 0x0;

	// todo: what are the actual defaults for these?
	for (int i = 0;i < 0x20;i++)
		set_201x_reg(i, 0x00);

	//m_201x_regs[0] = 0x86; // alt fix for ddrdismx would be to set the default palette mode here

	init_palette();

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

void ppu_vt03_device::make_sprite_pixel_data(uint8_t &pixel_data, int flipx)
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

			if(is16pix) {
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

void ppu_vt03_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap)
{
	int is4bpp = get_201x_reg(0x0) & 0x04;
	int is16pix = get_201x_reg(0x0) & 0x01;

	if (is4bpp)
	{
		if (!is16pix)
		{
			bitmap.pix32(m_scanline, sprite_xpos + pixel) = pen(pixel_data + (4 * color));
		}
		else
		{
			/* this mode makes use of the extra planes to increase sprite width instead
			   we probably need to split them out again and draw them at xpos+8 with a
			   cliprect - not seen used yet */
			if((pixel_data & 0x03) != 0)
				bitmap.pix32(m_scanline, sprite_xpos + pixel) = pen((pixel_data & 0x03) + (4 * color));
			if(((pixel_data >> 5) & 0x03) != 0)
				bitmap.pix32(m_scanline, sprite_xpos + pixel + 8) = pen(((pixel_data >> 5) & 0x03) + (4 * color));
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

void ppu_vt03_device::shift_tile_plane_data(uint8_t &pix)
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

void ppu_vt03_device::draw_tile_pixel(uint8_t pix, int color, pen_t back_pen, uint32_t *&dest, const pen_t *color_table)
{
	int is4bpp = get_201x_reg(0x0) & 0x02;

	if (!is4bpp)
	{
		ppu2c0x_device::draw_tile_pixel(pix, color, back_pen, dest, color_table);
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
			pen = 0; // fixme backpen logic probably differs on vt03 due to extra colours
		}
		*dest = this->pen(pen);
	}
}

void ppu_vt03_device::read_extra_sprite_bits(int sprite_index)
{
	m_extra_sprite_bits = (m_spriteram[sprite_index + 2] & 0x1c) >>2;
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
	uint8_t pal_mask = (m_pal_mode == PAL_MODE_NEW_VG) ? 0x08 : 0x80;
	if ((m_201x_regs[0x0] & pal_mask) != (data & pal_mask))
	{
		if (data & pal_mask)
		{
			for (int i = 0;i < 256;i++)
			{
				set_new_pen(i);
			}
		}
		else
		{
			for (int i = 0;i < 256;i++)
			{
				set_pen_indirect(i, i);
			}
		}
	}

	m_201x_regs[0x0] = data;
}

WRITE8_MEMBER(ppu_vt03_device::write)
{
	if (offset < 0x10)
	{
		ppu2c0x_device::write(space, offset, data);
	}
	else
	{
		logerror("%s: write to reg 0x20%02x %02x\n", machine().describe_context(), offset, data);
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
}
