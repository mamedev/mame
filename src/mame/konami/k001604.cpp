// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "k001604.h"
#include "screen.h"


/***************************************************************************/
/*                                                                         */
/*                                  001604                                 */
/*                                                                         */
/***************************************************************************/

/*
   Character RAM:
   * Foreground tiles 2x or 4x 1Mbit SRAM in a 16-bit bus.
       - GTI Club: 37C 34C 32C 29C filled
       - NWK-TR: 34A 31A filled, no empty solder pads
       - Cobra: 6F 6H
   * Background tiles 2x or 4x 1Mbit SRAM in a 16-bit bus.
       - GTI Club: 37A 34A filled, 32A 29A empty
       - NWK-TR: 34C 31C 28C 25C empty

   Tile RAM:
   * 3x 256Kbit SRAMs in a 24-bit bus
   * Each tile entry is 24 bits, 32768 total

   CLUT RAM:
   * 2x 256KBit SRAMs on NWK-TR in a 16-bit bus (32768 colors)
   * 2x 64Kbit SRAMs on "GTI Club" in a 16-bit bus (8192 colors)
   * 3x 256KBit SRAMs on Cobra in a 24-bit bus (32768 colors). Cobra uses 24-bit colors instead of 15-bit.

   Background and foreground layer with ROZ capabilities
   Both tilemaps are 128x128 tiles, with ability to use smaller sub-tilemaps
   Foreground seems to use 8x8 tiles only, background can select 8x8 or 16x16 tiles

   Registers:

   Offset Bits
   00     sxxxxxxxxxxxxxxx ---------------- Foreground X start (13.3 fixed point)
          ---------------- sxxxxxxxxxxxxxxx Foreground Y start (13.3 fixed point)
   04     sxxxxxxxxxxxxxxx ---------------- Foreground ROZ XY (5.11 fixed point) Tilemap Y-increment per screen pixel increment
          ---------------- sxxxxxxxxxxxxxxx Foreground ROZ YY (5.11 fixed point) Tilemap Y-increment per screen line increment
   08     sxxxxxxxxxxxxxxx ---------------- Foreground ROZ XX (5.11 fixed point) Tilemap X-increment per screen pixel increment
          ---------------- sxxxxxxxxxxxxxxx Foreground ROZ YX (5.11 fixed point) Tilemap X-increment per screen line increment
   0c     ???????????????? ----------------
          ---------------- ????????????????
   20     sxxxxxxxxxxxxxxx ---------------- Background X start (13.3 fixed point)
          ---------------- sxxxxxxxxxxxxxxx Background Y start (13.3 fixed point)
   24     sxxxxxxxxxxxxxxx ---------------- Background ROZ XX (5.11 fixed point)
          ---------------- sxxxxxxxxxxxxxxx Background ROZ YX (5.11 fixed point)
   28     sxxxxxxxxxxxxxxx ---------------- Background ROZ YY (5.11 fixed point)
          ---------------- sxxxxxxxxxxxxxxx Background ROZ XY (5.11 fixed point)
   2c     ???????????????? ----------------
          ---------------- ????????????????
   40     xxxxxxxxxxxxxxxx ----------------
          ---------------- xxxxxxxxxxxxxxxx
   44     xxxxxxxxxxxxxxxx ----------------
          ---------------- xxxxxxxxxxxxxxxx
   48     xxxxxxxxxxxxxxxx ----------------
          ---------------- xxxxxxxxxxxxxxxx
   50     xxxxxxxxxxxxxxxx ----------------
          ---------------- xxxxxxxxxxxxxxxx
   54     xxxxxxxxxxxxxxxx ----------------
          ---------------- xxxxxxxxxxxxxxxx
   60     x--------------- ---------------- FG tilemap enable?
          -x-------------- ---------------- BG tilemap enable?
          -------x-------- ---------------- Select FG/BG tiles for character RAM read/write access
          -------0-------- ---------------- FG character RAM
          -------1-------- ---------------- BG character RAM
          ---------x------ ---------------- BG tile size
          ---------0------ ---------------- 16x16
          ---------1------ ---------------- 8x8
          ---------------- --------------xx Character bank for FG tiles
          ---------------- ------xx-------- Character bank for BG tiles
          ------------xx-- ---------------- Tilemap layout (both bits either set or unset)
          ------------00-- ---------------- "landscape", 256 tiles wide
          ------------11-- ---------------- "portrait", 128 tiles wide
    6c    -x-------------- ---------------- Swap FG/BG tilemap location in portrait mode? (used by Solar Assault) Might also swap left/right in landscape mode but nothing uses this.
          -0-------------- ---------------- FG Tilemap at 0x0000, BG at 0x4000
          -1-------------- ---------------- FG Tilemap at 0x4000, BG at 0x0000
          --------x------- ---------------- ?
          ---------xx----- ---------------- FG sub tilemap width?
          ------------x--- ---------------- ?
          -------------xx- ---------------- FG sub tilemap height?
          ---------------- ----x----------- Enable BG sub tilemap?
          ---------------- -----xx--------- BG sub tilemap width?
          ---------------- -----00--------- 128 tiles
          ---------------- -----10--------- 64 tiles
          ---------------- -----11--------- 32 tiles
          ---------------- --------x------- ?
          ---------------- ---------xx----- BG sub tilemap height?
          ---------------- ---------00----- 128 tiles
          ---------------- ---------10----- 64 tiles
          ---------------- ---------11----- 32 tiles
          ---------------- ------------x--- ?
          ---------------- -------------xx- BG sub tilemap X (in units of 32 tiles)


   Tilemap layout:
   "landscape" mode:

           0               128              256
   0x0000  +----------------+----------------+
           |                |                |
           |                |                |
           |   Foreground   |   Background   |
           |  128x128 tiles |  128x128 tiles |
           |                |                |
           |                |                |
   0x8000  +----------------+----------------+

   "portrait" mode:

           0               128
   0x0000  +----------------+
           |                |
           |                |
           |   Foreground   |
           |  128x128 tiles |
           |                |
           |                |
   0x4000  +----------------+
           |                |
           |                |
           |   Background   |
           |  128x128 tiles |
           |                |
           |                |
           +----------------+

    Tilemap sub-split:
    Tilemap can be split in X and Y direction into 32/64/128 x 32/64/128 sub-tilemaps
    Sub-tilemap selection works in units of 32 tiles

           0     32    64    96   128
           +-----+-----+-----+-----+
           |     |     |     |     |
           | 0,0 | 1,0 | 2,0 | 3,0 |
        32 +-----+-----+-----+-----+
           |     |     |     |     |
           | 0,1 | 1,1 | 2,1 | 3,1 |
        64 +-----+-----+-----+-----+
           |     |     |     |     |
           | 0,2 | 1,2 | 2,2 | 3,2 |
        96 +-----+-----+-----+-----+
           |     |     |     |     |
           | 0,3 | 1,3 | 2,3 | 3,3 |
       128 +-----+-----+-----+-----+



*/

