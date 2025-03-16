// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

  K1GE/K2GE graphics emulation

  The K1GE graphics were used in the Neogeo pocket mono; the K2GE graphics were
  used in the Neogeo pocket color.

******************************************************************************/

#include "emu.h"
#include "k1ge.h"
#include "screen.h"


void k1ge_device::palette_init()
{
	for (int i = 0; i < PALETTE_SIZE; i++)
	{
		u8 const j = pal3bit(i);

		set_indirect_color(7 - i, rgb_t(j, j, j));
	}
	set_pen_indirect(BG_COLOR, 0);
	set_pen_indirect(OOW_COLOR, 0);
}


void k2ge_device::palette_init()
{
	for (int i = 0; i < PALETTE_SIZE; i++)
	{
		set_indirect_color(i, rgb_t::black());
	}
	for (int i = 0; i < MONO_COLOR; i++)
	{
		set_pen_indirect(i, i);
	}
	set_pen_indirect(BG_COLOR, 0xf0);
	set_pen_indirect(OOW_COLOR, 0xf8);
}


u8 k1ge_device::read(offs_t offset)
{
	assert(offset < 0x4000);

	u8 data = m_vram[offset];

	switch (offset)
	{
	case 0x008:     /* RAS.H */
		data = screen().hpos() >> 2;
		break;
	case 0x009:     /* RAS.V */
		data = screen().vpos();
		break;
	}
	return data;
}


void k1ge_device::write(offs_t offset, u8 data)
{
	assert(offset < 0x4000);

	switch (offset)
	{
	case 0x000:
		m_vblank_pin_w(BIT(data, 7) ? BIT(m_vram[0x010], 6) : 0);
		break;
	case 0x012:
		set_pen_indirect(OOW_COLOR, data & 7);
		break;
	case 0x030:
		data &= 0x80;
		break;
	case 0x100: case 0x101: case 0x102: case 0x103:
	case 0x104: case 0x105: case 0x106: case 0x107:
	case 0x108: case 0x109: case 0x10a: case 0x10b:
	case 0x10c: case 0x10d: case 0x10e: case 0x10f:
	case 0x110: case 0x111: case 0x112: case 0x113:
	case 0x114: case 0x115: case 0x116: case 0x117:
		data &= 0x07;
		set_pen_indirect(offset - 0x100, data);
		break;
	case 0x118:
		set_pen_indirect(BG_COLOR, ((data & 0xc0) == 0x80) ? data & 7 : 0);
		break;
	case 0x7e2:
		if (m_vram[0x7f0] != 0xaa)
			return;
		data &= 0x80;
		break;
	}

	m_vram[offset] = data;
}


void k2ge_device::write(offs_t offset, u8 data)
{
	assert(offset < 0x4000);

	switch (offset)
	{
	case 0x100: case 0x101: case 0x102: case 0x103:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + data);
		break;
	case 0x104: case 0x105: case 0x106: case 0x107:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + 0x08 + data);
		break;
	case 0x108: case 0x109: case 0x10a: case 0x10b:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + 0x10 + data);
		break;
	case 0x10c: case 0x10d: case 0x10e: case 0x10f:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + 0x18 + data);
		break;
	case 0x110: case 0x111: case 0x112: case 0x113:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + 0x20 + data);
		break;
	case 0x114: case 0x115: case 0x116: case 0x117:
		data &= 0x07;
		set_pen_indirect(offset - 0x100 + MONO_COLOR, MONO_COLOR + 0x28 + data);
		break;
	case 0x118:
		set_pen_indirect(BG_COLOR, MONO_COLOR + 0x30 + (((data & 0xc0) == 0x80) ? (data & 7) : 0));
		break;
	default:
		k1ge_device::write(offset, data);
		break;
	}

	/* Only the lower 4 bits of the palette entry high bytes can be written */
	if (offset >= 0x0200 && offset < 0x0400 && (offset & 1))
	{
		data &= 0x0f;
	}

	m_vram[offset] = data;

	// set palette
	if (offset >= 0x0200 && offset < 0x0400)
	{
		u16 const palette = m_vram[offset & ~1] | (m_vram[offset | 1] << 8);
		u8 const r = pal4bit(palette & 0xf);
		u8 const g = pal4bit((palette >> 4) & 0xf);
		u8 const b = pal4bit((palette >> 8) & 0xf);
		set_indirect_color((offset - 0x0200) >> 1, rgb_t(r, g, b));
	}
}


