// license:BSD-3-Clause
// copyright-holders: hap, Curt Coder, Nicola Salmoria, Mirko Buffoni, Couriersud, Manuel Abadia
// thanks-to: Kenneth Lin (original driver author)
/*

Konami 005885
------
Some games use two of these in pair. Jackal even puts together the two 4bpp
tilemaps to form a single 8bpp one.
It manages sprites and 32x32 or 64x32 tilemap (only Double Dribble uses the
64x32 one).
The chip also generates clock and interrupt signals suitable for a 6809.
It uses 0x2000 bytes of static RAM for the tilemaps and sprite lists, and two
64kx4bit DRAMs, presumably as a double frame buffer. The maximum addressable
ROM is 0x20000 bytes (addressed 16 bits at a time). Tile and sprite data both
come from the same ROM space. Double Dribble and Jackal have external circuitry
to extend the limits and use separated addressing spaces for sprites and tiles.
All games use external circuitry to reuse one or both the tile flip attributes
as an additional address bit.
Two 256x4 lookup PROMs are also used to increase the color combinations.
All tilemap / sprite priority handling is done internally and the chip exports
5 bits of color code, composed of 1 bit indicating tile or sprite, and 4 bits
of ROM data remapped through the PROM.

inputs:
- address lines (A0-A13)
- data lines (DB0-DB7)
- misc interface stuff
- data from the gfx ROMs (RDL0-RDL7, RDU0-RDU7)
- data from the tile lookup PROMs (VCD0-VCD3)
- data from the sprite lookup PROMs (OCD0-OCD3)

outputs:
- address lines for tilemap RAM (AX0-AX12)
- data lines for tilemap RAM (VO0-VO7)
- address lines for the DRAMs (FA0-FA7)
- control lines for the DRAMs (NWR0, NWR1, NRAS, NCAS, NFOE)
- data lines for the DRAMs (FD0-FD7)
- address lines for the gfx ROMs (R0-R15)
- address lines for the tile lookup PROMs (VCF0-VCF3, VCB0-VCB3)
- address lines for the sprite lookup PROMs (OCB0-OCB3, OCF0-OCF3)
- NNMI, NIRQ, NFIR, NCPE, NCPQ, NEQ for the main CPU
- misc interface stuff
- color code to be output on screen (COL0-COL4)


control registers
000:          scroll y
001:          scroll x (low 8 bits)
002: -------x scroll x (high bit)
     ----xxx- row/colscroll control
              000 = solid scroll (finalizr, ddribble bg)
              100 = solid scroll (jackal)
              001 = ? (ddribble fg)
              011 = colscroll (jackal high scores)
              101 = rowscroll (ironhors, jackal map)
003: ------xx high bits of the tile code
     -----x-- unknown (finalizr)
     ----x--- selects sprite buffer (and makes a copy to a private buffer?)
     --x----- unknown (ironhors)
     -x------ unknown (ironhors)
     x------- unknown (ironhors, jackal)
004: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen

Above undocumented bits/regs = unknown

TODO:
- Move sprites and tilemap emulation from drivers to this device.

*/

#include "emu.h"
#include "k005885.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(K005885, k005885_device, "k005885", "Konami 005885 Video Controller")

k005885_device::k005885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, K005885, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_gfx_interface(mconfig, *this),
	m_vram(*this, "vram", 0x1000, ENDIANNESS_BIG),
	m_spriteram(*this, "spriteram", 0x1000, ENDIANNESS_BIG),
	m_ctrlram{0},
	m_scrollram{0},
	m_flipscreen(false),
	m_tilemap{nullptr, nullptr},
	m_split_tilemap(false),
	m_flipscreen_cb(*this),
	m_irq_cb(*this),
	m_firq_cb(*this),
	m_nmi_cb(*this),
	m_tile_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005885_device::device_start()
{
	m_tile_cb.resolve();
	if (m_split_tilemap) // splited single tilemap to 2 splited
	{
		m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k005885_device::get_tile_info_split<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
		m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k005885_device::get_tile_info_split<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	}
	else // single big tilemap
	{
		m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k005885_device::get_tile_info_big)), tilemap_mapper_delegate(*this, FUNC(k005885_device::tilemap_scan)), 8, 8, 64, 32);
		m_tilemap[1] = nullptr;
	}

	save_item(NAME(m_ctrlram));
	save_item(NAME(m_scrollram));
	save_item(NAME(m_flipscreen));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k005885_device::device_reset()
{
	for (int i = 0; i < 5; i++)
		ctrl_w(i, 0);
}

//-------------------------------------------------
//  tilemap handlers
//-------------------------------------------------

TILEMAP_MAPPER_MEMBER(k005885_device::tilemap_scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);    // skip 0x400
}

template<unsigned Which>
TILE_GET_INFO_MEMBER(k005885_device::get_tile_info_split)
{
	int const attr = m_vram[tile_index + (Which << 11)];
	int code = m_vram[tile_index + 0x400 + (Which << 11)];
	int color = 0;
	int flags = 0;
	int gfx = 0;
	if (!m_tile_cb.isnull())
		m_tile_cb(Which, attr, gfx, code, color, flags, get_tilebank());

	tileinfo.set(gfx, code, color, flags);
}

TILE_GET_INFO_MEMBER(k005885_device::get_tile_info_big)
{
	int const attr = m_vram[tile_index];
	int code = m_vram[tile_index + 0x400];
	int color = 0;
	int flags = 0;
	int gfx = 0;
	if (!m_tile_cb.isnull())
		m_tile_cb(0, attr, gfx, code, color, flags, get_tilebank());

	tileinfo.set(gfx, code, color, flags);
}

template <class BitmapClass>
void k005885_device::tilemap_draw_common(int i, screen_device &screen, BitmapClass &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	m_tilemap[i]->draw(screen, dest, cliprect, flags, priority, priority_mask);
}

void k005885_device::tilemap_draw(int i, screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	tilemap_draw_common(i, screen, dest, cliprect, flags, priority, priority_mask);
}

void k005885_device::tilemap_draw(int i, screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	tilemap_draw_common(i, screen, dest, cliprect, flags, priority, priority_mask);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k005885_device::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	if (m_split_tilemap)
		m_tilemap[BIT(offset, 11)]->mark_tile_dirty(offset & 0x3ff);
	else
		m_tilemap[0]->mark_tile_dirty(offset & 0xbff);
}

void k005885_device::ctrl_w(offs_t offset, u8 data)
{
	offset &= 7;
	if (offset >= 5)
		return;

	u8 const old = m_ctrlram[offset];
	switch (offset)
	{
		case 3:
			if ((old ^ data) & 3)
			{
				m_tilemap[0]->mark_all_dirty();
				if (m_split_tilemap)
					m_tilemap[1]->mark_all_dirty();
			}
			break;
		case 4:
			// clear interrupts
			if (BIT(~data & m_ctrlram[4], 1))
				m_irq_cb(CLEAR_LINE);

			if (BIT(~data & m_ctrlram[4], 2))
				m_firq_cb(CLEAR_LINE);

			if (BIT(~data & m_ctrlram[4], 0))
				m_nmi_cb(CLEAR_LINE);

			// flipscreen
			if (BIT(old ^ data, 3))
			{
				m_flipscreen = BIT(data, 3);
				if (!m_flipscreen_cb.isunset())
					m_flipscreen_cb(BIT(data, 3));
			}
			break;
	}

	m_ctrlram[offset] = data;
}

