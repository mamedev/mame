// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "k001604.h"


/***************************************************************************/
/*                                                                         */
/*                                  001604                                 */
/*                                                                         */
/***************************************************************************/


#define K001604_NUM_TILES_LAYER0        16384
#define K001604_NUM_TILES_LAYER1        4096

DEFINE_DEVICE_TYPE(K001604, k001604_device, "k001604_device", "K001604 2D tilemaps + 2x ROZ")

k001604_device::k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K001604, tag, owner, clock),
	device_gfx_interface(mconfig, *this, nullptr),
	m_layer_size(0),
	m_roz_size(0),
	m_txt_mem_offset(0),
	m_roz_mem_offset(0),
	m_layer_roz(nullptr),
	m_tile_ram(nullptr),
	m_char_ram(nullptr),
	m_reg(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001604_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	static const gfx_layout k001604_char_layout_layer_8x8 =
	{
		8, 8,
		K001604_NUM_TILES_LAYER0,
		8,
		{ 8,9,10,11,12,13,14,15 },
		{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
		8*128
	};

	static const gfx_layout k001604_char_layout_layer_16x16 =
	{
		16, 16,
		K001604_NUM_TILES_LAYER1,
		8,
		{ 8,9,10,11,12,13,14,15 },
		{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16, 9*16, 8*16, 11*16, 10*16, 13*16, 12*16, 15*16, 14*16 },
		{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256 },
		16*256
	};

	int roz_tile_size;

	m_char_ram = make_unique_clear<uint32_t[]>(0x200000 / 4);
	m_tile_ram = make_unique_clear<uint32_t[]>(0x20000 / 4);
	m_reg = make_unique_clear<uint32_t[]>(0x400 / 4);

	/* create tilemaps */
	roz_tile_size = m_roz_size ? 16 : 8;

	if (m_layer_size)
	{
		m_layer_8x8[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_8x8)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_8x8_0_size1)), 8, 8, 64, 64);
		m_layer_8x8[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_8x8)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_8x8_1_size1)), 8, 8, 64, 64);

		m_layer_roz = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_roz)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_roz_256)), roz_tile_size, roz_tile_size, 128, 64);
	}
	else
	{
		m_layer_8x8[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_8x8)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_8x8_0_size0)), 8, 8, 64, 64);
		m_layer_8x8[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_8x8)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_8x8_1_size0)), 8, 8, 64, 64);

		m_layer_roz = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_layer_roz)), tilemap_mapper_delegate(*this, FUNC(k001604_device::scan_layer_roz_128)), roz_tile_size, roz_tile_size, 128, 64);
	}

	m_layer_8x8[0]->set_transparent_pen(0);
	m_layer_8x8[1]->set_transparent_pen(0);

	set_gfx(0, std::make_unique<gfx_element>(&palette(), k001604_char_layout_layer_8x8, (uint8_t*)&m_char_ram[0], 0, palette().entries() / 16, 0));
	set_gfx(1, std::make_unique<gfx_element>(&palette(), k001604_char_layout_layer_16x16, (uint8_t*)&m_char_ram[0], 0, palette().entries() / 16, 0));

	save_pointer(NAME(m_reg), 0x400 / 4);
	save_pointer(NAME(m_char_ram), 0x200000 / 4);
	save_pointer(NAME(m_tile_ram), 0x20000 / 4);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001604_device::device_reset()
{
	memset(m_char_ram.get(), 0, 0x200000);
	memset(m_tile_ram.get(), 0, 0x10000);
	memset(m_reg.get(), 0, 0x400);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* FIXME: The TILEMAP_MAPPER below depends on parameters passed by the device interface (being game dependent).
we might simplify the code, by passing the whole TILEMAP_MAPPER as a callback in the interface, but is it really worth? */

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_8x8_0_size0)
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + m_txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_8x8_0_size1)
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + m_txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_8x8_1_size0)
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64 + m_txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_8x8_1_size1)
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 64 + m_txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_roz_128)
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + m_roz_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::scan_layer_roz_256)
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 128 + m_roz_mem_offset;
}

