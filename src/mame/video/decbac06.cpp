// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*
 Deco BAC06 tilemap generator:

 this a direct relative of the later chip implemented in deco16ic.cpp
 we could implement this as either an 8-bit or a 16-bit chip, for now
 I'm using the 16-bit implementation from dec0.cpp

 used by:

 actfancr.cpp
 dec0.cpp
 dec8.cpp (oscar, cobracom, ghostb)
 madmotor.cpp
 stadhero.cpp
 pcktgal.cpp

 Notes (from dec0.cpp)

 All games contain three BAC06 background generator chips, usual (software)
configuration is 2 chips of 16*16 tiles, 1 of 8*8.

 Playfield control registers:
   bank 0:
   0:
        bit 0 (0x1) set = 8*8 tiles, else 16*16 tiles
        Bit 1 (0x2) set = row major tile layout, else column major*
        bit 2 (0x4) set enables rowscroll
        bit 3 (0x8) set enables colscroll
        bit 7 (0x80) set in playfield 1 is reverse screen (set via dip-switch)
        bit 7 (0x80) in other playfields unknown
   2: unknown (00 in bg, 03 in fg+text - maybe controls pf transparency?)
   4: unknown (always 00) [Used to access 2nd bank of tiles in Stadium Hero)
   6: playfield shape: 00 = 4x1, 01 = 2x2, 02 = 1x4 (low 4 bits only)

   bank 1:
   0: horizontal scroll
   2: vertical scroll
   4: colscroll shifter (low 4 bits, top 4 bits do nothing)
   6: rowscroll shifter (low 4 bits, top 4 bits do nothing)

   Row & column scroll can be applied simultaneously or by themselves.
   The shift register controls the granularity of the scroll offsets
   (more details given later).

   * Bandit is the only game known to use column major tile layout, when in this
   mode X scrolling is also inverted, and tile character data is flipped on X.

Playfield priority (Bad Dudes, etc):
    In the bottommost playfield, pens 8-15 can have priority over the next playfield.
    In that next playfield, pens 8-15 can have priority over sprites.

Bit 0:  Playfield inversion
Bit 1:  Enable playfield mixing (for palettes 8-15 only)
Bit 2:  Enable playfield/sprite mixing (for palettes 8-15 only)

Priority word (Midres):
    Bit 0 set = Playfield 3 drawn over Playfield 2
            ~ = Playfield 2 drawn over Playfield 3
    Bit 1 set = Sprites are drawn inbetween playfields
            ~ = Sprites are on top of playfields
    Bit 2
    Bit 3 set = ...

    Note that priority mixing is handled outside of the BAC-06 chip.

*/

#include "emu.h"
#include "decbac06.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(DECO_BAC06, deco_bac06_device, "deco_back06", "DECO BAC06 Tilemap")

deco_bac06_device::deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DECO_BAC06, tag, owner, clock)
	, m_pf_data(nullptr)
	, m_pf_rowscroll(nullptr)
	, m_pf_colscroll(nullptr)
	, m_tile_region_8(0)
	, m_tile_region_16(0)
	, m_supports_8x8(true)
	, m_supports_16x16(true)
	, m_supports_rc_scroll(true)
	, m_gfxcolmask(0)
	, m_rambank(0)
	, m_gfxregion8x8(0)
	, m_gfxregion16x16(0)
	, m_wide(0)
	, m_bppmult_8x8(0)
	, m_bppmask_8x8(0)
	, m_bppmult_16x16(0)
	, m_bppmask_16x16(0)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
	std::fill(std::begin(m_pf_control_0), std::end(m_pf_control_0), 0);
	std::fill(std::begin(m_pf_control_1), std::end(m_pf_control_1), 0);
}

