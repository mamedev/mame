#include "emu.h"
#include "includes/taito_l.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg18_tile_info )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	int attr = state->m_rambanks[2 * tile_index + 0x8000 + 1];
	int code = state->m_rambanks[2 * tile_index + 0x8000]
			| ((attr & 0x03) << 8)
			| ((state->m_bankc[(attr & 0xc) >> 2]) << 10)
			| (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_bg19_tile_info )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	int attr = state->m_rambanks[2 * tile_index + 0x9000 + 1];
	int code = state->m_rambanks[2 * tile_index + 0x9000]
			| ((attr & 0x03) << 8)
			| ((state->m_bankc[(attr & 0xc) >> 2]) << 10)
			| (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_ch1a_tile_info )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	int attr = state->m_rambanks[2 * tile_index + 0xa000 + 1];
	int code = state->m_rambanks[2 * tile_index + 0xa000] | ((attr & 0x01) << 8) | ((attr & 0x04) << 7);

	SET_TILE_INFO(
			2,
			code,
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( taitol )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	int i;

	state->m_bg18_tilemap = tilemap_create(machine, get_bg18_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_bg19_tilemap = tilemap_create(machine, get_bg19_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_ch1a_tilemap = tilemap_create(machine, get_ch1a_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_bg18_tilemap->set_transparent_pen(0);
	state->m_ch1a_tilemap->set_transparent_pen(0);

	for (i = 0; i < 256; i++)
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));

	state->m_ch1a_tilemap->set_scrolldx(-8, -8);
	state->m_bg18_tilemap->set_scrolldx(28, -11);
	state->m_bg19_tilemap->set_scrolldx(38, -21);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(taitol_state::horshoes_bankg_w)
{

	if (m_horshoes_gfxbank != data)
	{
		m_horshoes_gfxbank = data;

		m_bg18_tilemap->mark_all_dirty();
		m_bg19_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(taitol_state::taitol_bankc_w)
{

	if (m_bankc[offset] != data)
	{
		m_bankc[offset] = data;
//      logerror("Bankc %d, %02x (%04x)\n", offset, data, cpu_get_pc(&space.device()));

		m_bg18_tilemap->mark_all_dirty();
		m_bg19_tilemap->mark_all_dirty();
	}
}

READ8_MEMBER(taitol_state::taitol_bankc_r)
{
	return m_bankc[offset];
}


WRITE8_MEMBER(taitol_state::taitol_control_w)
{

//  logerror("Control Write %02x (%04x)\n", data, cpu_get_pc(&space.device()));

	m_cur_ctrl = data;
//popmessage("%02x",data);

	/* bit 0 unknown */

	/* bit 1 unknown */

	/* bit 3 controls sprite/tile priority - handled in vh_screenrefresh() */

	/* bit 4 flip screen */
	m_flipscreen = data & 0x10;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* bit 5 display enable - handled in vh_screenrefresh() */
}

READ8_MEMBER(taitol_state::taitol_control_r)
{

//  logerror("Control Read %02x (%04x)\n", cur_ctrl, cpu_get_pc(&space.device()));
	return m_cur_ctrl;
}

void taitol_chardef14_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 0);
}

void taitol_chardef15_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 128);
}

void taitol_chardef16_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 256);
}

void taitol_chardef17_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 384);
}

void taitol_chardef1c_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 512);
}

void taitol_chardef1d_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 640);
}

void taitol_chardef1e_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 768);
}

void taitol_chardef1f_m( running_machine &machine, int offset )
{
	gfx_element_mark_dirty(machine.gfx[2], offset / 32 + 896);
}

void taitol_bg18_m( running_machine &machine, int offset )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	state->m_bg18_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_bg19_m( running_machine &machine, int offset )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	state->m_bg19_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_char1a_m( running_machine &machine, int offset )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	state->m_ch1a_tilemap->mark_tile_dirty(offset / 2);
}

void taitol_obj1b_m( running_machine &machine, int offset )
{
#if 0
	if (offset >= 0x3f0 && offset <= 0x3ff)
	{
		/* scroll, handled in vh-screenrefresh */
	}
#endif
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
    Sprite format:
    00: xxxxxxxx tile number (low)
    01: xxxxxxxx tile number (high)
    02: ----xxxx color
        ----x--- priority
    03: -------x flip x
        ------x- flip y
    04: xxxxxxxx x position (low)
    05: -------x x position (high)
    06: xxxxxxxx y position
    07: xxxxxxxx unknown / ignored? Seems just garbage in many cases, e.g
                 plgirs2 bullets and raimais big bosses.
*/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	taitol_state *state = machine.driver_data<taitol_state>();
	int offs;

	/* at spriteram + 0x3f0 and 03f8 are the tilemap control registers; spriteram + 0x3e8 seems to be unused */
	for (offs = 0; offs < TAITOL_SPRITERAM_SIZE - 3 * 8; offs += 8)
	{
		int code, color, sx, sy, flipx, flipy;

		color = state->m_buff_spriteram[offs + 2] & 0x0f;
		code = state->m_buff_spriteram[offs] | (state->m_buff_spriteram[offs + 1] << 8);

		code |= (state->m_horshoes_gfxbank & 0x03) << 10;

		sx = state->m_buff_spriteram[offs + 4] | ((state->m_buff_spriteram[offs + 5] & 1) << 8);
		sy = state->m_buff_spriteram[offs + 6];
		if (sx >= 320)
			sx -= 512;
		flipx = state->m_buff_spriteram[offs + 3] & 0x01;
		flipy = state->m_buff_spriteram[offs + 3] & 0x02;

		if (state->m_flipscreen)
		{
			sx = 304 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				machine.priority_bitmap,
				(color & 0x08) ? 0xaa : 0x00,0);
	}
}


SCREEN_UPDATE_IND16( taitol )
{
	taitol_state *state = screen.machine().driver_data<taitol_state>();
	int dx, dy;

	dx = state->m_rambanks[0xb3f4] | (state->m_rambanks[0xb3f5] << 8);
	if (state->m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = state->m_rambanks[0xb3f6];

	state->m_bg18_tilemap->set_scrollx(0, -dx);
	state->m_bg18_tilemap->set_scrolly(0, -dy);

	dx = state->m_rambanks[0xb3fc] | (state->m_rambanks[0xb3fd] << 8);
	if (state->m_flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = state->m_rambanks[0xb3fe];

	state->m_bg19_tilemap->set_scrollx(0, -dx);
	state->m_bg19_tilemap->set_scrolly(0, -dy);

	if (state->m_cur_ctrl & 0x20)	/* display enable */
	{
		screen.machine().priority_bitmap.fill(0, cliprect);

		state->m_bg19_tilemap->draw(bitmap, cliprect, 0, 0);

		if (state->m_cur_ctrl & 0x08)	/* sprites always over BG1 */
			state->m_bg18_tilemap->draw(bitmap, cliprect, 0, 0);
		else					/* split priority */
			state->m_bg18_tilemap->draw(bitmap, cliprect, 0,1);

		draw_sprites(screen.machine(), bitmap, cliprect);

		state->m_ch1a_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(screen.machine().pens[0], cliprect);
	return 0;
}



SCREEN_VBLANK( taitol )
{
	// rising edge
	if (vblank_on)
	{
		taitol_state *state = screen.machine().driver_data<taitol_state>();
		UINT8 *spriteram = state->m_rambanks + 0xb000;

		memcpy(state->m_buff_spriteram, spriteram, TAITOL_SPRITERAM_SIZE);
	}
}
