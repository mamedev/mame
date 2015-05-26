// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
#include "emu.h"
#include "includes/intv.h"

void intv_state::video_start()
{
	m_tms9927_num_rows = 25;
}


/* very rudimentary support for the tms9927 character generator IC */


READ8_MEMBER( intv_state::intvkbd_tms9927_r )
{
	UINT8 rv;
	switch (offset)
	{
		case 8:
			rv = m_tms9927_cursor_row;
			break;
		case 9:
			/* note: this is 1-based */
			rv = m_tms9927_cursor_col;
			break;
		case 11:
			m_tms9927_last_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
			rv = m_tms9927_last_row;
			break;
		default:
			rv = 0;
	}
	return rv;
}

WRITE8_MEMBER( intv_state::intvkbd_tms9927_w )
{
	switch (offset)
	{
		case 3:
			m_tms9927_num_rows = (data & 0x3f) + 1;
			break;
		case 6:
			m_tms9927_last_row = data;
			break;
		case 11:
			m_tms9927_last_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
			break;
		case 12:
			/* note: this is 1-based */
			m_tms9927_cursor_col = data;
			break;
		case 13:
			m_tms9927_cursor_row = data;
			break;
	}
}


UINT32 intv_state::screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_stic->screen_update(screen, bitmap, cliprect);
	return 0;
}


UINT32 intv_state::screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int x,y,offs;
	int current_row;
//  char c;

	/* Draw the underlying INTV screen first */
	m_stic->screen_update(screen, bitmap, cliprect);

	/* if the intvkbd text is not blanked, overlay it */
	if (!m_intvkbd_text_blanked)
	{
		current_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
		for(y=0;y<24;y++)
		{
			for(x=0;x<40;x++)
			{
				offs = current_row*64+x;

					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					videoram[offs],
					7, /* white */
					0,0,
					x<<3,y<<3, 0);
			}
			if (current_row == m_tms9927_cursor_row)
			{
				/* draw the cursor as a solid white block */
				/* (should use a filled rect here!) */

					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					191, /* a block */
					7,   /* white   */
					0,0,
					(m_tms9927_cursor_col-1)<<3,y<<3, 0);
			}
			current_row = (current_row + 1) % m_tms9927_num_rows;
		}
	}

#if 0
	// debugging
	c = tape_motor_mode_desc[m_tape_motor_mode][0];
	m_gfxdecode->gfx(0)->transpen(bitmap,&machine().screen[0].visarea,
		c,
		1,
		0,0,
		0*8,0*8, 0);
	for(y=0;y<5;y++)
	{
		m_gfxdecode->gfx(0)->transpen(bitmap,&machine().screen[0].visarea,
			m_tape_unknown_write[y]+'0',
			1,
			0,0,
			0*8,(y+2)*8, 0);
	}
	m_gfxdecode->gfx(0)->transpen(bitmap,&machine().screen[0].visarea,
			m_tape_unknown_write[5]+'0',
			1,
			0,0,
			0*8,8*8, 0);
	m_gfxdecode->gfx(0)->transpen(bitmap,&machine().screen[0].visarea,
			m_tape_interrupts_enabled+'0',
			1,
			0,0,
			0*8,10*8, 0);
#endif
	return 0;
}
