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
	1,		// there may be two proms needed to construct color
	0,		// start at 0
	63,	// end at 255
	//  R,   G,   B,
	{   0,   0,   0, },		// offsets
	{   0,   3,   6, },		// shifts
	{0x07,0x07,0x03, }	    // masks
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

/***************************************************************************
 *
 * PALETTE_INIT
 *
 ***************************************************************************/

PALETTE_INIT( popper )
{
	rgb_t	*rgb;

	rgb = compute_res_net_all(machine, color_prom, &popper_decode_info, &popper_net_info);
	palette_set_colors(machine, 0, rgb, 64);
	palette_normalize_range(machine.palette, 0, 63, 0, 255);
	auto_free(machine, rgb);
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

static TILE_GET_INFO( get_popper_p123_tile_info )
{
	popper_state *state = machine.driver_data<popper_state>();
	UINT32 tile_number = state->m_videoram[tile_index];
	UINT8 attr = state->m_attribram[tile_index];
	tile_number += state->m_gfx_bank << 8;

	SET_TILE_INFO(
			0,
			tile_number,
			(attr & 0xf),
			0);
	tileinfo.group = (attr & 0x80) >> 7;
}

static TILE_GET_INFO( get_popper_p0_tile_info )
{
	popper_state *state = machine.driver_data<popper_state>();
	UINT32 tile_number = state->m_videoram[tile_index];
	UINT8 attr = state->m_attribram[tile_index];
	tile_number += state->m_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo.group = (attr & 0x70) ? ((attr & 0x80) >> 7) : 0;

	SET_TILE_INFO(
			0,
			tile_number,
			((attr & 0x70) >> 4) + 8,
			0);
}

static TILE_GET_INFO( get_popper_ol_p123_tile_info )
{
	popper_state *state = machine.driver_data<popper_state>();
	UINT32 tile_number = state->m_ol_videoram[tile_index];
	UINT8 attr  = state->m_ol_attribram[tile_index];
	tile_number += state->m_gfx_bank << 8;

	SET_TILE_INFO(
			0,
			tile_number,
			(attr & 0xf),
			0);
	tileinfo.group = (attr & 0x80) >> 7;
}

static TILE_GET_INFO( get_popper_ol_p0_tile_info )
{
	popper_state *state = machine.driver_data<popper_state>();
	UINT32 tile_number = state->m_ol_videoram[tile_index];
	UINT8 attr = state->m_ol_attribram[tile_index];
	tile_number += state->m_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo.group = (attr & 0x70) ? ((attr & 0x80) >> 7) : 0;

	SET_TILE_INFO(
			0,
			tile_number,
			((attr & 0x70) >> 4) + 8,
			0);
}

VIDEO_START( popper )
{
	popper_state *state = machine.driver_data<popper_state>();
	state->m_p123_tilemap    = tilemap_create(machine, get_popper_p123_tile_info,    tilemap_scan_cols, 8, 8, 33, 32 );
	state->m_p0_tilemap      = tilemap_create(machine, get_popper_p0_tile_info,      tilemap_scan_cols, 8, 8, 33, 32);
	state->m_ol_p123_tilemap = tilemap_create(machine, get_popper_ol_p123_tile_info, tilemap_scan_cols, 8, 8, 2, 32);
	state->m_ol_p0_tilemap   = tilemap_create(machine, get_popper_ol_p0_tile_info,   tilemap_scan_cols, 8, 8, 2, 32);

	state->m_p123_tilemap->set_transmask(0, 0x0f, 0x01);
	state->m_p123_tilemap->set_transmask(1, 0x01, 0x0f);
	state->m_p0_tilemap->set_transmask(0, 0x0f, 0x0e);
	state->m_p0_tilemap->set_transmask(1, 0x0e, 0x0f);
	state->m_ol_p123_tilemap->set_transmask(0, 0x0f, 0x01);
	state->m_ol_p123_tilemap->set_transmask(1, 0x01, 0x0f);
	state->m_ol_p0_tilemap->set_transmask(0, 0x0f, 0x0e);
	state->m_ol_p0_tilemap->set_transmask(1, 0x0e, 0x0f);

	state->m_tilemap_clip = machine.primary_screen->visible_area();
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	popper_state *state = machine.driver_data<popper_state>();
	int offs, sx, sy, flipx, flipy;

	for (offs = 0; offs < state->m_spriteram_size - 4; offs += 4)
	{
		//if y position is in the current strip
		if (state->m_spriteram[offs + 1] && (((state->m_spriteram[offs] + (state->m_flipscreen ? 2 : 0)) & 0xf0) == (0x0f - offs / 0x80) << 4))
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

			sx = state->m_spriteram[offs + 3];
			sy = 240 - state->m_spriteram[offs];
			flipx = (state->m_spriteram[offs + 2] & 0x40) >> 6;
			flipy = (state->m_spriteram[offs + 2] & 0x80) >> 7;

			if (state->m_flipscreen)
			{
				sx = 248 - sx;
				sy = 242 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
					state->m_spriteram[offs + 1],
					(state->m_spriteram[offs + 2] & 0x0f),
					flipx,flipy,
					sx,sy,0);
		}
	}
}

SCREEN_UPDATE_IND16( popper )
{
	popper_state *state = screen.machine().driver_data<popper_state>();
	rectangle finalclip = state->m_tilemap_clip;
	finalclip &= cliprect;

	//attribram
	//76543210
	//x------- draw over sprites
	//-xxx---- colour for pen 0 (from second prom?)
	//----xxxx colour for pens 1,2,3

	state->m_p123_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	state->m_p0_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	state->m_ol_p123_tilemap->draw(bitmap, finalclip, TILEMAP_DRAW_LAYER1, 0);
	state->m_ol_p0_tilemap->draw(bitmap, finalclip, TILEMAP_DRAW_LAYER1, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_p123_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	state->m_p0_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	state->m_ol_p123_tilemap->draw(bitmap, finalclip, TILEMAP_DRAW_LAYER0, 0);
	state->m_ol_p0_tilemap->draw(bitmap, finalclip, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