void k1ge_device::draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, int pal_base)
{
	int offset_x = (scroll_x >> 3) * 2;
	int px = scroll_x & 0x07;

	base += ((((scroll_y + line) >> 3) * 0x0040) & 0x7ff);

	/* setup */
	u16 map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
	bool hflip = BIT(map_data, 15);
	u16 pcode = pal_base + (BIT(map_data, 13) ? 4 : 0);
	u16 tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
	if (BIT(map_data, 14))
		tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
	else
		tile_addr += ((scroll_y + line) & 0x07) * 2;
	u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
	if (hflip)
		tile_data >>= 2 * (scroll_x & 0x07);
	else
		tile_data <<= 2 * (scroll_x & 0x07);

	/* draw pixels */
	for (int i = 0; i < 160; i++)
	{
		u16 col;

		if (hflip)
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if (col)
		{
			p[i] = pcode + col;
		}

		px++;
		if (px >= 8)
		{
			offset_x = (offset_x + 2) & 0x3f;
			map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
			hflip = BIT(map_data, 15);
			pcode = pal_base + (BIT(map_data, 13) ? 4 : 0);
			tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
			if (BIT(map_data, 14))
				tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
			else
				tile_addr += ((scroll_y + line) & 0x07) * 2;
			tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
			px = 0;
		}
	}
}


void k1ge_device::draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y)
{
	struct {
		u16 spr_data;
		u8 x;
		u8 y;
	} spr[64];
	int num_sprites = 0;
	u8 spr_y = 0;
	u8 spr_x = 0;

	priority <<= 11;

	/* Select sprites */
	for (int i = 0; i < 256; i += 4)
	{
		u16 const spr_data = m_vram[0x800 + i] | (m_vram[0x801 + i] << 8);
		u8 const x = m_vram[0x802 + i];
		u8 const y = m_vram[0x803 + i];

		spr_x = (BIT(spr_data, 10)) ? (spr_x + x) :  (scroll_x + x);
		spr_y = (BIT(spr_data, 9)) ? (spr_y + y) :  (scroll_y + y);

		if ((spr_data & 0x1800) == priority)
		{
			if ((line >= spr_y || spr_y > 0xf8) && line < ((spr_y + 8) & 0xff))
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for (int i = num_sprites - 1; i >= 0; i--)
	{
		u16 const pcode = BIT(spr[i].spr_data, 13) ? 4 : 0;

		u16 tile_addr = 0x2000 + ((spr[i].spr_data & 0x1ff) * 16);
		if (BIT(spr[i].spr_data, 14))
			tile_addr += (7 - ((line - spr[i].y) & 0x07)) * 2;
		else
			tile_addr += ((line - spr[i].y) & 0x07) * 2;
		u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);

		for (int j = 0; j < 8; j++)
		{
			u16 col;

			spr_x = spr[i].x + j;

			if (BIT(spr[i].spr_data, 15))
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if (spr_x < 160 && col)
			{
				p[spr_x] = pcode + col;
			}
		}
	}
}


void k1ge_device::draw(int line)
{
	u16 *const p = &m_bitmap.pix(line);
	u16 const oowcol = OOW_COLOR;

	if (line < m_wba_v || line >= m_wba_v + m_wsi_v)
	{
		for(int i = 0; i < 160; i++)
		{
			p[i] = oowcol;
		}
	}
	else
	{
		for (int i = 0; i < 160; i++)
			p[i] = BG_COLOR;

		if (BIT(m_vram[0x030], 7))
		{
			/* Draw sprites with 01 priority */
			draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

			/* Draw PF1 */
			draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x08);

			/* Draw sprites with 10 priority */
			draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

			/* Draw PF2 */
			draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x10);

			/* Draw sprites with 11 priority */
			draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
		}
		else
		{
			/* Draw sprites with 01 priority */
			draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

			/* Draw PF2 */
			draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x10);

			/* Draw sprites with 10 priority */
			draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

			/* Draw PF1 */
			draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x08);

			/* Draw sprites with 11 priority */
			draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
		}

		for(int i = 0; i < m_wba_h; i++)
		{
			p[i] = oowcol;
		}

		for(int i = m_wba_h + m_wsi_h; i < 160; i++)
		{
			p[i] = oowcol;
		}
	}
}


