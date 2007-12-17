/***************************************************************************

    Atari Subs hardware

***************************************************************************/

#include "driver.h"
#include "subs.h"
#include "sound/discrete.h"

WRITE8_HANDLER( subs_invert1_w )
{
	if ((offset & 0x01) == 1)
	{
		palette_set_color(Machine, 0, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(Machine, 1, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
	else
	{
		palette_set_color(Machine, 1, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(Machine, 0, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
}

WRITE8_HANDLER( subs_invert2_w )
{
	if ((offset & 0x01) == 1)
	{
		palette_set_color(Machine, 2, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(Machine, 3, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
	else
	{
		palette_set_color(Machine, 3, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(Machine, 2, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
}


VIDEO_UPDATE( subs )
{
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int charcode;
		int sx,sy;
		int left_enable,right_enable;
		int left_sonar_window,right_sonar_window;

		left_sonar_window = 0;
		right_sonar_window = 0;

		charcode = videoram[offs];

		/* Which monitor is this for? */
		right_enable = charcode & 0x40;
		left_enable = charcode & 0x80;

		sx = 8 * (offs % 32);
		sy = 8 * (offs / 32);

		/* Special hardware logic for sonar windows */
		if ((sy >= (128+64)) && (sx < 32))
			left_sonar_window = 1;
		else if ((sy >= (128+64)) && (sx >= (128+64+32)))
			right_sonar_window = 1;
		else
			charcode = charcode & 0x3F;

		/* Draw the left screen */
		if (screen==0)
		{
			if ((left_enable || left_sonar_window) && (!right_sonar_window))
			{
				drawgfx(bitmap,machine->gfx[0],
						charcode, 1,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_NONE,0);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[0],
						0, 1,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_NONE,0);
			}
		}

		/* Draw the right screen */
		if (screen==1)
		{
			if ((right_enable || right_sonar_window) && (!left_sonar_window))
			{
				drawgfx(bitmap,machine->gfx[0],
						charcode, 0,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_NONE,0);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[0],
						0, 0,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_NONE,0);
			}
		}
	}

	/* draw the motion objects */
	for (offs = 0; offs < 4; offs++)
	{
		int sx,sy;
		int charcode;
		int prom_set;
		int sub_enable;

		sx = spriteram[0x00 + (offs * 2)] - 16;
		sy = spriteram[0x08 + (offs * 2)] - 16;
		charcode = spriteram[0x09 + (offs * 2)];
		if (offs < 2)
			sub_enable = spriteram[0x01 + (offs * 2)] & 0x80;
		else
			sub_enable = 1;

		prom_set = charcode & 0x01;
		charcode = (charcode >> 3) & 0x1F;

		/* Left screen - special check for drawing right screen's sub */
		if (screen==0)
		{
			if ((offs!=0) || (sub_enable))
			{
				drawgfx(bitmap,machine->gfx[1],
						charcode + 32 * prom_set,
						0,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}


		/* Right screen - special check for drawing left screen's sub */
		if (screen==1)
		{
			if ((offs!=1) || (sub_enable))
			{
				drawgfx(bitmap,machine->gfx[1],
						charcode + 32 * prom_set,
						0,
						0,0,sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}

	/* Update sound */
	discrete_sound_w(SUBS_LAUNCH_DATA, spriteram[5] & 0x0f);	// Launch data
	discrete_sound_w(SUBS_CRASH_DATA, spriteram[5] >> 4);		// Crash/explode data
	return 0;
}
