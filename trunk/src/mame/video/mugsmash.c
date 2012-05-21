/* video/mugsmash.c - see drivers/mugsmash.c for more info */

#include "emu.h"
#include "includes/mugsmash.h"

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{

	/* Each Sprite takes 16 bytes, 5 used? */

	/* ---- ----  xxxx xxxx  ---- ----  aaaa aaaa  ---- ----  NNNN NNNN  ---- ----  nnnn nnnn  ---- ----  yyyy yyyy (rest unused?) */

	/* x = xpos LSB
       y = ypos LSB
       N = tile number MSB
       n = tile number LSB
       a = attribute / extra
            f?XY cccc

        f = x-flip
        ? = unknown, probably y-flip
        X = xpos MSB
        y = ypos MSB
        c = colour

    */

	mugsmash_state *state = machine.driver_data<mugsmash_state>();
	const UINT16 *source = state->m_spriteram;
	const UINT16 *finish = source + 0x2000;
	const gfx_element *gfx = machine.gfx[0];

	while (source < finish)
	{
		int xpos = source[0] & 0x00ff;
		int ypos = source[4] & 0x00ff;
		int num = (source[3] & 0x00ff) | ((source[2] & 0x00ff) << 8);
		int attr = source[1];
		int flipx = (attr & 0x0080) >> 7;
		int colour = (attr & 0x000f);

		xpos += ((attr & 0x0020) >> 5) * 0x100;
		ypos += ((attr & 0x0010) >> 4) * 0x100;

		xpos -= 28;
		ypos -= 16;

		drawgfx_transpen(
				bitmap,
				cliprect,
				gfx,
				num,
				colour,
				flipx,0,
				xpos,ypos,0
				);

		source += 0x8;
	}
}

static TILE_GET_INFO( get_mugsmash_tile_info1 )
{

	/* fF-- cccc  nnnn nnnn */

	/* c = colour?
       n = number?
       F = flip-X
       f = flip-Y
    */

	mugsmash_state *state = machine.driver_data<mugsmash_state>();
	int tileno, colour, fx;

	tileno = state->m_videoram1[tile_index * 2 + 1];
	colour = state->m_videoram1[tile_index * 2] & 0x000f;
	fx = (state->m_videoram1[tile_index * 2] & 0xc0) >> 6;

	SET_TILE_INFO(1, tileno, colour, TILE_FLIPYX(fx));
}

WRITE16_MEMBER(mugsmash_state::mugsmash_videoram1_w)
{

	m_videoram1[offset] = data;
	m_tilemap1->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_mugsmash_tile_info2 )
{

	/* fF-- cccc  nnnn nnnn */

	/* c = colour?
       n = number?
       F = flip-X
       f = flip-Y
    */

	mugsmash_state *state = machine.driver_data<mugsmash_state>();
	int tileno, colour, fx;

	tileno = state->m_videoram2[tile_index * 2 + 1];
	colour = state->m_videoram2[tile_index * 2] & 0x000f;
	fx = (state->m_videoram2[tile_index * 2] & 0xc0) >> 6;

	SET_TILE_INFO(1, tileno, 16 + colour, TILE_FLIPYX(fx));
}

WRITE16_MEMBER(mugsmash_state::mugsmash_videoram2_w)
{

	m_videoram2[offset] = data;
	m_tilemap2->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(mugsmash_state::mugsmash_reg_w)
{

	m_regs1[offset] = data;
//  popmessage ("Regs %04x, %04x, %04x, %04x", mugsmash_regs1[0], mugsmash_regs1[1],mugsmash_regs1[2], mugsmash_regs1[3]);

	switch (offset)
	{
	case 0:
		m_tilemap2->set_scrollx(0, m_regs1[2] + 7);
		break;
	case 1:
		m_tilemap2->set_scrolly(0, m_regs1[3] + 4);
		break;
	case 2:
		m_tilemap1->set_scrollx(0, m_regs1[0] + 3);
		break;
	case 3:
		m_tilemap1->set_scrolly(0, m_regs1[1] + 4);
		break;
	}
}

VIDEO_START( mugsmash )
{
	mugsmash_state *state = machine.driver_data<mugsmash_state>();

	state->m_tilemap1 = tilemap_create(machine, get_mugsmash_tile_info1, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tilemap1->set_transparent_pen(0);

	state->m_tilemap2 = tilemap_create(machine, get_mugsmash_tile_info2, tilemap_scan_rows, 16, 16, 32, 32);
}

SCREEN_UPDATE_IND16( mugsmash )
{
	mugsmash_state *state = screen.machine().driver_data<mugsmash_state>();

	state->m_tilemap2->draw(bitmap, cliprect, 0, 0);
	state->m_tilemap1->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
