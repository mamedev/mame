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
	uint16_t palval = (m_newpal[i&0x7f] & 0x3f) | ((m_newpal[(i&0x7f)+0x80] & 0x3f)<<6);
	
	// &0x3f so we don't attempt to use any of the extended colours right now because
	// I haven't managed to work out the format
	m_palette->set_pen_indirect(i&0x7f,palval&0x3f);
}

WRITE8_MEMBER(ppu_vt03_device::palette_write)
{
	if (m_201x_regs[0] & 0x80)
	{
		m_newpal[offset] = data;
		set_new_pen(offset);
	}
	else
	{
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

	set_2010_reg(0x00);

	// todo: what are the actual defaults for these?
	for (int i = 0;i < 0x20;i++)
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
	m_va34 = 1;
	m_extplanebuf[0] = m_read_sp((address + 0) & 0x1fff);
	m_extplanebuf[1] = m_read_sp((address + 8) & 0x1fff);
}

void ppu_vt03_device::make_sprite_pixel_data(uint8_t &pixel_data, int flipx)
{
	ppu2c0x_device::make_sprite_pixel_data(pixel_data, flipx);

	if (flipx)
	{
		pixel_data |= (((m_extplanebuf[0] & 1) << 5) | ((m_extplanebuf[1] & 1) << 6)); // yes, shift by 5 and 6 because of the way the palette is arranged in RAM
		m_extplanebuf[0] = m_extplanebuf[0] >> 1;
		m_extplanebuf[1] = m_extplanebuf[1] >> 1;
	}
	else
	{
		pixel_data |= (((m_extplanebuf[0] >> 7) & 1) << 5) | (((m_extplanebuf[1] >> 7) & 1) << 6); // yes, shift by 5 and 6 because of the way the palette is arranged in RAM
		m_extplanebuf[0] = m_extplanebuf[0] << 1;
		m_extplanebuf[1] = m_extplanebuf[1] << 1;
	}
}

void ppu_vt03_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_ind16& bitmap)
{
	bitmap.pix16(m_scanline, sprite_xpos + pixel) = pixel_data + (4 * color);
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

	if ((m_201x_regs[0x0] & 0x80) != (data & 0x80))
	{
		if (data & 0x80)
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
		switch (offset)
		{
		case 0x10:
			logerror("%s: write to reg 0x2010 %02x\n", machine().describe_context(), data);
			set_2010_reg(data);
			break;

		case 0x11:
			logerror("%s: write to reg 0x2011 %02x\n", machine().describe_context(), data);
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