/***************************************************************************

  cgenie.c

  Functions to emulate the video controller 6845.

***************************************************************************/

#include "emu.h"
#include "includes/cgenie.h"





/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void cgenie_state::video_start()
{
	m_screen->register_screen_bitmap(m_dlybitmap);
	m_screen->register_screen_bitmap(m_bitmap);
}

/***************************************************************************

  Calculate the horizontal and vertical offset for the
  current register settings of the 6845 CRTC

***************************************************************************/
void cgenie_state::cgenie_offset_xy()
{
	if( m_crt.horizontal_sync_pos )
		m_off_x = m_crt.horizontal_total - m_crt.horizontal_sync_pos - 14;
	else
		m_off_x = -15;

	m_off_y = (m_crt.vertical_total - m_crt.vertical_sync_pos) *
		(m_crt.scan_lines + 1) + m_crt.vertical_adjust
		- 32;

	if( m_off_y < 0 )
		m_off_y = 0;

	if( m_off_y > 128 )
		m_off_y = 128;

// logerror("cgenie offset x:%d  y:%d\n", m_off_x, m_off_y);
}


/***************************************************************************
  Write to an indexed register of the 6845 CRTC
***************************************************************************/
WRITE8_MEMBER( cgenie_state::cgenie_register_w )
{
	//int addr;

	switch (m_crt.idx)
	{
		case 0:
			if( m_crt.horizontal_total == data )
				break;
			m_crt.horizontal_total = data;
			cgenie_offset_xy();
			break;
		case 1:
			if( m_crt.horizontal_displayed == data )
				break;
			m_crt.horizontal_displayed = data;
			break;
		case 2:
			if( m_crt.horizontal_sync_pos == data )
				break;
			m_crt.horizontal_sync_pos = data;
			cgenie_offset_xy();
			break;
		case 3:
			m_crt.horizontal_length = data;
			break;
		case 4:
			if( m_crt.vertical_total == data )
				break;
			m_crt.vertical_total = data;
			cgenie_offset_xy();
			break;
		case 5:
			if( m_crt.vertical_adjust == data )
				break;
			m_crt.vertical_adjust = data;
			cgenie_offset_xy();
			break;
		case 6:
			if( m_crt.vertical_displayed == data )
				break;
			m_crt.vertical_displayed = data;
			break;
		case 7:
			if( m_crt.vertical_sync_pos == data )
				break;
			m_crt.vertical_sync_pos = data;
			cgenie_offset_xy();
			break;
		case 8:
			m_crt.crt_mode = data;
			break;
		case 9:
			data &= 15;
			if( m_crt.scan_lines == data )
				break;
			m_crt.scan_lines = data;
			cgenie_offset_xy();
			break;
		case 10:
			if( m_crt.cursor_top == data )
				break;
			m_crt.cursor_top = data;
			//addr = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;
			break;
		case 11:
			if( m_crt.cursor_bottom == data )
				break;
			m_crt.cursor_bottom = data;
			//addr = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;
			break;
		case 12:
			data &= 63;
			if( m_crt.screen_address_hi == data )
				break;
			m_crt.screen_address_hi = data;
			break;
		case 13:
			if( m_crt.screen_address_lo == data )
				break;
			m_crt.screen_address_lo = data;
			break;
		case 14:
			data &= 63;
			if( m_crt.cursor_address_hi == data )
				break;
			m_crt.cursor_address_hi = data;
			//addr = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;
			break;
		case 15:
			if( m_crt.cursor_address_lo == data )
				break;
			m_crt.cursor_address_lo = data;
			//addr = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;
			break;
	}
}

/***************************************************************************
  Write to the index register of the 6845 CRTC
***************************************************************************/
WRITE8_MEMBER( cgenie_state::cgenie_index_w )
{
	m_crt.idx = data & 15;
}

/***************************************************************************
  Read from an indexed register of the 6845 CRTC
***************************************************************************/
READ8_MEMBER( cgenie_state::cgenie_register_r )
{
	return cgenie_get_register(m_crt.idx);
}

/***************************************************************************
  Read from a register of the 6845 CRTC
***************************************************************************/
int cgenie_state::cgenie_get_register(int indx)
{
	switch (indx)
	{
		case 0:
			return m_crt.horizontal_total;
		case 1:
			return m_crt.horizontal_displayed;
		case 2:
			return m_crt.horizontal_sync_pos;
		case 3:
			return m_crt.horizontal_length;
		case 4:
			return m_crt.vertical_total;
		case 5:
			return m_crt.vertical_adjust;
		case 6:
			return m_crt.vertical_displayed;
		case 7:
			return m_crt.vertical_sync_pos;
		case 8:
			return m_crt.crt_mode;
		case 9:
			return m_crt.scan_lines;
		case 10:
			return m_crt.cursor_top;
		case 11:
			return m_crt.cursor_bottom;
		case 12:
			return m_crt.screen_address_hi;
		case 13:
			return m_crt.screen_address_lo;
		case 14:
			return m_crt.cursor_address_hi;
		case 15:
			return m_crt.cursor_address_lo;
	}
	return 0;
}

/***************************************************************************
  Read the index register of the 6845 CRTC
***************************************************************************/
READ8_MEMBER( cgenie_state::cgenie_index_r )
{
	return m_crt.idx;
}

/***************************************************************************
  Switch mode between character generator and graphics
***************************************************************************/
void cgenie_state::cgenie_mode_select(int mode)
{
	m_graphics = (mode) ? 1 : 0;
}


