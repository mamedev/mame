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

#ifdef DBGMODE
const char * register_names[]=
{
	"0x00 - CMD / STATUS",
	"0x01 - CTRL 1      ",
	"0x02 - CTRL 2      ",
	"0x03 - CSIZE       ",
	"0x04 - RESERVED    ",
	"0x05 - DELTA X     ",
	"0x06 - RESERVED    ",
	"0x07 - DELTA Y     ",
	"0x08 - X MSBs      ",
	"0x09 - X LSBs      ",
	"0x0A - Y MSBs      ",
	"0x0B - X MSBs      ",
	"0x0C - XLP         ",
	"0x0D - YLP         ",
	"0x0E - RESERVED    ",
	"0x0F - RESERVED    "
};
#endif

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
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, NULL, *ADDRESS_MAP_NAME(ef9365)),
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

unsigned int ef9365_device::get_x_reg()
{
	return (m_registers[EF9365_REG_X_MSB]<<8) | m_registers[EF9365_REG_X_LSB];
}

unsigned int ef9365_device::get_y_reg()
{
	return (m_registers[EF9365_REG_Y_MSB]<<8) | m_registers[EF9365_REG_Y_LSB];
}

void ef9365_device::set_x_reg(unsigned int x)
{
	m_registers[EF9365_REG_X_MSB] = x >> 8;
	m_registers[EF9365_REG_X_LSB] = x & 0xFF;
}

