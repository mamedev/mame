// license:BSD-3-Clause
// copyright-holders:Jean-Francois DEL NERO

/*********************************************************************

    ef9365.c

    Thomson EF9365/EF9366 video controller emulator code

    The EF9365/EF9366 is a video controller driving a frame buffer
    and having built-in vectors and characters drawing engines.
    This is natively a "black and white" chip (1 bitplane),
    but it is possible to add more bitplanes to have colors with a
    hardware trick. The system don't have direct access to the video
    memory, but indirect access is possible through the 0x0F command
    and some hardware glue logics.
    The current implementation emulate the main functions :

    Video modes supported (Hardware implementation dependent):
        - 256 x 256 (EF9365 with 4 bits shifters per bitplane and FMAT to VSS)
        - 512 x 512 interlaced (EF9365 with 8 bits shifters per bitplane and FMAT to VCC)
        - 512 x 256 non interlaced (EF9366 with 8 bits shifters per bitplane)
        - 128 x 128 (EF9365 with 2 bits shifters per bitplane and FMAT to VSS)
        - 64 x 64 (EF9365 with FMAT to VSS)

        - 1 bitplane up to 8 bitplanes hardware configuration.
        - 2 up to 256 colors fixed palette.

    Character & block drawing :
        - Normal / Titled mode
        - Horizontal / Vertical orientation
        - P & Q Zoom factors (1 up to 16)

    Vector drawing :
        - Normal / Dotted / Dashed / Dotted-Dashed mode
        - All directions and size supported.

    General :
        - Clear Screen
        - Fill Screen
        - Clear X & Y registers
        - Video RAM readback supported (Command 0x0F)

    What is NOT yet currently implemented:
        - Light pen support
        (To be done when i will find a software using the lightpen)

    What is implemented but not really tested:
        - Interrupts output.
        My target system (Squale Apollo 7) doesn't use the interruption
        for this chip. So i add the interrupt line support, but
        bug(s) is possible.

    The needed charset file charset_ef9365.rom (CRC 8d3053be) is available
    there : http://hxc2001.free.fr/Squale/rom/charset_ef9365.zip
    This ROM charset is into the EF9365/EF9366.

    To see how to use this driver, have a look to the Squale machine
    driver (squale.cpp).
    If you have any question, don't hesitate to contact me at the email
    present on this website : http://hxc2001.free.fr/

    12/29/2015
    Jean-Francois DEL NERO
*********************************************************************/

#include "emu.h"
#include "ef9365.h"

#ifdef DBGMODE
//-------------------------------------------------
// Some debug mode const strings
// to trace the commands and registers accesses.
//-------------------------------------------------

// Registers list
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
	"0x0B - Y LSBs      ",
	"0x0C - XLP         ",
	"0x0D - YLP         ",
	"0x0E - RESERVED    ",
	"0x0F - RESERVED    "
};

// Commands list
const char * commands_names[]=
{
	"0x00 - Set bit 1 of CTRL1   : Pen selection",
	"0x01 - Clear bit 1 of CTRL1 : Eraser selection",
	"0x02 - Set bit 0 of CTRL1   : Pen/Eraser down selection",
	"0x03 - Clear bit 0 of CTRL1 : Pen/Eraser up selection",
	"0x04 - Clear screen",
	"0x05 - X and Y registers reset to 0",
	"0x06 - X and Y registers reset to 0 and clear screen",
	"0x07 - Clear screen, set CSIZE to code \"minsize\". All other registers reset to 0",
	"0x08 - Light-pen initialization (/White forced low)",
	"0x09 - Light-pen initialization",
	"0x0A - 5x8 block drawing (size according to CSIZE)",
	"0x0B - 4x4 block drawing (size according to CSIZE)",
	"0x0C - Screen scanning : pen or Eraser as defined by CTRL1",
	"0x0D - X  reset to 0",
	"0x0E - Y  reset to 0",
	"0x0F - Direct image memory access request for the next free cycle.",
	"0x10<>0x17 - Vector generation",
	"0x18<>0x1F - Special direction vectors",
	"0x20<>0x7F - Character Drawing",
	"0x80<>0xFF - Small vector generation",
};

