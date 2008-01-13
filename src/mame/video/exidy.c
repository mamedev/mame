/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "driver.h"


UINT8 *exidy_videoram;
UINT8 *exidy_characterram;
UINT8 *exidy_color_latch;
UINT8 *exidy_sprite1_xpos;
UINT8 *exidy_sprite1_ypos;
UINT8 *exidy_sprite2_xpos;
UINT8 *exidy_sprite2_ypos;
UINT8 *exidy_spriteno;
UINT8 *exidy_sprite_enable;

static UINT8 collision_mask;
static UINT8 collision_invert;
static int is_2bpp;
static UINT8 int_condition;

static mame_bitmap *background_bitmap;
static mame_bitmap *motion_object_1_vid;
static mame_bitmap *motion_object_2_vid;
static mame_bitmap *motion_object_2_clip;



/*************************************
 *
 *  Video configuration
 *
 *************************************/

void exidy_video_config(UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp)
{
	collision_mask   = _collision_mask;
	collision_invert = _collision_invert;
	is_2bpp			 = _is_2bpp;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( exidy )
{
	background_bitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	motion_object_1_vid = auto_bitmap_alloc(16, 16, machine->screen[0].format);
	motion_object_2_vid = auto_bitmap_alloc(16, 16, machine->screen[0].format);
	motion_object_2_clip = auto_bitmap_alloc(16, 16, machine->screen[0].format);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INLINE void latch_condition(int collision)
{
	collision ^= collision_invert;
	int_condition = (readinputportbytag("INTSOURCE") & ~0x1c) | (collision & collision_mask);
}


INTERRUPT_GEN( exidy_vblank_interrupt )
{
	/* latch the current condition */
	latch_condition(0);
	int_condition &= ~0x80;

	/* set the IRQ line */
	cpunum_set_input_line(0, 0, ASSERT_LINE);
}


INTERRUPT_GEN( teetert_vblank_interrupt )
{
	/* standard stuff */
	if (cpu_getiloops() == 0)
		exidy_vblank_interrupt(machine, cpunum);

	/* plus a pulse on the NMI line */
	cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
}


READ8_HANDLER( exidy_interrupt_r )
{
	/* clear any interrupts */
	cpunum_set_input_line(0, 0, CLEAR_LINE);

	/* return the latched condition */
	return int_condition;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

INLINE void set_1_color(running_machine *machine, int index, int which)
{
	palette_set_color_rgb(machine, index,
						  pal1bit(exidy_color_latch[2] >> which),
						  pal1bit(exidy_color_latch[1] >> which),
						  pal1bit(exidy_color_latch[0] >> which));
}

static void set_colors(running_machine *machine)
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

static void draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	offs_t offs;

	pen_t off_pen = 0;

	for (offs = 0; offs < 0x400; offs++)
	{
		UINT8 cy;
		pen_t on_pen_1, on_pen_2;

		UINT8 y = offs >> 5 << 3;
		UINT8 code = exidy_videoram[offs];

		if (is_2bpp)
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

			if (is_2bpp)
			{
				UINT8 data1 = exidy_characterram[0x000 | (code << 3) | cy];
				UINT8 data2 = exidy_characterram[0x800 | (code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					if (data1 & 0x80)
						*BITMAP_ADDR16(bitmap, y, x) = (data2 & 0x80) ? on_pen_2 : on_pen_1;
					else
						*BITMAP_ADDR16(bitmap, y, x) = off_pen;

					x = x + 1;
					data1 = data1 << 1;
					data2 = data2 << 1;
				}
			}
			/* 1bpp */
			else
			{
				UINT8 data = exidy_characterram[(code << 3) | cy];

				for (i = 0; i < 8; i++)
				{
					*BITMAP_ADDR16(bitmap, y, x) = (data & 0x80) ? on_pen_1 : off_pen;

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

INLINE int sprite_1_enabled(void)
{
	/* if the collision_mask is 0x00, then we are on old hardware that always has */
	/* sprite 1 enabled regardless */
	return (!(*exidy_sprite_enable & 0x80) || (*exidy_sprite_enable & 0x10) || (collision_mask == 0x00));
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* draw sprite 2 first */
	int enable_set_2 = ((*exidy_sprite_enable & 0x40) != 0);

	int sx = 236 - *exidy_sprite2_xpos - 4;
	int sy = 244 - *exidy_sprite2_ypos - 4;

	drawgfx(bitmap, machine->gfx[0],
			((*exidy_spriteno >> 4) & 0x0f) + 32 + 16 * enable_set_2, 1,
			0, 0, sx, sy, cliprect, TRANSPARENCY_PEN, 0);

	/* draw sprite 1 next */
	if (sprite_1_enabled())
	{
		int enable_set_1 = ((*exidy_sprite_enable & 0x20) != 0);

		sx = 236 - *exidy_sprite1_xpos - 4;
		sy = 244 - *exidy_sprite1_ypos - 4;

		if (sy < 0) sy = 0;

		drawgfx(bitmap, machine->gfx[0],
				(*exidy_spriteno & 0x0f) + 16 * enable_set_1, 0,
				0, 0, sx, sy, cliprect, TRANSPARENCY_PEN, 0);
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
	latch_condition(param);

	/* set the IRQ line */
	cpunum_set_input_line(0, 0, ASSERT_LINE);
}


static void check_collision(running_machine *machine)
{
	UINT8 enable_set_1 = ((*exidy_sprite_enable & 0x20) != 0);
	UINT8 enable_set_2 = ((*exidy_sprite_enable & 0x40) != 0);
    static const rectangle clip = { 0, 15, 0, 15 };
    int bgmask = machine->gfx[0]->color_granularity - 1;
    int org_1_x = 0, org_1_y = 0;
    int org_2_x = 0, org_2_y = 0;
    int sx, sy;
	int count = 0;

	/* if there is nothing to detect, bail */
	if (collision_mask == 0)
		return;

	/* draw sprite 1 */
	fillbitmap(motion_object_1_vid, 0xff, &clip);
	if (sprite_1_enabled())
	{
		org_1_x = 236 - *exidy_sprite1_xpos - 4;
		org_1_y = 244 - *exidy_sprite1_ypos - 4;
		drawgfx(motion_object_1_vid, machine->gfx[0],
				(*exidy_spriteno & 0x0f) + 16 * enable_set_1, 0,
				0, 0, 0, 0, &clip, TRANSPARENCY_PEN, 0);
	}

	/* draw sprite 2 */
	fillbitmap(motion_object_2_vid, 0xff, &clip);
	org_2_x = 236 - *exidy_sprite2_xpos - 4;
	org_2_y = 244 - *exidy_sprite2_ypos - 4;
	drawgfx(motion_object_2_vid, machine->gfx[0],
			((*exidy_spriteno >> 4) & 0x0f) + 32 + 16 * enable_set_2, 0,
			0, 0, 0, 0, &clip, TRANSPARENCY_PEN, 0);

	/* draw sprite 2 clipped to sprite 1's location */
	fillbitmap(motion_object_2_clip, 0xff, &clip);
	if (sprite_1_enabled())
	{
		sx = org_2_x - org_1_x;
		sy = org_2_y - org_1_y;
		drawgfx(motion_object_2_clip, machine->gfx[0],
				((*exidy_spriteno >> 4) & 0x0f) + 32 + 16 * enable_set_2, 0,
				0, 0, sx, sy, &clip, TRANSPARENCY_PEN, 0);
	}

	/* scan for collisions */
	for (sy = 0; sy < 16; sy++)
		for (sx = 0; sx < 16; sx++)
		{
    		if (*BITMAP_ADDR16(motion_object_1_vid, sy, sx) != 0xff)
			{
				UINT8 collision_mask = 0;

				/* check for background collision (M1CHAR) */
				if (((*BITMAP_ADDR16(background_bitmap, org_1_y + sy, org_1_x + sx) - 4) & bgmask) != 0)
					collision_mask |= 0x04;

				/* check for motion object collision (M1M2) */
				if (*BITMAP_ADDR16(motion_object_2_clip, sy, sx) != 0xff)
					collision_mask |= 0x10;

				/* if we got one, trigger an interrupt */
				if ((collision_mask & collision_mask) && count++ < 128)
					timer_set(video_screen_get_time_until_pos(0, org_1_x + sx, org_1_y + sy), NULL, collision_mask, collision_irq_callback);
			}

			if (*BITMAP_ADDR16(motion_object_2_vid, sy, sx) != 0xff)
			{
				/* check for background collision (M2CHAR) */
				if (((*BITMAP_ADDR16(background_bitmap, org_2_y + sy, org_2_x + sx) - 4) & bgmask) != 0)
					if ((collision_mask & 0x08) && count++ < 128)
						timer_set(video_screen_get_time_until_pos(0, org_2_x + sx, org_2_y + sy), NULL, 0x08, collision_irq_callback);
			}
		}
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

VIDEO_UPDATE( exidy )
{
	/* refresh the colors from the palette (static or dynamic) */
	set_colors(machine);

	/* update the background and draw it */
	draw_background(machine, background_bitmap, NULL);
	copybitmap(bitmap, background_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_NONE, 0);

	/* draw the sprites */
	draw_sprites(machine, bitmap, NULL);

	/* check for collision, this will set the appropriate bits in collision_mask */
	check_collision(machine);

	return 0;
}