void ef9365_device::set_y_reg(unsigned int y)
{
	m_registers[EF9365_REG_Y_MSB] = y >> 8;
	m_registers[EF9365_REG_Y_LSB] = y & 0xFF;
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

void ef9365_device::plot(int x_pos,int y_pos)
{
	int p;

	if( ( x_pos >= 0 && y_pos >= 0 ) && ( x_pos < 256 && y_pos < 256 ) )
	{
		if ( m_registers[EF9365_REG_CTRL1] & 0x01 )
		{
			y_pos = ( 255 - y_pos );

			if( (m_registers[EF9365_REG_CTRL1] & 0x02) )
			{	// Pen
				for( p = 0 ; p < 4 ; p++ )
				{
					if( m_current_color & (0x01 << p) )
						m_videoram->write_byte ( (0x2000*p) + (((y_pos*256) + x_pos)>>3), m_videoram->read_byte( (0x2000*p) + (((y_pos*256) + x_pos)>>3)) |  (0x80 >> (((y_pos*256) + x_pos)&7) ) );
					else
						m_videoram->write_byte ( (0x2000*p) + (((y_pos*256) + x_pos)>>3), m_videoram->read_byte( (0x2000*p) + (((y_pos*256) + x_pos)>>3)) & ~(0x80 >> (((y_pos*256) + x_pos)&7) ) );
				}
			}
			else
			{	// Eraser
				for( p = 0 ; p < 4 ; p++ )
				{
					m_videoram->write_byte ( (0x2000*p) + (((y_pos*256) + x_pos)>>3), m_videoram->read_byte( (0x2000*p) + (((y_pos*256) + x_pos)>>3)) | (0x80 >> (((y_pos*256) + x_pos)&7) ) );
				}
			}
		}
	}
}

const static unsigned int vectortype_code[][8] =
{
	{0x8F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Continous drawing
	{0x82,0x02,0x00,0x00,0x00,0x00,0x00,0x00}, // dotted - 2 dots on, 2 dots off
	{0x84,0x04,0x00,0x00,0x00,0x00,0x00,0x00}, // dashed - 4 dots on, 4 dots off
	{0x8A,0x02,0x82,0x02,0x00,0x00,0x00,0x00}  // dotted-dashed - 10 dots on, 2 dots off, 2 dots on, 2 dots off
};

void ef9365_device::draw_vector(int x1,int y1,int x2,int y2)
{
	int dx;
	int dy,t;
	int e;
	int x,y;
	int incy;
	int diago,horiz;
	unsigned char c1;

	int pen_state;
	unsigned int state_counter;
	int dot_code_ptr;

	c1=0;
	incy=1;

	dot_code_ptr = 0;
	state_counter = vectortype_code[m_registers[EF9365_REG_CTRL2] & 0x3][dot_code_ptr&7];
	if(state_counter&0x80)
		pen_state = 1;
	else
		pen_state = 0;
	state_counter &= ~0x80;

	if(x2>x1)
		dx = x2 - x1;
	else
		dx = x1 - x2;

	if(y2>y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	if( dy > dx )
	{
		t = y2;
		y2 = x2;
		x2 = t;

		t = y1;
		y1 = x1;
		x1 = t;

		t = dx;
		dx = dy;
		dy = t;

		c1 = 1;
	}

	if( x1 > x2 )
	{
		t = y2;
		y2 = y1;
		y1 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	horiz = dy<<1;
	diago = ( dy - dx )<<1;
	e = ( dy<<1 ) - dx;

	if( y1 <= y2 )
		incy = 1;
	else
		incy = -1;

	x = x1;
	y = y1;

	if(c1)
	{
		do
		{
			if(pen_state)
				plot(y,x);

			set_x_reg(y);
			set_y_reg(x);

			state_counter--;

			if( !state_counter )
			{
				dot_code_ptr++;

				state_counter = vectortype_code[m_registers[EF9365_REG_CTRL2] & 0x3][dot_code_ptr&7];

				if(!state_counter)
				{
					dot_code_ptr = 0;
					state_counter = vectortype_code[m_registers[EF9365_REG_CTRL2] & 0x3][dot_code_ptr&7];
				}

				if( state_counter & 0x80 )
				{
					pen_state = 1;
				}
				else
				{
					pen_state = 0;
				}

				state_counter &= ~0x80;
			}

			if( e > 0 )
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		}while( x <= x2 && x<256 );
	}
	else
	{
		do
		{
			if(pen_state)
				plot(x,y);

			set_x_reg(x);
			set_y_reg(y);

			state_counter--;

			if( !state_counter )
			{
				dot_code_ptr++;

				state_counter = vectortype_code[m_registers[EF9365_REG_CTRL2] & 0x3][dot_code_ptr&7];

				if(!state_counter)
				{
					dot_code_ptr = 0;
					state_counter = vectortype_code[m_registers[EF9365_REG_CTRL2] & 0x3][dot_code_ptr&7];
				}

				if( state_counter & 0x80 )
				{
					pen_state = 1;
				}
				else
				{
					pen_state = 0;
				}

				state_counter &= ~0x80;
			}

			if( e > 0 )
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		}while( x <= x2 );
	}
}

int ef9365_device::get_char_pix( unsigned char c, int x, int y )
{
	int char_base,char_pix;

	if(c<96)
	{
		if( x < 5 && y < 8 )
		{
			char_base =  c * 5;
			char_pix = ( y * 5 ) + x;

			if ( m_charset->u8(char_base + (char_pix>>3) ) & ( 0x80 >> (char_pix&7)) )
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

void ef9365_device::draw_character( unsigned char c, int block, int smallblock )
{
	int x_char,y_char;
	unsigned int x, y;
	int x_char_res,y_char_res;
	int p_factor,q_factor,p,q;

	x = get_x_reg();
	y = get_y_reg();

	x_char_res = 5;
	y_char_res = 8;

	if( smallblock )
	{
		block = 1;
		x_char_res = 4;
		y_char_res = 4;
	}

	p_factor = (m_registers[EF9365_REG_CSIZE] >> 4);
	if(!p_factor)
		p_factor = 16;

	q_factor = (m_registers[EF9365_REG_CSIZE] &  0xF);
	if(!q_factor)
		q_factor = 16;

	if(c<96)
	{
		for( x_char=0 ; x_char < x_char_res ; x_char++ )
		{
			for( y_char = y_char_res - 1 ; y_char >= 0 ; y_char-- )
			{
				if ( block || get_char_pix( c, x_char, ( (y_char_res - 1) - y_char ) ) )
				{
					if( m_registers[EF9365_REG_CTRL2] & 0x04) // Titled character ?
					{
						for(q = 0; q < q_factor; q++)
						{
							for(p = 0; p < p_factor; p++)
							{
								if( !(m_registers[EF9365_REG_CTRL2] & 0x08) )
								{	// Titled - Horizontal orientation
									plot(
											x + ( (y_char*q_factor) + q ) + ( (x_char*p_factor) + p ),
											y + ( (y_char*q_factor) + q )
										);
								}
								else
								{
									// Titled - Vertical orientation
									plot(
											x - ( (y_char*q_factor)+ q ),
											y + ( (x_char*p_factor)+ p ) - ( ( ( (y_char_res - 1 ) - y_char) * q_factor ) + ( q_factor - q ) )
										);
								}
							}
						}
					}
					else
					{
						for(q = 0; q < q_factor; q++)
						{
							for(p = 0; p < p_factor; p++)
							{
								if( !(m_registers[EF9365_REG_CTRL2] & 0x08) )
								{	// Normal - Horizontal orientation
									plot(
											x + ( (x_char*p_factor) + p ),
											y + ( (y_char*q_factor) + q )
										);
								}
								else
								{	// Normal - Vertical orientation
									plot(
											x - ( (y_char*q_factor) + q ),
											y + ( (x_char*p_factor) + p )
										);
								}
							}
						}
					}
				}
			}
		}

		if(!(m_registers[EF9365_REG_CTRL2] & 0x08))
		{
			x = x + ( (x_char_res + 1 ) * p_factor ) ;
			set_x_reg(x);
		}
		else
		{
			y = y + ( (x_char_res + 1 ) * p_factor ) ;
			set_x_reg(y);
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
					m_videoram->write_byte ( (0x2000*p) + (((y*256) + x)>>3), m_videoram->read_byte( (0x2000*p) + (((y*256) + x)>>3)) | (0x80 >> (((y*256) + x)&7) ) );
				}
			}
		}
	}
}

// Execute EF9365 command
void ef9365_device::ef9365_exec(UINT8 cmd)
{
	int tmp_delta_x,tmp_delta_y;
	m_state = 0;

	if( ( cmd>>4 ) == 0 )
	{
		switch(cmd & 0xF)
		{
			case 0x0: // Set bit 1 of CTRL1 : Pen Selection
			#ifdef DBGMODE
				printf("Set bit 1 of CTRL1 : Pen Selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] |= 0x02;
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0x1: // Clear bit 1 of CTRL1 : Eraser Selection
			#ifdef DBGMODE
				printf("Clear bit 1 of CTRL1 : Eraser Selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] &= (~0x02);
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0x2: // Set bit 0 of CTRL1 : Pen/Eraser down selection
			#ifdef DBGMODE
				printf("Set bit 0 of CTRL1 : Pen/Eraser down selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] |= 0x01;
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0x3: // Clear bit 0 of CTRL1 : Pen/Eraser up selection
			#ifdef DBGMODE
				printf("Clear bit 0 of CTRL1 : Pen/Eraser up selection\n");
			#endif
				m_registers[EF9365_REG_CTRL1] &= (~0x01);
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0x4: // Clear screen
			#ifdef DBGMODE
				printf("Clear screen\n");
			#endif
				screen_scanning(1);
				set_busy_flag(30*40*25); // Timing to check on the real hardware
			break;
			case 0x5: // X and Y registers reset to 0
			#ifdef DBGMODE
				printf("X and Y registers reset to 0\n");
			#endif
				set_x_reg(0);
				set_y_reg(0);
				set_busy_flag(80); // Timing to check on the real hardware
			break;
			case 0x6: // X and Y registers reset to 0 and clear screen
			#ifdef DBGMODE
				printf("X and Y registers reset to 0 and clear screen\n");
			#endif
				set_x_reg(0);
				set_y_reg(0);
				screen_scanning(1);
				set_busy_flag(30*40*25); // Timing to check on the real hardware
			break;
			case 0x7: // Clear screen, set CSIZE to code "minsize". All other registers reset to 0
			#ifdef DBGMODE
				printf("Clear screen, set CSIZE to code \"minsize\". All other registers reset to 0\n");
			#endif
				m_registers[EF9365_REG_CSIZE] = 0x11;
				screen_scanning(1);
				set_busy_flag(30*40*25); // Timing to check on the real hardware
			break;
			case 0x8: // Light-pen initialization (/White forced low)
			#ifdef DBGMODE
				printf("Light-pen initialization (/White forced low)\n");
			#endif
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0x9: // Light-pen initialization
			#ifdef DBGMODE
				printf("Light-pen initialization\n");
			#endif
				set_busy_flag(10); // Timing to check on the real hardware
			break;
			case 0xA: // 5x8 block drawing (size according to CSIZE)
			#ifdef DBGMODE
				printf("5x8 block drawing (size according to CSIZE)\n");
			#endif
				draw_character( 0x00 , 1 , 0 );
				set_busy_flag(72); // Value measured from the Apollo Squale
			break;
			case 0xB: // 4x4 block drawing (size according to CSIZE)
			#ifdef DBGMODE
				printf("4x4 block drawing (size according to CSIZE)\n");
			#endif
				draw_character( 0x00 , 1 , 1 );
				set_busy_flag(72); // Timing to check on the real hardware
			break;
			case 0xC: // Screen scanning : pen or Eraser as defined by CTRL1
			#ifdef DBGMODE
				printf("Screen scanning : pen or Eraser as defined by CTRL1\n");
			#endif
				screen_scanning(0);
				set_busy_flag(30*40*25); // Timing to check
			break;
			case 0xD: // X  reset to 0
			#ifdef DBGMODE
				printf("X  reset to 0\n");
			#endif
				set_x_reg(0);
				set_busy_flag(5); // Timing to check on the real hardware
			break;
			case 0xE: // Y  reset to 0
			#ifdef DBGMODE
				printf("Y  reset to 0\n");
			#endif
				set_y_reg(0);
				set_busy_flag(5); // Timing to check on the real hardware
			break;
			case 0xF: // Direct image memory access request for the next free cycle.
			#ifdef DBGMODE
				printf("Direct image memory access request for the next free cycle.\n");
			#endif
				set_busy_flag(30); // Timing to check on the real hardware
			break;
			default:
				logerror("Unemulated EF9365 cmd: %02x\n", cmd);
		}
	}
	else
	{
		if ( ( cmd>>4 ) == 1 )
		{
			tmp_delta_x = m_registers[EF9365_REG_DELTAX];
			tmp_delta_y = m_registers[EF9365_REG_DELTAY];

			if( cmd & 0x08 )
			{
				if(tmp_delta_x > tmp_delta_y )
					tmp_delta_y = tmp_delta_x;
				if(tmp_delta_y > tmp_delta_x )
					tmp_delta_y = tmp_delta_x;
			}

			// Basic vector commands
			switch ( cmd & 0x7 )
			{
				case 0x1:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() + tmp_delta_y);
				break;
				case 0x3:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() + tmp_delta_y);
				break;
				case 0x5:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() - tmp_delta_y);
				break;
				case 0x7:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() - tmp_delta_y);
				break;

				case 0x0:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() );
				break;
				case 0x2:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() + tmp_delta_y);
				break;
				case 0x4:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() - tmp_delta_y);
				break;
				case 0x6:
					draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() );
				break;
			}

			// Vector generation ( for b2,b1,0 see small vector definition)
			#ifdef DBGMODE
			printf("Vector generation ( for b2,b1,0 see small vector definition)\n");
			#endif

			set_busy_flag(30); // Timing to check
		}
		else
		{
			if( ( cmd>>4 ) >= 0x8 )
			{
				// Small vector definition.
				#ifdef DBGMODE
				printf("Small vector definition.\n");
				#endif

				tmp_delta_x = ( cmd >> 5 ) & 3;
				tmp_delta_y = ( cmd >> 3 ) & 3;

				// Basic vector commands
				switch ( cmd & 0x7 )
				{
					case 0x1:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() + tmp_delta_y);
					break;
					case 0x3:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() + tmp_delta_y);
					break;
					case 0x5:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() - tmp_delta_y);
					break;
					case 0x7:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() - tmp_delta_y);
					break;

					case 0x0:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() );
					break;
					case 0x2:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() + tmp_delta_y);
					break;
					case 0x4:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() - tmp_delta_y);
					break;
					case 0x6:
						draw_vector	( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() );
					break;
				}
			}
			else
			{
				// Draw character
				#ifdef DBGMODE
				printf("Draw character\n");
				#endif
				draw_character( cmd - 0x20, 0 , 0 );
				set_busy_flag(30); // Value measured from the Squale
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
	unsigned char return_value;

	switch(offset & 0xF)
	{
		case EF9365_REG_STATUS:
			if (m_bf)
				m_state &= (~0x04);
			else
				m_state |= 0x04;

			return_value = m_state;
		break;
		case EF9365_REG_CTRL1:
			return_value = m_registers[EF9365_REG_CTRL1] & 0x7F;
		break;
		case EF9365_REG_CTRL2:
			return_value = m_registers[EF9365_REG_CTRL2] & 0x0F;
		break;
		case EF9365_REG_CSIZE:
			return_value = m_registers[EF9365_REG_CSIZE];
		break;
		case EF9365_REG_DELTAX:
			return_value = m_registers[EF9365_REG_DELTAX];
		break;
		case EF9365_REG_DELTAY:
			return_value = m_registers[EF9365_REG_DELTAY];
		break;
		case EF9365_REG_X_MSB:
			return_value = m_registers[EF9365_REG_X_MSB] & 0x0F;
		break;
		case EF9365_REG_X_LSB:
			return_value = m_registers[EF9365_REG_X_LSB];
		break;
		case EF9365_REG_Y_MSB:
			return_value = m_registers[EF9365_REG_Y_MSB] & 0x0F;
		break;
		case EF9365_REG_Y_LSB:
			return_value = m_registers[EF9365_REG_Y_LSB];
		break;
		case EF9365_REG_XLP:
			return_value = m_registers[EF9365_REG_XLP] & 0xFD;
		break;
		case EF9365_REG_YLP:
			return_value = m_registers[EF9365_REG_YLP];
		break;
		default:
			return_value = 0xFF;
		break;
	}

	#ifdef DBGMODE
	printf("EF9365 [ %s ] RD> [ 0x%.2X ]\n", register_names[offset&0xF],return_value );
	#endif

	return return_value;
}

