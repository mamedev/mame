// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x_sh6578.h"

#define LOG_PPU_EXTRA       (1U << 0)

//#define VERBOSE             (LOG_PPU_EXTRA)
#define VERBOSE             (0)

#include "logmacro.h"

// devices
DEFINE_DEVICE_TYPE(PPU_SH6578, ppu_sh6578_device, "ppu_sh6578", "SH6578 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device, "ppu_sh6578pal", "SH6578 PPU (PAL)")

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu_sh6578_device::ppu_internal_map), this))
{
	m_paletteram_in_ppuspace = false;
	m_line_write_increment_large = 64;
}

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_sh6578_device(mconfig, PPU_SH6578, tag, owner, clock)
{
}

ppu_sh6578pal_device::ppu_sh6578pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_sh6578_device(mconfig, PPU_SH6578PAL, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
}


// Palette RAM is _NOT_ mapped to PPU space here
void ppu_sh6578_device::ppu_internal_map(address_map& map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x27ff).ram();
	map(0x2800, 0x7fff).nopr();

	map(0x8000, 0xffff).ram(); // machine specific?
}

void ppu_sh6578_device::device_start()
{
	ppu2c0x_device::device_start();

	m_palette_ram.resize(0x40);

	for (int i = 0; i < 0x40; i++)
		m_palette_ram[i] = 0x00;
}

void ppu_sh6578_device::device_reset()
{
	ppu2c0x_device::device_reset();
	m_colsel_pntstart = 0x00;
}

void ppu_sh6578_device::scanline_increment_fine_ycounter()
{
	/* increment the fine y-scroll */
	m_refresh_data += 0x1000;

	/* if it's rolled, increment the coarse y-scroll */
	if (m_refresh_data & 0x8000)
	{
		uint16_t tmp;
		tmp = (m_refresh_data & 0x03e0) + 0x20;
		m_refresh_data &= 0x7c1f;

		if (tmp == 0x0400)
			m_refresh_data ^= 0x0800;
		else
			m_refresh_data |= (tmp & 0x03e0);

		//logerror("updating refresh_data: %04x\n", m_refresh_data);
	}
}


void ppu_sh6578_device::read_tile_plane_data(int address, int color)
{
	m_planebuf[0] = readbyte(address);
	m_planebuf[1] = readbyte(address + 8);

	m_extplanebuf[0] = readbyte(address + 16);
	m_extplanebuf[1] = readbyte(address + 24);
}

void ppu_sh6578_device::draw_tile(uint8_t* line_priority, int color_byte, int color_bits, int address, int start_x, pen_t back_pen, uint32_t*& dest, const pen_t* color_table)
{
	int color = color_byte & 0x03;

	read_tile_plane_data(address, color);

	/* render the pixel */
	for (int i = 0; i < 8; i++)
	{
		uint8_t pix;
		shift_tile_plane_data(pix);

		if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
		{
			pen_t pen;

			if (pix)
			{
				const pen_t* paldata = &color_table[4 * color];
				pen = this->pen(paldata[pix]);
			}
			else
			{
				pen = back_pen;
			}

			*dest = pen;		

			// priority marking
			if (pix)
				line_priority[start_x + i] |= 0x02;
		}
		dest++;
	}
}

void ppu_sh6578_device::draw_background(uint8_t* line_priority)
{
	bitmap_rgb32& bitmap = *m_bitmap;

	uint8_t color_mask;
	const pen_t* color_table;

	/* setup the color mask and colortable to use */
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
	{
		color_mask = 0xf0;
		color_table = m_colortable_mono.get();
	}
	else
	{
		color_mask = 0xff;
		color_table = m_colortable.get();
	}

	/* cache the background pen */
	pen_t back_pen = pen(m_back_color & color_mask);

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	uint8_t  scroll_x_coarse = m_refresh_data & 0x001f;
	uint8_t  scroll_y_coarse = (m_refresh_data & 0x03e0) >> 5;
	uint16_t nametable = (m_refresh_data & 0x0c00);
	uint8_t  scroll_y_fine = (m_refresh_data & 0x7000) >> 12;

	int x = scroll_x_coarse;

	/* get the tile index */
	int tile_index = (nametable<<1) + scroll_y_coarse * 64;

	/* set up dest */
	int start_x = (m_x_fine ^ 0x07) - 7;
	uint32_t* dest = &bitmap.pix32(m_scanline, start_x);

	m_tilecount = 0;

	/* draw the 32 or 33 tiles that make up a line */
	while (m_tilecount < 34)
	{
		int color_byte;
		int index1;
		int page2, address;

		index1 = tile_index + (x << 1);

		if (m_colsel_pntstart & 1)
		{
			index1 &= 0x7ff;
			index1 += 0x2000;
		}
		else
		{
			index1 &= 0x1fff;
		}

		// page2 is the output of the nametable read (this section is the FIRST read per tile!)
		page2 = readbyte(index1);

		/* Figure out which byte in the color table to use */
		color_byte = readbyte(index1 + 1);

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			address = ((page2 | (color_byte<<8)) & 0x0fff) << 4;
			// plus something that accounts for y
			address += scroll_y_fine;

			draw_tile(line_priority, (color_byte >> 12) & 0xf, 0, address, start_x, back_pen, dest, color_table);

			start_x += 8;

			/* move to next tile over and toggle the horizontal name table if necessary */
			x++;
			if (x > 31)
			{
				x = 0;
				tile_index ^= 0x800;
			}
		}
		m_tilecount++;
	}

	/* if the left 8 pixels for the background are off, blank 'em */
	if (!(m_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8))
	{
		dest = &bitmap.pix32(m_scanline);
		for (int i = 0; i < 8; i++)
		{
			*(dest++) = back_pen;
			line_priority[i] ^= 0x02;
		}
	}
}

void ppu_sh6578_device::draw_sprites(uint8_t* line_priority)
{

}

void ppu_sh6578_device::write(offs_t offset, uint8_t data)
{
	if (offset < 0x8)
	{
		ppu2c0x_device::write(offset, data);
	}
	else if (offset == 0x8)
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::write : Color Select & PNT Start Address : %02x\n", machine().describe_context(), data);
		m_colsel_pntstart = data;
	}
	else
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::write : unhandled offset %02x : %02x\n", machine().describe_context(), offset, data);
	}
}


uint8_t ppu_sh6578_device::read(offs_t offset)
{
	if (offset < 0x8)
	{
		return ppu2c0x_device::read(offset);
	}
	else if (offset == 0x8)
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::read : Color Select & PNT Start Address\n", machine().describe_context());
		return m_colsel_pntstart;
	}
	else
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::read : unhandled offset %02x\n", machine().describe_context(), offset);
		return 0x00;
	}
}

void ppu_sh6578_device::palette_write(offs_t offset, uint8_t data)
{
	m_palette_ram[offset] = data;
}

uint8_t ppu_sh6578_device::palette_read(offs_t offset)
{
	return m_palette_ram[offset];
}