TILE_GET_INFO_MEMBER(k001604_device::tile_info_layer_8x8)
{
	uint32_t val = m_tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0, tile, color, flags);
}

TILE_GET_INFO_MEMBER(k001604_device::tile_info_layer_roz)
{
	uint32_t val = m_tile_ram[tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = m_roz_size ? (val & 0x7ff) : (val & 0x1fff);

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += m_roz_size ? 0x800 : 0x2000;

	SET_TILE_INFO_MEMBER(m_roz_size, tile, color, flags);
}


void k001604_device::draw_back_layer( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(0, cliprect);

	if ((m_reg[0x60 / 4] & 0x40000000) == 0)
		return;

	int tile_size = m_roz_size ? 16 : 8;

	int32_t x  = (int16_t)((m_reg[0x08] >> 16) & 0xffff);
	int32_t y  = (int16_t)((m_reg[0x08] >>  0) & 0xffff);
	int32_t xx = (int16_t)((m_reg[0x09] >>  0) & 0xffff);
	int32_t xy = (int16_t)((m_reg[0x09] >> 16) & 0xffff);
	int32_t yx = (int16_t)((m_reg[0x0a] >>  0) & 0xffff);
	int32_t yy = (int16_t)((m_reg[0x0a] >> 16) & 0xffff);

	int pivotx = (int16_t)((m_reg[0x00] >> 16) & 0xffff);
	int pivoty = (int16_t)((m_reg[0x00] >>  0) & 0xffff);

	int startx  = ((x - pivotx) * 256) * 32;
	int starty  = ((y - pivoty) * 256) * 32;
	int incxx = (xx) * 32;
	int incxy = (-xy) * 32;
	int incyx = (-yx) * 32;
	int incyy = (yy) * 32;

	bitmap_ind16& pixmap = m_layer_roz->pixmap();

	// extract start/end points
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	const rgb_t *clut = palette().palette()->entry_list_raw();

	int window_x, window_y, window_xmask, window_ymask;

	int layer_size = (m_reg[0x1b] >> 9) & 3;

	if (m_roz_size)
		window_x = ((m_reg[0x1b] >> 1) & 3) * 512;
	else
		window_x = ((m_reg[0x1b] >> 1) & 1) * 512;

	window_y = 0;

	switch (layer_size)
	{
		case 0: window_xmask = (128 * tile_size) - 1; break;
		case 2: window_xmask = (64 * tile_size) - 1; break;
		case 3: window_xmask = (32 * tile_size) - 1; break;
		default: fatalerror("k001604_draw_back_layer(): layer_size %d\n", layer_size);
	}

	window_ymask = pixmap.height() - 1;


	// loop over rows
	while (sy <= ey)
	{
		// initialize X counters
		int x = sx;
		uint32_t cx = startx;
		uint32_t cy = starty;

		uint32_t *dest = &bitmap.pix(sy, sx);

		// loop over columns
		while (x <= ex)
		{
			*dest = clut[pixmap.pix16(((cy >> 16) & window_ymask) + window_y, ((cx >> 16) & window_xmask) + window_x)];

			// advance in X
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
		}

		// advance in Y
		startx += incyx;
		starty += incyy;
		sy++;
	}
}

void k001604_device::draw_front_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int32_t x = (int16_t)((m_reg[0x00] >> 16) & 0xffff);
	int32_t y = (int16_t)((m_reg[0x00] >> 0) & 0xffff);
	int32_t yy = (int16_t)((m_reg[0x01] >> 0) & 0xffff);
	int32_t xy = (int16_t)((m_reg[0x01] >> 16) & 0xffff);
	int32_t yx = (int16_t)((m_reg[0x02] >> 0) & 0xffff);
	int32_t xx = (int16_t)((m_reg[0x02] >> 16) & 0xffff);

	int pivotx = (int16_t)(0xfec0);
	int pivoty = (int16_t)(0xff28);

	int startx = ((x - pivotx) * 256) * 32;
	int starty = ((y - pivoty) * 256) * 32;
	int incxx = (xx) * 32;
	int incxy = (-xy) * 32;
	int incyx = (-yx) * 32;
	int incyy = (yy) * 32;

	bitmap_ind16& pixmap = m_layer_8x8[0]->pixmap();

	// extract start/end points
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	const rgb_t *clut = palette().palette()->entry_list_raw();

	int window_x, window_y, window_xmask, window_ymask;

	window_x = 0;
	window_y = 0;
	window_xmask = pixmap.width() - 1;
	window_ymask = pixmap.height() - 1;


	// loop over rows
	while (sy <= ey)
	{
		// initialize X counters
		int x = sx;
		uint32_t cx = startx;
		uint32_t cy = starty;

		uint32_t *dest = &bitmap.pix(sy, sx);

		// loop over columns
		while (x <= ex)
		{
			uint16_t pix = pixmap.pix16(((cy >> 16) & window_ymask) + window_y, ((cx >> 16) & window_xmask) + window_x);
			if ((pix & 0xff) != 0)
			{
				*dest = clut[pix];
			}

			// advance in X
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
		}

		// advance in Y
		startx += incyx;
		starty += incyy;
		sy++;
	}
}