WRITE8_MEMBER( ef9365_device::data_w )
{
	switch(offset & 0xF)
	{
		case EF9365_REG_CMD:
			ef9365_exec( data & 0xff);
		break;
		case EF9365_REG_CTRL1:
			m_registers[EF9365_REG_CTRL1] = data & 0x7F;
		break;
		case EF9365_REG_CTRL2:
			m_registers[EF9365_REG_CTRL2] = data & 0x0F;
		break;
		case EF9365_REG_CSIZE:
			m_registers[EF9365_REG_CSIZE] = data;
		break;
		case EF9365_REG_DELTAX:
			m_registers[EF9365_REG_DELTAX] = data;
		break;
		case EF9365_REG_DELTAY:
			m_registers[EF9365_REG_DELTAY] = data;
		break;
		case EF9365_REG_X_MSB:
			m_registers[EF9365_REG_X_MSB] = data & 0x0F;
		break;
		case EF9365_REG_X_LSB:
			m_registers[EF9365_REG_X_LSB] = data;
		break;
		case EF9365_REG_Y_MSB:
			m_registers[EF9365_REG_Y_MSB] = data & 0x0F;
		break;
		case EF9365_REG_Y_LSB:
			m_registers[EF9365_REG_Y_LSB] = data;
		break;
		case EF9365_REG_XLP:
		break;
		case EF9365_REG_YLP:
		break;
		default:
		break;
	}

	#ifdef DBGMODE
	printf("EF9365 [ %s ] <WR [ 0x%.2X ]\n", register_names[offset&0xF],data );
	#endif
}