DEFINE_DEVICE_TYPE(K001604, k001604_device, "k001604_device", "K001604 2D tilemaps + 2x ROZ")

k001604_device::k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K001604, tag, owner, clock),
	device_gfx_interface(mconfig, *this, nullptr),
	m_tile_ram(nullptr),
	m_fg_char_ram(nullptr),
	m_bg_char_ram(nullptr),
	m_reg(nullptr),
	m_irq(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001604_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	static const gfx_layout char_layout_8x8 =
	{
		8, 8,
		8192,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
		8*64
	};

	static const gfx_layout char_layout_16x16 =
	{
		16, 16,
		2048,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8, 9*8, 8*8, 11*8, 10*8, 13*8, 12*8, 15*8, 14*8 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
		16*128
	};

	m_fg_char_ram = make_unique_clear<uint8_t[]>(0x80000);      // 4x 128Kx8
	m_bg_char_ram = make_unique_clear<uint8_t[]>(0x80000);      // 4x 128Kx8
	m_tile_ram = make_unique_clear<uint32_t[]>(0x8000);         // 32K x 24 bits
	m_reg = make_unique_clear<uint32_t[]>(0x400 / 4);

	/* create tilemaps */
	m_fg_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_fg)), TILEMAP_SCAN_ROWS, 8, 8, 128, 128);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap8 = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_bg8)), TILEMAP_SCAN_ROWS, 8, 8, 128, 128);
	m_bg_tilemap8->set_transparent_pen(0);

	m_bg_tilemap16 = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k001604_device::tile_info_bg16)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
	m_bg_tilemap16->set_transparent_pen(0);

	set_gfx(0, std::make_unique<gfx_element>(&palette(), char_layout_8x8, &m_fg_char_ram[0], 0, palette().entries() / 256, 0));
	set_gfx(1, std::make_unique<gfx_element>(&palette(), char_layout_8x8, &m_bg_char_ram[0], 0, palette().entries() / 256, 0));
	set_gfx(2, std::make_unique<gfx_element>(&palette(), char_layout_16x16, &m_bg_char_ram[0], 0, palette().entries() / 256, 0));

	save_pointer(NAME(m_reg), 0x400 / 4);
	save_pointer(NAME(m_fg_char_ram), 0x80000);
	save_pointer(NAME(m_bg_char_ram), 0x80000);
	save_pointer(NAME(m_tile_ram), 0x8000);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001604_device::device_reset()
{
	memset(m_fg_char_ram.get(), 0, 0x80000);
	memset(m_bg_char_ram.get(), 0, 0x80000);
	memset(m_tile_ram.get(), 0, 0x8000);
	memset(m_reg.get(), 0, 0x400);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TILE_GET_INFO_MEMBER(k001604_device::tile_info_fg)
{
	uint32_t tilebase = (m_reg[0x18] & 0x40000) ? ((m_reg[0x1b] & 0x40000000) ? 0x4000 : 0x0000) : 0x0000;
	uint32_t x = tile_index & 0x7f;
	uint32_t y = tile_index / 128;
	uint32_t tilemap_pitch = (m_reg[0x18] & 0x40000) ? 128 : 256;
	uint32_t val = m_tile_ram[(y * tilemap_pitch) + x + tilebase];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x1fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tileinfo.set(0, tile, color, flags);
}

TILE_GET_INFO_MEMBER(k001604_device::tile_info_bg8)
{
	uint32_t tilebase = (m_reg[0x18] & 0x40000) ? ((m_reg[0x1b] & 0x40000000) ? 0x0000 : 0x4000) : 0x0000;
	uint32_t x = tile_index & 0x7f;
	uint32_t y = tile_index / 128;
	uint32_t tilemap_pitch = (m_reg[0x18] & 0x40000) ? 128 : 256;
	uint32_t tilemap_xstart = (m_reg[0x18] & 0x40000) ? 0 : 128;
	uint32_t val = m_tile_ram[(y * tilemap_pitch) + x + tilemap_xstart + tilebase];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x1fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tileinfo.set(1, tile, color, flags);
}

TILE_GET_INFO_MEMBER(k001604_device::tile_info_bg16)
{
	uint32_t tilebase = (m_reg[0x18] & 0x40000) ? ((m_reg[0x1b] & 0x40000000) ? 0x0000 : 0x4000) : 0x0000;
	uint32_t x = tile_index & 0x7f;
	uint32_t y = tile_index / 128;
	uint32_t tilemap_pitch = (m_reg[0x18] & 0x40000) ? 128 : 256;
	uint32_t tilemap_xstart = (m_reg[0x18] & 0x40000) ? 0 : 128;
	uint32_t val = m_tile_ram[(y * tilemap_pitch) + x + tilemap_xstart + tilebase];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x7ff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tileinfo.set(2, tile, color, flags);
}



void k001604_device::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool front, tilemap_t* tilemap)
{
	const rectangle& visarea = screen.visible_area();
	static const int SUBTILEMAP_DIMENSION[4] = { 128, 128, 64, 32 };        // entry 1 seems unused

	int32_t startx, starty, incxx, incyx, incxy, incyy;

	if (front)
	{
		startx = (int32_t)((int16_t)(m_reg[0x00] >> 16)) << 13;
		starty = (int32_t)((int16_t)(m_reg[0x00])) << 13;
		incyy = (int32_t)((int16_t)(m_reg[0x01])) << 5;
		incxy = (int32_t)((int16_t)(m_reg[0x01] >> 16)) << 5;
		incyx = (int32_t)((int16_t)(m_reg[0x02])) << 5;
		incxx= (int32_t)((int16_t)(m_reg[0x02] >> 16)) << 5;
	}
	else
	{
		startx = (int32_t)((int16_t)(m_reg[0x08] >> 16)) << 13;
		starty = (int32_t)((int16_t)(m_reg[0x08])) << 13;
		incxx = (int32_t)((int16_t)(m_reg[0x09])) << 5;
		incyx = (int32_t)((int16_t)(m_reg[0x09] >> 16)) << 5;
		incxy = (int32_t)((int16_t)(m_reg[0x0a])) << 5;
		incyy = (int32_t)((int16_t)(m_reg[0x0a] >> 16)) << 5;
	}

	bitmap_ind16& pixmap = tilemap->pixmap();
	const rgb_t* clut = palette().palette()->entry_list_raw();

	// extract start/end points
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	uint32_t sub_x, sub_y, sub_xmask, sub_ymask;
	if (front)
	{
		sub_xmask = (128 * 8) - 1;
		sub_ymask = (128 * 8) - 1;
		sub_x = 0;
		sub_y = 0;
	}
	else
	{
		int tile_size = (m_reg[0x18] & 0x400000) ? 8 : 16;

		if (m_reg[0x1b] & 0x800)
		{
			sub_xmask = (SUBTILEMAP_DIMENSION[(m_reg[0x1b] >> 9) & 0x3] * tile_size) - 1;
			sub_ymask = (SUBTILEMAP_DIMENSION[(m_reg[0x1b] >> 5) & 0x3] * tile_size) - 1;
			sub_x = (((m_reg[0x1b] >> 1) & 0x3) * 32)* tile_size;
			sub_y = 0 * tile_size;
		}
		else
		{
			sub_xmask = (128 * tile_size) - 1;
			sub_ymask = (128 * tile_size) - 1;
			sub_x = 0;
			sub_y = 0;
		}
	}

	// draw the tilemap
	//
	// loop over rows
	while (sy <= ey)
	{
		// initialize X counters
		int x = sx;
		uint32_t cx = startx;
		uint32_t cy = starty;

		uint32_t* dest = &bitmap.pix(sy, sx);

		// loop over columns
		while (x <= ex)
		{
			uint16_t pen = pixmap.pix((((cy >> 16) + visarea.min_y) & sub_ymask) + sub_y, (((cx >> 16) + visarea.min_x) & sub_xmask) + sub_x);
			if ((pen & 0xff) != 0 || !front)
			{
				*dest = clut[pen];
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



void k001604_device::draw_back_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(0, cliprect);

	if ((m_reg[0x60 / 4] & 0x40000000) == 0)
		return;

	draw_tilemap(screen, bitmap, cliprect, false, (m_reg[0x18] & 0x400000) ? m_bg_tilemap8 : m_bg_tilemap16);
}

void k001604_device::draw_front_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	if ((m_reg[0x60 / 4] & 0x80000000) == 0)
		return;

	draw_tilemap(screen, bitmap, cliprect, true, m_fg_tilemap);
}

uint32_t k001604_device::tile_r(offs_t offset)
{
	return m_tile_ram[offset];
}

uint32_t k001604_device::char_r(offs_t offset)
{
	int chip;
	uint32_t addr;

	bool bg = (m_reg[0x60 / 4] & 0x1000000) ? true : false;

	// select individual RAM chip to access
	if (bg)
		chip = (m_reg[0x60 / 4] >> 8) & 0x3;
	else
		chip = (m_reg[0x60 / 4] & 0x3);

	addr = (offset + (chip * 0x10000)) * 2;

	uint32_t res = 0;
	res |= (uint32_t)(bg ? m_bg_char_ram[addr+1] : m_fg_char_ram[addr+1]) << 24;
	res |= (uint32_t)(bg ? m_bg_char_ram[addr] : m_fg_char_ram[addr]) << 8;

	return res;
}

uint32_t k001604_device::reg_r(offs_t offset)
{
	switch (offset)
	{
		case 0x54/4:    return machine().rand() << 16;
		case 0x5c/4:    return machine().rand() << 16 | machine().rand();
	}

	return m_reg[offset];
}

void k001604_device::tile_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_tile_ram.get() + offset);

	if (m_reg[0x18] & 0x40000)
	{
		// portrait
		uint32_t fg_tilebase = (m_reg[0x1b] & 0x40000000) ? 0x4000 : 0x0000;
		uint32_t bg_tilebase = (m_reg[0x1b] & 0x40000000) ? 0x0000 : 0x4000;

		if (offset >= fg_tilebase && offset < fg_tilebase + 0x4000)
		{
			m_fg_tilemap->mark_tile_dirty(offset - fg_tilebase);
		}
		if (offset >= bg_tilebase && offset < bg_tilebase + 0x4000)
		{
			if (m_reg[0x18] & 0x400000)
				m_bg_tilemap8->mark_tile_dirty(offset - bg_tilebase);
			else
				m_bg_tilemap16->mark_tile_dirty(offset - bg_tilebase);
		}
	}
	else
	{
		// landscape
		uint32_t x = offset & 0xff;
		uint32_t y = offset / 256;
		if (x < 128)
		{
			m_fg_tilemap->mark_tile_dirty((y * 128) + x);
		}
		else
		{
			if (m_reg[0x18] & 0x400000)
				m_bg_tilemap8->mark_tile_dirty((y * 128) + (x - 128));
			else
				m_bg_tilemap16->mark_tile_dirty((y * 128) + (x - 128));
		}
	}
}