void deco_bac06_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_pf_data = make_unique_clear<u16[]>(0x4000 / 2); // 0x2000 is the maximum needed, some games / chip setups map less and mirror - stadium hero banks this to 0x4000?!
	m_pf_rowscroll = make_unique_clear<u16[]>(0x2000 / 2);
	m_pf_colscroll = make_unique_clear<u16[]>(0x2000 / 2);

	create_tilemaps(m_gfxregion8x8, m_gfxregion16x16);
	m_gfxcolmask = 0x0f;

	m_bppmult_8x8 = m_gfxdecode->gfx(m_gfxregion8x8)->granularity();
	m_bppmask_8x8 = m_gfxdecode->gfx(m_gfxregion8x8)->depth() - 1;

	m_bppmult_16x16 = m_gfxdecode->gfx(m_gfxregion16x16)->granularity();
	m_bppmask_16x16 = m_gfxdecode->gfx(m_gfxregion16x16)->depth() - 1;

	m_rambank = 0;
	m_flip_screen = false;

	save_pointer(NAME(m_pf_data), 0x4000 / 2);
	save_pointer(NAME(m_pf_rowscroll), 0x2000 / 2);
	save_pointer(NAME(m_pf_colscroll), 0x2000 / 2);
	save_item(NAME(m_pf_control_0));
	save_item(NAME(m_pf_control_1));
	save_item(NAME(m_gfxcolmask));
	save_item(NAME(m_rambank));
	save_item(NAME(m_flip_screen));
}

void deco_bac06_device::device_reset()
{
}

void deco_bac06_device::set_flip_screen(bool flip)
{
	if (m_flip_screen != flip)
	{
		m_flip_screen = flip;
		for (int i = 0; i < 3; i++)
		{
			m_pf8x8_tilemap[i]->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_pf16x16_tilemap[i]->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		}
	}
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape0_scan)
{
	if ((m_pf_control_0[0] & 2) == 0)
	{
		int col_mask = num_cols - 1;
		return (row & 0xf) + ((col_mask - (col & col_mask)) << 4);
	}
	return (col & 0xf) + ((row & 0xf) << 4) + ((col & 0x1f0) << 4);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape1_scan)
{
	//if (m_pf_control_0[0] & 2) // Needs testing on real hardware, not used by any game
	//  return (row & 0xf) + ((col & 0x1f) << 4) + ((col & 0xf0) << 5);
	return (col & 0xf) + ((row & 0x1f) << 4) + ((col & 0xf0) << 5);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape2_scan)
{
	//if (m_pf_control_0[0] & 2)  // Needs testing on real hardware, not used by any game
	//  return (col & 0xf) + ((row & 0x3f) << 4) + ((row & 0x70) << 6);
	return (col & 0xf) + ((row & 0x3f) << 4) + ((col & 0x70) << 6);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape0_8x8_scan)
{
	//if (m_pf_control_0[0] & 2)   // Needs testing on real hardware, not used by any game
	//  return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x60) << 5);
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape1_8x8_scan)
{
	//if (m_pf_control_0[0] & 2)   // Needs testing on real hardware, not used by any game
	//  return (row & 0x1f) + ((col & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape2_8x8_scan)
{
	//if (m_pf_control_0[0] & 2)   // Needs testing on real hardware, not used by any game
	//  return (row & 0x1f) + ((col & 0x7f) << 5);
	return (col & 0x1f) + ((row & 0x7f) << 5);
}

TILE_GET_INFO_MEMBER(deco_bac06_device::get_pf8x8_tile_info)
{
	if (m_rambank & 1) tile_index += 0x1000;
	int tile = m_pf_data[tile_index];
	int colourpri = (tile >> 12);
	int flags = (m_pf_control_0[0] & 2) ? 0 : TILE_FLIPX;
	SET_TILE_INFO_MEMBER(m_tile_region_8,tile & 0xfff,0,flags);
	tileinfo.category = colourpri;
}

TILE_GET_INFO_MEMBER(deco_bac06_device::get_pf16x16_tile_info)
{
	if (m_rambank & 1) tile_index += 0x1000;
	int tile = m_pf_data[tile_index];
	int colourpri = (tile >> 12);
	int flags = (m_pf_control_0[0] & 2) ? 0 : TILE_FLIPX;
	SET_TILE_INFO_MEMBER(m_tile_region_16,tile & 0xfff,0,flags);
	tileinfo.category = colourpri;
}

void deco_bac06_device::create_tilemaps(int region8x8, int region16x16)
{
	m_tile_region_8 = region8x8;
	m_tile_region_16 = region16x16;

	m_pf8x8_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_8x8_scan),this), 8, 8,128, 32);
	m_pf8x8_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_8x8_scan),this), 8, 8, 64, 64);
	m_pf8x8_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_8x8_scan),this), 8, 8, 32,128);

	if (m_wide == 2)
	{
		m_pf16x16_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this), 16, 16, 256, 16);
		m_pf16x16_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this), 16, 16, 128, 32);
		m_pf16x16_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this), 16, 16,  64, 64);
	}
	else if (m_wide == 1)
	{
		m_pf16x16_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this), 16, 16, 128, 16);
		m_pf16x16_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this), 16, 16,  64, 32);
		m_pf16x16_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this), 16, 16,  32, 64);
	}
	else
	{
		m_pf16x16_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this), 16, 16, 64, 16);
		m_pf16x16_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this), 16, 16, 32, 32);
		m_pf16x16_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this), 16, 16, 16, 64);
	}
}