#endif

// devices
const device_type EF9365 = &device_creator<ef9365_device>;

//-------------------------------------------------
// default address map
// Up to 512*512 per bitplane, 8 bitplanes max.
//-------------------------------------------------
static ADDRESS_MAP_START( ef9365, AS_0, 8, ef9365_device )
	AM_RANGE(0x00000, ( ( EF936X_BITPLANE_MAX_SIZE * EF936X_MAX_BITPLANES ) - 1 ) ) AM_RAM
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
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 18, 0, nullptr, *ADDRESS_MAP_NAME(ef9365)),
	m_charset(*this, DEVICE_SELF),
	m_palette(*this),
	m_irq_handler(*this)
{
	clock_freq = clock;
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
//  static_set_nb_of_bitplanes: Set the number of bitplanes
//-------------------------------------------------

void ef9365_device::static_set_nb_bitplanes(device_t &device, int nb_bitplanes )
{
	if( nb_bitplanes > 0 && nb_bitplanes <= 8 )
	{
		downcast<ef9365_device &>(device).nb_of_bitplanes = nb_bitplanes;
		downcast<ef9365_device &>(device).nb_of_colors = pow(2,nb_bitplanes);
	}
}

//-------------------------------------------------
//  static_set_display_mode: Set the display mode
//-------------------------------------------------

void ef9365_device::static_set_display_mode(device_t &device, int display_mode )
{
	switch(display_mode)
	{
		case EF936X_256x256_DISPLAY_MODE:
			downcast<ef9365_device &>(device).bitplane_xres = 256;
			downcast<ef9365_device &>(device).bitplane_yres = 256;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 250;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFF00;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFF00;
		break;
		case EF936X_512x512_DISPLAY_MODE:
			downcast<ef9365_device &>(device).bitplane_xres = 512;
			downcast<ef9365_device &>(device).bitplane_yres = 512;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 506;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFE00;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFE00;
		break;
		case EF936X_512x256_DISPLAY_MODE:
			downcast<ef9365_device &>(device).bitplane_xres = 512;
			downcast<ef9365_device &>(device).bitplane_yres = 256;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 250;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFE00;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFF00;
		break;
		case EF936X_128x128_DISPLAY_MODE:
			downcast<ef9365_device &>(device).bitplane_xres = 128;
			downcast<ef9365_device &>(device).bitplane_yres = 128;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 124;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFF80;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFF80;
		break;
		case EF936X_64x64_DISPLAY_MODE:
			downcast<ef9365_device &>(device).bitplane_xres = 64;
			downcast<ef9365_device &>(device).bitplane_yres = 64;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 62;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFFC0;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFFC0;
		break;
		default:
			downcast<ef9365_device &>(device).logerror("Invalid EF9365 Display mode: %02x\n", display_mode);
			downcast<ef9365_device &>(device).bitplane_xres = 256;
			downcast<ef9365_device &>(device).bitplane_yres = 256;
			downcast<ef9365_device &>(device).vsync_scanline_pos = 250;
			downcast<ef9365_device &>(device).overflow_mask_x = 0xFF00;
			downcast<ef9365_device &>(device).overflow_mask_y = 0xFF00;
		break;
	}
}

//-------------------------------------------------
//  set_color_entry: Set the color value
//  into the palette
//-------------------------------------------------

void ef9365_device::set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b )
{
	if( index < nb_of_colors )
	{
		palette[index] = rgb_t(r, g, b);
	}
	else
	{
		logerror("Invalid EF9365 Palette entry : %02x\n", index);
	}
}

//-------------------------------------------------
//  set_color_filler: Set the color number
//  used by the chip to draw/fill the memory
//-------------------------------------------------

