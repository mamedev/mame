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

void exidy_video_config(running_machine &machine, UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	state->m_collision_mask   = _collision_mask;
	state->m_collision_invert = _collision_invert;
	state->m_is_2bpp			 = _is_2bpp;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( exidy )
{
	exidy_state *state = machine.driver_data<exidy_state>();

	machine.primary_screen->register_screen_bitmap(state->m_background_bitmap);
	state->m_motion_object_1_vid.allocate(16, 16);
	state->m_motion_object_2_vid.allocate(16, 16);
	state->m_motion_object_2_clip.allocate(16, 16);

	state_save_register_global(machine, state->m_collision_mask);
	state_save_register_global(machine, state->m_collision_invert);
	state_save_register_global(machine, state->m_is_2bpp);
	state_save_register_global(machine, state->m_int_condition);
	state_save_register_global_bitmap(machine, &state->m_background_bitmap);
	state_save_register_global_bitmap(machine, &state->m_motion_object_1_vid);
	state_save_register_global_bitmap(machine, &state->m_motion_object_2_vid);
	state_save_register_global_bitmap(machine, &state->m_motion_object_2_clip);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INLINE void latch_condition(running_machine &machine, int collision)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	collision ^= state->m_collision_invert;
	state->m_int_condition = (input_port_read(machine, "INTSOURCE") & ~0x1c) | (collision & state->m_collision_mask);
}


INTERRUPT_GEN( exidy_vblank_interrupt )
{
	exidy_state *state = device->machine().driver_data<exidy_state>();
	/* latch the current condition */
	latch_condition(device->machine(), 0);
	state->m_int_condition &= ~0x80;

	/* set the IRQ line */
	device_set_input_line(device, 0, ASSERT_LINE);
}



READ8_MEMBER(exidy_state::exidy_interrupt_r)
{
	/* clear any interrupts */
	cputag_set_input_line(machine(), "maincpu", 0, CLEAR_LINE);

	/* return the latched condition */
	return m_int_condition;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

INLINE void set_1_color(running_machine &machine, int index, int which)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	palette_set_color_rgb(machine, index,
						  pal1bit(state->m_color_latch[2] >> which),
						  pal1bit(state->m_color_latch[1] >> which),
						  pal1bit(state->m_color_latch[0] >> which));
}

static void set_colors(running_machine &machine)
{
	/* motion object 1 */
	set_1_color(machine, 0, 0);
	set_1_color(machine, 1, 7);

	/* motion object 2 */
	set_1_color(machine, 2, 0);
	set_1_color(machine, 3, 6);

	/* characters */
	set_1_color(machine, 4, 4);
	set_1_color(machine, 5, 3);
	set_1_color(machine, 6, 2);
	set_1_color(machine, 7, 1);
}



/*************************************
 *
 *  Background update
 *
 *************************************/

static void draw_background(running_machine &machine)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	offs_t offs;

	pen_t off_pen = 0;

	for (offs = 0; offs < 0x400; offs++)
	{
		UINT8 cy;
		pen_t on_pen_1, on_pen_2;

		UINT8 y = offs >> 5 << 3;
		UINT8 code = state->m_videoram[offs];

		if (state->m_is_2bpp)
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

			if (state->m_is_2bpp)
			{
				UINT8 data1 = state->m_characterram[0x000 | (code << 3) | cy];
				UINT8 data2 = state->m_characterram[0x800 | (code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					if (data1 & 0x80)
						state->m_background_bitmap.pix16(y, x) = (data2 & 0x80) ? on_pen_2 : on_pen_1;
					else
						state->m_background_bitmap.pix16(y, x) = off_pen;

					x = x + 1;
					data1 = data1 << 1;
					data2 = data2 << 1;
				}
			}
			/* 1bpp */
			else
			{
				UINT8 data = state->m_characterram[(code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					state->m_background_bitmap.pix16(y, x) = (data & 0x80) ? on_pen_1 : off_pen;

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

INLINE int sprite_1_enabled(exidy_state *state)
{
	/* if the collision_mask is 0x00, then we are on old hardware that always has */
	/* sprite 1 enabled regardless */
	return (!(*state->m_sprite_enable & 0x80) || (*state->m_sprite_enable & 0x10) || (state->m_collision_mask == 0x00));
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	/* draw sprite 2 first */
	int sprite_set_2 = ((*state->m_sprite_enable & 0x40) != 0);

	int sx = 236 - *state->m_sprite2_xpos - 4;
	int sy = 244 - *state->m_sprite2_ypos - 4;

	drawgfx_transpen(bitmap, cliprect, machine.gfx[0],
			((*state->m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 1,
			0, 0, sx, sy, 0);

	/* draw sprite 1 next */
	if (sprite_1_enabled(state))
	{
		int sprite_set_1 = ((*state->m_sprite_enable & 0x20) != 0);

		sx = 236 - *state->m_sprite1_xpos - 4;
		sy = 244 - *state->m_sprite1_ypos - 4;

		if (sy < 0) sy = 0;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[0],
				(*state->m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
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

static TIMER_CALLBACK( collision_irq_callback )
{
	/* latch the collision bits */
	latch_condition(machine, param);

	/* set the IRQ line */
	cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
}


static void check_collision(running_machine &machine)
{
	exidy_state *state = machine.driver_data<exidy_state>();
	UINT8 sprite_set_1 = ((*state->m_sprite_enable & 0x20) != 0);
	UINT8 sprite_set_2 = ((*state->m_sprite_enable & 0x40) != 0);
	const rectangle clip(0, 15, 0, 15);
	int org_1_x = 0, org_1_y = 0;
	int org_2_x = 0, org_2_y = 0;
	int sx, sy;
	int count = 0;

	/* if there is nothing to detect, bail */
	if (state->m_collision_mask == 0)
		return;

	/* draw sprite 1 */
	state->m_motion_object_1_vid.fill(0xff, clip);
	if (sprite_1_enabled(state))
	{
		org_1_x = 236 - *state->m_sprite1_xpos - 4;
		org_1_y = 244 - *state->m_sprite1_ypos - 4;
		drawgfx_transpen(state->m_motion_object_1_vid, clip, machine.gfx[0],
				(*state->m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
				0, 0, 0, 0, 0);
	}

	/* draw sprite 2 */
	state->m_motion_object_2_vid.fill(0xff, clip);
	org_2_x = 236 - *state->m_sprite2_xpos - 4;
	org_2_y = 244 - *state->m_sprite2_ypos - 4;
	drawgfx_transpen(state->m_motion_object_2_vid, clip, machine.gfx[0],
			((*state->m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
			0, 0, 0, 0, 0);

	/* draw sprite 2 clipped to sprite 1's location */
	state->m_motion_object_2_clip.fill(0xff, clip);
	if (sprite_1_enabled(state))
	{
		sx = org_2_x - org_1_x;
		sy = org_2_y - org_1_y;
		drawgfx_transpen(state->m_motion_object_2_clip, clip, machine.gfx[0],
				((*state->m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
				0, 0, sx, sy, 0);
	}

	/* scan for collisions */
	for (sy = 0; sy < 16; sy++)
		for (sx = 0; sx < 16; sx++)
		{
			if (state->m_motion_object_1_vid.pix16(sy, sx) != 0xff)
			{
				UINT8 current_collision_mask = 0;

				/* check for background collision (M1CHAR) */
				if (state->m_background_bitmap.pix16(org_1_y + sy, org_1_x + sx) != 0)
					current_collision_mask |= 0x04;

				/* check for motion object collision (M1M2) */
				if (state->m_motion_object_2_clip.pix16(sy, sx) != 0xff)
					current_collision_mask |= 0x10;

				/* if we got one, trigger an interrupt */
				if ((current_collision_mask & state->m_collision_mask) && (count++ < 128))
					machine.scheduler().timer_set(machine.primary_screen->time_until_pos(org_1_x + sx, org_1_y + sy), FUNC(collision_irq_callback), current_collision_mask);
			}

			if (state->m_motion_object_2_vid.pix16(sy, sx) != 0xff)
			{
				/* check for background collision (M2CHAR) */
				if (state->m_background_bitmap.pix16(org_2_y + sy, org_2_x + sx) != 0)
					if ((state->m_collision_mask & 0x08) && (count++ < 128))
						machine.scheduler().timer_set(machine.primary_screen->time_until_pos(org_2_x + sx, org_2_y + sy), FUNC(collision_irq_callback), 0x08);
			}
		}
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

SCREEN_UPDATE_IND16( exidy )
{
	exidy_state *state = screen.machine().driver_data<exidy_state>();
	/* refresh the colors from the palette (static or dynamic) */
	set_colors(screen.machine());

	/* update the background and draw it */
	draw_background(screen.machine());
	copybitmap(bitmap, state->m_background_bitmap, 0, 0, 0, 0, cliprect);

	/* draw the sprites */
	draw_sprites(screen.machine(), bitmap, cliprect);

	/* check for collision, this will set the appropriate bits in collision_mask */
	check_collision(screen.machine());

	return 0;
}
