// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 8 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sprint8.h"


PALETTE_INIT_MEMBER(sprint8_state, sprint8)
{
	int i;

	for (i = 0; i < 0x10; i++)
	{
		palette.set_pen_indirect(2 * i + 0, 0x10);
		palette.set_pen_indirect(2 * i + 1, i);
	}

	palette.set_pen_indirect(0x20, 0x10);
	palette.set_pen_indirect(0x21, 0x10);
	palette.set_pen_indirect(0x22, 0x10);
	palette.set_pen_indirect(0x23, 0x11);
}


void sprint8_state::set_pens()
{
	int i;

	for (i = 0; i < 0x10; i += 8)
	{
		if (*m_team & 1)
		{
			m_palette->set_indirect_color(i + 0, rgb_t(0xff, 0x00, 0x00)); /* red     */
			m_palette->set_indirect_color(i + 1, rgb_t(0x00, 0x00, 0xff)); /* blue    */
			m_palette->set_indirect_color(i + 2, rgb_t(0xff, 0xff, 0x00)); /* yellow  */
			m_palette->set_indirect_color(i + 3, rgb_t(0x00, 0xff, 0x00)); /* green   */
			m_palette->set_indirect_color(i + 4, rgb_t(0xff, 0x00, 0xff)); /* magenta */
			m_palette->set_indirect_color(i + 5, rgb_t(0xe0, 0xc0, 0x70)); /* puce    */
			m_palette->set_indirect_color(i + 6, rgb_t(0x00, 0xff, 0xff)); /* cyan    */
			m_palette->set_indirect_color(i + 7, rgb_t(0xff, 0xaa, 0xaa)); /* pink    */
		}
		else
		{
			m_palette->set_indirect_color(i + 0, rgb_t(0xff, 0x00, 0x00)); /* red     */
			m_palette->set_indirect_color(i + 1, rgb_t(0x00, 0x00, 0xff)); /* blue    */
			m_palette->set_indirect_color(i + 2, rgb_t(0xff, 0x00, 0x00)); /* red     */
			m_palette->set_indirect_color(i + 3, rgb_t(0x00, 0x00, 0xff)); /* blue    */
			m_palette->set_indirect_color(i + 4, rgb_t(0xff, 0x00, 0x00)); /* red     */
			m_palette->set_indirect_color(i + 5, rgb_t(0x00, 0x00, 0xff)); /* blue    */
			m_palette->set_indirect_color(i + 6, rgb_t(0xff, 0x00, 0x00)); /* red     */
			m_palette->set_indirect_color(i + 7, rgb_t(0x00, 0x00, 0xff)); /* blue    */
		}
	}

	m_palette->set_indirect_color(0x10, rgb_t(0x00, 0x00, 0x00));
	m_palette->set_indirect_color(0x11, rgb_t(0xff, 0xff, 0xff));
}


TILE_GET_INFO_MEMBER(sprint8_state::get_tile_info1)
{
	UINT8 code = m_video_ram[tile_index];

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

	SET_TILE_INFO_MEMBER(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


TILE_GET_INFO_MEMBER(sprint8_state::get_tile_info2)
{
	UINT8 code = m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) != 0x28)
		color = 16;
	else
		color = 17;

	SET_TILE_INFO_MEMBER(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


WRITE8_MEMBER(sprint8_state::sprint8_video_ram_w)
{
	m_video_ram[offset] = data;
	m_tilemap1->mark_tile_dirty(offset);
	m_tilemap2->mark_tile_dirty(offset);
}


void sprint8_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper1);
	m_screen->register_screen_bitmap(m_helper2);

	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sprint8_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sprint8_state::get_tile_info2),this), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_tilemap1->set_scrolly(0, +24);
	m_tilemap2->set_scrolly(0, +24);
}


void sprint8_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		UINT8 code = m_pos_d_ram[i];

		int x = m_pos_h_ram[i];
		int y = m_pos_v_ram[i];

		if (code & 0x80)
			x |= 0x100;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code ^ 7,
			i,
			!(code & 0x10), !(code & 0x08),
			496 - x, y - 31, 0);
	}
}


TIMER_CALLBACK_MEMBER(sprint8_state::sprint8_collision_callback)
{
	sprint8_set_collision(param);
}


UINT32 sprint8_state::screen_update_sprint8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	m_tilemap1->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void sprint8_state::screen_eof_sprint8(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		int x;
		int y;
		const rectangle &visarea = m_screen->visible_area();

		m_tilemap2->draw(screen, m_helper2, visarea, 0, 0);

		m_helper1.fill(0x20, visarea);

		draw_sprites(m_helper1, visarea);

		for (y = visarea.min_y; y <= visarea.max_y; y++)
		{
			const UINT16* p1 = &m_helper1.pix16(y);
			const UINT16* p2 = &m_helper2.pix16(y);

			for (x = visarea.min_x; x <= visarea.max_x; x++)
				if (p1[x] != 0x20 && p2[x] == 0x23)
					machine().scheduler().timer_set(m_screen->time_until_pos(y + 24, x),
							timer_expired_delegate(FUNC(sprint8_state::sprint8_collision_callback),this),
							m_palette->pen_indirect(p1[x]));
		}
	}
}