void ef9365_device::set_color_filler( UINT8 color )
{
	m_current_color = color;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ef9365_device::device_start()
{
	int i;

	m_irq_handler.resolve_safe();

	m_busy_timer = timer_alloc(BUSY_TIMER);

	m_videoram = &space(0);
	m_current_color = 0x0F;

	m_irq_vb = 0;
	m_irq_lb = 0;
	m_irq_rdy = 0;
	m_irq_state = 0;

	// Default palette : Black and white
	palette[0] = rgb_t(0, 0, 0);
	for( i = 1; i < 16 ; i++ )
	{
		palette[i] = rgb_t(255, 255, 255);
	}

	m_screen_out.allocate( bitplane_xres, m_screen->height() );

	save_item(NAME(m_border));
	save_item(NAME(m_registers));
	save_item(NAME(m_bf));
	save_item(NAME(m_state));

	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_vb));
	save_item(NAME(m_irq_lb));
	save_item(NAME(m_irq_rdy));

	save_item(NAME(m_screen_out));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ef9365_device::device_reset()
{
	m_state = 0;

	m_bf = 0;
	m_irq_state = 0;
	m_irq_vb = 0;
	m_irq_lb = 0;
	m_irq_rdy = 0;

	memset(m_registers, 0, sizeof(m_registers));
	memset(m_border, 0, sizeof(m_border));

	m_screen_out.fill(0);

	set_video_mode();

	m_irq_handler(FALSE);
}

//-------------------------------------------------
//  update_interrupts
//-------------------------------------------------
void ef9365_device::update_interrupts()
{
	int new_state = ( m_irq_vb  && (m_registers[EF936X_REG_CTRL1] & 0x20) )
					|| ( m_irq_rdy && (m_registers[EF936X_REG_CTRL1] & 0x40) )
					|| ( m_irq_lb  && (m_registers[EF936X_REG_CTRL1] & 0x10) );

	if (new_state != m_irq_state)
	{
		m_irq_state = new_state;
		m_irq_handler(m_irq_state);
	}
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

			if( m_registers[EF936X_REG_CTRL1] & 0x40 )
			{
				m_irq_rdy = 1;
			}

			update_interrupts();

			break;
	}
}

//-------------------------------------------------
//  set_busy_flag: set busy flag and
//  timer to clear it
//-------------------------------------------------

void ef9365_device::set_busy_flag(int period)
{
	m_bf = 1;
	m_busy_timer->adjust(attotime::from_usec(period));
}

//-------------------------------------------------
//  get_x_reg: Get the X register value
//-------------------------------------------------

unsigned int ef9365_device::get_x_reg()
{
	return (m_registers[EF936X_REG_X_MSB]<<8) | m_registers[EF936X_REG_X_LSB];
}

//-------------------------------------------------
//  get_y_reg: Get the Y register value
//-------------------------------------------------

unsigned int ef9365_device::get_y_reg()
{
	return (m_registers[EF936X_REG_Y_MSB]<<8) | m_registers[EF936X_REG_Y_LSB];
}

//-------------------------------------------------
//  set_x_reg: Set the X register value
//-------------------------------------------------

void ef9365_device::set_x_reg(unsigned int x)
{
	m_registers[EF936X_REG_X_MSB] = x >> 8;
	m_registers[EF936X_REG_X_LSB] = x & 0xFF;
}

//-------------------------------------------------
//  set_y_reg: Set the Y register value
//-------------------------------------------------

void ef9365_device::set_y_reg(unsigned int y)
{
	m_registers[EF936X_REG_Y_MSB] = y >> 8;
	m_registers[EF936X_REG_Y_LSB] = y & 0xFF;
}

//-------------------------------------------------
//  set_video_mode: Set output screen format
//-------------------------------------------------

void ef9365_device::set_video_mode(void)
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
//  get_last_readback_word: Read back the latched
//  bitplane words
//-------------------------------------------------

UINT8 ef9365_device::get_last_readback_word(int bitplane_number, int * pixel_offset)
{
	if( pixel_offset )
		*pixel_offset = m_readback_latch_pix_offset;

	if( bitplane_number < nb_of_bitplanes )
	{
		return m_readback_latch[bitplane_number];
	}
	else
	{
		return 0x00;
	}
}

//-------------------------------------------------
//  draw_border: Draw the left and right borders
//  ( No border for the moment ;) )
//-------------------------------------------------

void ef9365_device::draw_border(UINT16 line)
{
}

