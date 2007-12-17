/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "driver.h"
#include "exidy.h"

UINT8 *exidy_characterram;

UINT8 exidy_collision_mask;
UINT8 exidy_collision_invert;

static mame_bitmap *motion_object_1_vid;
static mame_bitmap *motion_object_2_vid;
static mame_bitmap *motion_object_2_clip;

static UINT8 chardirty[256];
static UINT8 update_complete;

static UINT8 int_condition;

static UINT8 spriteno;
static UINT8 sprite_enable;
static UINT8 sprite1_xpos;
static UINT8 sprite1_ypos;
static UINT8 sprite2_xpos;
static UINT8 sprite2_ypos;

UINT8 exidy_color_latch[3];



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( exidy )
{
	video_start_generic(machine);

	motion_object_1_vid = auto_bitmap_alloc(16, 16, machine->screen[0].format);
	motion_object_2_vid = auto_bitmap_alloc(16, 16, machine->screen[0].format);
	motion_object_2_clip = auto_bitmap_alloc(16, 16, machine->screen[0].format);

	exidy_color_w(0, exidy_color_latch[0]);
	exidy_color_w(1, exidy_color_latch[1]);
	exidy_color_w(2, exidy_color_latch[2]);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INLINE void latch_condition(int collision)
{
	collision ^= exidy_collision_invert;
	int_condition = (readinputportbytag("INTSOURCE") & ~0x1c) | (collision & exidy_collision_mask);
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
		exidy_vblank_interrupt();

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
 *  Character RAM
 *
 *************************************/

WRITE8_HANDLER( exidy_characterram_w )
{
	if (exidy_characterram[offset] != data)
	{
		exidy_characterram[offset] = data;
		chardirty[offset / 8 % 256] = 1;
	}
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

INLINE void set_1_color(int index, int palette)
{
	palette_set_color_rgb(Machine, index, pal1bit(exidy_color_latch[2] >> palette), pal1bit(exidy_color_latch[1] >> palette), pal1bit(exidy_color_latch[0] >> palette));
}

WRITE8_HANDLER( exidy_color_w )
{
	exidy_color_latch[offset] = data;

	/* motion object 1 */
	set_1_color(0, 0);
	set_1_color(1, 7);

	/* motion object 2 */
	set_1_color(2, 0);
	set_1_color(3, 6);

	/* one-bit characters */
	if (Machine->gfx[0]->color_granularity == 2)
	{
		set_1_color(4, 0);		/* chars 0x00-0x3F */
		set_1_color(5, 4);
		set_1_color(6, 0);		/* chars 0x40-0x7F */
		set_1_color(7, 3);
		set_1_color(8, 0);		/* chars 0x80-0xBF */
		set_1_color(9, 2);
		set_1_color(10, 0);		/* chars 0xC0-0xFF */
		set_1_color(11, 1);
	}

	/* two-bit characters */
	else
	{
		set_1_color(4, 0);		/* chars 0x00-0x3F */
		set_1_color(5, 0);
		set_1_color(6, 4);
		set_1_color(7, 3);
		set_1_color(8, 0);		/* chars 0x40-0x7F */
		set_1_color(9, 0);
		set_1_color(10, 4);
		set_1_color(11, 3);
		set_1_color(12, 0);		/* chars 0x80-0xBF */
		set_1_color(13, 0);
		set_1_color(14, 2);
		set_1_color(15, 1);
		set_1_color(16, 0);		/* chars 0xC0-0xFF */
		set_1_color(17, 0);
		set_1_color(18, 2);
		set_1_color(19, 1);
	}
}



/*************************************
 *
 *  Sprite controls
 *
 *************************************/

WRITE8_HANDLER( exidy_sprite1_xpos_w )
{
	sprite1_xpos = data;
}

WRITE8_HANDLER( exidy_sprite1_ypos_w )
{
	sprite1_ypos = data;
}

WRITE8_HANDLER( exidy_sprite2_xpos_w )
{
	sprite2_xpos = data;
}

WRITE8_HANDLER( exidy_sprite2_ypos_w )
{
	sprite2_ypos = data;
}

WRITE8_HANDLER( exidy_spriteno_w )
{
	spriteno = data;
}

WRITE8_HANDLER( exidy_sprite_enable_w )
{
	sprite_enable = data;
}



/*************************************
 *
 *  Background update
 *
 *************************************/

static void update_background(running_machine *machine)
{
	int x, y, offs;

	/* update the background and any dirty characters in it */
	for (y = offs = 0; y < 32; y++)
		for (x = 0; x < 32; x++, offs++)
		{
			int code = videoram[offs];

			/* see if the character is dirty */
			if (chardirty[code] == 1)
			{
				decodechar(machine->gfx[0], code, exidy_characterram, machine->drv->gfxdecodeinfo[0].gfxlayout);
				chardirty[code] = 2;
			}

			/* see if the bitmap is dirty */
			if (dirtybuffer[offs] || chardirty[code])
			{
				int color = code >> 6;
				drawgfx(tmpbitmap, machine->gfx[0], code, color, 0, 0, x * 8, y * 8, NULL, TRANSPARENCY_NONE, 0);
				dirtybuffer[offs] = 0;
			}
		}

	/* reset the char dirty array */
	for (y = 0; y < 256; y++)
		if (chardirty[y] == 2)
			chardirty[y] = 0;
}



static TIMER_CALLBACK( collision_irq_callback )
{
	/* latch the collision bits */
	latch_condition(param);

	/* set the IRQ line */
	cpunum_set_input_line(0, 0, ASSERT_LINE);
}



/*************************************
 *
 *  End-of-frame callback
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

INLINE int sprite_1_enabled(void)
{
	/* if the exidy_collision_mask is 0x00, then we are on old hardware that always has */
	/* sprite 1 enabled regardless */
	return (!(sprite_enable & 0x80) || (sprite_enable & 0x10) || (exidy_collision_mask == 0x00));
}

INLINE int sprite_2_enabled(void)
{
	return (!(sprite_enable & 0x40));
}

VIDEO_EOF( exidy )
{
	UINT8 enable_set = ((sprite_enable & 0x20) != 0);
    static const rectangle clip = { 0, 15, 0, 15 };
    int bgmask = machine->gfx[0]->color_granularity - 1;
    int org_1_x = 0, org_1_y = 0;
    int org_2_x = 0, org_2_y = 0;
    int sx, sy;
	int count = 0;

	/* if there is nothing to detect, bail */
	if (exidy_collision_mask == 0)
		return;

	/* if the sprites aren't enabled, we can't collide */
	if (!sprite_1_enabled() && !sprite_2_enabled())
	{
		update_complete = 0;
		return;
	}

	/* update the background if necessary */
	if (!update_complete)
		update_background(machine);
	update_complete = 0;

	/* draw sprite 1 */
	fillbitmap(motion_object_1_vid, 0xff, &clip);
	if (sprite_1_enabled())
	{
		org_1_x = 236 - sprite1_xpos - 4;
		org_1_y = 244 - sprite1_ypos - 4;
		drawgfx(motion_object_1_vid, machine->gfx[1],
			(spriteno & 0x0f) + 16 * enable_set, 0,
			0, 0, 0, 0, &clip, TRANSPARENCY_PEN, 0);
	}

	/* draw sprite 2 */
	fillbitmap(motion_object_2_vid, 0xff, &clip);
	if (sprite_2_enabled())
	{
		org_2_x = 236 - sprite2_xpos - 4;
		org_2_y = 244 - sprite2_ypos - 4;
		drawgfx(motion_object_2_vid, machine->gfx[1],
			((spriteno >> 4) & 0x0f) + 32, 0,
			0, 0, 0, 0, &clip, TRANSPARENCY_PEN, 0);
	}

    /* draw sprite 2 clipped to sprite 1's location */
	fillbitmap(motion_object_2_clip, 0xff, &clip);
	if (sprite_1_enabled() && sprite_2_enabled())
	{
		sx = org_2_x - org_1_x;
		sy = org_2_y - org_1_y;
		drawgfx(motion_object_2_clip, machine->gfx[1],
			((spriteno >> 4) & 0x0f) + 32, 0,
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
				if (((*BITMAP_ADDR16(tmpbitmap, org_1_y + sy, org_1_x + sx) - 4) & bgmask) != 0)
					collision_mask |= 0x04;

                /* check for motion object collision (M1M2) */
				if (*BITMAP_ADDR16(motion_object_2_clip, sy, sx) != 0xff)
					collision_mask |= 0x10;

				/* if we got one, trigger an interrupt */
				if ((collision_mask & exidy_collision_mask) && count++ < 128)
					timer_set(video_screen_get_time_until_pos(0, org_1_x + sx, org_1_y + sy), collision_mask, collision_irq_callback);
            }
            if (*BITMAP_ADDR16(motion_object_2_vid, sy, sx) != 0xff)
    		{
                /* check for background collision (M2CHAR) */
				if (((*BITMAP_ADDR16(tmpbitmap, org_2_y + sy, org_2_x + sx) - 4) & bgmask) != 0)
					if ((exidy_collision_mask & 0x08) && count++ < 128)
						timer_set(video_screen_get_time_until_pos(0, org_2_x + sx, org_2_y + sy), 0x08, collision_irq_callback);
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
	int sx, sy;

	/* update the background and draw it */
	update_background(machine);
	copybitmap(bitmap, tmpbitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_NONE, 0);

	/* draw sprite 2 first */
	if (sprite_2_enabled())
	{
		sx = 236 - sprite2_xpos - 4;
		sy = 244 - sprite2_ypos - 4;

		drawgfx(bitmap, machine->gfx[1],
			((spriteno >> 4) & 0x0f) + 32, 1,
			0, 0, sx, sy, cliprect, TRANSPARENCY_PEN, 0);
	}

	/* draw sprite 1 next */
	if (sprite_1_enabled())
	{
		UINT8 enable_set = ((sprite_enable & 0x20) != 0);

		sx = 236 - sprite1_xpos - 4;
		sy = 244 - sprite1_ypos - 4;

		if (sy < 0) sy = 0;

		drawgfx(bitmap, machine->gfx[1],
			(spriteno & 0x0f) + 16 * enable_set, 0,
			0, 0, sx, sy, cliprect, TRANSPARENCY_PEN, 0);
	}

	/* indicate that we already updated the background */
	update_complete = 1;
	return 0;
}
