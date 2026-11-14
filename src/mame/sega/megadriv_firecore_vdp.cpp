// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "megadriv_firecore_vdp.h"


DEFINE_DEVICE_TYPE(FIRECORE_VDP, megadriv_firecore_vdp_device, "megadriv_firecore_vdp_device", "Firecore / RedKid VDP (extended Mega Drive VDP)")

/*
    Firecore extended video mode

    Enabled with VDP register 0x18 bit 7 (the menus use 0x80 / 0x82 / 0x8a, games switch back to 0x00).
    Reverse engineered from the RedKid menus (all of this is unverified against hardware documentation):

    - 128KiB VRAM, address bit 16 comes from bit 2 of the 2nd VDP command word
    - tiles are 8x8 with 6 bits per pixel (48 bytes per tile, tile n at n * 48), the pixels are packed
      LSB first into consecutive 16-bit VRAM words
    - name table / sprite attribute entries: bits 15-13 palette (8 palettes of 64 colours), bit 12 V flip,
      bit 11 H flip, bits 10-0 tile number, there is no priority bit
    - CRAM holds 512 colours of 2 words each: word 0 = GGRR, word 1 = 00BB
    - 68k -> VDP DMA transfers twice the programmed length (the length is in 32-bit units)
    - the table locations don't follow the standard register formulas; the menus always use (and their
      software copies of the addresses say) plane A at 0x12000, plane B at 0x14000, H scroll at 0x16000 and
      the sprite table at 0x17000, so these are hardcoded for now.  Plane B is drawn above plane A.

	TODO
	 - see if we can reuse some of the base VDP handling without being too disruptive
	 - colours are incorrect in several games (eg sprites and background fade for "Black Sheep" in atgames40)
	   and "Zuma Bubbles" found in pxp3, mahg156
*/

uint16_t megadriv_firecore_vdp_device::xvdp_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled() && ((offset & 0x0e) == 0x02)) // control port read clears the pending command
		m_command_pending = 0;

	return sega315_5313_device::vdp_r(offset, mem_mask);
}

void megadriv_firecore_vdp_device::xvdp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// for the data and control port we have special handling
	if (xvdp_enabled())
	{
		if ((offset & 0x0e) == 0x00)
		{
			xvdp_data_w(data);
			return;
		}
		else if ((offset & 0x0e) == 0x02)
		{
			xvdp_ctrl_w(data);
			return;
		}
	}

	// otherwies fall through to the base VDP handling
	sega315_5313_device::vdp_w(offset, data, mem_mask);
}

u8 megadriv_firecore_vdp_device::read_vram8(u32 address)
{
	u32 offset = address >> 1;
	if (address & 1)
	{
		return (m_vram[offset] >> 0) & 0xff;
	}
	else
	{
		return (m_vram[offset] >> 8) & 0xff;
	}
}

void megadriv_firecore_vdp_device::write_vram8(u32 address, u8 data)
{
	u32 offset = address >> 1;
	if (address & 1)
	{
		m_vram[offset] = (m_vram[offset] & 0xff00) | (data);
	}
	else
	{
		m_vram[offset] = (m_vram[offset] & 0x00ff) | (data << 8);
	}
}

void megadriv_firecore_vdp_device::xvdp_data_w(uint16_t data)
{
	m_command_pending = 0;

	if (m_vram_fill_pending)
	{
		m_vram_fill_pending = 0;
		uint32_t length = m_regs[0x13] | (m_regs[0x14] << 8);
		if (!length)
			length = 0x10000;

		m_vram[(m_vdp_address >> 1) & 0xffff] = data;
		for (uint32_t i = 0; i < length; i++)
		{
			const u8 byte = data >> 8;
			write_vram8((m_vdp_address ^ 1) & 0x1ffff, byte);

			m_vdp_address = (m_vdp_address + m_regs[0x0f]) & 0x1ffff;
		}
		return;
	}

	switch (m_vdp_code & 0x0f)
	{
	case 0x01: // VRAM
		m_vram[(m_vdp_address >> 1) & 0xffff] = data;
		break;
	case 0x03: // CRAM
		m_cram[(m_vdp_address >> 1) & 0x3ff] = data;
		break;
	case 0x05: // VSRAM
		m_vsram[(m_vdp_address >> 1) & 0x3f] = data;
		break;
	}
	m_vdp_address = (m_vdp_address + m_regs[0x0f]) & 0x1ffff;
}