void deco_bac06_device::custom_tilemap_draw(bitmap_ind16 &bitmap,
		bitmap_ind8 &primap,
		const rectangle &cliprect,
		tilemap_t *tilemap_ptr,
		const u16 *rowscroll_ptr,
		const u16 *colscroll_ptr,
		const u16 *control0,
		const u16 *control1,
		int flags,
		u16 penmask,
		u16 pencondition,
		u16 colprimask,
		u16 colpricondition,
		u8 bppmult,
		u8 bppmask,
		u8 pri,
		u8 pmask)
{
	const bitmap_ind16 &src_bitmap = tilemap_ptr->pixmap();
	const bitmap_ind8 &flags_bitmap = tilemap_ptr->flagsmap();
	int column_offset = 0, src_x = 0, src_y = 0;
	u32 scrollx = 0;
	u32 scrolly = 0;

	if (control1)
	{
		if (control0 && (control0[0] & 2) == 0) // Use of column major mode inverts scroll direction
			scrollx = -control1[0] - 0x100;
		else
			scrollx = control1[0];
		scrolly = control1[1];
	}

	int row_scroll_enabled = 0;
	int col_scroll_enabled = 0;

	if (m_supports_rc_scroll)
	{
		if (control0)
		{
			row_scroll_enabled = (rowscroll_ptr && (control0[0] & 0x4));
			col_scroll_enabled = (colscroll_ptr && (control0[0] & 0x8));
		}
	}

	const int width_mask = src_bitmap.width() - 1;
	const int height_mask = src_bitmap.height() - 1;

	/* Column scroll & row scroll may per applied per pixel, there are
	shift registers for each which control the granularity of the row/col
	offset (down to per line level for row, and per 8 lines for column).

	Nb:  The row & col selectors are _not_ affected by the shape of the
	playfield (ie, 256*1024, 512*512 or 1024*256).  So even if the tilemap
	width is only 256, 'src_x' should not wrap at 256 in the code below (to
	do so would mean the top half of row RAM would never be accessed which
	is incorrect).

	Nb2:  Real hardware exhibits a strange bug with column scroll on 'mode 2'
	(256*1024) - the first column has a strange additional offset, but
	curiously the first 'wrap' (at scroll offset 256) does not have this offset,
	it is displayed as expected.  The bug is confimed to only affect this mode,
	the other two modes work as expected.  This bug is not emulated, as it
	doesn't affect any games.
	*/

	if (m_flip_screen)
		src_y = (src_bitmap.height() - 256) - scrolly;
	else
		src_y = scrolly;

	src_y += cliprect.top();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 *dstpix = &bitmap.pix16(y);
		u8 *dstpri = &primap.pix8(y);
		if (row_scroll_enabled)
			src_x=scrollx + rowscroll_ptr[(src_y >> (control1[3] & 0xf)) & (0x1ff >> (control1[3] & 0xf))];
		else
			src_x=scrollx;

		if (m_flip_screen)
			src_x=(src_bitmap.width() - 256) - src_x;

		src_x += cliprect.left();
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			if (col_scroll_enabled)
				column_offset=colscroll_ptr[((src_x >> 3) >> (control1[2] & 0xf)) & (0x3f >> (control1[2] & 0xf))];

			const u16 p = src_bitmap.pix16((src_y + column_offset) & height_mask, src_x & width_mask);
			const u8 colpri = flags_bitmap.pix8((src_y + column_offset) & height_mask, src_x & width_mask) & 0xf;

			src_x++;
			if ((flags & TILEMAP_DRAW_OPAQUE) || (p & bppmask))
			{
				if ((p & penmask) == pencondition)
					if((colpri & colprimask) == colpricondition)
					{
						dstpix[x] = p + (colpri & m_gfxcolmask) * bppmult;
						dstpri[x] = (dstpri[x] & pmask) | pri;
					}
			}
		}
		src_y++;
	}
}