void cgenie_state::cgenie_refresh_monitor(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int i, address, offset, cursor, size, code, x, y;
	rectangle r;

	bitmap.fill(m_palette->black_pen(), cliprect);

	if(m_crt.vertical_displayed || m_crt.horizontal_displayed)
	{
		offset = 256 * m_crt.screen_address_hi + m_crt.screen_address_lo;
		size = m_crt.horizontal_displayed * m_crt.vertical_displayed;
		cursor = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;

		/*
		 * for every character in the Video RAM, check if it has been modified since
		 * last time and update it accordingly.
		 */
		for( address = 0; address < size; address++ )
		{
			i = (offset + address) & 0x3fff;
			x = address % m_crt.horizontal_displayed + m_off_x;
			y = address / m_crt.horizontal_displayed;

			r.min_x = x * 8;
			r.max_x = r.min_x + 7;
			r.min_y = y * (m_crt.scan_lines + 1) + m_off_y;
			r.max_y = r.min_y + m_crt.scan_lines;

			if( m_graphics )
			{
				/* get graphics code */
				code = videoram[i];
				m_gfxdecode->gfx(1)->opaque(bitmap,r, code, 0,
					0, 0, r.min_x, r.min_y);
			}
			else
			{
				/* get character code */
				code = videoram[i];

				/* translate defined character sets */
				code += m_font_offset[(code >> 6) & 3];
				m_gfxdecode->gfx(0)->opaque(bitmap,r, code, m_colorram[i&0x3ff],
					0, 0, r.min_x, r.min_y);
			}

			if( i == cursor )
			{
			rectangle rc;

			/* check if cursor turned off */
				if( (m_crt.cursor_top & 0x60) == 0x20 )
					continue;

				if( (m_crt.cursor_top & 0x60) == 0x60 )
				{
					m_crt.cursor_visible = 1;
				}
				else
				{
					m_crt.cursor_phase++;
					m_crt.cursor_visible = (m_crt.cursor_phase >> 3) & 1;
				}

				if( !m_crt.cursor_visible )
					continue;

				rc.min_x = r.min_x;
				rc.max_x = r.max_x;
				rc.min_y = r.min_y + (m_crt.cursor_top & 15);
				rc.max_y = r.min_y + (m_crt.cursor_bottom & 15);
				m_gfxdecode->gfx(0)->opaque(bitmap,rc, 0x7f, m_colorram[i&0x3ff],
					0, 0, rc.min_x, rc.min_y);
			}
		}
	}
}

void cgenie_state::cgenie_refresh_tv_set(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int i, address, offset, cursor, size, code, x, y;
	rectangle r;

	m_bitmap.fill(m_palette->black_pen(), cliprect);
	m_dlybitmap.fill(m_palette->black_pen(), cliprect);

	if(m_crt.vertical_displayed || m_crt.horizontal_displayed)
	{
		offset = 256 * m_crt.screen_address_hi + m_crt.screen_address_lo;
		size = m_crt.horizontal_displayed * m_crt.vertical_displayed;
		cursor = 256 * m_crt.cursor_address_hi + m_crt.cursor_address_lo;

		/*
		 * for every character in the Video RAM, check if it has been modified since
		 * last time and update it accordingly.
		 */
		for( address = 0; address < size; address++ )
		{
			i = (offset + address) & 0x3fff;
			x = address % m_crt.horizontal_displayed + m_off_x;
			y = address / m_crt.horizontal_displayed;

			r.min_x = x * 8;
			r.max_x = r.min_x + 7;
			r.min_y = y * (m_crt.scan_lines + 1) + m_off_y;
			r.max_y = r.min_y + m_crt.scan_lines;

			if( m_graphics )
			{
				/* get graphics code */
				code = videoram[i];
				m_gfxdecode->gfx(1)->opaque(m_bitmap,r, code, 1,
					0, 0, r.min_x, r.min_y);
				m_gfxdecode->gfx(1)->opaque(m_dlybitmap,r, code, 2,
					0, 0, r.min_x, r.min_y);
			}
			else
			{
				/* get character code */
				code = videoram[i];

				/* translate defined character sets */
				code += m_font_offset[(code >> 6) & 3];
				m_gfxdecode->gfx(0)->opaque(m_bitmap,r, code, m_colorram[i&0x3ff] + 16,
					0, 0, r.min_x, r.min_y);
				m_gfxdecode->gfx(0)->opaque(m_dlybitmap,r, code, m_colorram[i&0x3ff] + 32,
					0, 0, r.min_x, r.min_y);
			}

			if( i == cursor )
			{
				rectangle rc;

				/* check if cursor turned off */
				if( (m_crt.cursor_top & 0x60) == 0x20 )
					continue;

				if( (m_crt.cursor_top & 0x60) == 0x60 )
				{
					m_crt.cursor_visible = 1;
				}
				else
				{
					m_crt.cursor_phase++;
					m_crt.cursor_visible = (m_crt.cursor_phase >> 3) & 1;
				}

				if( !m_crt.cursor_visible )
					continue;

				rc.min_x = r.min_x;
				rc.max_x = r.max_x;
				rc.min_y = r.min_y + (m_crt.cursor_top & 15);
				rc.max_y = r.min_y + (m_crt.cursor_bottom & 15);

				m_gfxdecode->gfx(0)->opaque(m_bitmap,rc, 0x7f, m_colorram[i&0x3ff] + 16,
					0, 0, rc.min_x, rc.min_y);
				m_gfxdecode->gfx(0)->opaque(m_dlybitmap,rc, 0x7f, m_colorram[i&0x3ff] + 32,
					0, 0, rc.min_x, rc.min_y);
			}
		}
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_dlybitmap, 0, 0, 1, 0, cliprect, 0);
}

/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
***************************************************************************/
UINT32 cgenie_state::screen_update_cgenie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if( m_tv_mode )
		cgenie_refresh_tv_set(bitmap, cliprect);
	else
		cgenie_refresh_monitor(bitmap, cliprect);
	return 0;
}
