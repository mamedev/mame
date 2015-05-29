// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    i8244.c

    Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip


***************************************************************************/

#include "emu.h"
#include "i8244.h"


// device type definition
const device_type I8244 = &device_creator<i8244_device>;
const device_type I8245 = &device_creator<i8245_device>;


// Kevtris verified that the data below matches a dump
// taken from a real chip.
static const UINT8 c_shape[0x40 * 8] =
{
	0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00, // 0
	0x18,0x38,0x18,0x18,0x18,0x18,0x3C,0x00,
	0x3C,0x66,0x0C,0x18,0x30,0x60,0x7E,0x00,
	0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00,
	0xCC,0xCC,0xCC,0xFE,0x0C,0x0C,0x0C,0x00,
	0xFE,0xC0,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
	0x7C,0xC6,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,
	0xFE,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,
	0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00,
	0x7C,0xC6,0xC6,0x7E,0x06,0xC6,0x7C,0x00,
	0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00,
	0x18,0x7E,0x58,0x7E,0x1A,0x7E,0x18,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00,
	0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00,
	0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00,
	0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,
	0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00,
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00,
	0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00,
	0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00,
	0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,
	0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0x7C,0xC6,0xC6,0xC6,0xDE,0xCC,0x76,0x00,
	0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
	0xFC,0xC6,0xC6,0xC6,0xC6,0xC6,0xFC,0x00,
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00,
	0x7C,0xC6,0xC0,0xC0,0xCE,0xC6,0x7E,0x00,
	0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,
	0x06,0x06,0x06,0x06,0x06,0xC6,0x7C,0x00,
	0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00,
	0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,
	0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,
	0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00,
	0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,
	0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00,
	0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00,
	0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00,
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,
	0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,
	0x00,0x66,0x3C,0x18,0x3C,0x66,0x00,0x00,
	0x00,0x18,0x00,0x7E,0x00,0x18,0x00,0x00,
	0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00,
	0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00,
	0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0x00,
	0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	0xCE,0xDB,0xDB,0xDB,0xDB,0xDB,0xCE,0x00,
	0x00,0x00,0x3C,0x7E,0x7E,0x7E,0x3C,0x00,
	0x1C,0x1C,0x18,0x1E,0x18,0x18,0x1C,0x00,
	0x1C,0x1C,0x18,0x1E,0x18,0x34,0x26,0x00,
	0x38,0x38,0x18,0x78,0x18,0x2C,0x64,0x00,
	0x38,0x38,0x18,0x78,0x18,0x18,0x38,0x00,
	0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00,
	0x18,0x3C,0x7E,0xFF,0xFF,0x18,0x18,0x00,
	0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0x00,
	0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,0x00,
	0x38,0x38,0x12,0xFE,0xB8,0x28,0x6C,0x00,
	0xC0,0x60,0x30,0x18,0x0C,0x06,0x03,0x00,
	0x00,0x00,0x0C,0x08,0x08,0x7F,0x3E,0x00,
	0x00,0x03,0x63,0xFF,0xFF,0x18,0x08,0x00,
	0x00,0x00,0x00,0x10,0x38,0xFF,0x7E,0x00
};


// Background and grid information is stored in RGB format
// while the character and sprite colors are stored in BGR
// format.
static const UINT8 bgr2rgb[8] =
{
	0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07
};


//-------------------------------------------------
//  i8244_device - constructor
//-------------------------------------------------

i8244_device::i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8244, "I8244", tag, owner, clock, "i8244", __FILE__)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_irq_func(*this)
	, m_postprocess_func(*this)
	, m_start_vpos(START_Y)
	, m_start_vblank(START_Y + SCREEN_HEIGHT)
	, m_screen_lines(LINES)
{
}


i8244_device::i8244_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int lines, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_irq_func(*this)
	, m_postprocess_func(*this)
	, m_start_vpos(START_Y)
	, m_start_vblank(START_Y + SCREEN_HEIGHT)
	, m_screen_lines(lines)
{
}


