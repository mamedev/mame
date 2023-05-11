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

#include "screen.h"


// devices
DEFINE_DEVICE_TYPE(EF9364, ef9364_device, "ef9364", "Thomson EF9364")

//-------------------------------------------------
// default address map
//-------------------------------------------------
void ef9364_device::ef9364(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, ef9364_device::TXTPLANE_MAX_SIZE * ef9364_device::MAX_TXTPLANES - 1).ram();
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ef9364_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
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

ef9364_device::ef9364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EF9364, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("textram", ENDIANNESS_LITTLE, 8, 12, 0, address_map_constructor(FUNC(ef9364_device::ef9364), this)),
	m_charset(*this, DEVICE_SELF),
	m_palette(*this, finder_base::DUMMY_TAG)
{
	clock_freq = clock;

	erase_char = 0x00;
}

//-------------------------------------------------
//  set_color_entry: Set the color value
//  into the palette
//-------------------------------------------------

void ef9364_device::set_color_entry( int index, uint8_t r, uint8_t g, uint8_t b )
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
	// assumes it can make an address mask with m_charset.length() - 1
	assert(!(m_charset.length() & (m_charset.length() - 1)));

	m_textram = &space(0);

	bitplane_xres = NB_OF_COLUMNS*8;
	bitplane_yres = NB_OF_ROWS*(8+4);

	vsync_scanline_pos = 250;

	// Default palette : Black and white
	palette[0] = rgb_t(0, 0, 0);
	palette[1] = rgb_t(255, 255, 255);

	cursor_cnt = 0;
	cursor_state = 0;

	save_item(NAME(m_border));
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

	for(i = 0; i < NB_OF_COLUMNS * NB_OF_ROWS * nb_of_pages; i++)
	{
		m_textram->write_byte ( i , erase_char );
	}

	memset(m_border, 0, sizeof(m_border));

	set_video_mode();
}

//-------------------------------------------------
//  set_video_mode: Set output screen format
//-------------------------------------------------

void ef9364_device::set_video_mode(void)
{
	uint16_t new_width = bitplane_xres;

	if (screen().width() != new_width)
	{
		rectangle visarea = screen().visible_area();
		visarea.max_x = new_width - 1;

		screen().configure(new_width, screen().height(), visarea, screen().frame_period().attoseconds());
	}

	//border color
	memset(m_border, 0, sizeof(m_border));
}

//-------------------------------------------------
//  draw_border: Draw the left and right borders
//  ( No border for the moment ;) )
//-------------------------------------------------

void ef9364_device::draw_border(uint16_t line)
{
}

//-------------------------------------------------
// screen_update: Framebuffer video output
//-------------------------------------------------

uint32_t ef9364_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for( int r = 0 ; r < NB_OF_ROWS ; r++ )
	{
		for( int y = 0 ; y < 12 ; y++ )
		{
			for( int x = 0 ; x < NB_OF_COLUMNS * 8 ; x++ )
			{
				if( ( ( x >> 3 ) != x_curs_pos ) || ( r != y_curs_pos ) || !cursor_state)
				{
					unsigned char c = m_textram->read_byte( ( r * NB_OF_COLUMNS ) + ( x>>3 ) );
					int ra = (y & 8) ? 0 : y;
					if( BIT(m_charset[((c<<3) + ra) & (m_charset.length() - 1)], 7 - (x & 7)) )
						bitmap.pix((r*12)+y, x) = palette[1];
					else
						bitmap.pix((r*12)+y, x) = palette[0];
				}
				else
				{
					if(y != 7)
						bitmap.pix((r*12)+y, x) = palette[0];
					else
						bitmap.pix((r*12)+y, x) = palette[1];
				}
			}
		}
	}

	cursor_cnt = (cursor_cnt + 1) % 13;
	if(!cursor_cnt)
		cursor_state ^= 1;

	return 0;
}

//-------------------------------------------------
// update_scanline: Scanline callback
//-------------------------------------------------

void ef9364_device::update_scanline(uint16_t scanline)
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

void ef9364_device::command_w(uint8_t cmd)
{
	int x,y,i,j;

	switch( cmd&7 )
	{
		case 0x0: // Page erase and cursor home
			for( y=0 ; y < NB_OF_ROWS ; y++ )
			{
				for( x=0 ; x < NB_OF_COLUMNS ; x++ )
				{
					m_textram->write_byte ( y * NB_OF_COLUMNS + x , erase_char );
				}
			}
			x_curs_pos = 0;
			y_curs_pos = 0;
			break;

		case 0x1: // Erase to end of line and return cursor
			for( ; x_curs_pos < NB_OF_COLUMNS ; x_curs_pos++ )
			{
				m_textram->write_byte ( y_curs_pos * NB_OF_COLUMNS + x_curs_pos , erase_char );
			}
			x_curs_pos = 0;
			break;

		case 0x2: // Line feed
			y_curs_pos++;
			if( y_curs_pos >= NB_OF_ROWS )
			{
				// Scroll
				for( j = 1 ; j < NB_OF_ROWS ; j++ )
				{
					for( i = 0 ; i < NB_OF_COLUMNS ; i++ )
					{
						m_textram->write_byte ( (j-1) * NB_OF_COLUMNS + i ,  m_textram->read_byte ( j * NB_OF_COLUMNS + i ) );
					}
				}
				// Erase last line
				for( i = 0 ; i < NB_OF_COLUMNS ; i++ )
				{
					m_textram->write_byte ( ( NB_OF_ROWS - 1 ) * NB_OF_COLUMNS + i , erase_char );
				}

				y_curs_pos = NB_OF_ROWS - 1;
			}
			break;

		case 0x3: // No operation
			break;

		case 0x4: // Cursor left
			if(x_curs_pos)
				x_curs_pos--;
			break;

		case 0x5: // Erasure of cursor line
			for( x = 0 ; x < NB_OF_COLUMNS ; x++ )
			{
				m_textram->write_byte ( y_curs_pos * NB_OF_COLUMNS + x , erase_char );
			}
			break;

		case 0x6: // Cursor up
			if(y_curs_pos)
				y_curs_pos--;
			break;

		case 0x7: // Normal character
			if(cmd&0x8)
				m_textram->write_byte ( y_curs_pos * NB_OF_COLUMNS + x_curs_pos , char_latch );

			x_curs_pos++;
			if( x_curs_pos >= NB_OF_COLUMNS )
			{
				x_curs_pos=0;
				y_curs_pos++;
				if( y_curs_pos >= NB_OF_ROWS )
				{
					// Scroll
					for( j = 1 ; j < NB_OF_ROWS ; j++ )
					{
						for( i = 0 ; i < NB_OF_COLUMNS ; i++ )
						{
							m_textram->write_byte ( (j-1) * NB_OF_COLUMNS + i ,  m_textram->read_byte ( j * NB_OF_COLUMNS + i ) );
						}
					}
					// Erase last line
					for( i = 0 ; i < NB_OF_COLUMNS ; i++ )
					{
						m_textram->write_byte ( ( NB_OF_ROWS - 1 ) * NB_OF_COLUMNS + i , erase_char );
					}

					y_curs_pos = NB_OF_ROWS - 1;
				}
			}
			break;
	}
}

void ef9364_device::char_latch_w(uint8_t data)
{
	char_latch = data;
}