//-------------------------------------------------
//  plot: Plot a pixel to the bitplanes
//  at the x & y position with the m_current_color color
//-------------------------------------------------

void ef9365_device::plot(int x_pos,int y_pos)
{
	int p;

	if( ( x_pos >= 0 && y_pos >= 0 ) && ( x_pos < bitplane_xres && y_pos < bitplane_yres ) )
	{
		if ( m_registers[EF936X_REG_CTRL1] & 0x01 )
		{
			y_pos = ( (bitplane_yres - 1) - y_pos );

			if( (m_registers[EF936X_REG_CTRL1] & 0x02) )
			{
				// Pen
				for( p = 0 ; p < nb_of_bitplanes ; p++ )
				{
					if( m_current_color & (0x01 << p) )
						m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3)) |  (0x80 >> (((y_pos*bitplane_xres) + x_pos)&7) ) );
					else
						m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3)) & ~(0x80 >> (((y_pos*bitplane_xres) + x_pos)&7) ) );
				}
			}
			else
			{
				// Eraser
				for( p = 0 ; p < nb_of_bitplanes ; p++ )
				{
					m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y_pos*bitplane_xres) + x_pos)>>3)) | (0x80 >> (((y_pos*bitplane_xres) + x_pos)&7) ) );
				}
			}
		}
	}
}


