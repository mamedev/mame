// license:???
// copyright-holders:insideoutboy, Nicola Salmoria
/***************************************************************************

    Car Jamboree
    Omori Electric CAD (OEC) 1983

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/carjmbre.h"

// palette info from Popper (OEC 1983, very similar video hw)
static const res_net_decode_info carjmbre_decode_info =
{
	1,      // there may be two proms needed to construct color
	0, 63,  // start/end
	//  R,   G,   B,
	{   0,   0,   0, },     // offsets
	{   0,   3,   6, },     // shifts
	{0x07,0x07,0x03, }      // masks
};

static const res_net_info carjmbre_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

PALETTE_INIT_MEMBER(carjmbre_state, carjmbre)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, carjmbre_decode_info, carjmbre_net_info);
	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 63);
}



WRITE8_MEMBER(carjmbre_state::carjmbre_flipscreen_w)
{
	m_flipscreen = (data & 1) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	machine().tilemap().set_flip_all(m_flipscreen);
}

WRITE8_MEMBER(carjmbre_state::carjmbre_bgcolor_w)
{
	data = ~data & 0x3f;

	if (data != m_bgcolor)
	{
		int i;

		m_bgcolor = data;
		if (data & 3)
			for (i = 0; i < 64; i += 4)
				m_palette->set_pen_color(i, data);
		else
			// restore to initial state (black)
			for (i = 0; i < 64; i += 4)
				m_palette->set_pen_color(i, rgb_t::black);
	}
}

WRITE8_MEMBER(carjmbre_state::carjmbre_8806_w)
{
	// unknown, gets updated at same time as carjmbre_bgcolor_w
}

WRITE8_MEMBER(carjmbre_state::carjmbre_videoram_w)
{
	m_videoram[offset] = data;
	m_cj_tilemap->mark_tile_dirty(offset & 0x3ff);
}



TILE_GET_INFO_MEMBER(carjmbre_state::get_carjmbre_tile_info)
{
	UINT32 tile_number = m_videoram[tile_index] & 0xff;
	UINT8 attr = m_videoram[tile_index + 0x400];
	tile_number += (attr & 0x80) << 1; /* bank */

	SET_TILE_INFO_MEMBER(0,
			tile_number,
			attr & 0xf,
			0);
}

void carjmbre_state::video_start()
{
	m_cj_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(carjmbre_state::get_carjmbre_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_bgcolor));
}

UINT32 carjmbre_state::screen_update_carjmbre(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, troffs, sx, sy, flipx, flipy;

	//colorram
	//76543210
	//x------- graphic bank
	//-xxx---- unused
	//----xxxx colour

	m_cj_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	//spriteram[offs]
	//+0       y pos
	//+1       sprite number
	//+2
	//76543210
	//x------- flipy
	//-x------ flipx
	//--xx---- unused
	//----xxxx colour
	//+3       x pos
	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		//before copying the sprites to spriteram the game reorders the first
		//sprite to last, sprite ordering is incorrect if this isn't undone
		troffs = (offs - 4 + m_spriteram.bytes()) % m_spriteram.bytes();

		//unused sprites are marked with ypos <= 0x02 (or >= 0xfd if screen flipped)
		if (m_spriteram[troffs] > 0x02 && m_spriteram[troffs] < 0xfd)
		{
			sx = m_spriteram[troffs + 3] - 7;
			sy = 241 - m_spriteram[troffs];
			flipx = (m_spriteram[troffs + 2] & 0x40) >> 6;
			flipy = (m_spriteram[troffs + 2] & 0x80) >> 7;

			if (m_flipscreen)
			{
				sx = (256 + (226 - sx)) % 256;
				sy = 242 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_spriteram[troffs + 1],
					m_spriteram[troffs + 2] & 0xf,
					flipx,flipy,
					sx,sy,0);
		}
	}
	return 0;
}
