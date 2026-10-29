// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#include "emu.h"
#include "ppu2c0x_sh6578.h"

#define LOG_PPU_EXTRA       (1U << 1)

//#define VERBOSE             (LOG_PPU_EXTRA)

#include "logmacro.h"

// devices
DEFINE_DEVICE_TYPE(PPU_SH6578, ppu_sh6578_device, "ppu_sh6578", "SH6578 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device, "ppu_sh6578pal", "SH6578 PPU (PAL)")

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu_sh6578_device::ppu_internal_map), this))
{
	m_paletteram_in_ppuspace = false;
	m_line_write_increment_large = 32;
	m_videoram_addr_mask = 0xffff;
	m_global_refresh_mask = 0xffff;
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
}

void ppu_sh6578_device::device_start()
{
	start_nopalram();

	m_palette_ram.resize(0x40);

	for (int i = 0; i < 0x40; i++)
		m_palette_ram[i] = 0x00;

	save_item(NAME(m_palette_ram));
	save_item(NAME(m_extplanebuf));
	save_item(NAME(m_colsel_pntstart));
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
	if (BIT(m_refresh_data, 15))
	{
		const uint16_t tmp = (m_refresh_data & 0x03e0) + 0x20;
		m_refresh_data &= 0x7c1f;

		if (tmp == 0x0400)
			m_refresh_data ^= 0x0800;
		else
			m_refresh_data |= (tmp & 0x03e0);

		//logerror("updating refresh_data: %04x\n", m_refresh_data);
	}
}

void ppu_sh6578_device::draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap)
{
	bitmap.pix(m_scanline, sprite_xpos + pixel) = pen_color(m_palette_ram[(pixel_data | color << 2)] & 0x3f);
}

void ppu_sh6578_device::read_tile_plane_data(int address, int color)
{
	m_planebuf[0] = readbyte(address);
	m_planebuf[1] = readbyte(address + 8);

	if (m_colsel_pntstart & 0x80)
	{
		m_extplanebuf[0] = readbyte(address + 16);
		m_extplanebuf[1] = readbyte(address + 24);
	}
}

void ppu_sh6578_device::draw_tile(uint8_t* line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t*& dest)
{
	int color = color_byte;

	if (BIT(m_colsel_pntstart, 7))
	{
		color &= 0xc;
	}
	else
	{
		color &= 0xf;
	}

	read_tile_plane_data(address, color);

	/* render the pixel */
	for (int i = 0; i < 8; i++)
	{
		uint8_t pix;

		pix =  ((m_planebuf[0] & 0x80) >> 7);
		pix |= ((m_planebuf[1] & 0x80) >> 6);

		if (m_colsel_pntstart & 0x80)
		{
			pix |= ((m_extplanebuf[0] & 0x80) >> 5);
			pix |= ((m_extplanebuf[1] & 0x80) >> 4);
		}

		m_planebuf[0] <<= 1;
		m_planebuf[1] <<= 1;

		if (m_colsel_pntstart & 0x80)
		{
			m_extplanebuf[0] <<= 1;
			m_extplanebuf[1] <<= 1;
		}

		if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
		{
			rgb_t pen;

			const uint8_t palval = m_palette_ram[(pix | color << 2)] & 0x3f;

			bool trans = false;
			if ((palval & 0x1f) == 0x1f)
				trans = true;

			if (!trans)
			{
				pen = pen_color(palval);
			}
			else
			{
				const uint8_t palval = m_palette_ram[0x0] & 0x3f;
				pen = pen_color(palval);
			}

			*dest = pen;

			// priority marking
			if (!trans)
				line_priority[start_x + i] |= 0x02;
		}
		dest++;
	}
}

