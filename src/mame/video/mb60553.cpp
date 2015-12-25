// license:BSD-3-Clause
// copyright-holders:David Haywood
/* This is the tilemap chip used by Grand Striker, Tecmo World Cup '94 and V Goal Soccer for the backgrounds

  the actual line scroll / zoom is not properly understood

  interestingly the chip seems to require doubled up ROMs (2 copies of each ROM) to draw just the single layer.

*/

#include "emu.h"
#include "mb60553.h"


const device_type MB60553 = &device_creator<mb60553_zooming_tilemap_device>;

mb60553_zooming_tilemap_device::mb60553_zooming_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB60553, "MB60553 Zooming Tilemap", tag, owner, clock, "mb60553", __FILE__),
	m_vram(nullptr),
	m_pal_base(0),
	m_lineram(nullptr),
	m_gfx_region(0),
	m_gfxdecode(*this)
{
	for (int i = 0; i < 8; i++)
	{
		m_regs[i] = 0;
		m_bank[i] = 0;
	}
}


void mb60553_zooming_tilemap_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_lineram = make_unique_clear<UINT16[]>(0x1000/2);
	m_vram = make_unique_clear<UINT16[]>(0x4000/2);

	save_pointer(NAME(m_lineram.get()), 0x1000/2);
	save_pointer(NAME(m_vram.get()), 0x4000/2);
	save_item(NAME(m_pal_base));
	save_item(NAME(m_bank));
	save_item(NAME(m_regs));

	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mb60553_zooming_tilemap_device::get_tile_info),this),tilemap_mapper_delegate(FUNC(mb60553_zooming_tilemap_device::twc94_scan),this), 16,16,128,64);
	m_tmap->set_transparent_pen(0);
}

void mb60553_zooming_tilemap_device::device_reset()
{
}


void mb60553_zooming_tilemap_device::set_gfx_region(device_t &device, int gfxregion)
{
	mb60553_zooming_tilemap_device &dev = downcast<mb60553_zooming_tilemap_device &>(device);
	dev.m_gfx_region = gfxregion;
}

void mb60553_zooming_tilemap_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<mb60553_zooming_tilemap_device &>(device).m_gfxdecode.set_tag(tag);
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
	return (row*64) + (col&63) + ((col&64)<<6);
}

void mb60553_zooming_tilemap_device::set_pal_base( int pal_base)
{
	m_pal_base = pal_base;
}


void mb60553_zooming_tilemap_device::draw_roz_core(screen_device &screen, bitmap_ind16 &destbitmap, const rectangle &cliprect,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound)
{
	// pre-cache all the inner loop values
	//const rgb_t *clut = m_palette->palette()->entry_list_adjusted();
	const int xmask = m_tmap->pixmap().width() - 1;
	const int ymask = m_tmap->pixmap().height() - 1;
	const int widthshifted = m_tmap->pixmap().width() << 16;
	const int heightshifted = m_tmap->pixmap().height() << 16;
	UINT8 mask = 0x1f;// blit.mask;
	UINT8 value = 0x10;// blit.value;
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

		UINT16 *dest = &destbitmap.pix(sy, sx);

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

	for (line = 0; line < 224;line++)
	{
//      int scrollx;
//      int scrolly;

		UINT32 startx,starty;

		UINT32 incxx,incyy;

		startx = m_regs[0];
		starty = m_regs[1];

		startx += (24<<4); // maybe not..

		startx -=  m_lineram[(line)*8+7]/2;

		incxx = m_lineram[(line)*8+0]<<4;
		incyy = m_lineram[(line)*8+3]<<4;

		clip.min_y = clip.max_y = line;

		draw_roz_core(screen, bitmap, clip, startx<<12,starty<<12,
				incxx,0,0,incyy,
				1
				);

	}
}

tilemap_t* mb60553_zooming_tilemap_device::get_tilemap()
{
	return m_tmap;
}


WRITE16_MEMBER(mb60553_zooming_tilemap_device::regs_w)
{
	UINT16 oldreg = m_regs[offset];

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