void megadriv_firecore_vdp_device::xvdp_ctrl_w(uint16_t data)
{
	if (!m_command_pending)
	{
		if ((data & 0xc000) == 0x8000)
		{
			vdp_set_register((data >> 8) & 0x1f, data & 0xff);
			m_vdp_code = 0;
			m_vdp_address = 0;
			return;
		}

		m_command_pending = 1;
		m_vdp_code = (m_vdp_code & 0x3c) | (data >> 14);
		m_vdp_address = (m_vdp_address & 0x1c000) | (data & 0x3fff);
		return;
	}

	m_command_pending = 0;
	m_vdp_code = (m_vdp_code & 0x03) | ((data >> 2) & 0x3c);
	m_vdp_address = (m_vdp_address & 0x03fff) | ((data & 0x03) << 14) | ((data & 0x04) << 14);

	if (!(m_vdp_code & 0x20) || !BIT(m_regs[0x01], 4))
		return;

	switch (m_regs[0x17] >> 6)
	{
	case 0:
	case 1: // 68k -> VDP
	{
		uint32_t length = m_regs[0x13] | (m_regs[0x14] << 8);
		if (!length)
			length = 0x10000;

		uint32_t source = (m_regs[0x15] | (m_regs[0x16] << 8) | ((m_regs[0x17] & 0x7f) << 16)) << 1;
		for (uint32_t i = 0; i < length; i++)
		{
			// the standard VDP wraps the source within 128KiB, the extended mode transfers can cross that boundary
			// length is also in dwords here, not words
			xvdp_data_w(m_space68k->read_word(source));
			source = ((source + 2) & 0xfffffe);
			xvdp_data_w(m_space68k->read_word(source));
			source = ((source + 2) & 0xfffffe);
		}
		break;
	}

	case 2: // fill, happens on the next data port write
		m_vram_fill_pending = 1;
		break;

	case 3: // VRAM copy
	{
		uint32_t length = m_regs[0x13] | (m_regs[0x14] << 8);
		if (!length)
			length = 0x10000;

		uint32_t source = m_regs[0x15] | (m_regs[0x16] << 8);

		for (uint32_t i = 0; i < length; i++)
		{
			const u8 byte = read_vram8((source ^ 1) & 0x1ffff);
			write_vram8((m_vdp_address ^ 1) & 0x1ffff, byte);
			source = (source + 1) & 0xffff;
			m_vdp_address = (m_vdp_address + m_regs[0x0f]) & 0x1ffff;
		}
		break;
	}
	}
}

uint32_t megadriv_firecore_vdp_device::xvdp_color(int index) const
{
	const uint16_t gr = m_cram[(index * 2 + 0) & 0x3ff];
	const uint16_t b = m_cram[(index * 2 + 1) & 0x3ff];
	return rgb_t(gr & 0xff, gr >> 8, b & 0xff);
}

uint8_t megadriv_firecore_vdp_device::xvdp_tile_pixel(uint32_t tile, int x, int y)
{
	const uint32_t bit = ((y * 8) + x) * 6;
	const uint32_t base = tile * 48;
	const uint32_t word0 = base + ((bit >> 4) << 1);
	const uint32_t word1 = word0 + 2;
	const uint32_t w0 = m_vram[(word0 >> 1) & 0xffff];
	const uint32_t w1 = m_vram[(word1 >> 1) & 0xffff];
	return ((w0 | (w1 << 16)) >> (bit & 15)) & 0x3f;
}

