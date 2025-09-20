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

#define LOG_UNKNOWN (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

void k005885_device::regs_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(k005885_device::yscroll_r), FUNC(k005885_device::yscroll_w));
	map(0x01, 0x01).rw(FUNC(k005885_device::xscroll_lsb_r), FUNC(k005885_device::xscroll_lsb_w));
	map(0x02, 0x02).rw(FUNC(k005885_device::scrollmode_r), FUNC(k005885_device::scrollmode_w));
	map(0x03, 0x03).rw(FUNC(k005885_device::bank_r), FUNC(k005885_device::bank_w));
	map(0x04, 0x04).rw(FUNC(k005885_device::irq_flip_r), FUNC(k005885_device::irq_flip_w));
}

DEFINE_DEVICE_TYPE(K005885, k005885_device, "k005885", "Konami 005885 Video Controller")

k005885_device::k005885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, K005885, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_gfx_interface(mconfig, *this),
	m_vram(*this, "vram", 0x1000, ENDIANNESS_BIG),
	m_spriteram(*this, "spriteram", 0x1000, ENDIANNESS_BIG),
	m_scrollram{0},
	m_xscroll(0),
	m_yscroll(0),
	m_scrollmode(0),
	m_tilebank(0),
	m_spriterambank(0),
	m_nmi_enable(false),
	m_irq_enable(false),
	m_firq_enable(false),
	m_flipscreen(false),
	m_tilemap{nullptr, nullptr},
	m_split_tilemap(false),
	m_flipscreen_cb(*this),
	m_irq_cb(*this),
	m_firq_cb(*this),
	m_nmi_cb(*this),
	m_sprite_cb(*this),
	m_tile_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005885_device::device_start()
{
	m_sprite_cb.resolve();
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

	save_item(NAME(m_scrollram));
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_scrollmode));
	save_item(NAME(m_tilebank));
	save_item(NAME(m_spriterambank));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_firq_enable));
	save_item(NAME(m_flipscreen));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k005885_device::device_reset()
{
	m_xscroll = 0;
	m_yscroll = 0;
	m_scrollmode = 0;
	m_tilebank = 0;
	m_spriterambank = 0;
	irq_flip_w(0);
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

//-------------------------------------------------
//  sprite handlers
//-------------------------------------------------

/*
	Sprite format (5 bytes per sprites)

	Offset Bit       Description
	       7654 3210
	00     xxxx xxxx Code LSB
	01     xxxx ---- Palette index
	       ---- xx-- Code MSB or Sprite quarter select in 8x8 sprite mode*
	       ---- --xx Code MSB
	02     xxxx xxxx Y position
	03     xxxx xxxx X position LSB
	04     -x-- ---- Flip Y
	       --x- ---- Flip X
		   ---x xx-- Sprite size*
	       ---- ---x X position MSB*

	* Depended by hardware?
	Unmarked bits are unused/unknown
*/
void k005885_device::sprite_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip, u8 bank, u32 len, u8 gfxnum)
{
	u8 const *const src = &m_spriteram[bank << 11];
	len -= len % 5;
	gfx_element *sgfx = gfx(gfxnum);
	for (int offs = 0; offs < len; offs += 5)
	{
		int sy  = src[offs + 2];
		int sx  = src[offs + 3];
		int const attr = src[offs + 4];
		bool flipx = BIT(attr, 5);
		bool flipy = BIT(attr, 6);
		int code = src[offs] | ((src[offs + 1] & 0x0f) << 8);
		int color = ((src[offs + 1] & 0xf0) >> 4);
		sx |= (attr & 0x01) << 8;

		if (flip)
		{
			flipx = !flipx;
			flipy = !flipy;
		}
		if (!m_sprite_cb.isnull())
			m_sprite_cb(screen, bitmap, cliprect,
				attr & 0x1c, flip, sgfx, code, color, sx, sy, flipx, flipy);
	}
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

u8 k005885_device::yscroll_r()
{
	return m_yscroll;
}

u8 k005885_device::xscroll_lsb_r()
{
	return u8(m_xscroll);
}

u8 k005885_device::scrollmode_r()
{
	return (m_scrollmode << 1) | BIT(m_xscroll, 8);
}

u8 k005885_device::bank_r()
{
	return (m_spriterambank << 3) | m_tilebank;
}

u8 k005885_device::irq_flip_r()
{
	return (m_nmi_enable ? 0x01 : 0x00) |
			(m_irq_enable ? 0x02 : 0x00) |
			(m_firq_enable ? 0x04 : 0x00) |
			(m_flipscreen ? 0x08 : 0x00);
}

void k005885_device::yscroll_w(u8 data)
{
	m_yscroll = data;
}

void k005885_device::xscroll_lsb_w(u8 data)
{
	m_xscroll = (m_xscroll & ~0x0ff) | data;
}

void k005885_device::scrollmode_w(u8 data)
{
	m_scrollmode = (data & 0x0e) >> 1;
	m_xscroll = (m_xscroll & ~0x100) | (u16(data & 1) << 8);
	if (data & 0xf0)
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown scrollmode_w write %02x", machine().describe_context(), data);
}

void k005885_device::bank_w(u8 data)
{
	if ((m_tilebank ^ data) & 3)
	{
		m_tilemap[0]->mark_all_dirty();
		if (m_split_tilemap)
			m_tilemap[1]->mark_all_dirty();
	}
	m_tilebank = data & 3;
	m_spriterambank = BIT(data, 3);
	if (data & 0xf4)
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown bank_w write %02x", machine().describe_context(), data);
}

void k005885_device::irq_flip_w(u8 data)
{
	// clear interrupts
	if (bool(BIT(~data, 1)) && m_irq_enable)
		m_irq_cb(CLEAR_LINE);

	if (bool(BIT(~data, 2)) && m_firq_enable)
		m_firq_cb(CLEAR_LINE);

	if (bool(BIT(~data, 0)) && m_nmi_enable)
		m_nmi_cb(CLEAR_LINE);

	m_nmi_enable = BIT(data, 0);
	m_irq_enable = BIT(data, 1);
	m_firq_enable = BIT(data, 2);

	// flipscreen
	if (m_flipscreen != bool(BIT(data, 3)))
	{
		m_flipscreen = BIT(data, 3);
		if (!m_flipscreen_cb.isunset())
			m_flipscreen_cb(BIT(data, 3));
	}
	if (data & 0xf0)
		LOGMASKED(LOG_UNKNOWN, "%s: Unknown irq_flip_w write %02x", machine().describe_context(), data);
}