const static unsigned int vectortype_code[][8] =
{
	{0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Continous drawing
	{0x82,0x02,0x00,0x00,0x00,0x00,0x00,0x00}, // Dotted - 2 dots on, 2 dots off
	{0x84,0x04,0x00,0x00,0x00,0x00,0x00,0x00}, // Dashed - 4 dots on, 4 dots off
	{0x8A,0x02,0x82,0x02,0x00,0x00,0x00,0x00}  // Dotted-Dashed - 10 dots on, 2 dots off, 2 dots on, 2 dots off
};

//-------------------------------------------------
//  draw_vector: Vector drawing function
//  from the x1 & y1 position to the x2 & y2 position
//  with the m_current_color color
//  (Bresenham's line algorithm)
//-------------------------------------------------

int ef9365_device::draw_vector(int x1,int y1,int x2,int y2)
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
	int compute_cycles;

	compute_cycles = 0;

	c1=0;
	incy=1;

	dot_code_ptr = 0;
	state_counter = vectortype_code[m_registers[EF936X_REG_CTRL2] & 0x3][dot_code_ptr&7];
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

			compute_cycles++;

			set_x_reg(y);
			set_y_reg(x);

			state_counter--;

			if( !state_counter )
			{
				dot_code_ptr++;

				state_counter = vectortype_code[m_registers[EF936X_REG_CTRL2] & 0x3][dot_code_ptr&7];

				if(!state_counter)
				{
					dot_code_ptr = 0;
					state_counter = vectortype_code[m_registers[EF936X_REG_CTRL2] & 0x3][dot_code_ptr&7];
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
	else
	{
		do
		{
			if(pen_state)
				plot(x,y);

			compute_cycles++;

			set_x_reg(x);
			set_y_reg(y);

			state_counter--;

			if( !state_counter )
			{
				dot_code_ptr++;

				state_counter = vectortype_code[m_registers[EF936X_REG_CTRL2] & 0x3][dot_code_ptr&7];

				if(!state_counter)
				{
					dot_code_ptr = 0;
					state_counter = vectortype_code[m_registers[EF936X_REG_CTRL2] & 0x3][dot_code_ptr&7];
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

	return compute_cycles;
}

//-------------------------------------------------
//  get_char_pix: Get a character pixel state
//  from the charset.
//-------------------------------------------------

int ef9365_device::get_char_pix( unsigned char c, int x, int y )
{
	int char_base,char_pix;

	if(c<96)
	{
		if( x < 5 && y < 8 )
		{
			char_base =  c * 5;
			char_pix = ( y * 5 ) + x;

			if ( m_charset[char_base + (char_pix>>3)] & ( 0x80 >> (char_pix&7)) )
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

//-------------------------------------------------
//  draw_character: Character and block drawing function
//  Set smallblock to draw a 4x4 block
//  Set block to draw a 5x8 block
//-------------------------------------------------

int ef9365_device::draw_character( unsigned char c, int block, int smallblock )
{
	int x_char,y_char;
	unsigned int x, y;
	int x_char_res,y_char_res;
	int p_factor,q_factor,p,q;
	int compute_cycles;

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

	p_factor = (m_registers[EF936X_REG_CSIZE] >> 4);
	if(!p_factor)
		p_factor = 16;

	q_factor = (m_registers[EF936X_REG_CSIZE] &  0xF);
	if(!q_factor)
		q_factor = 16;

	compute_cycles = ( ( x_char_res + 1 ) * p_factor ) * ( y_char_res * q_factor );

	if(c<96)
	{
		for( x_char=0 ; x_char < x_char_res ; x_char++ )
		{
			for( y_char = y_char_res - 1 ; y_char >= 0 ; y_char-- )
			{
				if ( block || get_char_pix( c, x_char, ( (y_char_res - 1) - y_char ) ) )
				{
					if( m_registers[EF936X_REG_CTRL2] & 0x04) // Titled character ?
					{
						for(q = 0; q < q_factor; q++)
						{
							for(p = 0; p < p_factor; p++)
							{
								if( !(m_registers[EF936X_REG_CTRL2] & 0x08) )
								{   // Titled - Horizontal orientation
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
								if( !(m_registers[EF936X_REG_CTRL2] & 0x08) )
								{   // Normal - Horizontal orientation
									plot(
											x + ( (x_char*p_factor) + p ),
											y + ( (y_char*q_factor) + q )
										);
								}
								else
								{   // Normal - Vertical orientation
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

		if(!(m_registers[EF936X_REG_CTRL2] & 0x08))
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

	return compute_cycles;
}

//-------------------------------------------------
//  cycles_to_us: Convert a number of clock cycles to us
//-------------------------------------------------

int ef9365_device::cycles_to_us(int cycles)
{
	return ( (float)cycles * ( (float)1000000 / (float)clock_freq ) );
}

//-------------------------------------------------
// dump_bitplanes_word: Latch the bitplane words
// pointed by the x & y regiters
// (Memory read back function)
//-------------------------------------------------

void ef9365_device::dump_bitplanes_word()
{
	int p;
	int pixel_ptr;

	pixel_ptr = ( ( ( ( bitplane_yres - 1 ) - ( get_y_reg() & ( bitplane_yres - 1 ) ) ) * bitplane_xres ) + ( get_x_reg() & ( bitplane_xres - 1 ) ) );

	#ifdef DBGMODE
	printf("dump : x = %d , y = %d\n", get_x_reg() ,get_y_reg());
	#endif

	for( p = 0; p < nb_of_bitplanes ; p++ )
	{
		if( pixel_ptr & 0x4 )
		{
			m_readback_latch[p] = ( m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (pixel_ptr>>3) )  ) & 0xF ;
		}
		else
		{
			m_readback_latch[p] = ( m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (pixel_ptr>>3) ) >> 4 ) & 0xF ;
		}

	}

	m_readback_latch_pix_offset = pixel_ptr & 0x3;
}

//-------------------------------------------------
// screen_scanning: Fill / Clear framebuffer memory
//-------------------------------------------------

void ef9365_device::screen_scanning( int force_clear )
{
	int x,y,p;

	if( (m_registers[EF936X_REG_CTRL1] & 0x02) && !force_clear )
	{
		for( y = 0; y < bitplane_yres; y++ )
		{
			for( x = 0; x < bitplane_xres; x++ )
			{
				for( p = 0 ; p < nb_of_bitplanes ; p++ )
				{
					if( m_current_color & (0x01 << p) )
						m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3)) |  (0x80 >> (((y*bitplane_xres) + x)&7) ) );
					else
						m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3)) & ~(0x80 >> (((y*bitplane_xres) + x)&7) ) );
				}
			}
		}
	}
	else
	{
		for( y = 0; y < bitplane_yres; y++)
		{
			for( x = 0; x < bitplane_xres; x++)
			{
				for( p = 0 ; p < nb_of_bitplanes ; p++ )
				{
					m_videoram->write_byte ( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3), m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (((y*bitplane_xres) + x)>>3)) | (0x80 >> (((y*bitplane_xres) + x)&7) ) );
				}
			}
		}
	}
}

//-------------------------------------------------
// ef9365_exec: EF936X Command decoder and execution
//-------------------------------------------------

void ef9365_device::ef9365_exec(UINT8 cmd)
{
	int tmp_delta_x,tmp_delta_y;
	int busy_cycles = 0;
	m_state = 0;

	if( ( cmd>>4 ) == 0 )
	{
		#ifdef DBGMODE
		printf("EF9365 Command : %s\n", commands_names[cmd & 0xF]);
		#endif

		switch(cmd & 0xF)
		{
			case 0x0: // Set bit 1 of CTRL1 : Pen Selection
				m_registers[EF936X_REG_CTRL1] |= 0x02;
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x1: // Clear bit 1 of CTRL1 : Eraser Selection
				m_registers[EF936X_REG_CTRL1] &= (~0x02);
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x2: // Set bit 0 of CTRL1 : Pen/Eraser down selection
				m_registers[EF936X_REG_CTRL1] |= 0x01;
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x3: // Clear bit 0 of CTRL1 : Pen/Eraser up selection
				m_registers[EF936X_REG_CTRL1] &= (~0x01);
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x4: // Clear screen
				screen_scanning(1);
				set_busy_flag( cycles_to_us( bitplane_xres*bitplane_yres ) ); // Timing to check on the real hardware
			break;
			case 0x5: // X and Y registers reset to 0
				set_x_reg(0);
				set_y_reg(0);
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x6: // X and Y registers reset to 0 and clear screen
				set_x_reg(0);
				set_y_reg(0);
				screen_scanning(1);
				set_busy_flag( cycles_to_us( bitplane_xres*bitplane_yres ) ); // Timing to check on the real hardware
			break;
			case 0x7: // Clear screen, set CSIZE to code "minsize". All other registers reset to 0
				m_registers[EF936X_REG_CSIZE] = 0x11;
				screen_scanning(1);
				set_busy_flag( cycles_to_us( bitplane_xres*bitplane_yres ) ); // Timing to check on the real hardware
			break;
			case 0x8: // Light-pen initialization (/White forced low)
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0x9: // Light-pen initialization
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0xA: // 5x8 block drawing (size according to CSIZE)
				busy_cycles = draw_character( 0x00 , 1 , 0 );
				set_busy_flag( cycles_to_us( busy_cycles ) );
			break;
			case 0xB: // 4x4 block drawing (size according to CSIZE)
				busy_cycles = draw_character( 0x00 , 1 , 1 );
				set_busy_flag( cycles_to_us( busy_cycles ) );
			break;
			case 0xC: // Screen scanning : pen or Eraser as defined by CTRL1
				screen_scanning(0);
				set_busy_flag( cycles_to_us( bitplane_xres*bitplane_yres ) ); // Timing to check on the real hardware
			break;
			case 0xD: // X  reset to 0
				set_x_reg(0);
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0xE: // Y  reset to 0
				set_y_reg(0);
				set_busy_flag( cycles_to_us( 4 ) ); // Timing to check on the real hardware
			break;
			case 0xF: // Direct image memory access request for the next free cycle.
				set_busy_flag( cycles_to_us( 64 ) ); // Timing to check on the real hardware
				dump_bitplanes_word();
			break;
			default:
				logerror("Unemulated EF9365 cmd: %02x\n", cmd);
		}
	}
	else
	{
		if ( ( cmd>>4 ) == 1 )
		{
			#ifdef DBGMODE
			if( cmd & 0x08 )
				printf("EF9365 Command : [0x%.2X] %s\n", cmd, commands_names[0x11]);
			else
				printf("EF9365 Command : [0x%.2X] %s\n", cmd, commands_names[0x10]);
			#endif

			tmp_delta_x = m_registers[EF936X_REG_DELTAX];
			tmp_delta_y = m_registers[EF936X_REG_DELTAY];

			if( cmd & 0x08 )
			{
				if(tmp_delta_x > tmp_delta_y )
					tmp_delta_y = tmp_delta_x;
				if(tmp_delta_y > tmp_delta_x )
					tmp_delta_y = tmp_delta_x;
			}

			// Vector / Special direction vector generation
			switch ( cmd & 0x7 ) // Direction code
			{
				case 0x1:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() + tmp_delta_y);
				break;
				case 0x3:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() + tmp_delta_y);
				break;
				case 0x5:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() - tmp_delta_y);
				break;
				case 0x7:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() - tmp_delta_y);
				break;

				case 0x0:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() );
				break;
				case 0x2:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() + tmp_delta_y);
				break;
				case 0x4:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() - tmp_delta_y);
				break;
				case 0x6:
					busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() );
				break;
			}
			set_busy_flag( cycles_to_us( busy_cycles ) );
		}
		else
		{
			if( ( cmd>>4 ) >= 0x8 )
			{
				#ifdef DBGMODE
				printf("EF9365 Command : [0x%.2X] %s\n", cmd, commands_names[0x13]);
				#endif

				tmp_delta_x = ( cmd >> 5 ) & 3;
				tmp_delta_y = ( cmd >> 3 ) & 3;

				// Small vector.
				switch ( cmd & 0x7 ) // Direction code
				{
					case 0x1:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() + tmp_delta_y);
					break;
					case 0x3:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() + tmp_delta_y);
					break;
					case 0x5:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() - tmp_delta_y);
					break;
					case 0x7:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() - tmp_delta_y);
					break;

					case 0x0:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() + tmp_delta_x, get_y_reg() );
					break;
					case 0x2:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() + tmp_delta_y);
					break;
					case 0x4:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg(), get_y_reg() - tmp_delta_y);
					break;
					case 0x6:
						busy_cycles = draw_vector   ( get_x_reg(), get_y_reg(), get_x_reg() - tmp_delta_x, get_y_reg() );
					break;
				}

				set_busy_flag( cycles_to_us( busy_cycles ) );
			}
			else
			{
				// Draw character

				#ifdef DBGMODE
				printf("EF9365 Command : [0x%.2X] %s\n", cmd, commands_names[0x12]);
				#endif

				busy_cycles = draw_character( cmd - 0x20, 0 , 0 );
				set_busy_flag( cycles_to_us( busy_cycles ) );
			}
		}
	}
}

