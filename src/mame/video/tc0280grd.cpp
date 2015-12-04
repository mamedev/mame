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

#define TC0280GRD_RAM_SIZE 0x2000

const device_type TC0280GRD = &device_creator<tc0280grd_device>;

tc0280grd_device::tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0280GRD, "Taito TC0280GRD & TC0430GRW", tag, owner, clock, "tc0280grd", __FILE__),
	m_ram(nullptr),
	//m_ctrl[8](0),
	m_base_color(0),
	m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void tc0280grd_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<tc0280grd_device &>(device).m_gfxdecode.set_tag(tag);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0280grd_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0280grd_device::tc0280grd_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap->set_transparent_pen(0);

	m_ram = auto_alloc_array_clear(machine(), UINT16, TC0280GRD_RAM_SIZE / 2);

	save_pointer(NAME(m_ram), TC0280GRD_RAM_SIZE / 2);
	save_item(NAME(m_ctrl));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0280grd_device::device_reset()
{
	int i;

	for (i = 0; i < 8; i++)
		m_ctrl[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TILE_GET_INFO_MEMBER(tc0280grd_device::tc0280grd_get_tile_info)
{
	int attr = m_ram[tile_index];
	SET_TILE_INFO_MEMBER(m_gfxnum,
			attr & 0x3fff,
			((attr & 0xc000) >> 14) + m_base_color,
			0);
}

READ16_MEMBER( tc0280grd_device::tc0280grd_word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( tc0280grd_device::tc0280grd_word_w )
{
	COMBINE_DATA(&m_ram[offset]);
	m_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( tc0280grd_device::tc0280grd_ctrl_word_w )
{
	COMBINE_DATA(&m_ctrl[offset]);
}

READ16_MEMBER( tc0280grd_device::tc0430grw_word_r )
{
	return tc0280grd_word_r(space, offset, mem_mask);
}

WRITE16_MEMBER( tc0280grd_device::tc0430grw_word_w )
{
	tc0280grd_word_w(space, offset, data, mem_mask);
}

WRITE16_MEMBER( tc0280grd_device::tc0430grw_ctrl_word_w )
{
	tc0280grd_ctrl_word_w(space, offset, data, mem_mask);
}

void tc0280grd_device::tc0280grd_tilemap_update( int base_color )
{
	if (m_base_color != base_color)
	{
		m_base_color = base_color;
		m_tilemap->mark_all_dirty();
	}
}

void tc0280grd_device::tc0430grw_tilemap_update( int base_color )
{
	tc0280grd_tilemap_update(base_color);
}

void tc0280grd_device::zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority, int xmultiply )
{
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	/* 24-bit signed */
	startx = ((m_ctrl[0] & 0xff) << 16) + m_ctrl[1];

	if (startx & 0x800000)
		startx -= 0x1000000;

	incxx = (INT16)m_ctrl[2];
	incxx *= xmultiply;
	incyx = (INT16)m_ctrl[3];

	/* 24-bit signed */
	starty = ((m_ctrl[4] & 0xff) << 16) + m_ctrl[5];

	if (starty & 0x800000)
		starty -= 0x1000000;

	incxy = (INT16)m_ctrl[6];
	incxy *= xmultiply;
	incyy = (INT16)m_ctrl[7];

	startx -= xoffset * incxx + yoffset * incyx;
	starty -= xoffset * incxy + yoffset * incyy;

	m_tilemap->draw_roz(screen, bitmap, cliprect, startx << 4, starty << 4,
			incxx << 4, incxy << 4, incyx << 4, incyy << 4,
			1,  /* copy with wraparound */
			0, priority);
}

void tc0280grd_device::tc0280grd_zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority )
{
	zoom_draw(screen, bitmap, cliprect, xoffset, yoffset, priority, 2);
}

void tc0280grd_device::tc0430grw_zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority )
{
	zoom_draw(screen, bitmap, cliprect, xoffset, yoffset, priority, 1);
}