void deco_bac06_device::deco_bac06_pf_draw(screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,u16 penmask, u16 pencondition,u16 colprimask, u16 colpricondition, u8 pri, u8 primask)
{
	tilemap_t* tm = nullptr;
	u8 bppmult = 0, bppmask = 0;

	int tm_dimensions = m_pf_control_0[3] & 0x3;
	if (tm_dimensions == 3) tm_dimensions = 1; // 3 is invalid / the same as 1?

	if (m_pf_control_0[0] & 0x1) // is 8x8 tiles mode selected?
	{
		if (m_supports_8x8)
		{
			tm = m_pf8x8_tilemap[tm_dimensions];
			bppmult = m_bppmult_8x8;
			bppmask = m_bppmask_8x8;
		}
		else if (m_supports_16x16)
		{
			tm = m_pf16x16_tilemap[tm_dimensions];
			bppmult = m_bppmult_16x16;
			bppmask = m_bppmask_16x16;
		}
	}
	else // 16x16 tiles mode is selected
	{
		if (m_supports_16x16)
		{
			tm = m_pf16x16_tilemap[tm_dimensions];
			bppmult = m_bppmult_16x16;
			bppmask = m_bppmask_16x16;
		}
		else if (m_supports_8x8)
		{
			tm = m_pf8x8_tilemap[tm_dimensions];
			bppmult = m_bppmult_8x8;
			bppmask = m_bppmask_8x8;
		}
	}

	if (tm)
		custom_tilemap_draw(bitmap,screen.priority(),cliprect,tm,m_pf_rowscroll.get(),m_pf_colscroll.get(),m_pf_control_0,m_pf_control_1,flags, penmask, pencondition, colprimask, colpricondition, bppmult, bppmask, pri, primask);
}

// used for pocket gal bootleg, which doesn't set registers properly and simply expects a fixed size tilemap.
void deco_bac06_device::deco_bac06_pf_draw_bootleg(screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect,int flags, int mode, int type, u8 pri, u8 primask)
{
	tilemap_t* tm = nullptr;
	u8 bppmult, bppmask;
	if (!mode)
	{
		tm = m_pf8x8_tilemap[type];
		bppmult = m_bppmult_8x8;
		bppmask = m_bppmask_8x8;
	}
	else
	{
		tm = m_pf16x16_tilemap[type];
		bppmult = m_bppmult_16x16;
		bppmask = m_bppmask_16x16;
	}

	custom_tilemap_draw(bitmap,screen.priority(),cliprect,tm,m_pf_rowscroll.get(),m_pf_colscroll.get(),nullptr,nullptr,flags, 0, 0, 0, 0, bppmult, bppmask, pri, primask);
}



void deco_bac06_device::pf_control_0_w(offs_t offset, u16 data, u16 mem_mask)
{
	int old_register0 = m_pf_control_0[0];

	offset &= 3;

	COMBINE_DATA(&m_pf_control_0[offset]);

	bool dirty_all = false;
	if (offset == 0)
	{
		if ((old_register0 & 2) != (m_pf_control_0[offset] & 2))
		{
			// The tilemap has changed from row major to column major or vice versa.
			// Must force an update of the mapping.
			for (int i = 0; i < 3; i++)
			{
				m_pf8x8_tilemap[i]->mark_mapping_dirty();
				m_pf16x16_tilemap[i]->mark_mapping_dirty();
			}
			dirty_all = true;
		}
	}
	if (offset == 2)
	{
		int newbank = m_pf_control_0[offset] & 1;
		if ((newbank & 1) != (m_rambank & 1))
		{
			// I don't know WHY Stadium Hero uses this as a bank but the RAM test expects it..
			// I'm curious as to if anything else sets it tho
			if (strcmp(machine().system().name,"stadhero"))
				logerror("tilemap ram bank change to %02x\n", newbank & 1);

			dirty_all = true;
			m_rambank = newbank & 1;
		}
	}

	if (dirty_all)
	{
		for (int i = 0; i < 3; i++)
		{
			m_pf8x8_tilemap[i]->mark_all_dirty();
			m_pf16x16_tilemap[i]->mark_all_dirty();
		}
	}
}

u16 deco_bac06_device::pf_control_1_r(offs_t offset)
{
	offset &= 7;
	return m_pf_control_1[offset];
}

void deco_bac06_device::pf_control_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= 7;
	COMBINE_DATA(&m_pf_control_1[offset]);
}