i8245_device::i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8244_device(mconfig, I8245, "I8245", tag, owner, clock, i8245_device::LINES, "i8245", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8244_device::device_start()
{
	// Let the screen create our temporary bitmap with the screen's dimensions
	m_screen->register_screen_bitmap(m_tmp_bitmap);

	m_line_timer = timer_alloc(TIMER_LINE);
	m_line_timer->adjust( m_screen->time_until_pos(1, START_ACTIVE_SCAN ), 0,  m_screen->scan_period() );

	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_hblank_timer->adjust( m_screen->time_until_pos(1, END_ACTIVE_SCAN + 18 ), 0, m_screen->scan_period() );

	m_irq_func.resolve_safe();
	m_postprocess_func.resolve_safe();

	// allocate a stream
	m_stream = stream_alloc( 0, 1, clock()/(LINE_CLOCKS*4) );

	// register our state
	save_pointer(NAME(m_vdc.reg), 0x100);
	save_item(NAME(m_sh_count));
	save_item(NAME(m_x_beam_pos));
	save_item(NAME(m_y_beam_pos));
	save_item(NAME(m_control_status));
	save_item(NAME(m_collision_status));
	save_item(NAME(m_iff));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8244_device::device_reset()
{
	memset(m_vdc.reg, 0, 0x100);

	m_sh_count = 0;
	m_x_beam_pos = 0;
	m_y_beam_pos = 0;
	m_control_status = 0;
	m_collision_status = 0;
	m_iff = 0;
}


PALETTE_INIT_MEMBER(i8244_device, i8244)
{
	static const UINT8 i8244_colors[3*16] =
	{
		0x00, 0x00, 0x00, // i r g b
		0x00, 0x00, 0xAA, // i r g B
		0x00, 0xAA, 0x00, // i r G b
		0x00, 0xAA, 0xAA, // i r G B
		0xAA, 0x00, 0x00, // i R g b
		0xAA, 0x00, 0xAA, // i R g B
		0xAA, 0xAA, 0x00, // i R G b
		0xAA, 0xAA, 0xAA, // i R G B
		0x55, 0x55, 0x55, // I r g b
		0x55, 0x55, 0xFF, // I r g B
		0x55, 0xFF, 0x55, // I r G b
		0x55, 0xFF, 0xFF, // I r G B
		0xFF, 0x55, 0x55, // I R g b
		0xFF, 0x55, 0xFF, // I R g B
		0xFF, 0xFF, 0x55, // I R G b
		0xFF, 0xFF, 0xFF, // I R G B
	};

	for ( int i = 0; i < 16; i++ )
	{
		palette.set_pen_color( i, i8244_colors[i*3], i8244_colors[i*3+1], i8244_colors[i*3+2] );
	}
}


void i8244_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int vpos = m_screen->vpos();

	switch ( id )
	{
		case TIMER_LINE:
			// handle i824x line timer
			render_scanline(vpos);
			break;

		case TIMER_HBLANK:
			// handle i824x HBlank timer
			if ( vpos < m_start_vpos - 1 )
			{
				return;
			}

			if ( vpos < m_start_vblank - 1 )
			{
				m_control_status |= 0x01;
			}
			break;
	}
}


int i8244_device::get_y_beam()
{
	int y = m_screen->vpos() - m_start_vpos;

	// The Y register becomes 0 only when the VBlank signal is turned off!
	if ( y < 0 || ( y == 0 && m_screen->hpos() < 366+42 ) )
	{
		y += m_screen_lines;
	}

	return y;
}


int i8244_device::get_x_beam()
{
	int x = m_screen->hpos() - START_ACTIVE_SCAN;

	if ( x < 0 )
	{
		x += LINE_CLOCKS;
	}

	return x >> 1;
}


offs_t i8244_device::fix_register_mirrors( offs_t offset )
{
	// registers $40,$41 are mirrored at $44,$45, $48,$49, and $4C,$4D
	if ( ( offset & 0xF2 ) == 0x40 )
	{
		offset &= ~0x0C;
	}

	// registers $A0-$AF are mirrored at $B0-$BF
	if ( ( offset & 0xF0 ) == 0xB0 )
	{
		offset &= ~0x10;
	}

	return offset;
}


READ8_MEMBER(i8244_device::read)
{
	UINT8 data = 0;

	offset = fix_register_mirrors( offset );

	switch (offset)
	{
		case 0xa1:
			data = m_control_status & 0xFE;
			m_iff = 0;
			m_irq_func(CLEAR_LINE);
			m_control_status &= ~0x08;
			if ( hblank() )
			{
				data |= 1;
			}
			break;

		case 0xa2:
			data = m_collision_status;
			m_collision_status = 0;
			break;

		case 0xa4:
			if (m_vdc.s.control & VDC_CONTROL_REG_STROBE_XY)
			{
				m_y_beam_pos = get_y_beam();
			}
			data = m_y_beam_pos;
			break;


		case 0xa5:
			if ((m_vdc.s.control & VDC_CONTROL_REG_STROBE_XY))
			{
				m_x_beam_pos = get_x_beam();
			}
			data = m_x_beam_pos;
			break;

		default:
			data = m_vdc.reg[offset];
			break;
	}

	return data;
}


WRITE8_MEMBER(i8244_device::write)
{
	offset = fix_register_mirrors( offset );

	/* Update the sound */
	if( offset >= 0xa7 && offset <= 0xaa )
	{
		m_stream->update();
	}

	if (offset == 0xa0)
	{
		if ( ( m_vdc.s.control & VDC_CONTROL_REG_STROBE_XY )
			&& !(data & VDC_CONTROL_REG_STROBE_XY))
		{
			/* Toggling strobe bit, tuck away values */
			m_x_beam_pos = get_x_beam();
			m_y_beam_pos = get_y_beam();
		}
	}

	m_vdc.reg[offset] = data;
}


READ_LINE_MEMBER(i8244_device::vblank)
{
	if ( m_screen->vpos() > m_start_vpos && m_screen->vpos() < m_start_vblank )
	{
		return 0;
	}
	return 1;
}


READ_LINE_MEMBER(i8244_device::hblank)
{
	int hpos = m_screen->hpos();
	int vpos = m_screen->vpos();

	if ( hpos >= START_ACTIVE_SCAN && hpos < END_ACTIVE_SCAN )
	{
		return 0;
	}

	// Before active area?
	if ( vpos < m_start_vpos - 1 )
	{
		return 0;
	}

	// During active area?
	if ( vpos < m_start_vblank - 1 )
	{
		return 1;
	}

	// After active area
	return 0;
}


void i8244_device::render_scanline(int vpos)
{
	// Some local constants for this method
	//static const UINT8 COLLISION_SPRITE_0        = 0x01;
	//static const UINT8 COLLISION_SPRITE_1        = 0x02;
	//static const UINT8 COLLISION_SPRITE_2        = 0x04;
	//static const UINT8 COLLISION_SPRITE_3        = 0x08;
	static const UINT8 COLLISION_VERTICAL_GRID   = 0x10;
	static const UINT8 COLLISION_HORIZ_GRID_DOTS = 0x20;
	//static const UINT8 COLLISION_EXTERNAL_UNUSED = 0x40;
	static const UINT8 COLLISION_CHARACTERS      = 0x80;

	UINT8   collision_map[160];

	if ( vpos == m_start_vpos )
	{
		m_control_status &= ~0x08;
	}

	if ( m_start_vpos < vpos && vpos < m_start_vblank )
	{
		rectangle rect;
		int scanline = vpos - m_start_vpos;

		m_control_status &= ~ 0x01;

		/* Draw a line */
		rect.set(START_ACTIVE_SCAN, END_ACTIVE_SCAN - 1, vpos, vpos);
		m_tmp_bitmap.fill( (m_vdc.s.color >> 3) & 0x7, rect );

		/* Clear collision map */
		memset( collision_map, 0, sizeof( collision_map ) );

		/* Display grid if enabled */
		if ( m_vdc.s.control & 0x08 )
		{
			UINT16 color = ( m_vdc.s.color & 7 ) | ( ( m_vdc.s.color >> 3 ) & 0x08 );
			int    x_grid_offset = 8;
			int    y_grid_offset = 24;
			int    width = 16;
			int    height = 24;
			int    w = ( m_vdc.s.control & 0x80 ) ? width : 2;

			/* Draw horizontal part of grid */
			for ( int j = 1, y = 0; y < 9; y++, j <<= 1 )
			{
				if ( y_grid_offset + y * height <= scanline && scanline < y_grid_offset + y * height + 3 )
				{
					for ( int i = 0; i < 9; i++ )
					{
						if ( ( m_vdc.s.hgrid[0][i] & j ) || ( m_vdc.s.hgrid[1][i] & ( j >> 8 ) ) )
						{
							for ( int k = 0; k < width + 2; k++ )
							{
								int px = x_grid_offset + i * width + k;

								if ( px < 160 )
								{
									collision_map[ px ] |= COLLISION_HORIZ_GRID_DOTS;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * px ) = color;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * px + 1 ) = color;
								}
							}
						}
					}
				}
			}

			/* Draw vertical part of grid */
			for( int j = 1, y = 0; y < 8; y++, j <<= 1 )
			{
				if ( y_grid_offset + y * height <= scanline && scanline < y_grid_offset + ( y + 1 ) * height )
				{
					for ( int i = 0; i < 10; i++ )
					{
						if ( m_vdc.s.vgrid[i] & j )
						{
							for ( int k = 0; k < w; k++ )
							{
								int px = x_grid_offset + i * width + k;

								if ( px < 160 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ px ] & m_vdc.s.collision )
									{
										m_collision_status |= COLLISION_VERTICAL_GRID;
									}
									/* Check if an already drawn object would collide with us */
									if ( COLLISION_VERTICAL_GRID & m_vdc.s.collision && collision_map[ px ] )
									{
										m_collision_status |= collision_map[ px ];
									}
									collision_map[ px ] |= COLLISION_VERTICAL_GRID;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * px ) = color;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * px + 1 ) = color;
								}
							}
						}
					}
				}
			}
		}

		/* Display objects if enabled */
		if ( m_vdc.s.control & 0x20 )
		{
			/* Regular foreground objects */
			for ( int i = 0; i < ARRAY_LENGTH( m_vdc.s.foreground ); i++ )
			{
				int y = m_vdc.s.foreground[i].y & 0xFE;
				int height = 8 - ( ( ( y >> 1 ) + m_vdc.s.foreground[i].ptr ) & 7 );

				if ( y >= 0x0E && y <= scanline && scanline < y + height * 2 )
				{
					UINT16 color = 8 + bgr2rgb[ ( ( m_vdc.s.foreground[i].color >> 1 ) & 0x07 ) ];
					int    offset = ( m_vdc.s.foreground[i].ptr | ( ( m_vdc.s.foreground[i].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( scanline - y ) >> 1 );
					UINT8  chr = c_shape[ offset & 0x1FF ];
					int    x = m_vdc.s.foreground[i].x;

					for ( UINT8 m = 0x80; m > 0; m >>= 1, x++ )
					{
						if ( chr & m )
						{
							if ( x >= 0 && x < 160 )
							{
								/* Check if we collide with an already drawn source object */
								if ( collision_map[ x ] & m_vdc.s.collision )
								{
									m_collision_status |= COLLISION_CHARACTERS;
								}
								/* Check if an already drawn object would collide with us */
								if ( COLLISION_CHARACTERS & m_vdc.s.collision && collision_map[ x ] )
								{
									m_collision_status |= collision_map[ x ];
								}
								collision_map[ x ] |= COLLISION_CHARACTERS;
								m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * x ) = color;
								m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * x + 1 ) = color;
							}
						}
					}
				}
			}

			/* Quad objects */
			for ( int i = 0; i < ARRAY_LENGTH( m_vdc.s.quad ); i++ )
			{
				int y = m_vdc.s.quad[i].single[0].y;
				int height = 8;

				if ( y <= scanline && scanline < y + height * 2 )
				{
					int x = m_vdc.s.quad[i].single[0].x;

					// Charaecter height is always determined by the height of the 4th character
					int char_height = 8 - ( ( ( y >> 1 ) + m_vdc.s.quad[i].single[3].ptr ) & 7 );

					for ( int j = 0; j < ARRAY_LENGTH( m_vdc.s.quad[0].single ); j++, x += 8 )
					{
						if ( y <= scanline && scanline < y + char_height * 2 )
						{
							UINT16 color = 8 + bgr2rgb[ ( ( m_vdc.s.quad[i].single[j].color >> 1 ) & 0x07 ) ];
							int offset = ( m_vdc.s.quad[i].single[j].ptr | ( ( m_vdc.s.quad[i].single[j].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( scanline - y ) >> 1 );
							UINT8 chr = c_shape[ offset & 0x1FF ];

							for ( UINT8 m = 0x80; m > 0; m >>= 1, x++ )
							{
								if ( chr & m )
								{
									if ( x >= 0 && x < 160 )
									{
										/* Check if we collide with an already drawn source object */
										if ( collision_map[ x ] & m_vdc.s.collision )
										{
											m_collision_status |= COLLISION_CHARACTERS;
										}
										/* Check if an already drawn object would collide with us */
										if ( COLLISION_CHARACTERS & m_vdc.s.collision && collision_map[ x ] )
										{
											m_collision_status |= collision_map[ x ];
										}
										collision_map[ x ] |= COLLISION_CHARACTERS;
										m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * x ) = color;
										m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + 2 * x + 1 ) = color;
									}
								}
							}
						}
						else
						{
							x += 8;
						}
					}
				}
			}

			/* Sprites */
			for ( int i = 0; i < ARRAY_LENGTH( m_vdc.s.sprites ); i++ )
			{
				int y = m_vdc.s.sprites[i].y;
				int height = 8;

				if ( m_vdc.s.sprites[i].color & 4 )
				{
					/* Zoomed sprite */
					if ( y <= scanline && scanline < y + height * 4 )
					{
						UINT16 color = 8 + bgr2rgb[ ( ( m_vdc.s.sprites[i].color >> 3 ) & 0x07 ) ];
						UINT8  chr = m_vdc.s.shape[i][ ( ( scanline - y ) >> 2 ) ];
						int    x = m_vdc.s.sprites[i].x;
						int    x_shift = 0;

						switch ( m_vdc.s.sprites[i].color & 0x03 )
						{
							case 1:    // Xg attribute set
								x_shift = 2;
								break;
							case 2:    // S attribute set
								x_shift = ( ( ( scanline - y ) >> 1 ) & 0x01 ) ^ 0x01;
								break;
							case 3:    // Xg and S attributes set
								x_shift = ( ( scanline - y ) >> 1 ) & 0x01;
								break;
						}
						x_shift <<= 1;

						for ( UINT8 m = 0x01; m > 0; m <<= 1, x += 2 )
						{
							if ( chr & m )
							{
								if ( x >= 0 && x < 160 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ x ] & m_vdc.s.collision )
									{
										m_collision_status |= ( 1 << i );
									}
									/* Check if an already drawn object would collide with us */
									if ( ( 1 << i ) & m_vdc.s.collision && collision_map[ x ] )
									{
										m_collision_status |= collision_map[ x ];
									}
									collision_map[ x ] |= ( 1 << i );
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x ) = color;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x + 1 ) = color;
								}
								if ( x >= -1 && x < 159 )
								{
									if ( x >= 0 )
									{
										/* Check if we collide with an already drawn source object */
										if ( collision_map[ x ] & m_vdc.s.collision )
										{
											m_collision_status |= ( 1 << i );
										}
										/* Check if an already drawn object would collide with us */
										if ( ( 1 << i ) & m_vdc.s.collision && collision_map[ x ] )
										{
											m_collision_status |= collision_map[ x ];
										}
										collision_map[ x ] |= ( 1 << i );
									}
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x + 2 ) = color;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x + 3 ) = color;
								}
							}
						}
					}
				}
				else
				{
					/* Regular sprite */
					if ( y <= scanline && scanline < y + height * 2 )
					{
						UINT16 color = 8 + bgr2rgb[ ( ( m_vdc.s.sprites[i].color >> 3 ) & 0x07 ) ];
						UINT8  chr = m_vdc.s.shape[i][ ( ( scanline - y ) >> 1 ) ];
						int    x = m_vdc.s.sprites[i].x;
						int    x_shift = 0;

						switch ( m_vdc.s.sprites[i].color & 0x03 )
						{
							case 1:    // Xg attribute set
								x_shift = 1;
								break;
							case 2:    // S attribute set
								x_shift = ( ( ( scanline - y ) >> 1 ) & 0x01 ) ^ 0x01;
								break;
							case 3:    // Xg and S attributes set
								x_shift = ( ( scanline - y ) >> 1 ) & 0x01;
								break;
						}

						for ( UINT8 m = 0x01; m > 0; m <<= 1, x++ )
						{
							if ( chr & m )
							{
								if ( x >= 0 && x < 160 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ x ] & m_vdc.s.collision )
									{
										m_collision_status |= ( 1 << i );
									}
									/* Check if an already drawn object would collide with us */
									if ( ( 1 << i ) & m_vdc.s.collision && collision_map[ x ] )
									{
										m_collision_status |= collision_map[ x ];
									}
									collision_map[ x ] |= ( 1 << i );
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x ) = color;
									m_tmp_bitmap.pix16( vpos, START_ACTIVE_SCAN + 10 + x_shift + 2 * x + 1 ) = color;
								}
							}
						}
					}
				}
			}
		}
	}

	// Allow driver to do additional processing
	m_postprocess_func( vpos );

	/* Check for start of VBlank */
	if ( vpos == m_start_vblank )
	{
		m_control_status |= 0x08;
		if ( ! m_iff )
		{
			m_iff = 1;
			m_irq_func(ASSERT_LINE);
		}
	}
}


