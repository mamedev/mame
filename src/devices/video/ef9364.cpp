// license:BSD-3-Clause
// copyright-holders:Jean-Francois DEL NERO

/*********************************************************************

    ef9364.cpp

    Thomson EF9364 / Sescosem SFF96364 video controller emulator code

    This circuit is a simple black and white 8x8 character generator.
    It display 64 columns * 16 rows text page.
    It is able to do automatic text scrolling, page erase.
    The characters font is stored into an external 1KB EPROM.

    To see how to use this driver, have a look to the Goupil machine
    driver (goupil.cpp).
    If you have any question or remark, don't hesitate to contact me
    at the email present on this website : http://hxc2001.free.fr/

    01/20/2016
    Jean-Francois DEL NERO
*********************************************************************/

#include "emu.h"
#include "ef9364.h"

// devices
const device_type EF9364 = &device_creator<ef9364_device>;

//-------------------------------------------------
// default address map
//-------------------------------------------------
static ADDRESS_MAP_START( ef9364, AS_0, 8, ef9364_device )
	AM_RANGE(0x00000, ( ( EF9364_TXTPLANE_MAX_SIZE * EF9364_MAX_TXTPLANES ) - 1 ) ) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *ef9364_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  ef9364_device - constructor
//-------------------------------------------------