READ32_MEMBER( k001604_device::tile_r )
{
	return m_tile_ram[offset];
}

READ32_MEMBER( k001604_device::char_r )
{
	int set, bank;
	uint32_t addr;

	set = (m_reg[0x60 / 4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (m_reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (m_reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	return m_char_ram[addr];
}

READ32_MEMBER( k001604_device::reg_r )
{
	switch (offset)
	{
		case 0x54/4:    return machine().rand() << 16;
		case 0x5c/4:    return machine().rand() << 16 | machine().rand();
	}

	return m_reg[offset];
}

WRITE32_MEMBER( k001604_device::tile_w )
{
	int x/*, y*/;
	COMBINE_DATA(m_tile_ram.get() + offset);

	if (m_layer_size)
	{
		x = offset & 0xff;
		/*y = offset / 256;*/
	}
	else
	{
		x = offset & 0x7f;
		/*y = offset / 128;*/
	}

	if (m_layer_size)
	{
		if (x < 64)
		{
			m_layer_8x8[0]->mark_tile_dirty(offset);
		}
		else if (x < 128)
		{
			m_layer_8x8[1]->mark_tile_dirty(offset);
		}
		else
		{
			m_layer_roz->mark_tile_dirty(offset);
		}
	}
	else
	{
		if (x < 64)
		{
			m_layer_8x8[0]->mark_tile_dirty(offset);
		}
		else
		{
			m_layer_8x8[1]->mark_tile_dirty(offset);
		}

		m_layer_roz->mark_tile_dirty(offset);
	}
}

WRITE32_MEMBER( k001604_device::char_w )
{
	int set, bank;
	uint32_t addr;

	set = (m_reg[0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (m_reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (m_reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	COMBINE_DATA(m_char_ram.get() + addr);

	gfx(0)->mark_dirty(addr / 32);
	gfx(1)->mark_dirty(addr / 128);
}

WRITE32_MEMBER( k001604_device::reg_w )
{
	COMBINE_DATA(m_reg.get() + offset);

	switch (offset)
	{
		case 0x8:
		case 0x9:
		case 0xa:
			//printf("K001604_reg_w %02X, %08X, %08X\n", offset, data, mem_mask);
			break;
	}

	if (offset != 0x08 && offset != 0x09 && offset != 0x0a /*&& offset != 0x17 && offset != 0x18*/)
	{
		//printf("K001604_reg_w (%d), %02X, %08X, %08X at %s\n", chip, offset, data, mem_mask, m_maincpu->pc());
	}
}