void i8244_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT32 old_signal, signal;
	int ii;
	int period;
	stream_sample_t *buffer = outputs[0];

	/* Generate the signal */
	old_signal = signal = m_vdc.s.shift3 | (m_vdc.s.shift2 << 8) | (m_vdc.s.shift1 << 16);

	if( m_vdc.s.sound & 0x80 )  /* Sound is enabled */
	{
		for( ii = 0; ii < samples; ii++, buffer++ )
		{
			*buffer = 0;
			*buffer = signal & 0x1;
			period = (m_vdc.s.sound & 0x20) ? 1 : 4;
			if( ++m_sh_count >= period )
			{
				m_sh_count = 0;
				signal >>= 1;
				/* Loop sound */
				signal |= *buffer << 23;
				/* Check if noise should be applied */
				if ( m_vdc.s.sound & 0x10 )
				{
					/* Noise tap is on bits 0 and 5 and fed back to bits 15 (and 23!) */
					UINT32 new_bit = ( ( old_signal ) ^ ( old_signal >> 5 ) ) & 0x01;
					signal = ( old_signal & 0xFF0000 ) | ( ( old_signal & 0xFFFF ) >> 1 ) | ( new_bit << 15 ) | ( new_bit << 23 );
				}
				m_vdc.s.shift3 = signal & 0xFF;
				m_vdc.s.shift2 = ( signal >> 8 ) & 0xFF;
				m_vdc.s.shift1 = ( signal >> 16 ) & 0xFF;
				old_signal = signal;
			}

			/* Throw an interrupt if enabled */
			if( m_vdc.s.control & 0x4 )
			{
				// This feature does not seem to be finished/enabled in hardware!
			}

			/* Adjust volume */
			*buffer *= m_vdc.s.sound & 0xf;
			/* Pump the volume up */
			*buffer <<= 10;
		}
	}
	else
	{
		/* Sound disabled, so clear the buffer */
		for( ii = 0; ii < samples; ii++, buffer++ )
		{
			*buffer = 0;
		}
	}
}


UINT32 i8244_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, m_tmp_bitmap, 0, 0, 0, 0, cliprect );

	return 0;
}
