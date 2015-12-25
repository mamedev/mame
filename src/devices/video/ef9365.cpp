// license:BSD-3-Clause
// copyright-holders:Jean-François DEL NERO
/*********************************************************************

    ef9365.c

    Thomson EF9365 video controller emulator code

*********************************************************************/

#include "emu.h"
#include "ef9365.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type EF9365 = &device_creator<ef9365_device>;

// default address map
// Up to 4 bitplans @ 256*256
static ADDRESS_MAP_START( ef9365, AS_0, 8, ef9365_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *ef9365_device::memory_space_config(address_spacenum spacenum) const
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
//  ef9365_device - constructor
//-------------------------------------------------

ef9365_device::ef9365_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EF9365, "EF9365", tag, owner, clock, "ef9365", __FILE__),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, nullptr, *ADDRESS_MAP_NAME(ef9365)),
	m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void ef9365_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<ef9365_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  static_set_color_filler: Set the color value
//  used by the chip to draw/fill the memory
//-------------------------------------------------

void ef9365_device::static_set_color_filler( UINT8 color )
{
	m_current_color = color;
}

//-------------------------------------------------
//  static_set_color_entry: Set the color value
//  into the palette
//-------------------------------------------------

void ef9365_device::static_set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b )
{
	if( index < 16 )
	{
		palette[index] = rgb_t(r, g, b);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ef9365_device::device_start()
{
	int i;

	m_busy_timer = timer_alloc(BUSY_TIMER);

	m_videoram = &space(0);
	m_charset = region();
	m_current_color = 0x0F;

	// Default palette : Black and white
	palette[0] = rgb_t(0, 0, 0);
	for( i = 1; i < 16 ; i++ )
	{
		palette[i] = rgb_t(255, 255, 255);
	}

	m_screen_out.allocate(496, m_screen->height());

	save_item(NAME(m_border));
	save_item(NAME(m_registers));
	save_item(NAME(m_bf));
	save_item(NAME(m_state));

	save_item(NAME(m_screen_out));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void ef9365_device::device_reset()
{
	m_state = 0;
	m_bf = 0;

	memset(m_registers, 0, sizeof(m_registers));
	memset(m_border, 0, sizeof(m_border));

	m_screen_out.fill(0);

	set_video_mode();
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void ef9365_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case BUSY_TIMER:
			m_bf = 0;
			break;
	}
}

// set busy flag and timer to clear it
void ef9365_device::set_busy_flag(int period)
{
	m_bf = 1;
	m_busy_timer->adjust(attotime::from_usec(period));
}

// set then ef9365 mode
void ef9365_device::set_video_mode(void)
{
	UINT16 new_width = 256;

	if (m_screen->width() != new_width)
	{
		rectangle visarea = m_screen->visible_area();
		visarea.max_x = new_width - 1;

		m_screen->configure(new_width, m_screen->height(), visarea, m_screen->frame_period().attoseconds());
	}

	//border color
	memset(m_border, 0, sizeof(m_border));
}

void ef9365_device::draw_border(UINT16 line)
{

}

void ef9365_device::draw_character( unsigned char c, int block, int smallblock )
{
	int x_char,y_char;
	int char_base,char_pix;
	unsigned int x, y, p;
	int x_char_res,y_char_res;

	x = ( (m_registers[EF9365_REG_X_MSB]<<8) | m_registers[EF9365_REG_X_LSB]);
	y = ( (m_registers[EF9365_REG_Y_MSB]<<8) | m_registers[EF9365_REG_Y_LSB]);

	x_char_res = 5;
	y_char_res = 8;

	if( smallblock )
	{
		block = 1;
		x_char_res = 4;
		y_char_res = 4;
	}

	if(c<96)
	{
		y = ( 256 - y ) - y_char_res;

		if( ( x < ( 256 - 5 ) ) && ( y < ( 256 - y_char_res ) ) )
		{
			char_base = c * 5; // 5 bytes per char.
			char_pix = 0;

			if ( block )
			{
				// 5x8 or 4x4 block

				for( y_char = 0 ; y_char < y_char_res ; y_char++ )
				{
					for( x_char = 0 ; x_char < x_char_res ; x_char++ )
					{
						for( p = 0 ; p < 4 ; p++ )
						{
							if( m_current_color & (0x01 << p) )
								m_videoram->write_byte ( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3), m_videoram->read_byte( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3)) |  (0x80 >> ((((y_char+y)*256) + (x_char+x))&7) ) );
							else
								m_videoram->write_byte ( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3), m_videoram->read_byte( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3)) & ~(0x80 >> ((((y_char+y)*256) + (x_char+x))&7) ) );
						}
						char_pix++;
					}
				}
			}
			else
			{
				// 5x8 character
				for( y_char=0 ; y_char < y_char_res ;y_char++ )
				{
					for( x_char=0 ; x_char < x_char_res ; x_char++ )
					{
						if ( m_charset->u8(char_base + (char_pix>>3) ) & ( 0x80 >> (char_pix&7)))
						{
							for( p = 0 ; p < 4 ; p++ )
							{
								if( m_current_color & (0x01 << p) )
									m_videoram->write_byte ( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3), m_videoram->read_byte( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3)) |  (0x80 >> ((((y_char+y)*256) + (x_char+x))&7) ) );
								else
									m_videoram->write_byte ( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3), m_videoram->read_byte( (0x2000*p) + ((((y_char+y)*256) + (x_char+x))>>3)) & ~(0x80 >> ((((y_char+y)*256) + (x_char+x))&7) ) );
							}
						}

						char_pix++;
					}
				}
			}

			x = x + ( x_char_res + 1 );

			m_registers[EF9365_REG_X_MSB] = x >> 8;
			m_registers[EF9365_REG_X_LSB] = x & 0xFF;
		}
	}
}

