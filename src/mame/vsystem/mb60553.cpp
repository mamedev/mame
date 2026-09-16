// license:BSD-3-Clause
// copyright-holders:David Haywood
/* This is the tilemap chip used by Grand Striker, Tecmo World Cup '94 and V Goal Soccer for the backgrounds

  the per-line zoom/scroll table format was worked out from the Grand Striker
  and V Goal Soccer pitch tables and the title screen zooms, see draw() below

  interestingly the chip seems to require doubled up ROMs (2 copies of each ROM) to draw just the single layer.

*/
/*
    Tecmo World Cup '94 "service mode" has an item for testing zooming, this is:
    0xffdf12 target zoom code
    0xffdf16 current zoom code
*/

#include "emu.h"
#include "mb60553.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(MB60553, mb60553_zooming_tilemap_device, "mb60553", "MB60553 Zooming Tilemap")

mb60553_zooming_tilemap_device::mb60553_zooming_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MB60553, tag, owner, clock)
	, m_tmap(nullptr)
	, m_vram()
	, m_regs{ 0, 0, 0, 0, 0, 0, 0, 0 }
	, m_bank{ 0, 0, 0, 0, 0, 0, 0, 0, }
	, m_pal_base(0)
	, m_lineram()
	, m_gfx_region(0)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}


void mb60553_zooming_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_lineram = make_unique_clear<uint16_t[]>(0x1000/2);
	m_vram = make_unique_clear<uint16_t[]>(0x4000/2);

	save_pointer(NAME(m_lineram), 0x1000/2);
	save_pointer(NAME(m_vram), 0x4000/2);
	save_item(NAME(m_pal_base));
	save_item(NAME(m_bank));
	save_item(NAME(m_regs));

	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mb60553_zooming_tilemap_device::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(mb60553_zooming_tilemap_device::twc94_scan)), 16,16,128,64);
	m_tmap->set_transparent_pen(0);
}

void mb60553_zooming_tilemap_device::device_reset()
{
}


/*** Fujitsu MB60553 (screen tilemap) **********************************************/

/*

    Fujitsu MB60553 - Tilemap chip
    ------------------------------

- 1 Plane
- Tiles 16x16, 4bpp
- Map 64x64
- Scrolling
- Indexed banking (8 banks)
- Per-line zoom/scroll table (see draw() below)


    Videoram format
    ---------------

pppp bbbt tttt tttt

t=tile, b=bank, p=palette


    Registers
    ---------

0 - Start X
Fixed point 12.4 signed, latched at the top of the frame (see the line table notes below)

1 - Start Y
Fixed point 12.4 signed, latched at the top of the frame

2 - ????

3 - ????

4 - Tilebank #0/#1   ---a aaaa  ---b bbbb
5 - Tilebank #2/#3   ---a aaaa  ---b bbbb
6 - Tilebank #4/#5   ---a aaaa  ---b bbbb
7 - Tilebank #6/#7   ---a aaaa  ---b bbbb

Indexed tilebank. Each bank is 0x200 tiles wide. Notice that within each register, the bank with the lower
index is in the MSB. gstriker uses 5 bits for banking, but the chips could be able to do more.

*/



TILE_GET_INFO_MEMBER(mb60553_zooming_tilemap_device::get_tile_info)
{
	int data, bankno;
	int tileno, pal;

	data = m_vram[tile_index];

	tileno = data & 0x1FF;
	pal = (data >> 12) & 0xF;
	bankno = (data >> 9) & 0x7;

	tileinfo.set(m_gfx_region, tileno + m_bank[bankno] * 0x200, pal + m_pal_base, 0);
}

void mb60553_zooming_tilemap_device::reg_written( int num_reg)
{
	switch (num_reg)
	{
	case 0:
	case 1:
		// start X/Y, used directly by draw()
		break;

	case 2:
		osd_printf_debug("reg , reg 2 %04x\n", m_regs[2]);
		break;

	case 3:
		osd_printf_debug("reg , reg 3 %04x\n", m_regs[3]);
		break;

	case 4:
		m_bank[0] = (m_regs[4] >> 8) & 0x1F;
		m_bank[1] = (m_regs[4] >> 0) & 0x1F;
		m_tmap->mark_all_dirty();
		break;

	case 5:
		m_bank[2] = (m_regs[5] >> 8) & 0x1F;
		m_bank[3] = (m_regs[5] >> 0) & 0x1F;
		m_tmap->mark_all_dirty();
		break;

	case 6:
		m_bank[4] = (m_regs[6] >> 8) & 0x1F;
		m_bank[5] = (m_regs[6] >> 0) & 0x1F;
		m_tmap->mark_all_dirty();
		break;

	case 7:
		m_bank[6] = (m_regs[7] >> 8) & 0x1F;
		m_bank[7] = (m_regs[7] >> 0) & 0x1F;
		m_tmap->mark_all_dirty();
		break;
	}
}

