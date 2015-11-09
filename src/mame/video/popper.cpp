// license:???
// copyright-holders:David Haywood,insideoutboy, Pierpaolo Prazzoli
/***************************************************************************

    Popper

    Omori Electric CAD (OEC) 1983

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/popper.h"


/***************************************************************************
 *
 * Color guns - from schematics
 *
 ***************************************************************************/

static const res_net_decode_info popper_decode_info =
{
	1,      // there may be two proms needed to construct color
	0,      // start at 0
	63, // end at 255
	//  R,   G,   B,
	{   0,   0,   0, },     // offsets
	{   0,   3,   6, },     // shifts
	{0x07,0x07,0x03, }      // masks
};

static const res_net_info popper_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

PALETTE_INIT_MEMBER(popper_state, popper)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, popper_decode_info, popper_net_info);
	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 63);
}

WRITE8_MEMBER(popper_state::popper_ol_videoram_w)
{
	m_ol_videoram[offset] = data;
	m_ol_p123_tilemap->mark_tile_dirty(offset);
	m_ol_p0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popper_state::popper_videoram_w)
{
	m_videoram[offset] = data;
	m_p123_tilemap->mark_tile_dirty(offset);
	m_p0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popper_state::popper_ol_attribram_w)
{
	m_ol_attribram[offset] = data;
	m_ol_p123_tilemap->mark_tile_dirty(offset);
	m_ol_p0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popper_state::popper_attribram_w)
{
	m_attribram[offset] = data;
	m_p123_tilemap->mark_tile_dirty(offset);
	m_p0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(popper_state::popper_flipscreen_w)
{
	m_flipscreen = data;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	if (m_flipscreen)
		m_tilemap_clip.min_x = m_tilemap_clip.max_x - 15;
	else
		m_tilemap_clip.max_x = 15;
}

WRITE8_MEMBER(popper_state::popper_e002_w)
{
	m_e002 = data;
}

WRITE8_MEMBER(popper_state::popper_gfx_bank_w)
{
	if (m_gfx_bank != data)
	{
		m_gfx_bank = data;
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(popper_state::get_popper_p123_tile_info)
{
	UINT32 tile_number = m_videoram[tile_index];
	UINT8 attr = m_attribram[tile_index];
	tile_number += m_gfx_bank << 8;

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			(attr & 0xf),
			0);
	tileinfo.group = (attr & 0x80) >> 7;
}

TILE_GET_INFO_MEMBER(popper_state::get_popper_p0_tile_info)
{
	UINT32 tile_number = m_videoram[tile_index];
	UINT8 attr = m_attribram[tile_index];
	tile_number += m_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo.group = (attr & 0x70) ? ((attr & 0x80) >> 7) : 0;

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			((attr & 0x70) >> 4) + 8,
			0);
}

TILE_GET_INFO_MEMBER(popper_state::get_popper_ol_p123_tile_info)
{
	UINT32 tile_number = m_ol_videoram[tile_index];
	UINT8 attr  = m_ol_attribram[tile_index];
	tile_number += m_gfx_bank << 8;

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			(attr & 0xf),
			0);
	tileinfo.group = (attr & 0x80) >> 7;
}

TILE_GET_INFO_MEMBER(popper_state::get_popper_ol_p0_tile_info)
{
	UINT32 tile_number = m_ol_videoram[tile_index];
	UINT8 attr = m_ol_attribram[tile_index];
	tile_number += m_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo.group = (attr & 0x70) ? ((attr & 0x80) >> 7) : 0;

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			((attr & 0x70) >> 4) + 8,
			0);
}

void popper_state::video_start()
{
	m_p123_tilemap    = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popper_state::get_popper_p123_tile_info),this),    TILEMAP_SCAN_COLS, 8, 8, 33, 32 );
	m_p0_tilemap      = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popper_state::get_popper_p0_tile_info),this),      TILEMAP_SCAN_COLS, 8, 8, 33, 32);
	m_ol_p123_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popper_state::get_popper_ol_p123_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 2, 32);
	m_ol_p0_tilemap   = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(popper_state::get_popper_ol_p0_tile_info),this),   TILEMAP_SCAN_COLS, 8, 8, 2, 32);

	m_p123_tilemap->set_transmask(0, 0x0f, 0x01);
	m_p123_tilemap->set_transmask(1, 0x01, 0x0f);
	m_p0_tilemap->set_transmask(0, 0x0f, 0x0e);
	m_p0_tilemap->set_transmask(1, 0x0e, 0x0f);
	m_ol_p123_tilemap->set_transmask(0, 0x0f, 0x01);
	m_ol_p123_tilemap->set_transmask(1, 0x01, 0x0f);
	m_ol_p0_tilemap->set_transmask(0, 0x0f, 0x0e);
	m_ol_p0_tilemap->set_transmask(1, 0x0e, 0x0f);

	m_tilemap_clip = m_screen->visible_area();
}

void popper_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	int offs, sx, sy, flipx, flipy;

	for (offs = 0; offs < m_spriteram.bytes() - 4; offs += 4)
	{
		//if y position is in the current strip
		if (m_spriteram[offs + 1] && (((m_spriteram[offs] + (m_flipscreen ? 2 : 0)) & 0xf0) == (0x0f - offs / 0x80) << 4))
		{
			//offs     y pos
			//offs+1   sprite number
			//offs+2
			//76543210
			//x------- flipy
			//-x------ flipx
			//--xx---- unused
			//----xxxx colour
			//offs+3   x pos

			sx = m_spriteram[offs + 3];
			sy = 240 - m_spriteram[offs];
			flipx = (m_spriteram[offs + 2] & 0x40) >> 6;
			flipy = (m_spriteram[offs + 2] & 0x80) >> 7;

			if (m_flipscreen)
			{
				sx = 248 - sx;
				sy = 242 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_spriteram[offs + 1],
					(m_spriteram[offs + 2] & 0x0f),
					flipx,flipy,
					sx,sy,0);
		}
	}
}

UINT32 popper_state::screen_update_popper(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle finalclip = m_tilemap_clip;
	finalclip &= cliprect;

	//attribram
	//76543210
	//x------- draw over sprites
	//-xxx---- colour for pen 0 (from second prom?)
	//----xxxx colour for pens 1,2,3

	m_p123_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_p0_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_ol_p123_tilemap->draw(screen, bitmap, finalclip, TILEMAP_DRAW_LAYER1, 0);
	m_ol_p0_tilemap->draw(screen, bitmap, finalclip, TILEMAP_DRAW_LAYER1, 0);

	draw_sprites(bitmap, cliprect);

	m_p123_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_p0_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_ol_p123_tilemap->draw(screen, bitmap, finalclip, TILEMAP_DRAW_LAYER0, 0);
	m_ol_p0_tilemap->draw(screen, bitmap, finalclip, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