void ef9365_device::screen_scanning( int force_clear )
{
	int x,y,p;

	if( (m_registers[EF9365_REG_CTRL1] & 0x02) && !force_clear )
	{
		for( y = 0; y < 256; y++ )
		{
			for( x = 0; x < 256; x++ )
			{
				for( p = 0 ; p < 4 ; p++ )
				{
					if( m_current_color & (0x01 << p) )
						m_videoram->write_byte ( (0x2000*p) + (((y*256) + x)>>3), m_videoram->read_byte( (0x2000*p) + (((y*256) + x)>>3)) |  (0x80 >> (((y*256) + x)&7) ) );
					else
						m_videoram->write_byte ( (0x2000*p) + (((y*256) + x)>>3), m_videoram->read_byte( (0x2000*p) + (((y*256) + x)>>3)) & ~(0x80 >> (((y*256) + x)&7) ) );
				}
			}
		}
	}
	else
	{
		for( y = 0; y < 256; y++)
		{
			for( x = 0; x < 256; x++)
			{
				for( p = 0 ; p < 4 ; p++ )
				{
					m_videoram->write_byte ( (0x2000*p) + (((y*256) + x)>>3), m_videoram->read_byte( (0x2000*p) + (((y*256) + x)>>3)) & ~(0x80 >> (((y*256) + x)&7) ) );
				}
			}
		}
	}
}

