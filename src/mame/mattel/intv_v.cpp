// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
#include "emu.h"
#include "video/tms9927.h"
#include "intv.h"

void intv_state::video_start()
{
}

uint32_t intv_state::screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_stic->screen_update(screen, bitmap, cliprect);
	return 0;
}

uint32_t intv_state::screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Draw the underlying INTV screen first */
	m_stic->screen_update(screen, bitmap, cliprect);

	/* if the intvkbd text is not blanked, overlay it */
	if (!m_intvkbd_text_blanked)
	{
		uint8_t *videoram = m_videoram;
		int xoffset = stic_device::OVERSCAN_LEFT_WIDTH*stic_device::X_SCALE*INTVKBD_X_SCALE;
		int yoffset = stic_device::OVERSCAN_TOP_HEIGHT*stic_device::Y_SCALE*INTVKBD_Y_SCALE;

		rectangle cursor_rect;
		m_crtc->cursor_bounds(cursor_rect);
		int cursor_col = cursor_rect.min_x / 8;
		int cursor_row = cursor_rect.min_y / 8;

		int current_row = m_crtc->upscroll_offset() % 24;

		for(int y=0;y<24;y++)
		{
			for(int x=0;x<40;x++)
			{
				if ((cursor_row == y) && (cursor_col == x+1)) {
					/* draw the cursor as a solid white block */
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					191, /* a block */
					7,   /* white   */
					0,0,
					xoffset+(x<<3), yoffset+(y<<3), 0);
				} else {
					int offs = current_row*64+x;
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					videoram[offs],
					7, /* white */
					0,0,
					xoffset+(x<<3), yoffset+(y<<3), 0);
				}
			}
			current_row = (current_row + 1) % 24;
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