//-------------------------------------------------
// screen_update: Framebuffer video ouput
//-------------------------------------------------

UINT32 ef9365_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i,j,ptr,p;
	unsigned char color_index;

	for(j=0;j<bitplane_yres;j++)
	{
		for(i=0;i<bitplane_xres;i++)
		{
			color_index = 0x00;

			ptr = ( bitplane_xres * j ) + i;

			for( p = 0; p < nb_of_bitplanes; p++)
			{
				if( m_videoram->read_byte( (EF936X_BITPLANE_MAX_SIZE*p) + (ptr>>3)) & (0x80>>(ptr&7)))
				{
					color_index |= (0x01<<p);
				}
			}

			m_screen_out.pix32(j, i) = palette[ color_index ];
		}
	}

	copybitmap(bitmap, m_screen_out, 0, 0, 0, 0, cliprect);
	return 0;
}

//-------------------------------------------------
// update_scanline: Scanline callback
//-------------------------------------------------

void ef9365_device::update_scanline(UINT16 scanline)
{
	if (scanline == vsync_scanline_pos)
	{
		m_state |= (0x02); // vsync
		if( m_registers[EF936X_REG_CTRL1] & 0x20 )
		{
			m_irq_vb = 1;
		}

		update_interrupts();
	}

	if (scanline == 0)
	{
		m_state &= (~0x02);
		draw_border(0);
	}
}

