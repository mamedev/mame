/***************************************************************************

    Atari Subs hardware

***************************************************************************/

#include "emu.h"
#include "includes/subs.h"
#include "sound/discrete.h"

WRITE8_MEMBER(subs_state::subs_invert1_w)
{
	if ((offset & 0x01) == 1)
	{
		palette_set_color(machine(), 0, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine(), 1, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
	else
	{
		palette_set_color(machine(), 1, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine(), 0, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
}

WRITE8_MEMBER(subs_state::subs_invert2_w)
{
	if ((offset & 0x01) == 1)
	{
		palette_set_color(machine(), 2, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine(), 3, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
	else
	{
		palette_set_color(machine(), 3, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine(), 2, MAKE_RGB(0xFF, 0xFF, 0xFF));
	}
}


SCREEN_UPDATE_IND16( subs_left )
{
	subs_state *state = screen.machine().driver_data<subs_state>();
	UINT8 *videoram = state->m_videoram;
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	device_t *discrete = screen.machine().device("discrete");

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int charcode;
		int sx,sy;
		int left_enable; //,right_enable;
		int left_sonar_window,right_sonar_window;

		left_sonar_window = 0;
		right_sonar_window = 0;

		charcode = videoram[offs];

		/* Which monitor is this for? */
//      right_enable = charcode & 0x40;
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

		/* draw the left screen */
		if ((left_enable || left_sonar_window) && (!right_sonar_window))
			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
					charcode, 1,
					0,0,sx,sy);
		else
			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
					0, 1,
					0,0,sx,sy);
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

		/* left screen - special check for drawing right screen's sub */
		if ((offs!=0) || (sub_enable))
			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[1],
					charcode + 32 * prom_set,
					0,
					0,0,sx,sy,0);
	}

	/* Update sound */
	discrete_sound_w(discrete, SUBS_LAUNCH_DATA, spriteram[5] & 0x0f);	// Launch data
	discrete_sound_w(discrete, SUBS_CRASH_DATA, spriteram[5] >> 4);		// Crash/explode data
	return 0;
}

SCREEN_UPDATE_IND16( subs_right )
{
	subs_state *state = screen.machine().driver_data<subs_state>();
	UINT8 *videoram = state->m_videoram;
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int charcode;
		int sx,sy;
		int right_enable; //, left_enable;
		int left_sonar_window,right_sonar_window;

		left_sonar_window = 0;
		right_sonar_window = 0;

		charcode = videoram[offs];

		/* Which monitor is this for? */
		right_enable = charcode & 0x40;
		//left_enable = charcode & 0x80;

		sx = 8 * (offs % 32);
		sy = 8 * (offs / 32);

		/* Special hardware logic for sonar windows */
		if ((sy >= (128+64)) && (sx < 32))
			left_sonar_window = 1;
		else if ((sy >= (128+64)) && (sx >= (128+64+32)))
			right_sonar_window = 1;
		else
			charcode = charcode & 0x3F;

		/* draw the right screen */
		if ((right_enable || right_sonar_window) && (!left_sonar_window))
			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
					charcode, 0,
					0,0,sx,sy);
		else
			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
					0, 0,
					0,0,sx,sy);
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

		if ((offs!=1) || (sub_enable))
			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[1],
					charcode + 32 * prom_set,
					0,
					0,0,sx,sy,0);
	}

	return 0;
}