void k2ge_device::draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, u16 pal_base)
{
	int offset_x = (scroll_x >> 3) * 2;
	int px = scroll_x & 0x07;

	base += ((((scroll_y + line) >> 3) * 0x0040) & 0x7ff);

	/* setup */
	u16 map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
	bool hflip = BIT(map_data, 15);
	u16 pcode = pal_base + ((map_data & 0x1e00) >> 7);
	u16 tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
	if (BIT(map_data, 14))
		tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
	else
		tile_addr += ((scroll_y + line) & 0x07) * 2;
	u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
	if (hflip)
		tile_data >>= 2 * (scroll_x & 0x07);
	else
		tile_data <<= 2 * (scroll_x & 0x07);

	/* draw pixels */
	for (int i = 0; i < 160; i++)
	{
		u16 col;

		if (hflip)
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if (col)
		{
			p[i] = pcode + col;
		}

		px++;
		if (px >= 8)
		{
			offset_x = (offset_x + 2) & 0x3f;
			map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
			hflip = BIT(map_data, 15);
			pcode = pal_base + ((map_data & 0x1e00) >> 7);
			tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
			if (BIT(map_data, 14))
				tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
			else
				tile_addr += ((scroll_y + line) & 0x07) * 2;
			tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
			px = 0;
		}
	}
}


void k2ge_device::draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y)
{
	struct {
		u16 spr_data;
		u8 x;
		u8 y;
		u8 index;
	} spr[64];
	int num_sprites = 0;
	u8 spr_y = 0;
	u8 spr_x = 0;

	priority <<= 11;

	/* Select sprites */
	for (int i = 0; i < 256; i += 4)
	{
		u16 const spr_data = m_vram[0x800 + i] | (m_vram[0x801 + i] << 8);
		u8 const x = m_vram[0x802 + i];
		u8 const y = m_vram[0x803 + i];

		spr_x = (BIT(spr_data, 10)) ? (spr_x + x) :  (scroll_x + x);
		spr_y = (BIT(spr_data, 9)) ? (spr_y + y) :  (scroll_y + y);

		if ((spr_data & 0x1800) == priority)
		{
			if ((line >= spr_y || spr_y > 0xf8) && line < ((spr_y + 8) & 0xff))
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				spr[num_sprites].index = i >> 2;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for (int i = num_sprites - 1; i >= 0; i--)
	{
		u16 const pcode = (m_vram[0x0c00 + spr[i].index] & 0x0f) << 2;

		u16 tile_addr = 0x2000 + ((spr[i].spr_data & 0x1ff) * 16);
		if (BIT(spr[i].spr_data, 14))
			tile_addr += (7 - ((line - spr[i].y) & 0x07)) * 2;
		else
			tile_addr += ((line - spr[i].y) & 0x07) * 2;
		u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);

		for (int j = 0; j < 8; j++)
		{
			u16 col;

			spr_x = spr[i].x + j;

			if (BIT(spr[i].spr_data, 15))
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if (spr_x < 160 && col)
			{
				p[spr_x] = pcode + col;
			}
		}
	}
}


void k2ge_device::k1ge_draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, u16 pal_base)
{
	int offset_x = (scroll_x >> 3) * 2;
	int px = scroll_x & 0x07;

	base += ((((scroll_y + line) >> 3) * 0x0040) & 0x7ff);

	/* setup */
	u16 map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
	bool hflip = BIT(map_data, 15);
	u16 pcode = MONO_COLOR + pal_base + (BIT(map_data, 13) ? 4 : 0);
	u16 tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
	if (BIT(map_data, 14))
		tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
	else
		tile_addr += ((scroll_y + line) & 0x07) * 2;
	u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
	if (hflip)
		tile_data >>= 2 * (scroll_x & 0x07);
	else
		tile_data <<= 2 * (scroll_x & 0x07);

	/* draw pixels */
	for (int i = 0; i < 160; i++)
	{
		u16 col;

		if (hflip)
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if (col)
		{
			p[i] = pcode + col;
		}

		px++;
		if (px >= 8)
		{
			offset_x = (offset_x + 2) & 0x3f;
			map_data = m_vram[base + offset_x] | (m_vram[base + offset_x + 1] << 8);
			hflip = BIT(map_data, 15);
			pcode = MONO_COLOR + pal_base + (BIT(map_data, 13) ? 4 : 0);
			tile_addr = 0x2000 + ((map_data & 0x1ff) * 16);
			if (BIT(map_data, 14))
				tile_addr += (7 - ((scroll_y + line) & 0x07)) * 2;
			else
				tile_addr += ((scroll_y + line) & 0x07) * 2;
			tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);
			px = 0;
		}
	}
}