//-------------------------------------------------
// data_r: Registers read access callback
//-------------------------------------------------

READ8_MEMBER( ef9365_device::data_r )
{
	unsigned char return_value;

	switch(offset & 0xF)
	{
		case EF936X_REG_STATUS:
			if (m_bf)
				m_state &= (~0x04);
			else
				m_state |= 0x04;

			if ( ( overflow_mask_x & get_x_reg() ) || ( overflow_mask_y & get_y_reg() ) )
			{
				m_state |= 0x08;
			}

			if( m_irq_vb || m_irq_lb || m_irq_rdy )
			{
				m_state |= 0x80;
			}

			if( m_irq_lb )
			{
				m_state |= 0x10;
				m_irq_lb = 0;
			}

			if( m_irq_vb )
			{
				m_state |= 0x20;
				m_irq_vb = 0;
			}

			if( m_irq_rdy )
			{
				m_state |= 0x40;
				m_irq_rdy = 0;
			}

			update_interrupts();

			return_value = m_state;
		break;
		case EF936X_REG_CTRL1:
			return_value = m_registers[EF936X_REG_CTRL1] & 0x7F;
		break;
		case EF936X_REG_CTRL2:
			return_value = m_registers[EF936X_REG_CTRL2] & 0x0F;
		break;
		case EF936X_REG_CSIZE:
			return_value = m_registers[EF936X_REG_CSIZE];
		break;
		case EF936X_REG_DELTAX:
			return_value = m_registers[EF936X_REG_DELTAX];
		break;
		case EF936X_REG_DELTAY:
			return_value = m_registers[EF936X_REG_DELTAY];
		break;
		case EF936X_REG_X_MSB:
			return_value = m_registers[EF936X_REG_X_MSB] & 0x0F;
		break;
		case EF936X_REG_X_LSB:
			return_value = m_registers[EF936X_REG_X_LSB];
		break;
		case EF936X_REG_Y_MSB:
			return_value = m_registers[EF936X_REG_Y_MSB] & 0x0F;
		break;
		case EF936X_REG_Y_LSB:
			return_value = m_registers[EF936X_REG_Y_LSB];
		break;
		case EF936X_REG_XLP:
			return_value = m_registers[EF936X_REG_XLP] & 0xFD;
		break;
		case EF936X_REG_YLP:
			return_value = m_registers[EF936X_REG_YLP];
		break;
		default:
			return_value = 0xFF;
		break;
	}

	#ifdef DBGMODE
	printf("EF9365 [ %s ] RD> [ 0x%.2X ] - %s\n", register_names[offset&0xF],return_value, machine().describe_context() );
	#endif

	return return_value;
}

