/***************************************************************************

  video/odyssey2.c

  2012-02-04 DanBoris
    - Changed color of background grid color 0 to match sprite color 0 (Fixes KTAA title screen)
    - Fixed Odyssey2_video_w so that m_o2_vdc.reg[] is always updated (Fixes Blockout)
    - Changed quad character generation so character height is always taken from 4th character (KTAA level 2)


***************************************************************************/

#include "emu.h"
#include "includes/odyssey2.h"
#include "video/ef9341_chargen.h"


#define I824X_START_ACTIVE_SCAN         6
#define I824X_END_ACTIVE_SCAN           (6 + 160)
#define I824X_START_Y                   1
#define I824X_SCREEN_HEIGHT             243
#define I824X_LINE_CLOCKS               228


void odyssey2_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int vpos = m_screen->vpos();

	switch ( id )
	{
		case TIMER_LINE:
			if ( m_g7400 )
			{
				ef9340_scanline(vpos);
			}
			break;

		case TIMER_HBLANK:
			break;
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void odyssey2_state::video_start()
{
	m_start_vpos = I824X_START_Y;
	m_start_vblank = I824X_START_Y + I824X_SCREEN_HEIGHT;

	m_screen->register_screen_bitmap(m_tmp_bitmap);

	m_line_timer = timer_alloc(TIMER_LINE);
	m_line_timer->adjust( m_screen->time_until_pos(1, I824X_START_ACTIVE_SCAN ), 0,  m_screen->scan_period() );

	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_hblank_timer->adjust( m_screen->time_until_pos(1, I824X_END_ACTIVE_SCAN + 18 ), 0, m_screen->scan_period() );
}


void odyssey2_state::video_start_g7400()
{
	video_start();

	m_ef9340.X = 0;
	m_ef9340.Y = 0;
	m_ef9340.Y0 = 0;
	m_ef9340.R = 0;
	m_ef9340.M = 0;
	m_ef9341.TA = 0;
	m_ef9341.TB = 0;
	m_ef9341.busy = 0;

	m_g7400 = true;
}



/*
    Thomson EF9340/EF9341 extra chips in the g7400
 */

UINT16 odyssey2_state::ef9340_get_c_addr(UINT8 x, UINT8 y)
{
	if ( ( y & 0x18 ) == 0x18 )
	{
		return 0x318 | ( ( x & 0x38 ) << 2 ) | ( x & 0x07 );
	}
	if ( x & 0x20 )
	{
		return 0x300 | ( ( y & 0x07 ) << 5 ) | ( y & 0x18 ) | ( x & 0x07 );
	}
	return y << 5 | x;
}


void odyssey2_state::ef9340_inc_c()
{
	m_ef9340.X++;
	if ( m_ef9340.X >= 40 )
	{
		m_ef9340.Y = ( m_ef9340.Y + 1 ) % 24;
		m_ef9340.X = 0;
	}
}


UINT16 odyssey2_state::external_chargen_address(UINT8 b, UINT8 slice)
{
	UINT8 cc = b & 0x7f;

	if ( slice & 8 )
	{
		// 0 0 CCE4 CCE3 CCE2 CCE1 CCE0 CCE6 CCE5 ADR0
		return ( ( cc << 3 ) & 0xf8 ) | ( ( cc >> 4 ) & 0x06) | ( slice & 0x01 );
	}
	// CCE6 CCE5 CCE4 CCE3 CCE2 CCE1 CCE0 ADR2 ADR1 ADR0
	return  ( cc << 3 ) | ( slice & 0x07 );
}


void odyssey2_state::ef9341_w( UINT8 command, UINT8 b, UINT8 data )
{
	logerror("ef9341 %s write, t%s, data %02X\n", command ? "command" : "data", b ? "B" : "A", data );

	if ( command )
	{
		if ( b )
		{
			m_ef9341.TB = data;
			m_ef9341.busy = 0x80;
			switch( m_ef9341.TB & 0xE0 )
			{
			case 0x00:  /* Begin row */
				m_ef9340.X = 0;
				m_ef9340.Y = m_ef9341.TA & 0x1F;
				break;
			case 0x20:  /* Load Y */
				m_ef9340.Y = m_ef9341.TA & 0x1F;
				break;
			case 0x40:  /* Load X */
				m_ef9340.X = m_ef9341.TA & 0x3F;
				break;
			case 0x60:  /* INC C */
				ef9340_inc_c();
				break;
			case 0x80:  /* Load M */
				m_ef9340.M = m_ef9341.TA;
				break;
			case 0xA0:  /* Load R */
				m_ef9340.R = m_ef9341.TA;
				break;
			case 0xC0:  /* Load Y0 */
				m_ef9340.Y0 = m_ef9341.TA & 0x3F;
				break;
			}
			m_ef9341.busy = 0;
		}
		else
		{
			m_ef9341.TA = data;
		}
	}
	else
	{
		if ( b )
		{
			UINT16 addr = ef9340_get_c_addr( m_ef9340.X, m_ef9340.Y ) & 0x3ff;

			m_ef9341.TB = data;
			m_ef9341.busy = 0x80;
			switch ( m_ef9340.M & 0xE0 )
			{
				case 0x00:  /* Write */
					m_ef934x_ram_a[addr] = m_ef9341.TA;
					m_ef934x_ram_b[addr] = m_ef9341.TB;
					ef9340_inc_c();
					break;

				case 0x20:  /* Read */
					m_ef9341.TA = m_ef934x_ram_a[addr];
					m_ef9341.TB = m_ef934x_ram_b[addr];
					ef9340_inc_c();
					break;

				case 0x40:  /* Write without increment */
					m_ef934x_ram_a[addr] = m_ef9341.TA;
					m_ef934x_ram_b[addr] = m_ef9341.TB;
					break;

				case 0x60:  /* Read without increment */
					m_ef9341.TA = m_ef934x_ram_a[addr];
					m_ef9341.TB = m_ef934x_ram_b[addr];
					break;

				case 0x80:  /* Write slice */
					{
						UINT8 b = m_ef934x_ram_b[addr];
						UINT8 slice = ( m_ef9340.M & 0x0f ) % 10;

						if ( b >= 0xa0 )
						{
							m_ef934x_ext_char_ram[ external_chargen_address( b, slice ) ] = m_ef9341.TA;
						}

						// Increment slice number
						m_ef9340.M = ( m_ef9340.M & 0xf0) | ( ( slice + 1 ) % 10 );
					}
					break;

				case 0xA0:  /* Read slice */
					fatalerror/*logerror*/("ef9341 unimplemented data action %02X\n", m_ef9340.M & 0xE0 );
					break;
			}
			m_ef9341.busy = 0;
		}
		else
		{
			m_ef9341.TA = data;
		}
	}
}


UINT8 odyssey2_state::ef9341_r( UINT8 command, UINT8 b )
{
	UINT8   data = 0xFF;

	logerror("ef9341 %s read, t%s\n", command ? "command" : "data", b ? "B" : "A" );
	if ( command )
	{
		if ( b )
		{
			data = 0xFF;
		}
		else
		{
			data = m_ef9341.busy;
		}
	}
	else
	{
		if ( b )
		{
			data = m_ef9341.TB;
		}
		else
		{
			data = m_ef9341.TA;
		}
	}
	return data;
}


void odyssey2_state::ef9340_scanline(int vpos)
{
	if ( vpos < m_start_vpos )
	{
		return;
	}

	if ( vpos < m_start_vblank )
	{
		int y = vpos - m_start_vpos;
		int y_row, slice;

		if ( y < 10 )
		{
			// Service row

			if ( m_ef9340.R & 0x08 )
			{
				// Service row is enabled

				y_row = 31;
				slice = y;
			}
			else
			{
				// Service row is disabled

				for ( int i = 0; i < 40 * 8; i++ )
				{
					m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN*2 + i ) = 24;
				}
				return;
			}
		}
		else
		{
			// Displaying regular row
			y_row = (y - 10) / 10;
			slice = (y - 10) % 10;
		}

		for ( int x = 0; x < 40; x++ )
		{
			UINT16 addr = ef9340_get_c_addr( x, y_row );
			UINT8 a = m_ef934x_ram_a[addr];
			UINT8 b = m_ef934x_ram_b[addr];
			UINT8 fg = 24;
			UINT8 bg = 24;
			UINT8 char_data = 0x00;

			if ( a & 0x80 )
			{
				// Graphics
			}
			else
			{
				// Alphannumeric
				if ( b & 0x80 )
				{
					// Special (DEL or Extension)
				}
				else
				{
					// Normal
					char_data = ef9341_char_set[0][b & 0x7f][slice];
					fg = 24 + ( a & 0x07 );
				}
			}

			for ( int i = 0; i < 8; i++ )
			{
				m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN*2 + x*8 + i ) = (char_data & 0x80) ? fg : bg;
				char_data <<= 1;
			}
		}
	}
}