ef9364_device::ef9364_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EF9364, "EF9364", tag, owner, clock, "ef9364", __FILE__),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("textram", ENDIANNESS_LITTLE, 8, 12, 0, nullptr, *ADDRESS_MAP_NAME(ef9364)),
	m_charset(*this, DEVICE_SELF),
	m_palette(*this)
{
	clock_freq = clock;
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void ef9364_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<ef9364_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  static_set_nb_of_pages: Set the number of hardware pages
//-------------------------------------------------

void ef9364_device::static_set_nb_of_pages(device_t &device, int nb_of_pages )
{
	if( nb_of_pages > 0 && nb_of_pages <= 8 )
	{
		downcast<ef9364_device &>(device).nb_of_pages = nb_of_pages;
	}
}

//-------------------------------------------------
//  set_color_entry: Set the color value
//  into the palette
//-------------------------------------------------

void ef9364_device::set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b )
{
	if( index < 2 )
	{
		palette[index] = rgb_t(r, g, b);
	}
	else
	{
		logerror("Invalid EF9364 Palette entry : %02x\n", index);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ef9364_device::device_start()
{
	m_textram = &space(0);

	bitplane_xres = EF9364_NB_OF_COLUMNS*8;
	bitplane_yres = EF9364_NB_OF_ROWS*(8+4);

	vsync_scanline_pos = 250;

	// Default palette : Black and white
	palette[0] = rgb_t(0, 0, 0);
	palette[1] = rgb_t(255, 255, 255);

	m_screen_out.allocate( bitplane_xres, m_screen->height() );

	cursor_cnt = 0;
	cursor_state = 0;

	save_item(NAME(m_border));

	save_item(NAME(m_screen_out));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ef9364_device::device_reset()
{
	int i;

	x_curs_pos = 0;
	y_curs_pos = 0;

	char_latch = 0x00;

	for(i=0;i<EF9364_NB_OF_COLUMNS * EF9364_NB_OF_ROWS * nb_of_pages;i++)
	{
		m_textram->write_byte ( i , 0x7F );
	}

	memset(m_border, 0, sizeof(m_border));

	m_screen_out.fill(0);

	set_video_mode();
}

//-------------------------------------------------
//  set_video_mode: Set output screen format
//-------------------------------------------------

void ef9364_device::set_video_mode(void)
{
	UINT16 new_width = bitplane_xres;

	if (m_screen->width() != new_width)
	{
		rectangle visarea = m_screen->visible_area();
		visarea.max_x = new_width - 1;

		m_screen->configure(new_width, m_screen->height(), visarea, m_screen->frame_period().attoseconds());
	}

	//border color
	memset(m_border, 0, sizeof(m_border));
}

//-------------------------------------------------
//  draw_border: Draw the left and right borders
//  ( No border for the moment ;) )
//-------------------------------------------------

void ef9364_device::draw_border(UINT16 line)
{
}

//-------------------------------------------------
// screen_update: Framebuffer video ouput
//-------------------------------------------------

UINT32 ef9364_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,r;
	unsigned char c;

	for( r = 0 ; r < EF9364_NB_OF_ROWS ; r++ )
	{
		for( y = 0 ; y < 8 ; y++ )
		{
			for( x = 0 ; x < EF9364_NB_OF_COLUMNS * 8 ; x++ )
			{
				if( ( ( x >> 3 ) != x_curs_pos )   ||  ( r != y_curs_pos ) || !cursor_state)
				{
					c = m_textram->read_byte( ( r * EF9364_NB_OF_COLUMNS ) + ( x>>3 ) );

					if( m_charset[((c&0x7F)<<3) + y] & (0x80>>(x&7)) )
						m_screen_out.pix32((r*12)+y, x) = palette[1];
					else
						m_screen_out.pix32((r*12)+y, x) = palette[0];
				}
				else
				{
					if(y != 7)
						m_screen_out.pix32((r*12)+y, x) = palette[0];
					else
						m_screen_out.pix32((r*12)+y, x) = palette[1];
				}
			}
		}
	}

	cursor_cnt = (cursor_cnt + 1) % 13;
	if(!cursor_cnt)
		cursor_state ^= 1;

	copybitmap(bitmap, m_screen_out, 0, 0, 0, 0, cliprect);
	return 0;
}

//-------------------------------------------------
// update_scanline: Scanline callback
//-------------------------------------------------

void ef9364_device::update_scanline(UINT16 scanline)
{
	if (scanline == vsync_scanline_pos)
	{
		// vsync
	}

	if (scanline == 0)
	{
		draw_border(0);
	}
}

//-------------------------------------------------
// data_w: Registers write access callback
//-------------------------------------------------

void ef9364_device::command_w(UINT8 cmd)
{
	int x,y,i,j;

	switch( cmd&7 )
	{
		case 0x0: // Page Erase & Cursor home
			for( y=0 ; y < EF9364_NB_OF_ROWS ; y++ )
			{
				for( x=0 ; x < EF9364_NB_OF_COLUMNS ; x++ )
				{
					m_textram->write_byte ( y * EF9364_NB_OF_COLUMNS + x , 0x7F );
				}
			}
			x_curs_pos = 0;
			y_curs_pos = 0;
		break;

		case 0x1: // Erase to end of the line and return cursor
			for( ; x_curs_pos < EF9364_NB_OF_COLUMNS ; x_curs_pos++ )
			{
				m_textram->write_byte ( y_curs_pos * EF9364_NB_OF_COLUMNS + x_curs_pos , 0x7F );
			}
			x_curs_pos = 0;
		break;

		case 0x2: // Line feed
			y_curs_pos++;
			if( y_curs_pos >= EF9364_NB_OF_ROWS )
			{
				// Scroll
				for( j = 1 ; j < EF9364_NB_OF_ROWS ; j++ )
				{
					for( i = 0 ; i < EF9364_NB_OF_COLUMNS ; i++ )
					{
						m_textram->write_byte ( (j-1) * EF9364_NB_OF_COLUMNS + i ,  m_textram->read_byte ( j * EF9364_NB_OF_COLUMNS + i ) );
					}
				}
				// Erase last line
				for( i = 0 ; i < EF9364_NB_OF_COLUMNS ; i++ )
				{
					m_textram->write_byte ( ( EF9364_NB_OF_ROWS - 1 ) * EF9364_NB_OF_COLUMNS + i , 0x7F );
				}

				y_curs_pos = EF9364_NB_OF_ROWS - 1;
			}
		break;

		case 0x3: // Nop

		break;

		case 0x4: // Cursor left
			if(x_curs_pos)
				x_curs_pos--;
		break;

		case 0x5: // Erasure of cursor Line.
			for( x = 0 ; x < EF9364_NB_OF_COLUMNS ; x++ )
			{
				m_textram->write_byte ( y_curs_pos * EF9364_NB_OF_COLUMNS + x , 0x7F );
			}
		break;

		case 0x6: // Cursor up
			if(y_curs_pos)
				y_curs_pos--;
		break;

		case 0x7: // Write char
			if(cmd&0x8)
				m_textram->write_byte ( y_curs_pos * EF9364_NB_OF_COLUMNS + x_curs_pos , char_latch );

			x_curs_pos++;
			if( x_curs_pos >= EF9364_NB_OF_COLUMNS )
			{
				x_curs_pos=0;
				y_curs_pos++;
				if( y_curs_pos >= EF9364_NB_OF_ROWS )
				{
					// Scroll
					for( j = 1 ; j < EF9364_NB_OF_ROWS ; j++ )
					{
						for( i = 0 ; i < EF9364_NB_OF_COLUMNS ; i++ )
						{
							m_textram->write_byte ( (j-1) * EF9364_NB_OF_COLUMNS + i ,  m_textram->read_byte ( j * EF9364_NB_OF_COLUMNS + i ) );
						}
					}
					// Erase last line
					for( i = 0 ; i < EF9364_NB_OF_COLUMNS ; i++ )
					{
						m_textram->write_byte ( ( EF9364_NB_OF_ROWS - 1 ) * EF9364_NB_OF_COLUMNS + i , 0x7F );
					}

					y_curs_pos = EF9364_NB_OF_ROWS - 1;
				}
			}
		break;

	}
}

void ef9364_device::char_latch_w(UINT8 data)
{
	char_latch = data;
}