/* twc94 has the tilemap made of 2 pages .. it needs this */
TILEMAP_MAPPER_MEMBER(mb60553_zooming_tilemap_device::twc94_scan)
{
	/* logical (col,row) -> memory offset */
	return (row << 6) + (col & 0x003f) + (BIT(col, 6) << 12);
}

/*
    Per-line table (0x1000 bytes = 256 entries of 8 words)

    The chip keeps a source X/Y accumulator (16.16 internally here).  At the
    first line of the frame it is loaded from registers 0/1 (12.4 signed).
    For every following line the entry's per-line steps are added, and the
    line is then drawn by stepping the per-pixel increments from the
    accumulator.  The horizontal pixel counter is 21 pixels ahead of the
    first visible pixel: a step of 1.0 with register 0 = -21 (0xfeb0, the
    value the games program at reset and on the title screens) is an
    identity mapping.

    w0  X step per pixel, 4.12 signed
    w1  unknown (always 0)
    w2  unknown (always 0)
    w3  Y step per line, 4.12 signed
    w4  Y step per pixel, 4.12 signed
    w5  unknown (always 0)
    w6  unknown (always 0)
    w7  X step per line, 4.12 signed, subtracted

    V Goal Soccer's rotating intro programs w0 = w3 = s*cos(a) and
    w4 = w7 = s*sin(a) on every line, which fixes the sign of w7.

    Grand Striker and V Goal Soccer draw their pitch with a fixed ROM table
    of w0/w3/w7 per line (w0 follows a 1/depth law, w7 = -181 * delta(w0)
    so that the centre column stays under screen X = 160), scrolling only
    with registers 0/1.  Tecmo World Cup '94 rewrites w0-w3 every frame.
*/
void mb60553_zooming_tilemap_device::draw_line(bitmap_ind16 &destbitmap, int line, int min_x, int max_x, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy)
{
	const int xmask = m_tmap->pixmap().width() - 1;
	const int ymask = m_tmap->pixmap().height() - 1;
	const bitmap_ind16 &srcbitmap = m_tmap->pixmap();
	const bitmap_ind8 &flagsbitmap = m_tmap->flagsmap();

	uint16_t *dest = &destbitmap.pix(line, min_x);
	int32_t xxx = startx + (min_x + PIXEL_OFFSET) * incxx;
	int32_t yyy = starty + (min_x + PIXEL_OFFSET) * incxy;

	for (int sx = min_x; sx <= max_x; sx++)
	{
		const int xx = (xxx >> 16) & xmask;
		const int yy = (yyy >> 16) & ymask;

		if ((flagsbitmap.pix(yy, xx) & TILEMAP_PIXEL_LAYER0) != 0)
			*dest = srcbitmap.pix(yy, xx);

		dest++;
		xxx += incxx;
		yyy += incxy;
	}
}

void mb60553_zooming_tilemap_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	const rectangle &visarea = screen.visible_area();

	// registers 0/1 are 12.4, line table entries are 4.12, accumulator is 16.16
	int32_t xacc = int32_t(int16_t(m_regs[0])) << 12;
	int32_t yacc = int32_t(int16_t(m_regs[1])) << 12;

	for (int line = visarea.min_y; line <= visarea.max_y; line++)
	{
		const uint16_t *const entry = &m_lineram[(line & 0xff) * 8];

		if (line != visarea.min_y)
		{
			xacc -= int32_t(int16_t(entry[7])) << 4;
			yacc += int32_t(int16_t(entry[3])) << 4;
		}

		if (line >= cliprect.min_y && line <= cliprect.max_y)
		{
			const int32_t incxx = int32_t(int16_t(entry[0])) << 4;
			const int32_t incxy = int32_t(int16_t(entry[4])) << 4;

			draw_line(bitmap, line, cliprect.min_x, cliprect.max_x, xacc, yacc, incxx, incxy);
		}
	}
}

void mb60553_zooming_tilemap_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldreg = m_regs[offset];

	COMBINE_DATA(&m_regs[offset]);

	if (m_regs[offset] != oldreg)
		reg_written(offset);
}

void mb60553_zooming_tilemap_device::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);

	m_tmap->mark_tile_dirty(offset);
}

void mb60553_zooming_tilemap_device::line_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_lineram[offset]);
}


uint16_t mb60553_zooming_tilemap_device::regs_r(offs_t offset)
{
	return m_regs[offset];
}

uint16_t mb60553_zooming_tilemap_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

uint16_t mb60553_zooming_tilemap_device::line_r(offs_t offset)
{
	return m_lineram[offset];
}