void k001604_device::char_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int chip;
	uint32_t addr;

	bool bg = (m_reg[0x60 / 4] & 0x1000000) ? true : false;

	// select individual RAM chip to access
	if (bg)
		chip = (m_reg[0x60 / 4] >> 8) & 0x3;
	else
		chip = (m_reg[0x60 / 4] & 0x3);

	addr = (offset + (chip * 0x10000)) * 2;

	if (bg)
	{
		if (ACCESSING_BITS_24_31)
		{
			m_bg_char_ram[addr+1] = data >> 24;
		}
		if (ACCESSING_BITS_8_15)
		{
			m_bg_char_ram[addr] = data >> 8;
		}

		gfx(1)->mark_dirty(addr / 64);
		gfx(2)->mark_dirty(addr / 256);
	}
	else
	{
		if (ACCESSING_BITS_24_31)
		{
			m_fg_char_ram[addr+1] = data >> 24;
		}
		if (ACCESSING_BITS_8_15)
		{
			m_fg_char_ram[addr] = data >> 8;
		}

		gfx(0)->mark_dirty(addr / 64);
	}
}

void k001604_device::reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_reg.get() + offset);

	if (offset == 0x5c / 4)
	{
		if (ACCESSING_BITS_24_31)
		{
			// Cobra clears and enables 0x1 in the IRQ handler
			// (1 = enable VBLANK IRQ, 0 = clear IRQ?)
			if ((data & 0x1) == 0)
			{
				m_irq(CLEAR_LINE);
			}
		}
	}
}