void ppu_sh6578_device::draw_background(uint8_t* line_priority)
{
	uint8_t color_mask = 0xff;
	//const pen_t* color_table;

	/* setup the color mask and colortable to use */

	//TODO FIX
	if (m_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
	{
		color_mask = 0xf0;
	}
	else
	{
		color_mask = 0xff;
	}


	/* cache the background pen */
	const pen_t back_pen = pen_color(m_back_color & color_mask);

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	const uint8_t  scroll_x_coarse = m_refresh_data & 0x001f;
	const uint8_t  scroll_y_coarse = (m_refresh_data & 0x03e0) >> 5;
	const uint16_t nametable = (m_refresh_data & 0x0c00);
	const uint8_t  scroll_y_fine = (m_refresh_data & 0x7000) >> 12;

	int x = scroll_x_coarse;

	/* get the tile index */
	int tile_index = (nametable << 1) + scroll_y_coarse * 64;

	/* set up dest */
	int start_x = (m_x_fine ^ 0x07) - 7;
	uint32_t* dest = &m_bitmap.pix(m_scanline, start_x);

	m_tilecount = 0;

	/* draw the 32 or 33 tiles that make up a line */
	while (m_tilecount < 34)
	{
		int index1 = tile_index + (x << 1);

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
		const int page2 = readbyte(index1);

		/* Figure out which byte in the color table to use */
		const int color_byte = readbyte(index1 + 1);

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			int address = ((page2 | (color_byte<<8)) & 0x0fff) << 4;
			// plus something that accounts for y
			address += scroll_y_fine;

			draw_tile(line_priority, (color_byte >> 4) & 0xf, 0, address, start_x, back_pen, dest);

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
		dest = &m_bitmap.pix(m_scanline);
		for (int i = 0; i < 8; i++)
		{
			*(dest++) = back_pen;
			line_priority[i] &= ~0x02;
		}
	}
}

void ppu_sh6578_device::read_sprite_plane_data(int address)
{
	m_planebuf[0] = readbyte((address + 0) & 0x3fff);
	m_planebuf[1] = readbyte((address + 8) & 0x3fff);
}


int ppu_sh6578_device::apply_sprite_pattern_page(int index1, int size)
{
	index1 += (((m_colsel_pntstart >> 2) & 0x3) * 0x1000);
	return index1;
}

/*
void ppu_sh6578_device::draw_sprites(uint8_t* line_priority)
{
}
*/

// Dot renderer hooks, from the scanline renderer's draw_background/draw_tile above: each tile is two bytes in the
// name table (tile number bits 0-7, then a colour byte with tile number bits 8-11 in its low nibble and the tile's
// palette in its high nibble), 64 bytes a row, at $0000-$1FFF or (Color Select bit 0) $2000-$27FF; the pattern address
// is the 12-bit tile number * 16 plus fine Y, plane 1 at +8 and, in 4bpp mode (bit 7), planes 2-3 at +16 and +24 (read
// with the tile's other planes; the SH6578's fetch timing for them is not documented). A pixel is transparent when its
// palette entry's low five bits are all set.
u16 ppu_sh6578_device::dot_nt_addr() const
{
	const u16 v = m_refresh_data;
	int index = ((v & 0x0c00) << 1) + ((v >> 5) & 0x1f) * 64 + (v & 0x1f) * 2;
	if (m_colsel_pntstart & 1)
		index = (index & 0x7ff) + 0x2000;
	else
		index &= 0x1fff;
	return u16(index);
}

u8 ppu_sh6578_device::dot_at_decode(u8 data)
{
	m_next_tile_hi = data;
	const u8 color = (data >> 4) & 0x0f;
	return BIT(m_colsel_pntstart, 7) ? (color & 0x0c) : color;
}

u16 ppu_sh6578_device::dot_pattern_addr(bool high) const
{
	return u16((((m_bg_next_nt | (m_next_tile_hi << 8)) & 0x0fff) << 4) + ((m_refresh_data >> 12) & 7) + (high ? 8 : 0));
}

u8 ppu_sh6578_device::dot_bg_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra)
{
	const u16 full = dot_pattern_addr(high);   // all 16 bits; the 2C02 bus model keeps 14
	extra = (m_colsel_pntstart & 0x80) ? readbyte(full + 16) : 0;
	return readbyte(full);
}

u8 ppu_sh6578_device::dot_sp_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra)
{
	// sprite pattern page from Color Select bits 2-3 (apply_sprite_pattern_page, read_sprite_plane_data)
	extra = 0;
	return readbyte((addr + ((m_colsel_pntstart >> 2) & 3) * 0x1000) & 0x3fff);
}

u8 ppu_sh6578_device::dot_bg_index(u8 bg_pix, u8 bg_pal) const
{
	// planes 2-3 come from the dot renderer in pixel bits 5-6; here they are bits 2-3
	const u8 pix = (bg_pix & 0x03) | ((bg_pix >> 3) & 0x0c);
	return (pix | (bg_pal << 2)) & 0x3f;
}

bool ppu_sh6578_device::dot_bg_opaque(u8 bg_pix, u8 bg_pal) const
{
	return (m_palette_ram[dot_bg_index(bg_pix, bg_pal)] & 0x1f) != 0x1f;
}

bool ppu_sh6578_device::dot_pixel_rgb(int x, u8 bg_pix, u8 bg_pal, bool bg_opaque, u8 sp_pix, u8 sp_pal, bool sprite_wins, u32 *dest)
{
	// an opaque background pixel may have all four bits clear: opacity comes from its palette entry (bg_opaque)
	if (sprite_wins)
		*dest = pen_color(m_palette_ram[(sp_pix | (sp_pal << 2)) & 0x3f] & 0x3f);
	else if (bg_opaque)
		*dest = pen_color(m_palette_ram[dot_bg_index(bg_pix, bg_pal)] & 0x3f);
	else
		*dest = pen_color(m_palette_ram[0] & 0x3f);
	return true;
}

void ppu_sh6578_device::write_extended(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::write : Color Select & PNT Start Address : %02x\n", machine().describe_context(), data);
	m_colsel_pntstart = data;
}


uint8_t ppu_sh6578_device::read_extended(offs_t offset)
{
	LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::read : Color Select & PNT Start Address\n", machine().describe_context());
	return m_colsel_pntstart;
}

void ppu_sh6578_device::palette_write(offs_t offset, uint8_t data)
{
	m_palette_ram[offset] = data;
}

uint8_t ppu_sh6578_device::palette_read(offs_t offset)
{
	return m_palette_ram[offset];
}
