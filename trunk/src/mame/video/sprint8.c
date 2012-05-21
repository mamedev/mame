/***************************************************************************

Atari Sprint 8 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sprint8.h"


PALETTE_INIT( sprint8 )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x12);

	for (i = 0; i < 0x10; i++)
	{
		colortable_entry_set_value(machine.colortable, 2 * i + 0, 0x10);
		colortable_entry_set_value(machine.colortable, 2 * i + 1, i);
	}

	colortable_entry_set_value(machine.colortable, 0x20, 0x10);
	colortable_entry_set_value(machine.colortable, 0x21, 0x10);
	colortable_entry_set_value(machine.colortable, 0x22, 0x10);
	colortable_entry_set_value(machine.colortable, 0x23, 0x11);
}


static void set_pens(sprint8_state *state, colortable_t *colortable)
{
	int i;

	for (i = 0; i < 0x10; i += 8)
	{
		if (*state->m_team & 1)
		{
			colortable_palette_set_color(colortable, i + 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 2, MAKE_RGB(0xff, 0xff, 0x00)); /* yellow  */
			colortable_palette_set_color(colortable, i + 3, MAKE_RGB(0x00, 0xff, 0x00)); /* green   */
			colortable_palette_set_color(colortable, i + 4, MAKE_RGB(0xff, 0x00, 0xff)); /* magenta */
			colortable_palette_set_color(colortable, i + 5, MAKE_RGB(0xe0, 0xc0, 0x70)); /* puce    */
			colortable_palette_set_color(colortable, i + 6, MAKE_RGB(0x00, 0xff, 0xff)); /* cyan    */
			colortable_palette_set_color(colortable, i + 7, MAKE_RGB(0xff, 0xaa, 0xaa)); /* pink    */
		}
		else
		{
			colortable_palette_set_color(colortable, i + 0, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 1, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 2, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 3, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 4, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 5, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
			colortable_palette_set_color(colortable, i + 6, MAKE_RGB(0xff, 0x00, 0x00)); /* red     */
			colortable_palette_set_color(colortable, i + 7, MAKE_RGB(0x00, 0x00, 0xff)); /* blue    */
		}
	}

	colortable_palette_set_color(colortable, 0x10, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(colortable, 0x11, MAKE_RGB(0xff, 0xff, 0xff));
}


static TILE_GET_INFO( get_tile_info1 )
{
	sprint8_state *state = machine.driver_data<sprint8_state>();
	UINT8 code = state->m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x30) != 0x30) /* ? */
		color = 17;
	else
	{
		if ((tile_index + 1) & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;

	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


static TILE_GET_INFO( get_tile_info2 )
{
	sprint8_state *state = machine.driver_data<sprint8_state>();
	UINT8 code = state->m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) != 0x28)
		color = 16;
	else
		color = 17;

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


WRITE8_MEMBER(sprint8_state::sprint8_video_ram_w)
{
	m_video_ram[offset] = data;
	m_tilemap1->mark_tile_dirty(offset);
	m_tilemap2->mark_tile_dirty(offset);
}


VIDEO_START( sprint8 )
{
	sprint8_state *state = machine.driver_data<sprint8_state>();
	machine.primary_screen->register_screen_bitmap(state->m_helper1);
	machine.primary_screen->register_screen_bitmap(state->m_helper2);

	state->m_tilemap1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 16, 8, 32, 32);
	state->m_tilemap2 = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 16, 8, 32, 32);

	state->m_tilemap1->set_scrolly(0, +24);
	state->m_tilemap2->set_scrolly(0, +24);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	sprint8_state *state = machine.driver_data<sprint8_state>();
	int i;

	for (i = 0; i < 16; i++)
	{
		UINT8 code = state->m_pos_d_ram[i];

		int x = state->m_pos_h_ram[i];
		int y = state->m_pos_v_ram[i];

		if (code & 0x80)
			x |= 0x100;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[2],
			code ^ 7,
			i,
			!(code & 0x10), !(code & 0x08),
			496 - x, y - 31, 0);
	}
}


static TIMER_CALLBACK( sprint8_collision_callback )
{
	sprint8_set_collision(machine, param);
}


SCREEN_UPDATE_IND16( sprint8 )
{
	sprint8_state *state = screen.machine().driver_data<sprint8_state>();
	set_pens(state, screen.machine().colortable);
	state->m_tilemap1->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}


SCREEN_VBLANK( sprint8 )
{
	// rising edge
	if (vblank_on)
	{
		sprint8_state *state = screen.machine().driver_data<sprint8_state>();
		int x;
		int y;
		const rectangle &visarea = screen.machine().primary_screen->visible_area();

		state->m_tilemap2->draw(state->m_helper2, visarea, 0, 0);

		state->m_helper1.fill(0x20, visarea);

		draw_sprites(screen.machine(), state->m_helper1, visarea);

		for (y = visarea.min_y; y <= visarea.max_y; y++)
		{
			const UINT16* p1 = &state->m_helper1.pix16(y);
			const UINT16* p2 = &state->m_helper2.pix16(y);

			for (x = visarea.min_x; x <= visarea.max_x; x++)
				if (p1[x] != 0x20 && p2[x] == 0x23)
					screen.machine().scheduler().timer_set(screen.machine().primary_screen->time_until_pos(y + 24, x),
							FUNC(sprint8_collision_callback),
							colortable_entry_get_value(screen.machine().colortable, p1[x]));
		}
	}
}