void megadriv_firecore_vdp_device::xvdp_draw_plane(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t nametable, int plane)
{
	static const int sizes[4] = { 32, 64, 64, 128 };
	const int hsize = sizes[m_regs[0x10] & 3];
	const int vsize = sizes[(m_regs[0x10] >> 4) & 3];

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int hscroll;
		switch (m_regs[0x0b] & 3)
		{
		default:
		case 0: hscroll = 0x16000; break;
		case 2: hscroll = 0x16000 + (y & ~7) * 4; break;
		case 3: hscroll = 0x16000 + y * 4; break;
		}
		hscroll += plane * 2;
		const int hs = m_vram[(hscroll >> 1) & 0xffff] & 0x3ff;
		const int vs = m_vsram[plane] & 0x3ff;

		uint32_t *const dest = &bitmap.pix(y);
		const int py = (y + vs) & ((vsize * 8) - 1);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const int px = (x - hs) & ((hsize * 8) - 1);
			const uint32_t entryaddr = nametable + (((py >> 3) * hsize) + (px >> 3)) * 2;
			const uint16_t entry = m_vram[(entryaddr >> 1) & 0xffff];
			int tx = px & 7, ty = py & 7;
			if (BIT(entry, 11)) tx ^= 7;
			if (BIT(entry, 12)) ty ^= 7;
			const uint8_t pix = xvdp_tile_pixel(entry & 0x7ff, tx, ty);
			if (pix)
				dest[x] = xvdp_color(((entry >> 13) * 64) + pix);
		}
	}
}

void megadriv_firecore_vdp_device::xvdp_draw_sprites(bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	const uint32_t table = 0x17000;
	const int maxsprites = BIT(m_regs[0x0c], 0) ? 80 : 64;

	// collect the link list, the first sprite in the list has the highest priority
	int order[80];
	int count = 0;
	int link = 0;
	do
	{
		order[count++] = link;
		const uint32_t a = table + link * 8;
		link = m_vram[((a + 3) >> 1) & 0xffff] & 0x7f;
	} while (link && (link < maxsprites) && (count < maxsprites));

	for (int i = count - 1; i >= 0; i--)
	{
		const uint32_t a = table + order[i] * 8;
		auto word = [this, a](int n) { return m_vram[((a + n * 2) >> 1) & 0xffff]; };
		const int ypos = (word(0) & 0x3ff) - 128;
		const int w = ((word(1) >> 10) & 3) + 1;
		const int h = ((word(1) >> 8) & 3) + 1;
		const uint16_t attr = word(2);
		const int xpos = (word(3) & 0x1ff) - 128;
		const bool fx = BIT(attr, 11), fy = BIT(attr, 12);
		const int pal = attr >> 13;

		for (int sy = 0; sy < h * 8; sy++)
		{
			const int y = ypos + sy;
			if (y < cliprect.min_y || y > cliprect.max_y)
				continue;
			uint32_t *const dest = &bitmap.pix(y);
			const int ty = fy ? (h * 8 - 1 - sy) : sy;
			for (int sx = 0; sx < w * 8; sx++)
			{
				const int x = xpos + sx;
				if (x < cliprect.min_x || x > cliprect.max_x)
					continue;
				const int tx = fx ? (w * 8 - 1 - sx) : sx;
				const uint32_t tile = (attr & 0x7ff) + (tx >> 3) * h + (ty >> 3);
				const uint8_t pix = xvdp_tile_pixel(tile & 0x7ff, tx & 7, ty & 7);
				if (pix)
					dest[x] = xvdp_color((pal * 64) + pix);
			}
		}
	}
}

void megadriv_firecore_vdp_device::draw_firecore(bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	if (!BIT(m_regs[0x01], 6)) // display disabled
		return;

	bitmap.fill(xvdp_color(m_regs[0x07] & 0x3f), cliprect);
	xvdp_draw_plane(bitmap, cliprect, 0x12000, 0);
	xvdp_draw_plane(bitmap, cliprect, 0x14000, 1);
	xvdp_draw_sprites(bitmap, cliprect);
}