void k2ge_device::k1ge_draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y)
{
	struct {
		u16 spr_data;
		u8 x;
		u8 y;
	} spr[64];
	int num_sprites = 0;
	u8 spr_y = 0;
	u8 spr_x = 0;

	priority <<= 11;

	/* Select sprites */
	for (int i = 0; i < 256; i += 4)
	{
		u16 const spr_data = m_vram[0x800 + i] | (m_vram[0x801 + i] << 8);
		u8 const x = m_vram[0x802 + i];
		u8 const y = m_vram[0x803 + i];

		spr_x = (BIT(spr_data, 10)) ? (spr_x + x) :  (scroll_x + x);
		spr_y = (BIT(spr_data, 9)) ? (spr_y + y) :  (scroll_y + y);

		if ((spr_data & 0x1800) == priority)
		{
			if ((line >= spr_y || spr_y > 0xf8) && line < ((spr_y + 8) & 0xff))
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for (int i = num_sprites - 1; i >= 0; i--)
	{
		u16 const pcode = MONO_COLOR + (BIT(spr[i].spr_data, 13) ? 4 : 0);

		u16 tile_addr = 0x2000 + ((spr[i].spr_data & 0x1ff) * 16);
		if (BIT(spr[i].spr_data, 14))
			tile_addr += (7 - ((line - spr[i].y) & 0x07)) * 2;
		else
			tile_addr += ((line - spr[i].y) & 0x07) * 2;
		u16 tile_data = m_vram[tile_addr] | (m_vram[tile_addr + 1] << 8);

		for (int j = 0; j < 8; j++)
		{
			u16 col;

			spr_x = spr[i].x + j;

			if (BIT(spr[i].spr_data, 15))
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if (spr_x < 160 && col)
			{
				p[spr_x] = pcode + col;
			}
		}
	}
}


void k2ge_device::draw(int line)
{
	u16 *const p = &m_bitmap.pix(line);
	u16 const oowcol = OOW_COLOR;

	if (line < m_wba_v || line >= m_wba_v + m_wsi_v)
	{
		for(int i = 0; i < 160; i++)
		{
			p[i] = oowcol;
		}
	}
	else
	{
		for (int i = 0; i < 160; i++)
			p[i] = BG_COLOR;

		if (BIT(m_vram[0x7e2], 7))
		{
			/* K1GE compatibility mode */
			if (BIT(m_vram[0x030], 7))
			{
				/* Draw sprites with 01 priority */
				k1ge_draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF1 */
				k1ge_draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x08);

				/* Draw sprites with 10 priority */
				k1ge_draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF2 */
				k1ge_draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x10);

				/* Draw sprites with 11 priority */
				k1ge_draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
			}
			else
			{
				/* Draw sprites with 01 priority */
				k1ge_draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF2 */
				k1ge_draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x10);

				/* Draw sprites with 10 priority */
				k1ge_draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF1 */
				k1ge_draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x08);

				/* Draw sprites with 11 priority */
				k1ge_draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
			}
		}
		else
		{
			/* K2GE mode */
			if (BIT(m_vram[0x030], 7))
			{
				/* Draw sprites with 01 priority */
				draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF1 */
				draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x040);

				/* Draw sprites with 10 priority */
				draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF2 */
				draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x080);

				/* Draw sprites with 11 priority */
				draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
			}
			else
			{
				/* Draw sprites with 01 priority */
				draw_sprite_plane(p, 1, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF2 */
				draw_scroll_plane(p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x080);

				/* Draw sprites with 10 priority */
				draw_sprite_plane(p, 2, line, m_vram[0x020], m_vram[0x021]);

				/* Draw PF1 */
				draw_scroll_plane(p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x040);

				/* Draw sprites with 11 priority */
				draw_sprite_plane(p, 3, line, m_vram[0x020], m_vram[0x021]);
			}
		}

		for (int i = 0; i < m_wba_h; i++)
		{
			p[i] = oowcol;
		}

		for (int i = m_wba_h + m_wsi_h; i < 160; i++)
		{
			p[i] = oowcol;
		}
	}
}


TIMER_CALLBACK_MEMBER(k1ge_device::hblank_on_timer_callback)
{
	m_hblank_pin_w(0);
}