//-------------------------------------------------
// data_w: Registers write access callback
//-------------------------------------------------

WRITE8_MEMBER( ef9365_device::data_w )
{
	#ifdef DBGMODE
	printf("EF9365 [ %s ] <WR [ 0x%.2X ] - %s\n", register_names[offset&0xF],data, machine().describe_context() );
	#endif

	switch(offset & 0xF)
	{
		case EF936X_REG_CMD:
			ef9365_exec( data & 0xff);
		break;
		case EF936X_REG_CTRL1:
			m_registers[EF936X_REG_CTRL1] = data & 0x7F;
		break;
		case EF936X_REG_CTRL2:
			m_registers[EF936X_REG_CTRL2] = data & 0x0F;
		break;
		case EF936X_REG_CSIZE:
			m_registers[EF936X_REG_CSIZE] = data;
		break;
		case EF936X_REG_DELTAX:
			m_registers[EF936X_REG_DELTAX] = data;
		break;
		case EF936X_REG_DELTAY:
			m_registers[EF936X_REG_DELTAY] = data;
		break;
		case EF936X_REG_X_MSB:
			m_registers[EF936X_REG_X_MSB] = data & 0x0F;
		break;
		case EF936X_REG_X_LSB:
			m_registers[EF936X_REG_X_LSB] = data;
		break;
		case EF936X_REG_Y_MSB:
			m_registers[EF936X_REG_Y_MSB] = data & 0x0F;
		break;
		case EF936X_REG_Y_LSB:
			m_registers[EF936X_REG_Y_LSB] = data;
		break;
		case EF936X_REG_XLP:
		break;
		case EF936X_REG_YLP:
		break;
		default:
		break;
	}
}
