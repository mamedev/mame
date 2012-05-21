/***************************************************************************

Atari Tank 8 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/tank8.h"


PALETTE_INIT( tank8 )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x0a);

	colortable_palette_set_color(machine.colortable, 8, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(machine.colortable, 9, MAKE_RGB(0xff, 0xff, 0xff));

	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine.colortable, 2 * i + 0, 8);
		colortable_entry_set_value(machine.colortable, 2 * i + 1, i);
	}

	/* walls */
	colortable_entry_set_value(machine.colortable, 0x10, 8);
	colortable_entry_set_value(machine.colortable, 0x11, 9);

	/* mines */
	colortable_entry_set_value(machine.colortable, 0x12, 8);
	colortable_entry_set_value(machine.colortable, 0x13, 9);
}


static void set_pens(tank8_state *state, colortable_t *colortable)
{
	if (*state->m_team & 0x01)
	{
		colortable_palette_set_color(colortable, 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 2, MAKE_RGB(0xff, 0xff, 0x00)); /* yellow  */
		colortable_palette_set_color(colortable, 3, MAKE_RGB(0x00, 0xff, 0x00)); /* green   */
		colortable_palette_set_color(colortable, 4, MAKE_RGB(0xff, 0x00, 0xff)); /* magenta */
		colortable_palette_set_color(colortable, 5, MAKE_RGB(0xe0, 0xc0, 0x70)); /* puce    */
		colortable_palette_set_color(colortable, 6, MAKE_RGB(0x00, 0xff, 0xff)); /* cyan    */
		colortable_palette_set_color(colortable, 7, MAKE_RGB(0xff, 0xaa, 0xaa)); /* pink    */
	}
	else
	{
		colortable_palette_set_color(colortable, 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 2, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 4, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 6, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
		colortable_palette_set_color(colortable, 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 3, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 5, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		colortable_palette_set_color(colortable, 7, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
	}
}


WRITE8_MEMBER(tank8_state::tank8_video_ram_w)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}



static TILE_GET_INFO( tank8_get_tile_info )
{
	tank8_state *state = machine.driver_data<tank8_state>();
	UINT8 code = state->m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) == 0x28)
	{
		if ((code & 7) != 3)
			color = 8; /* walls */
		else
			color = 9; /* mines */
	}
	else
	{
		if (tile_index & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;
	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



VIDEO_START( tank8 )
{
	tank8_state *state = machine.driver_data<tank8_state>();
	machine.primary_screen->register_screen_bitmap(state->m_helper1);
	machine.primary_screen->register_screen_bitmap(state->m_helper2);
	machine.primary_screen->register_screen_bitmap(state->m_helper3);

	state->m_tilemap = tilemap_create(machine, tank8_get_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	/* VBLANK starts on scanline #256 and ends on scanline #24 */

	state->m_tilemap->set_scrolly(0, 2 * 24);
}


static int get_x_pos(tank8_state *state, int n)
{
	return 498 - state->m_pos_h_ram[n] - 2 * (state->m_pos_d_ram[n] & 128); /* ? */
}


static int get_y_pos(tank8_state *state, int n)
{
	return 2 * state->m_pos_v_ram[n] - 62;
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tank8_state *state = machine.driver_data<tank8_state>();
	int i;

	for (i = 0; i < 8; i++)
	{
		UINT8 code = ~state->m_pos_d_ram[i];

		int x = get_x_pos(state, i);
		int y = get_y_pos(state, i);

		drawgfx_transpen(bitmap, cliprect, machine.gfx[(code & 0x04) ? 2 : 3],
			code & 0x03,
			i,
			code & 0x10,
			code & 0x08,
			x,
			y, 0);
	}
}


static void draw_bullets(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tank8_state *state = machine.driver_data<tank8_state>();
	int i;

	for (i = 0; i < 8; i++)
	{
		int x = get_x_pos(state, 8 + i);
		int y = get_y_pos(state, 8 + i);

		x -= 4; /* ? */

		rectangle rect(x, x + 3, y, y + 4);
		rect &= cliprect;

		bitmap.fill((i << 1) | 0x01, rect);
	}
}


static TIMER_CALLBACK( tank8_collision_callback )
{
	tank8_set_collision(machine, param);
}


SCREEN_UPDATE_IND16( tank8 )
{
	tank8_state *state = screen.machine().driver_data<tank8_state>();
	set_pens(state, screen.machine().colortable);
	state->m_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);
	draw_bullets(screen.machine(), bitmap, cliprect);
	return 0;
}


SCREEN_VBLANK( tank8 )
{
	// on falling edge
	if (!vblank_on)
	{
		tank8_state *state = screen.machine().driver_data<tank8_state>();
		int x;
		int y;
		const rectangle &visarea = screen.machine().primary_screen->visible_area();

		state->m_tilemap->draw(state->m_helper1, visarea, 0, 0);

		state->m_helper2.fill(8, visarea);
		state->m_helper3.fill(8, visarea);

		draw_sprites(screen.machine(), state->m_helper2, visarea);
		draw_bullets(screen.machine(), state->m_helper3, visarea);

		for (y = visarea.min_y; y <= visarea.max_y; y++)
		{
			int _state = 0;

			const UINT16* p1 = &state->m_helper1.pix16(y);
			const UINT16* p2 = &state->m_helper2.pix16(y);
			const UINT16* p3 = &state->m_helper3.pix16(y);

			if (y % 2 != screen.machine().primary_screen->frame_number() % 2)
				continue; /* video display is interlaced */

			for (x = visarea.min_x; x <= visarea.max_x; x++)
			{
				UINT8 index;

				/* neither wall nor mine */
				if ((p1[x] != 0x11) && (p1[x] != 0x13))
				{
					_state = 0;
					continue;
				}

				/* neither tank nor bullet */
				if ((p2[x] == 8) && (p3[x] == 8))
				{
					_state = 0;
					continue;
				}

				/* bullets cannot hit mines */
				if ((p3[x] != 8) && (p1[x] == 0x13))
				{
					_state = 0;
					continue;
				}

				if (_state)
					continue;

				if (p3[x] != 8)
				{
					index = ((p3[x] & ~0x01) >> 1) | 0x18;

					if (1)
						index |= 0x20;

					if (0)
						index |= 0x40;

					if (1)
						index |= 0x80;
				}
				else
				{
					int sprite_num = (p2[x] & ~0x01) >> 1;
					index = sprite_num | 0x10;

					if (p1[x] == 0x11)
						index |= 0x20;

					if (y - get_y_pos(state, sprite_num) >= 8)
						index |= 0x40; /* collision on bottom side */

					if (x - get_x_pos(state, sprite_num) >= 8)
						index |= 0x80; /* collision on right side */
				}

				screen.machine().scheduler().timer_set(screen.time_until_pos(y, x), FUNC(tank8_collision_callback), index);

				_state = 1;
			}
		}
	}
}