void deco_bac06_device::pf_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_rambank & 1) offset += 0x1000;

	COMBINE_DATA(&m_pf_data[offset]);

	for (int i = 0; i < 3; i++)
	{
		m_pf8x8_tilemap[i]->mark_tile_dirty(offset);
		m_pf16x16_tilemap[i]->mark_tile_dirty(offset);
	}
}

u16 deco_bac06_device::pf_data_r(offs_t offset)
{
	if (m_rambank & 1) offset += 0x1000;

	return m_pf_data[offset];
}

void deco_bac06_device::pf_data_8bit_w(offs_t offset, u8 data)
{
	if (offset & 1)
		pf_data_w(offset / 2, data, 0x00ff);
	else
		pf_data_w(offset / 2, data << 8, 0xff00);
}

u8 deco_bac06_device::pf_data_8bit_r(offs_t offset)
{
	if (offset & 1) /* MSB */
		return pf_data_r(offset / 2);
	else
		return pf_data_r(offset / 2)>>8;
}

void deco_bac06_device::pf_rowscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_rowscroll[offset]);
}

void deco_bac06_device::pf_colscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_colscroll[offset]);
}

u16 deco_bac06_device::pf_rowscroll_r(offs_t offset)
{
	return m_pf_rowscroll[offset];
}

u16 deco_bac06_device::pf_colscroll_r(offs_t offset)
{
	return m_pf_colscroll[offset];
}

/* used by dec8.cpp */
void deco_bac06_device::pf_control0_8bit_w(offs_t offset, u8 data)
{
	if (offset & 1)
		pf_control_0_w(offset / 2, data, 0x00ff); // oscar (mirrors?)
	else
		pf_control_0_w(offset / 2, data, 0x00ff);
}

/* used by dec8.cpp */
u8 deco_bac06_device::pf_control1_8bit_r(offs_t offset)
{
	if (offset & 1)
		return pf_control_1_r(offset / 2);
	else
		return pf_control_1_r(offset / 2)>>8;
}

/* used by dec8.cpp */
void deco_bac06_device::pf_control1_8bit_w(offs_t offset, u8 data)
{
	if (offset<4) // these registers are 16-bit?
	{
		if (offset & 1)
			pf_control_1_w(offset / 2, data, 0x00ff);
		else
			pf_control_1_w(offset / 2, data << 8, 0xff00);
	}
	else // these registers are 8-bit and mirror? (triothep vs actfancr)
	{
		if (offset & 1)
			pf_control_1_w(offset / 2, data, 0x00ff);
		else
			pf_control_1_w(offset / 2, data, 0x00ff);
	}
}

u8 deco_bac06_device::pf_rowscroll_8bit_r(offs_t offset)
{
	if (offset & 1)
		return pf_rowscroll_r(offset / 2);
	else
		return pf_rowscroll_r(offset / 2)>>8;
}


void deco_bac06_device::pf_rowscroll_8bit_w(offs_t offset, u8 data)
{
	if (offset & 1)
		pf_rowscroll_w(offset / 2, data, 0x00ff);
	else
		pf_rowscroll_w(offset / 2, data << 8, 0xff00);
}

u8 deco_bac06_device::pf_rowscroll_8bit_swap_r(offs_t offset)
{
	if (offset & 1)
		return pf_rowscroll_r(offset / 2)>>8;
	else
		return pf_rowscroll_r(offset / 2);
}

void deco_bac06_device::pf_rowscroll_8bit_swap_w(offs_t offset, u8 data)
{
	if (offset & 1)
		pf_rowscroll_w(offset / 2, data << 8, 0xff00);
	else
		pf_rowscroll_w(offset / 2, data, 0x00ff);
}


/* used by hippodrm */
void deco_bac06_device::pf_control0_8bit_packed_w(offs_t offset, u8 data)
{
	if (offset & 1)
		pf_control_0_w(offset / 2, data << 8, 0xff00);
	else
		pf_control_0_w(offset / 2, data, 0x00ff);
}

/* used by hippodrm */
void deco_bac06_device::pf_control1_8bit_swap_w(offs_t offset, u8 data)
{
	pf_control1_8bit_w(offset ^ 1, data);
}

/* used by hippodrm */
u8 deco_bac06_device::pf_data_8bit_swap_r(offs_t offset)
{
	return pf_data_8bit_r(offset ^ 1);
}

/* used by hippodrm */
void deco_bac06_device::pf_data_8bit_swap_w(offs_t offset, u8 data)
{
	pf_data_8bit_w(offset ^ 1, data);
}
