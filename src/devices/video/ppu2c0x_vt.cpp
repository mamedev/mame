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
	if(m_pal_mode == PAL_MODE_NEW_RGB) {
		uint16_t rgbval = (m_newpal[i&0x7f] & 0xff) | ((m_newpal[(i&0x7f)+0x80] & 0xff)<<8);
		uint8_t blue = (rgbval & 0x001f) << 3;
		uint8_t green = (rgbval & 0x3e0) >> 2;
		uint8_t red  = (rgbval & 0x7C00) >> 7;
		m_palette->set_pen_color(i & 0x7f, rgb_t(red, green, blue));
	} else {
		// TODO: should this be tidied up?
		uint16_t palval = (m_newpal[i&0x7f] & 0x3f) | ((m_newpal[(i&0x7f)+0x80] & 0x3f)<<6);

		uint8_t rhue = palval & 0x0F;
		uint8_t rlum = (palval >> 4) & 0x0F;
		uint8_t rsat = (palval >> 8) & 0x0F;
		//See http://wiki.nesdev.com/w/index.php/VT03%2B_Enhanced_Palette#Inverted_extended_color_numbers
		bool inverted = (rlum < ((rsat + 1) >> 1) || rlum > (15 - (rsat >>1)));
		if(inverted && (m_pal_mode != PAL_MODE_NEW_VG)) {
			rsat = 16 - rsat;
			rlum = (rlum - 8) & 0x0F;
			uint8_t hue_lut[16] = {0xD, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x0, 0xE, 0xF};
			rhue = hue_lut[rhue];
		}

		// Get base color
		double hue = 287.0;

		double Kr = 0.2989;
		double Kb = 0.1145;
		double Ku = 2.029;
		double Kv = 1.140;

		double sat;
		double y, u, v;
		double rad;
		int R, G, B;
		switch (rhue)
		{
		case 0:
			sat = 0; rad = 0;
			y = 1.0;
			break;

		case 13:
			sat = 0; rad = 0;
			y = 0.77;
			break;

		case 14:
		case 15:
			sat = 0; rad = 0; y = 0;
			break;

		default:
			sat = (m_pal_mode == PAL_MODE_NEW_VG) ? 0.5 : 0.7;
			rad = M_PI * ((rhue * 30 + hue) / 180.0);
			y = (m_pal_mode == PAL_MODE_NEW_VG) ? 0.4 : 0.9;
			break;
		}

		sat *= (rsat / 15.0);
		y *= (rlum / 15.0);
		u = sat * cos(rad);
		v = sat * sin(rad);

		/* Transform to RGB */
		R = (y + Kv * v) * 255.0;
		G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
		B = (y + Ku * u) * 255.0;

		/* Clipping, in case of saturation */
		if (R < 0)
			R = 0;
		if (R > 255)
			R = 255;
		if (G < 0)
			G = 0;
		if (G > 255)
			G = 255;
		if (B < 0)
			B = 0;
		if (B > 255)
			B = 255;

		m_palette->set_pen_color(i & 0x7f, rgb_t(R, G ,B));
	}

}

WRITE8_MEMBER(ppu_vt03_device::palette_write)
{
	//logerror("pal write %d %02x\n", offset, data);
	uint8_t pal_mask = (m_pal_mode == PAL_MODE_NEW_VG) ? 0x04 : 0x80;

	if (m_201x_regs[0] & pal_mask)
	{
		m_newpal[offset] = data;
		set_new_pen(offset);
	}
	else
	{
		if(m_pal_mode == PAL_MODE_NEW_VG)
			m_newpal[offset] = data;
		ppu2c0x_device::palette_write(space, offset, data);
	}
}

READ8_MEMBER( ppu_vt03_device::read )
{
	 return ppu2c0x_device::read(space,offset);
}

void ppu_vt03_device::init_palette(palette_device &palette, int first_entry)
{
	// todo, work out the format of the 12 palette bits instead of just calling the main init
	m_palette = &palette;
	ppu2c0x_device::init_palette(palette, first_entry, true);
}

void ppu_vt03_device::device_start()
{
	ppu2c0x_device::device_start();

	m_newpal = std::make_unique<uint8_t[]>(0x100);
	save_pointer(&m_newpal[0], "m_newpal", 0x100);

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
	set_2010_reg(0x00);
	set_2010_reg(0x80);
	set_2010_reg(0x00);

	// todo: what are the actual defaults for these?
	for (int i = 0;i < 0x20;i++)
		set_201x_reg(i, 0x00);

	init_palette(*m_palette, 0);
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

	if (is4bpp)
	{
		if (flipx)
		{
			// yes, shift by 5 and 6 because of the way the palette is arranged in RAM
			pixel_data |= (((m_extplanebuf[0] & 1) << 5) | ((m_extplanebuf[1] & 1) << 6));
			m_extplanebuf[0] = m_extplanebuf[0] >> 1;
			m_extplanebuf[1] = m_extplanebuf[1] >> 1;
		}
		else
		{
			pixel_data |= (((m_extplanebuf[0] >> 7) & 1) << 5) | (((m_extplanebuf[1] >> 7) & 1) << 6);
			m_extplanebuf[0] = m_extplanebuf[0] << 1;
			m_extplanebuf[1] = m_extplanebuf[1] << 1;
		}
	}
}

void ppu_vt03_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_ind16& bitmap)
{
	int is4bpp = get_201x_reg(0x0) & 0x04;
	int is16pix = get_201x_reg(0x0) & 0x01;

	if (is4bpp)
	{
		if (!is16pix)
		{
			bitmap.pix16(m_scanline, sprite_xpos + pixel) = pixel_data + (4 * color);
		}
		else
		{
			/* this mode makes use of the extra planes to increase sprite width instead
			   we probably need to split them out again and draw them at xpos+8 with a
			   cliprect - not seen used yet */
			bitmap.pix16(m_scanline, sprite_xpos + pixel) = pixel_data + (machine().rand()&3);
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

void ppu_vt03_device::draw_tile_pixel(uint8_t pix, int color, uint16_t back_pen, uint16_t *&dest, const pen_t *color_table)
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
			pen = back_pen; // fixme backpen logic probably differs on vt03 due to extra colours
		}
		*dest = pen;
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
				m_palette->set_pen_indirect(i, i);
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
			break;

		case 0x12:
			m_201x_regs[0x2] = data;
			break;

		case 0x13:
			m_201x_regs[0x3] = data;
			break;

		case 0x14:
			m_201x_regs[0x4] = data;
			break;

		case 0x15:
			m_201x_regs[0x5] = data;
			break;

		case 0x16:
			m_201x_regs[0x6] = data;
			break;

		case 0x17:
			logerror("set reg 7 %02x\n", data);
			m_201x_regs[0x7] = data;
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