// Execute EF9365 command
void ef9365_device::ef9365_exec(UINT8 cmd)
{
	m_state = 0;

	set_busy_flag(4);

	if( ( cmd>>4 ) == 0 )
	{
		switch(cmd & 0xF)
		{
			case 0x0: // Set bit 1 of CTRL1 : Pen Selection
			#ifdef DBGMODE
				printf("Set bit 1 of CTRL1 : Pen Selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] |= 0x02;
			break;
			case 0x1: // Clear bit 1 of CTRL1 : Eraser Selection
			#ifdef DBGMODE
				printf("Clear bit 1 of CTRL1 : Eraser Selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] &= (~0x02);
			break;
			case 0x2: // Set bit 0 of CTRL1 : Pen/Eraser down selection
			#ifdef DBGMODE
				printf("Set bit 0 of CTRL1 : Pen/Eraser down selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] |= 0x01;
			break;
			case 0x3: // Clear bit 0 of CTRL1 : Pen/Eraser up selection
			#ifdef DBGMODE
				printf("Clear bit 0 of CTRL1 : Pen/Eraser up selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] &= (~0x01);
			break;
			case 0x4: // Clear screen
			#ifdef DBGMODE
				printf("Clear screen\n");
			#endif
				screen_scanning(1);
			break;
			case 0x5: // X and Y registers reset to 0
			#ifdef DBGMODE
				printf("X and Y registers reset to 0\n");
			#endif
				m_registers[EF9365_REG_X_MSB] = 0;
				m_registers[EF9365_REG_X_LSB] = 0;

				m_registers[EF9365_REG_Y_MSB] = 0;
				m_registers[EF9365_REG_Y_LSB] = 0;
			break;
			case 0x6: // X and Y registers reset to 0 and clear screen
			#ifdef DBGMODE
				printf("X and Y registers reset to 0 and clear screen\n");
			#endif
				m_registers[EF9365_REG_X_MSB] = 0;
				m_registers[EF9365_REG_X_LSB] = 0;

				m_registers[EF9365_REG_Y_MSB] = 0;
				m_registers[EF9365_REG_Y_LSB] = 0;
			break;
			case 0x7: // Clear screen, set CSIZE to code "minsize". All other registers reset to 0
			#ifdef DBGMODE
				printf("Clear screen, set CSIZE to code \"minsize\". All other registers reset to 0\n");
			#endif
				screen_scanning(1);
			break;
			case 0x8: // Light-pen initialization (/White forced low)
			#ifdef DBGMODE
				printf("Light-pen initialization (/White forced low)\n");
			#endif
			break;
			case 0x9: // Light-pen initialization
			#ifdef DBGMODE
				printf("Light-pen initialization\n");
			#endif
			break;
			case 0xA: // 5x8 block drawing (size according to CSIZE)
			#ifdef DBGMODE
				printf("5x8 block drawing (size according to CSIZE)\n");
			#endif
				draw_character( 0x00 , 1 , 0 );
			break;
			case 0xB: // 4x4 block drawing (size according to CSIZE)
			#ifdef DBGMODE
				printf("4x4 block drawing (size according to CSIZE)\n");
			#endif
				draw_character( 0x00 , 1 , 1 );
			break;
			case 0xC: // Screen scanning : pen or Eraser as defined by CTRL1
			#ifdef DBGMODE
				printf("Screen scanning : pen or Eraser as defined by CTRL1\n");
			#endif
				screen_scanning(0);
			break;
			case 0xD: // X  reset to 0
			#ifdef DBGMODE
				printf("X  reset to 0\n");
			#endif
				m_registers[EF9365_REG_X_MSB] = 0;
				m_registers[EF9365_REG_X_LSB] = 0;
			break;
			case 0xE: // Y  reset to 0
			#ifdef DBGMODE
				printf("Y  reset to 0\n");
			#endif
				m_registers[EF9365_REG_Y_MSB] = 0;
				m_registers[EF9365_REG_Y_LSB] = 0;
			break;
			case 0xF: // Direct image memory access request for the next free cycle.
			#ifdef DBGMODE
				printf("Direct image memory access request for the next free cycle.\n");
			#endif
			break;
			default:
				logerror("Unemulated EF9365 cmd: %02x\n", cmd);
		}
	}
	else
	{
		if ( ( cmd>>4 ) == 1 )
		{
			if( ( cmd & 0xF ) < 0x8 )
			{
				// Vector generation ( for b2,b1,0 see small vector definition)
				#ifdef DBGMODE
				printf("Vector generation ( for b2,b1,0 see small vector definition)\n");
				#endif
			}
			else
			{
				// Special direction vectors generation ( for b2,b1,0 see small vector definition)
				#ifdef DBGMODE
				printf("Special direction vectors generation ( for b2,b1,0 see small vector definition)\n");
				#endif
			}
		}
		else
		{
			if( ( cmd>>4 ) >= 0x8 )
			{
				// Small vector definition.
				#ifdef DBGMODE
				printf("Small vector definition.\n");
				#endif
			}
			else
			{
				// Draw character
				#ifdef DBGMODE
				printf("Draw character\n");
				#endif
				draw_character( cmd - 0x20, 0 , 0 );
			}
		}
	}
}


/**************************************************************
            EF9365 interface
**************************************************************/

UINT32 ef9365_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i,j,k;
	unsigned char color_index;

	k = 0;
	for(i=0;i<256;i++)
	{
		for(j=0;j<256;j++)
		{
			color_index = 0x00;

			if( m_videoram->read_byte(0x0000 + (k>>3)) & (0x80>>(k&7)))
			{
				color_index |= 0x01;
			}

			if( m_videoram->read_byte(0x2000 + (k>>3)) & (0x80>>(k&7)))
			{
				color_index |= 0x02;
			}

			if( m_videoram->read_byte(0x4000 + (k>>3)) & (0x80>>(k&7)))
			{
				color_index |= 0x04;
			}

			if( m_videoram->read_byte(0x6000 + (k>>3)) & (0x80>>(k&7)))
			{
				color_index |= 0x08;
			}

			m_screen_out.pix32(i, j) = palette[ color_index&0xF ];

			k++;
		}
	}

	copybitmap(bitmap, m_screen_out, 0, 0, 0, 0, cliprect);
	return 0;
}

void ef9365_device::update_scanline(UINT16 scanline)
{
	if (scanline == 250)
		m_state |= (0x02); // vsync

	if (scanline == 0)
	{
		m_state &= (~0x02);
		draw_border(0);
	}

	else if (scanline < 120)
	{
	}
	else if (scanline < 250)
	{
	}
}

READ8_MEMBER( ef9365_device::data_r )
{
	if (offset & 0xF)
		return m_registers[offset & 0xF];

	if (m_bf)
		m_state &= (~0x04);
	else
		m_state |= 0x04;

	#ifdef DBGMODE
	printf("rd ef9365 [0x%2x] = [0x%2x]\n", offset&0xF,m_state );
	#endif
	return m_state;
}

WRITE8_MEMBER( ef9365_device::data_w )
{
	m_registers[offset & 0xF] = data;
	#ifdef DBGMODE
	printf("wr ef9365 [0x%2x] = [0x%2x]\n", offset&0xF,data );
	#endif

	if (!offset)
		ef9365_exec(m_registers[0] & 0xff);
}