TIMER_CALLBACK_MEMBER(k1ge_device::timer_callback)
{
	int y = screen().vpos();

	/* Check for start of VBlank */
	if (y >= 152)
	{
		m_vram[0x010] |= 0x40;
		if (BIT(m_vram[0x000], 7))
		{
			m_vblank_pin_w(1);
		}
	}

	/* Check for end of VBlank */
	if (y == 0)
	{
		m_wba_h = m_vram[0x002];
		m_wba_v = m_vram[0x003];
		m_wsi_h = m_vram[0x004];
		m_wsi_v = m_vram[0x005];
		m_vram[0x010] &= ~ 0x40;
		if (BIT(m_vram[0x000], 7))
		{
			m_vblank_pin_w(0);
		}
	}

	/* Check if Hint should be triggered */
	if (y == K1GE_SCREEN_HEIGHT - 1 || y < 151)
	{
		if (!m_hblank_pin_w.isunset())
		{
			if (BIT(m_vram[0x000], 6))
			{
				m_hblank_pin_w(1);
			}
			m_hblank_on_timer->adjust(screen().time_until_pos(y, 480));
		}
	}

	/* Draw a line when inside visible area */
	if (y && y < 153)
	{
		draw(y - 1);
	}

	m_timer->adjust(screen().time_until_pos((y + 1) % K1GE_SCREEN_HEIGHT, 0));
}


u32 k1ge_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1ge_device::device_start()
{
	m_timer = timer_alloc(FUNC(k1ge_device::timer_callback), this);
	m_hblank_on_timer = timer_alloc(FUNC(k1ge_device::hblank_on_timer_callback), this);
	m_vram = make_unique_clear<u8[]>(0x4000);
	screen().register_screen_bitmap(m_bitmap);

	save_pointer(NAME(m_vram), 0x4000);
	save_item(NAME(m_wba_h));
	save_item(NAME(m_wba_v));
	save_item(NAME(m_wsi_h));
	save_item(NAME(m_wsi_v));
	save_item(NAME(m_bitmap));

	palette_init();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1ge_device::device_reset()
{
	write(0x000, 0x00);   /* Interrupt enable */
	write(0x002, 0x00);   /* WBA.H */
	write(0x003, 0x00);   /* WVA.V */
	write(0x004, 0xFF);   /* WSI.H */
	write(0x005, 0xFF);   /* WSI.V */
	write(0x007, 0xc6);   /* REF */
	write(0x012, 0x00);   /* 2D control */
	write(0x020, 0x00);   /* PO.H */
	write(0x021, 0x00);   /* PO.V */
	write(0x030, 0x00);   /* PF */
	write(0x032, 0x00);   /* S1SO.H */
	write(0x033, 0x00);   /* S1SO.V */
	write(0x034, 0x00);   /* S2SO.H */
	write(0x035, 0x00);   /* S2SO.V */
	write(0x101, 0x07);   /* SPPLT01 */
	write(0x102, 0x07);   /* SPPLT02 */
	write(0x103, 0x07);   /* SPPLT03 */
	write(0x105, 0x07);   /* SPPLT11 */
	write(0x106, 0x07);   /* SPPLT12 */
	write(0x107, 0x07);   /* SPPLT13 */
	write(0x109, 0x07);   /* SC1PLT01 */
	write(0x10a, 0x07);   /* SC1PLT02 */
	write(0x10b, 0x07);   /* SC1PLT03 */
	write(0x10d, 0x07);   /* SC1PLT11 */
	write(0x10e, 0x07);   /* SC1PLT12 */
	write(0x10f, 0x07);   /* SC1PLT13 */
	write(0x111, 0x07);   /* SC2PLT01 */
	write(0x112, 0x07);   /* SC2PLT02 */
	write(0x113, 0x07);   /* SC2PLT03 */
	write(0x115, 0x07);   /* SC2PLT11 */
	write(0x116, 0x07);   /* SC2PLT12 */
	write(0x117, 0x07);   /* SC2PLT13 */
	write(0x118, 0x07);   /* BG */
	write(0x400, 0xFF);   /* LED control */
	write(0x402, 0x80);   /* LEDFREG */
	write(0x7e0, 0x52);   /* RESET */
	write(0x7e2, 0x00);   /* MODE */

	m_timer->adjust(screen().time_until_pos((screen().vpos() + 1) % K1GE_SCREEN_HEIGHT, 0));
}


DEFINE_DEVICE_TYPE(K1GE, k1ge_device, "k1ge", "K1GE Monochrome Graphics + LCD")

k1ge_device::k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k1ge_device(mconfig, K1GE, tag, owner, clock)
{
}

k1ge_device::k1ge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_vblank_pin_w(*this)
	, m_hblank_pin_w(*this)
{
}


DEFINE_DEVICE_TYPE(K2GE, k2ge_device, "k2ge", "K2GE Color Graphics + LCD")

k2ge_device::k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: k1ge_device(mconfig, K2GE, tag, owner, clock)
{
}
