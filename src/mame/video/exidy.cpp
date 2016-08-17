// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "emu.h"
#include "includes/exidy.h"



/*************************************
 *
 *  Video configuration
 *
 *************************************/

void exidy_state::exidy_video_config(UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp)
{
	m_collision_mask   = _collision_mask;
	m_collision_invert = _collision_invert;
	m_is_2bpp             = _is_2bpp;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void exidy_state::video_start()
{
	m_screen->register_screen_bitmap(m_background_bitmap);
	m_motion_object_1_vid.allocate(16, 16);
	m_motion_object_2_vid.allocate(16, 16);
	m_motion_object_2_clip.allocate(16, 16);

	save_item(NAME(m_collision_mask));
	save_item(NAME(m_collision_invert));
	save_item(NAME(m_is_2bpp));
	save_item(NAME(m_int_condition));
	save_item(NAME(m_background_bitmap));
	save_item(NAME(m_motion_object_1_vid));
	save_item(NAME(m_motion_object_2_vid));
	save_item(NAME(m_motion_object_2_clip));
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

inline void exidy_state::latch_condition(int collision)
{
	collision ^= m_collision_invert;
	m_int_condition = (ioport("INTSOURCE")->read() & ~0x1c) | (collision & m_collision_mask);
}


INTERRUPT_GEN_MEMBER(exidy_state::exidy_vblank_interrupt)
{
	/* latch the current condition */
	latch_condition(0);
	m_int_condition &= ~0x80;

	/* set the IRQ line */
	device.execute().set_input_line(0, ASSERT_LINE);
}



READ8_MEMBER(exidy_state::exidy_interrupt_r)
{
	/* clear any interrupts */
	m_maincpu->set_input_line(0, CLEAR_LINE);

	/* return the latched condition */
	return m_int_condition;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

inline void exidy_state::set_1_color(int index, int which)
{
	m_palette->set_pen_color(index,
							pal1bit(m_color_latch[2] >> which),
							pal1bit(m_color_latch[1] >> which),
							pal1bit(m_color_latch[0] >> which));
}

void exidy_state::set_colors()
{
	/* motion object 1 */
	set_1_color(0, 0);
	set_1_color(1, 7);

	/* motion object 2 */
	set_1_color(2, 0);
	set_1_color(3, 6);

	/* characters */
	set_1_color(4, 4);
	set_1_color(5, 3);
	set_1_color(6, 2);
	set_1_color(7, 1);
}



/*************************************
 *
 *  Background update
 *
 *************************************/

void exidy_state::draw_background()
{
	offs_t offs;

	pen_t off_pen = 0;

	for (offs = 0; offs < 0x400; offs++)
	{
		UINT8 cy;
		pen_t on_pen_1, on_pen_2;

		UINT8 y = offs >> 5 << 3;
		UINT8 code = m_videoram[offs];

		if (m_is_2bpp)
		{
			on_pen_1 = 4 + ((code >> 6) & 0x02);
			on_pen_2 = 5 + ((code >> 6) & 0x02);
		}
		else
		{
			on_pen_1 = 4 + ((code >> 6) & 0x03);
			on_pen_2 = off_pen;  /* unused */
		}

		for (cy = 0; cy < 8; cy++)
		{
			int i;
			UINT8 x = offs << 3;

			if (m_is_2bpp)
			{
				UINT8 data1 = m_characterram[0x000 | (code << 3) | cy];
				UINT8 data2 = m_characterram[0x800 | (code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					if (data1 & 0x80)
						m_background_bitmap.pix16(y, x) = (data2 & 0x80) ? on_pen_2 : on_pen_1;
					else
						m_background_bitmap.pix16(y, x) = off_pen;

					x = x + 1;
					data1 = data1 << 1;
					data2 = data2 << 1;
				}
			}
			/* 1bpp */
			else
			{
				UINT8 data = m_characterram[(code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					m_background_bitmap.pix16(y, x) = (data & 0x80) ? on_pen_1 : off_pen;

					x = x + 1;
					data = data << 1;
				}
			}

			y = y + 1;
		}
	}
}



/*************************************
 *
 *  Sprite hardware
 *
 *************************************/

inline int exidy_state::sprite_1_enabled()
{
	/* if the collision_mask is 0x00, then we are on old hardware that always has */
	/* sprite 1 enabled regardless */
	return (!(*m_sprite_enable & 0x80) || (*m_sprite_enable & 0x10) || (m_collision_mask == 0x00));
}


void exidy_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw sprite 2 first */
	int sprite_set_2 = ((*m_sprite_enable & 0x40) != 0);

	int sx = 236 - *m_sprite2_xpos - 4;
	int sy = 244 - *m_sprite2_ypos - 4;

	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 1,
			0, 0, sx, sy, 0);

	/* draw sprite 1 next */
	if (sprite_1_enabled())
	{
		int sprite_set_1 = ((*m_sprite_enable & 0x20) != 0);

		sx = 236 - *m_sprite1_xpos - 4;
		sy = 244 - *m_sprite1_ypos - 4;

		if (sy < 0) sy = 0;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				(*m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
				0, 0, sx, sy, 0);
	}

}



/*************************************
 *
 *  Collision detection
 *
 *************************************/

/***************************************************************************

    Exidy hardware checks for two types of collisions based on the video
    signals.  If the Motion Object 1 and Motion Object 2 signals are on at
    the same time, an M1M2 collision bit gets set.  If the Motion Object 1
    and Background Character signals are on at the same time, an M1CHAR
    collision bit gets set.  So effectively, there's a pixel-by-pixel
    collision check comparing Motion Object 1 (the player) to the
    background and to the other Motion Object (typically a bad guy).

***************************************************************************/

void exidy_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_COLLISION_IRQ:
		/* latch the collision bits */
		latch_condition(param);

		/* set the IRQ line */
		m_maincpu->set_input_line(0, ASSERT_LINE);

		break;
	default:
		assert_always(FALSE, "Unknown id in exidy_state::device_timer");
	}
}


void exidy_state::check_collision()
{
	UINT8 sprite_set_1 = ((*m_sprite_enable & 0x20) != 0);
	UINT8 sprite_set_2 = ((*m_sprite_enable & 0x40) != 0);
	const rectangle clip(0, 15, 0, 15);
	int org_1_x = 0, org_1_y = 0;
	int org_2_x = 0, org_2_y = 0;
	int sx, sy;
	int count = 0;

	/* if there is nothing to detect, bail */
	if (m_collision_mask == 0)
		return;

	/* draw sprite 1 */
	m_motion_object_1_vid.fill(0xff, clip);
	if (sprite_1_enabled())
	{
		org_1_x = 236 - *m_sprite1_xpos - 4;
		org_1_y = 244 - *m_sprite1_ypos - 4;
		m_gfxdecode->gfx(0)->transpen(m_motion_object_1_vid,clip,
				(*m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
				0, 0, 0, 0, 0);
	}

	/* draw sprite 2 */
	m_motion_object_2_vid.fill(0xff, clip);
	org_2_x = 236 - *m_sprite2_xpos - 4;
	org_2_y = 244 - *m_sprite2_ypos - 4;
	m_gfxdecode->gfx(0)->transpen(m_motion_object_2_vid,clip,
			((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
			0, 0, 0, 0, 0);

	/* draw sprite 2 clipped to sprite 1's location */
	m_motion_object_2_clip.fill(0xff, clip);
	if (sprite_1_enabled())
	{
		sx = org_2_x - org_1_x;
		sy = org_2_y - org_1_y;
		m_gfxdecode->gfx(0)->transpen(m_motion_object_2_clip,clip,
				((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
				0, 0, sx, sy, 0);
	}

	/* scan for collisions */
	for (sy = 0; sy < 16; sy++)
		for (sx = 0; sx < 16; sx++)
		{
			if (m_motion_object_1_vid.pix16(sy, sx) != 0xff)
			{
				UINT8 current_collision_mask = 0;

				/* check for background collision (M1CHAR) */
				if (m_background_bitmap.pix16(org_1_y + sy, org_1_x + sx) != 0)
					current_collision_mask |= 0x04;

				/* check for motion object collision (M1M2) */
				if (m_motion_object_2_clip.pix16(sy, sx) != 0xff)
					current_collision_mask |= 0x10;

				/* if we got one, trigger an interrupt */
				if ((current_collision_mask & m_collision_mask) && (count++ < 128))
					timer_set(m_screen->time_until_pos(org_1_x + sx, org_1_y + sy), TIMER_COLLISION_IRQ, current_collision_mask);
			}

			if (m_motion_object_2_vid.pix16(sy, sx) != 0xff)
			{
				/* check for background collision (M2CHAR) */
				if (m_background_bitmap.pix16(org_2_y + sy, org_2_x + sx) != 0)
					if ((m_collision_mask & 0x08) && (count++ < 128))
						timer_set(m_screen->time_until_pos(org_2_x + sx, org_2_y + sy), TIMER_COLLISION_IRQ, 0x08);
			}
		}
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

UINT32 exidy_state::screen_update_exidy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* refresh the colors from the palette (static or dynamic) */
	set_colors();

	/* update the background and draw it */
	draw_background();
	copybitmap(bitmap, m_background_bitmap, 0, 0, 0, 0, cliprect);

	/* draw the sprites */
	draw_sprites(bitmap, cliprect);

	/* check for collision, this will set the appropriate bits in collision_mask */
	check_collision();

	return 0;
}
