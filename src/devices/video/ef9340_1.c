// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    ef9340_1.h

    Thomson EF9340 + EF9341 teletext graphics chips with 1KB external
    character ram.

***************************************************************************/

#include "ef9340_1.h"
#include "ef9341_chargen.h"


// device type definition
const device_type EF9340_1 = &device_creator<ef9340_1_device>;


static const UINT8 bgr2rgb[8] =
{
	0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07
};


ef9340_1_device::ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EF9340_1, "EF9340+EF9341", tag, owner, clock, "ef9340_1", __FILE__)
	, device_video_interface(mconfig, *this)
	//, m_start_vpos(START_Y)
	//, m_start_vblank(START_Y + SCREEN_HEIGHT)
	//, m_screen_lines(LINES)
{
}


void ef9340_1_device::device_start()
{
	// Let the screen create our temporary bitmap with the screen's dimensions
	m_screen->register_screen_bitmap(m_tmp_bitmap);

	m_line_timer = timer_alloc(TIMER_LINE);
	m_line_timer->adjust( m_screen->time_until_pos(0, 0), 0,  m_screen->scan_period() );

	// register our state
	save_item(NAME(m_ef9341.TA));
	save_item(NAME(m_ef9341.TB));
	save_item(NAME(m_ef9341.busy));
	save_item(NAME(m_ef9340.X));
	save_item(NAME(m_ef9340.Y));
	save_item(NAME(m_ef9340.Y0));
	save_item(NAME(m_ef9340.R));
	save_item(NAME(m_ef9340.M));
	save_pointer(NAME(m_ef934x_ram_a), 1024);
	save_pointer(NAME(m_ef934x_ram_b), 1024);
	save_pointer(NAME(m_ef934x_ext_char_ram), 1024);
}


void ef9340_1_device::device_reset()
{
	memset(m_ef934x_ram_a, 0, sizeof(m_ef934x_ram_a));
	memset(m_ef934x_ram_b, 0, sizeof(m_ef934x_ram_b));

	m_ef9340.X = 0;
	m_ef9340.Y = 0;
	m_ef9340.Y0 = 0;
	m_ef9340.R = 0;
	m_ef9340.M = 0;
	m_ef9340.max_vpos = 210;
	m_ef9341.TA = 0;
	m_ef9341.TB = 0;
	m_ef9341.busy = 0;
}


void ef9340_1_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch ( id )
	{
		case TIMER_LINE:
			ef9340_scanline(m_screen->vpos());
			break;
	}
}


UINT16 ef9340_1_device::ef9340_get_c_addr(UINT8 x, UINT8 y)
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


void ef9340_1_device::ef9340_inc_c()
{
	m_ef9340.X++;
	if ( m_ef9340.X == 40 || m_ef9340.X == 48 || m_ef9340.X == 56 || m_ef9340.X == 64 )
	{
		m_ef9340.Y = ( m_ef9340.Y + 1 ) & 0x1f;
		if ( m_ef9340.Y == 24 )
		{
			m_ef9340.Y = 0;
		}
		m_ef9340.X = 0;
	}
}


UINT16 ef9340_1_device::external_chargen_address(UINT8 b, UINT8 slice)
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


void ef9340_1_device::ef9341_write( UINT8 command, UINT8 b, UINT8 data )
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
				m_ef9340.max_vpos = ( m_ef9340.R & 0x40 ) ? 250 : 210;
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
						UINT8 a = m_ef934x_ram_a[addr];
						UINT8 b = m_ef934x_ram_b[addr];
						UINT8 slice = ( m_ef9340.M & 0x0f ) % 10;

						if ( b >= 0xa0 )
						{
							m_ef934x_ext_char_ram[ ( ( a & 0x80 ) << 3 ) | external_chargen_address( b, slice ) ] = BITSWAP8(m_ef9341.TA,0,1,2,3,4,5,6,7);
						}

						// Increment slice number
						m_ef9340.M = ( m_ef9340.M & 0xf0) | ( ( slice + 1 ) % 10 );
					}
					break;

				case 0xA0:  /* Read slice */
				default:
					fatalerror/*logerror*/("ef9341 unimplemented data action %02X\n", m_ef9340.M & 0xE0 );
			}
			m_ef9341.busy = 0;
		}
		else
		{
			m_ef9341.TA = data;
		}
	}
}


UINT8 ef9340_1_device::ef9341_read( UINT8 command, UINT8 b )
{
	UINT8   data = 0xFF;

	logerror("ef9341 %s read, t%s\n", command ? "command" : "data", b ? "B" : "A" );
	if ( command )
	{
		if ( b )
		{
			data = 0;
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


void ef9340_1_device::ef9340_scanline(int vpos)
{
	if ( vpos < m_ef9340.max_vpos )
	{
		int y = vpos - 0;
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
					m_tmp_bitmap.pix16(vpos, 0 + i ) = 24;
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
			UINT8 fg = 0;
			UINT8 bg = 0;
			UINT8 char_data = 0x00;

			if ( a & 0x80 )
			{
				// Graphics
				if ( b & 0x80 )
				{
					if ( b & 0x60 )
					{
						// Extension
						char_data = m_ef934x_ext_char_ram[ 0x400 | external_chargen_address( b & 0x7f, slice ) ];
						fg = bgr2rgb[ a & 0x07 ];
						bg = bgr2rgb[ ( a >> 4 ) & 0x07 ];
					}
				}
				else
				{
					// Normal
					char_data = ef9341_char_set[1][b & 0x7f][slice];
					fg = bgr2rgb[ a & 0x07 ];
					bg = bgr2rgb[ ( a >> 4 ) & 0x07 ];
				}
			}
			else
			{
				// Alphannumeric
				if ( b & 0x80 )
				{
					if ( b & 0x60 )
					{
						// Extension
						char_data = m_ef934x_ext_char_ram[ external_chargen_address( b & 0x7f, slice ) ];

						if ( a & 0x40 )
						{
							fg = bg;
							bg = bgr2rgb[ a & 0x07 ];
						}
						else
						{
							fg = bgr2rgb[ a & 0x07 ];
						}
					}
					else
					{
						// DEL
						char_data = 0xff;
						fg = bgr2rgb[ a & 0x07 ];
					}
				}
				else
				{
					// Normal
					char_data = ef9341_char_set[0][b & 0x7f][slice];

					if ( a & 0x40 )
					{
						fg = bg;
						bg = bgr2rgb[ a & 0x07 ];
					}
					else
					{
						fg = bgr2rgb[ a & 0x07 ];
					}
				}
			}

			for ( int i = 0; i < 8; i++ )
			{
				m_tmp_bitmap.pix16(vpos, 0 + x*8 + i ) = (char_data & 0x80) ? fg : bg;
				char_data <<= 1;
			}
		}
	}
}
