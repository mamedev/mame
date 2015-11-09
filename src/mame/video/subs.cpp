// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Subs hardware

***************************************************************************/

#include "emu.h"
#include "includes/subs.h"
#include "sound/discrete.h"

WRITE8_MEMBER(subs_state::invert1_w)
{
	if ((offset & 0x01) == 1)
	{
		m_palette->set_pen_color(0, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(1, rgb_t(0xFF, 0xFF, 0xFF));
	}
	else
	{
		m_palette->set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(0, rgb_t(0xFF, 0xFF, 0xFF));
	}
}

WRITE8_MEMBER(subs_state::invert2_w)
{
	if ((offset & 0x01) == 1)
	{
		m_palette->set_pen_color(2, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(3, rgb_t(0xFF, 0xFF, 0xFF));
	}
	else
	{
		m_palette->set_pen_color(3, rgb_t(0x00, 0x00, 0x00));
		m_palette->set_pen_color(2, rgb_t(0xFF, 0xFF, 0xFF));
	}
}


UINT32 subs_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (int offs = 0x400 - 1; offs >= 0; offs--)
	{
		int charcode;
		int sx,sy;
		int left_enable; //,right_enable;
		int left_sonar_window,right_sonar_window;

		left_sonar_window = 0;
		right_sonar_window = 0;

		charcode = m_videoram[offs];

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
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
					charcode, 1,
					0,0,sx,sy);
		else
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
					0, 1,
					0,0,sx,sy);
	}

	/* draw the motion objects */
	for (int offs = 0; offs < 4; offs++)
	{
		int sx,sy;
		int charcode;
		int prom_set;
		int sub_enable;

		sx = m_spriteram[0x00 + (offs * 2)] - 16;
		sy = m_spriteram[0x08 + (offs * 2)] - 16;
		charcode = m_spriteram[0x09 + (offs * 2)];
		if (offs < 2)
			sub_enable = m_spriteram[0x01 + (offs * 2)] & 0x80;
		else
			sub_enable = 1;

		prom_set = charcode & 0x01;
		charcode = (charcode >> 3) & 0x1F;

		/* left screen - special check for drawing right screen's sub */
		if ((offs!=0) || (sub_enable))
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					charcode + 32 * prom_set,
					0,
					0,0,sx,sy,0);
	}

	/* Update sound */
	address_space &space = machine().driver_data()->generic_space();
	m_discrete->write(space, SUBS_LAUNCH_DATA, m_spriteram[5] & 0x0f);   // Launch data
	m_discrete->write(space, SUBS_CRASH_DATA, m_spriteram[5] >> 4);      // Crash/explode data
	return 0;
}

UINT32 subs_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (int offs = 0x400 - 1; offs >= 0; offs--)
	{
		int charcode;
		int sx,sy;
		int right_enable; //, left_enable;
		int left_sonar_window,right_sonar_window;

		left_sonar_window = 0;
		right_sonar_window = 0;

		charcode = m_videoram[offs];

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
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
					charcode, 0,
					0,0,sx,sy);
		else
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
					0, 0,
					0,0,sx,sy);
	}

	/* draw the motion objects */
	for (int offs = 0; offs < 4; offs++)
	{
		int sx,sy;
		int charcode;
		int prom_set;
		int sub_enable;

		sx = m_spriteram[0x00 + (offs * 2)] - 16;
		sy = m_spriteram[0x08 + (offs * 2)] - 16;
		charcode = m_spriteram[0x09 + (offs * 2)];
		if (offs < 2)
			sub_enable = m_spriteram[0x01 + (offs * 2)] & 0x80;
		else
			sub_enable = 1;

		prom_set = charcode & 0x01;
		charcode = (charcode >> 3) & 0x1F;

		if ((offs!=1) || (sub_enable))
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					charcode + 32 * prom_set,
					0,
					0,0,sx,sy,0);
	}

	return 0;
}
