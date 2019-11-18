// license:BSD-3-Clause
// copyright-holders:David Haywood
/* This is the tilemap chip used by Grand Striker, Tecmo World Cup '94 and V Goal Soccer for the backgrounds

  the actual line scroll / zoom is not properly understood

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
- Surely another effect like roz/tilezoom, yet to be implemented


    Videoram format
    ---------------

pppp bbbt tttt tttt

t=tile, b=bank, p=palette


    Registers
    ---------

0 - Scroll X
Fixed point 12.4 (seems wrong)

1 - Scroll Y
Fixed point 12.4 (seems wrong)

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

	SET_TILE_INFO_MEMBER(m_gfx_region, tileno + m_bank[bankno] * 0x200, pal + m_pal_base, 0);
}

void mb60553_zooming_tilemap_device::reg_written( int num_reg)
{
	switch (num_reg)
	{
	case 0:
		m_tmap->set_scrollx(0, m_regs[0]>>4);
		break;

	case 1:
		m_tmap->set_scrolly(0, m_regs[1]>>4);
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

void mb60553_zooming_tilemap_device::draw_roz_core(screen_device &screen, bitmap_ind16 &destbitmap, const rectangle &cliprect,
		uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, bool wraparound)
{
	// pre-cache all the inner loop values
	//const rgb_t *clut = m_palette->palette()->entry_list_adjusted();
	const int xmask = m_tmap->pixmap().width() - 1;
	const int ymask = m_tmap->pixmap().height() - 1;
	const int widthshifted = m_tmap->pixmap().width() << 16;
	const int heightshifted = m_tmap->pixmap().height() << 16;
	uint8_t mask = 0x1f;// blit.mask;
	uint8_t value = 0x10;// blit.value;
	bitmap_ind16 &srcbitmap = m_tmap->pixmap();
	bitmap_ind8 &flagsbitmap = m_tmap->flagsmap();

	// extract start/end points
	int sy = cliprect.min_y;
	int ey = cliprect.max_y;

	// loop over rows
	while (sy <= ey)
	{
		// get dest and priority pointers
		int sx = cliprect.min_x;
		int ex = cliprect.max_x;

		uint16_t *dest = &destbitmap.pix(sy, sx);

		// loop over columns
		while (sx <= ex)
		{
			int xxx = startx + (sy*incyx) + (sx*incxx);
			int yyy = starty + (sy*incyy) + (sx*incxy);

			if (wraparound)
			{
				yyy = (yyy >> 16) & ymask;
				xxx = (xxx >> 16) & xmask;

				if ((flagsbitmap.pix(yyy, xxx) & mask) == value)
				{
					*dest = (srcbitmap.pix(yyy, xxx));
				}
			}
			else
			{
				if (xxx < widthshifted && yyy < heightshifted)
				{
					yyy = (yyy >> 16);
					xxx = (xxx >> 16);

					if ((flagsbitmap.pix(yyy, xxx) & mask) == value)
					{
						*dest = (srcbitmap.pix(yyy, xxx));
					}
				}
			}

			// advance in X
			sx++;
			dest++;
			//pri++;
		}

		// advance in Y
		sy++;
	}
}





/* THIS IS STILL WRONG! */
void mb60553_zooming_tilemap_device::draw( screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority)
{
	int line;
	rectangle clip;

	clip.min_x = screen.visible_area().min_x;
	clip.max_x = screen.visible_area().max_x;

	for (line = screen.visible_area().min_y; line < screen.visible_area().max_y;line++)
	{
//      int scrollx;
//      int scrolly;
		uint32_t startx,starty;
		int32_t incxx,incyy;
		int32_t incxy,incyx;
		float xoffset;

		// confirmed on how ROZ is used
		incyy = ((int16_t)m_lineram[(line)*8+0])<<4;
		incxx = ((int16_t)m_lineram[(line)*8+3])<<4;

		// startx has an offset based off current x zoom value
		// This is confirmed by Tecmo World Cup '94 startx being 0xff40 (-192) when showing footballer pics on attract mode (incxx is 0x800)
		// TODO: slightly offset?
		xoffset = ((float)incyy/(float)0x10000) * 384.0;

		startx = m_regs[0] + (int32_t)xoffset;
		starty = m_regs[1];

		// TODO: what's this? Used by Grand Striker playfield
		incyx =  ((int16_t)m_lineram[(line)*8+7])<<4;
		// V Goal Soccer rotation
		incxy =  ((int16_t)m_lineram[(line)*8+4])<<4;

		clip.min_y = clip.max_y = line;

		draw_roz_core(screen, bitmap, clip, startx<<12,starty<<12,
				incxx,incxy,-incyx,incyy,
				1
				);

	}
}

WRITE16_MEMBER(mb60553_zooming_tilemap_device::regs_w)
{
	uint16_t oldreg = m_regs[offset];

	COMBINE_DATA(&m_regs[offset]);

	if (m_regs[offset] != oldreg)
		reg_written(offset);
}

WRITE16_MEMBER(mb60553_zooming_tilemap_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);

	m_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(mb60553_zooming_tilemap_device::line_w)
{
	COMBINE_DATA(&m_lineram[offset]);
}


READ16_MEMBER(mb60553_zooming_tilemap_device::regs_r)
{
	return m_regs[offset];
}

READ16_MEMBER(mb60553_zooming_tilemap_device::vram_r)
{
	return m_vram[offset];
}

READ16_MEMBER(mb60553_zooming_tilemap_device::line_r)
{
	return m_lineram[offset];
}
