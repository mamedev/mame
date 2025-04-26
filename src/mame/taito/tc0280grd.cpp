// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0280GRD
Taito TC0430GRW
---------
These generate a zooming/rotating tilemap. The TC0280GRD has to be used in
pairs, while the TC0430GRW is a newer, single-chip solution.
Regardless of the hardware differences, the two are functionally identical
except for incxx and incxy, which need to be multiplied by 2 in the TC0280GRD
to bring them to the same scale of the other parameters (maybe the chip has
half-pixel resolution?).

control registers:
000-003 start x
004-005 incxx
006-007 incyx
008-00b start y
00c-00d incxy
00e-00f incyy
*/

#include "emu.h"
#include "tc0280grd.h"

#include <algorithm>

static constexpr unsigned TC0280GRD_RAM_SIZE = 0x2000;

DEFINE_DEVICE_TYPE(TC0280GRD, tc0280grd_device, "tc0280grd", "Taito TC0280GRD")
DEFINE_DEVICE_TYPE(TC0430GRW, tc0430grw_device, "tc0430grw", "Taito TC0430GRW")

tc0280grd_device::tc0280grd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int xmultiply)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_ram(nullptr)
	, m_base_color(0)
	, m_colorbase(0)
	, m_xmultiply(xmultiply)
{
	std::fill(std::begin(m_ctrl), std::end(m_ctrl), 0);
}

tc0280grd_device::tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0280grd_device(mconfig, TC0280GRD, tag, owner, clock, 2)
{
}

tc0430grw_device::tc0430grw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0280grd_device(mconfig, TC0430GRW, tag, owner, clock, 1)
{
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

GFXDECODE_MEMBER(tc0280grd_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_packed_msb, 0, 256)
GFXDECODE_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0280grd_device::device_start()
{
	// decode our graphics
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colorbase);

	m_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0280grd_device::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap->set_transparent_pen(0);

	m_ram = make_unique_clear<u16 []>(TC0280GRD_RAM_SIZE / 2);

	save_pointer(NAME(m_ram), TC0280GRD_RAM_SIZE / 2);
	save_item(NAME(m_ctrl));
	save_item(NAME(m_base_color));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0280grd_device::device_reset()
{
	std::fill(std::begin(m_ctrl), std::end(m_ctrl), 0);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TILE_GET_INFO_MEMBER(tc0280grd_device::get_tile_info)
{
	int const attr = m_ram[tile_index];
	tileinfo.set(0,
			attr & 0x3fff,
			((attr & 0xc000) >> 14) + m_base_color,
			0);
}

u16 tc0280grd_device::word_r(offs_t offset)
{
	return m_ram[offset];
}

void tc0280grd_device::word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);
	m_tilemap->mark_tile_dirty(offset);
}

void tc0280grd_device::ctrl_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ctrl[offset]);
}

void tc0280grd_device::tilemap_update(int base_color)
{
	if (m_base_color != base_color)
	{
		m_base_color = base_color;
		m_tilemap->mark_all_dirty();
	}
}

void tc0280grd_device::zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u8 priority, u8 priority_mask)
{
	/* 24-bit signed */
	u32 startx = ((m_ctrl[0] & 0xff) << 16) + m_ctrl[1];

	if (BIT(startx, 23))
		startx -= 0x1000000;

	int incxx = (s16)m_ctrl[2];
	incxx *= m_xmultiply;
	int const incyx = (s16)m_ctrl[3];

	/* 24-bit signed */
	u32 starty = ((m_ctrl[4] & 0xff) << 16) + m_ctrl[5];

	if (BIT(starty, 23))
		starty -= 0x1000000;

	int incxy = (s16)m_ctrl[6];
	incxy *= m_xmultiply;
	int const incyy = (s16)m_ctrl[7];

	startx -= xoffset * incxx + yoffset * incyx;
	starty -= xoffset * incxy + yoffset * incyy;

	m_tilemap->draw_roz(screen, bitmap, cliprect, startx << 4, starty << 4,
			incxx << 4, incxy << 4, incyx << 4, incyy << 4,
			1,  // copy with wraparound
			0, priority, priority_mask);
}
