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
	1,		// there may be two proms needed to construct color
	0, 63,	// start/end
	//  R,   G,   B,
	{   0,   0,   0, },		// offsets
	{   0,   3,   6, },		// shifts
	{0x07,0x07,0x03, }	    // masks
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

PALETTE_INIT( carjmbre )
{
	rgb_t *rgb;

	rgb = compute_res_net_all(machine, color_prom, &carjmbre_decode_info, &carjmbre_net_info);
	palette_set_colors(machine, 0, rgb, 64);
	palette_normalize_range(machine.palette, 0, 63, 0, 255);
	auto_free(machine, rgb);
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
				palette_set_color(machine(), i, palette_get_color(machine(), data));
		else
			// restore to initial state (black)
			for (i = 0; i < 64; i += 4)
				palette_set_color(machine(), i, RGB_BLACK);
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



static TILE_GET_INFO( get_carjmbre_tile_info )
{
	carjmbre_state *state = machine.driver_data<carjmbre_state>();
	UINT32 tile_number = state->m_videoram[tile_index] & 0xff;
	UINT8 attr = state->m_videoram[tile_index + 0x400];
	tile_number += (attr & 0x80) << 1; /* bank */

	SET_TILE_INFO(
			0,
			tile_number,
			attr & 0xf,
			0);
}

VIDEO_START( carjmbre )
{
	carjmbre_state *state = machine.driver_data<carjmbre_state>();

	state->m_cj_tilemap = tilemap_create(machine, get_carjmbre_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->save_item(NAME(state->m_flipscreen));
	state->save_item(NAME(state->m_bgcolor));
}

SCREEN_UPDATE_IND16( carjmbre )
{
	carjmbre_state *state = screen.machine().driver_data<carjmbre_state>();
	int offs, troffs, sx, sy, flipx, flipy;

	//colorram
	//76543210
	//x------- graphic bank
	//-xxx---- unused
	//----xxxx colour

	state->m_cj_tilemap->draw(bitmap, cliprect, 0, 0);

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
	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		//before copying the sprites to spriteram the game reorders the first
		//sprite to last, sprite ordering is incorrect if this isn't undone
		troffs = (offs - 4 + state->m_spriteram_size) % state->m_spriteram_size;

		//unused sprites are marked with ypos <= 0x02 (or >= 0xfd if screen flipped)
		if (state->m_spriteram[troffs] > 0x02 && state->m_spriteram[troffs] < 0xfd)
		{
			sx = state->m_spriteram[troffs + 3] - 7;
			sy = 241 - state->m_spriteram[troffs];
			flipx = (state->m_spriteram[troffs + 2] & 0x40) >> 6;
			flipy = (state->m_spriteram[troffs + 2] & 0x80) >> 7;

			if (state->m_flipscreen)
			{
				sx = (256 + (226 - sx)) % 256;
				sy = 242 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
					state->m_spriteram[troffs + 1],
					state->m_spriteram[troffs + 2] & 0xf,
					flipx,flipy,
					sx,sy,0);
		}
	}
	return 0;
}
