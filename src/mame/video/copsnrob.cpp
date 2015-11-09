// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Atari Cops'n Robbers hardware

***************************************************************************/

#include "emu.h"
#include "includes/copsnrob.h"


UINT32 copsnrob_state::screen_update_copsnrob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, x, y;

	/* redrawing the entire display is faster in this case */

	for (offs = m_videoram.bytes(); offs >= 0; offs--)
	{
		int sx,sy;

		sx = 31 - (offs % 32);
		sy = offs / 32;

		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
				m_videoram[offs] & 0x3f,0,
				0,0,
				8*sx,8*sy);
	}


	/* Draw the cars. Positioning was based on a screen shot */
	if (m_cary[0])
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_carimage[0],0,
				1,0,
				0xe4,256 - m_cary[0],0);

	if (m_cary[1])
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_carimage[1],0,
				1,0,
				0xc4,256 - m_cary[1],0);

	if (m_cary[2])
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_carimage[2],0,
				0,0,
				0x24,256 - m_cary[2],0);

	if (m_cary[3])
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_carimage[3],0,
				0,0,
				0x04,256 - m_cary[3],0);


	/* Draw the beer truck. Positioning was based on a screen shot.
	    We scan the truck's window RAM for a location whose bit is set and
	    which corresponds either to the truck's front end or the truck's back
	    end (based on the value of the truck image line sync register). We
	    then draw a truck image in the proper place and continue scanning.
	    This is not a perfect emulation of the game hardware, but it should
	    suffice for the way the game software uses the hardware.  It does take
	    care of the problem of displaying multiple beer trucks and of scrolling
	    truck images smoothly off the top of the screen. */

	for (y = 0; y < 256; y++)
	{
		/* y is going up the screen, but the truck window RAM locations
		go down the screen. */

		if (m_truckram[255 - y])
		{
			/* The hardware only uses the low 5 bits of the truck image line
			sync register. */
			if ((m_trucky[0] & 0x1f) == ((y + 31) & 0x1f))
			{
				/* We've hit a truck's back end, so draw the truck.  The front
				   end may be off the top of the screen, but we don't care. */
				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						0,0,
						0,0,
						0x80,256 - (y + 31),0);
				/* Skip past this truck's front end so we don't draw this
				truck twice. */
				y += 31;
			}
			else if ((m_trucky[0] & 0x1f) == (y & 0x1f))
			{
				/* We missed a truck's back end (it was off the bottom of the
				   screen) but have hit its front end, so draw the truck. */
				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						0,0,
						0,0,
						0x80,256 - y,0);
			}
		}
	}


	/* Draw the bullets.
	   They are flickered on/off every frame by the software, so don't
	   play it with frameskip 1 or 3, as they could become invisible */

	for (x = 0; x < 256; x++)
	{
		int bullet, mask1, mask2, val;

		val = m_bulletsram[x];

		// Check for the most common case
		if (!(val & 0x0f))
			continue;

		mask1 = 0x01;
		mask2 = 0x10;

		// Check each bullet
		for (bullet = 0; bullet < 4; bullet++)
		{
			if (val & mask1)
			{
				for (y = cliprect.min_y; y <= cliprect.max_y; y++)
					if (m_bulletsram[y] & mask2)
						bitmap.pix16(y, 256 - x) = 1;
			}

			mask1 <<= 1;
			mask2 <<= 1;
		}
	}
	return 0;
}
