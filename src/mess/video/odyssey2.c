/***************************************************************************

  video/odyssey2.c

  2012-02-04 DanBoris
    - Changed color of background grid color 0 to match sprite color 0 (Fixes KTAA title screen)
    - Fixed Odyssey2_video_w so that m_o2_vdc.reg[] is always updated (Fixes Blockout)
    - Changed quad character generation so character height is always taken from 4th character (KTAA level 2)


***************************************************************************/

#include "emu.h"
#include "includes/odyssey2.h"


#define COLLISION_SPRITE_0			0x01
#define COLLISION_SPRITE_1			0x02
#define COLLISION_SPRITE_2			0x04
#define COLLISION_SPRITE_3			0x08
#define COLLISION_VERTICAL_GRID		0x10
#define COLLISION_HORIZ_GRID_DOTS	0x20
#define COLLISION_EXTERNAL_UNUSED	0x40
#define COLLISION_CHARACTERS		0x80

/* character sprite colors
   dark grey, red, green, orange, blue, violet, light grey, white
   dark back / grid colors
   black, dark blue, dark green, light green, red, violet, orange, light grey
   light back / grid colors
   black, blue, green, light green, red, violet, orange, light grey */

const UINT8 odyssey2_colors[] =
{
	/* Background,Grid Dim */
	0x00,0x00,0x00,
	0x00,0x00,0xFF,   /* Blue */
	0x00,0x80,0x00,   /* DK Green */
	0xff,0x9b,0x60,
	0xCC,0x00,0x00,   /* Red */
	0xa9,0x80,0xff,
	0x82,0xfd,0xdb,
	0xFF,0xFF,0xFF,

	/* Background,Grid Bright */
	0x80,0x80,0x80,
	0x50,0xAE,0xFF,   /* Blue */
	0x00,0xFF,0x00,   /* Dk Green */
	0x82,0xfb,0xdb,   /* Lt Grey */
	0xEC,0x02,0x60,   /* Red */
	0xa9,0x80,0xff,   /* Violet */
	0xff,0x9b,0x60,   /* Orange */
	0xFF,0xFF,0xFF,

	/* Character,Sprite colors */
	0x80,0x80,0x80,   /* Dark Grey */
	0xFF,0x80,0x80,   /* Red */
	0x00,0xC0,0x00,   /* Green */
	0xff,0x9b,0x60,   /* Orange */
	0x50,0xAE,0xFF,   /* Blue */
	0xa9,0x80,0xff,   /* Violet */
	0x82,0xfb,0xdb,   /* Lt Grey */
	0xff,0xff,0xff    /* White */
};

static const UINT8 o2_shape[0x40][8]={
    { 0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00 }, // 0
    { 0x18,0x38,0x18,0x18,0x18,0x18,0x3C,0x00 },
    { 0x3C,0x66,0x0C,0x18,0x30,0x60,0x7E,0x00 },
    { 0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00 },
    { 0xCC,0xCC,0xCC,0xFE,0x0C,0x0C,0x0C,0x00 },
    { 0xFE,0xC0,0xC0,0x7C,0x06,0xC6,0x7C,0x00 },
    { 0x7C,0xC6,0xC0,0xFC,0xC6,0xC6,0x7C,0x00 },
    { 0xFE,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00 },
    { 0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00 },
    { 0x7C,0xC6,0xC6,0x7E,0x06,0xC6,0x7C,0x00 },
    { 0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00 },
    { 0x18,0x7E,0x58,0x7E,0x1A,0x7E,0x18,0x00 },
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    { 0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00 },
    { 0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00 },
    { 0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00 },
    { 0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00 },
    { 0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00 },
    { 0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00 },
    { 0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00 },
    { 0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00 },
    { 0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00 },
    { 0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00 },
    { 0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00 },
    { 0x7C,0xC6,0xC6,0xC6,0xDE,0xCC,0x76,0x00 },
    { 0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00 },
    { 0xFC,0xC6,0xC6,0xC6,0xC6,0xC6,0xFC,0x00 },
    { 0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00 },
    { 0x7C,0xC6,0xC0,0xC0,0xCE,0xC6,0x7E,0x00 },
    { 0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00 },
    { 0x06,0x06,0x06,0x06,0x06,0xC6,0x7C,0x00 },
    { 0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00 },
    { 0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00 },
    { 0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00 },
    { 0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00 },
    { 0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00 },
    { 0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00 },
    { 0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00 },
    { 0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00 },
    { 0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00 },
    { 0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00 },
    { 0x00,0x66,0x3C,0x18,0x3C,0x66,0x00,0x00 },
    { 0x00,0x18,0x00,0x7E,0x00,0x18,0x00,0x00 },
    { 0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00 },
    { 0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00 },
    { 0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0x00 },
    { 0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00 },
    { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00 },
    { 0xCE,0xDB,0xDB,0xDB,0xDB,0xDB,0xCE,0x00 },
    { 0x00,0x00,0x3C,0x7E,0x7E,0x7E,0x3C,0x00 },
    { 0x1C,0x1C,0x18,0x1E,0x18,0x18,0x1C,0x00 },
    { 0x1C,0x1C,0x18,0x1E,0x18,0x34,0x26,0x00 },
    { 0x38,0x38,0x18,0x78,0x18,0x2C,0x64,0x00 },
    { 0x38,0x38,0x18,0x78,0x18,0x18,0x38,0x00 },
    { 0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00 },
    { 0x18,0x3C,0x7E,0xFF,0xFF,0x18,0x18,0x00 },
    { 0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0x00 },
    { 0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,0x00 },
    { 0x38,0x38,0x12,0xFE,0xB8,0x28,0x6C,0x00 },
    { 0xC0,0x60,0x30,0x18,0x0C,0x06,0x03,0x00 },
    { 0x00,0x00,0x0C,0x08,0x08,0x7F,0x3E,0x00 },
    { 0x00,0x03,0x63,0xFF,0xFF,0x18,0x08,0x00 },
    { 0x00,0x00,0x00,0x10,0x38,0xFF,0x7E,0x00 }
};




void odyssey2_state::palette_init()
{
	int i;

	for ( i = 0; i < 24; i++ )
	{
		palette_set_color_rgb( machine(), i, odyssey2_colors[i*3], odyssey2_colors[i*3+1], odyssey2_colors[i*3+2] );
	}
}

READ8_MEMBER(odyssey2_state::odyssey2_video_r)
{
    UINT8 data = 0;

    switch (offset)
    {
        case 0xa1:
			data = m_control_status;
			m_iff = 0;
			machine().device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
			m_control_status &= ~ 0x08;
			if ( machine().primary_screen->hpos() < I824X_START_ACTIVE_SCAN || machine().primary_screen->hpos() > I824X_END_ACTIVE_SCAN )
			{
				data |= 1;
			}

            break;

        case 0xa2:
			data = m_collision_status;
			m_collision_status = 0;

            break;

        case 0xa4:

            if ((m_o2_vdc.s.control & VDC_CONTROL_REG_STROBE_XY))
                m_y_beam_pos = machine().primary_screen->vpos() - m_start_vpos;

            data = m_y_beam_pos;

            break;


        case 0xa5:

            if ((m_o2_vdc.s.control & VDC_CONTROL_REG_STROBE_XY))
			{
                m_x_beam_pos = machine().primary_screen->hpos();
				if ( m_x_beam_pos < I824X_START_ACTIVE_SCAN )
				{
					m_x_beam_pos = m_x_beam_pos - I824X_START_ACTIVE_SCAN + I824X_LINE_CLOCKS;
				}
				else
				{
					m_x_beam_pos = m_x_beam_pos - I824X_START_ACTIVE_SCAN;
				}
			}

            data = m_x_beam_pos;

            break;

        default:
            data = m_o2_vdc.reg[offset];
    }

    return data;
}

WRITE8_MEMBER(odyssey2_state::odyssey2_video_w)
{
	/* Update the sound */
	if( offset >= 0xa7 && offset <= 0xaa )
		m_sh_channel->update();

    if (offset == 0xa0) {


        if (    m_o2_vdc.s.control & VDC_CONTROL_REG_STROBE_XY
             && !(data & VDC_CONTROL_REG_STROBE_XY))
        {
            /* Toggling strobe bit, tuck away values */
            m_x_beam_pos = machine().primary_screen->hpos();
			if ( m_x_beam_pos < I824X_START_ACTIVE_SCAN )
			{
				m_x_beam_pos = m_x_beam_pos - I824X_START_ACTIVE_SCAN + 228;
			}
			else
			{
				m_x_beam_pos = m_x_beam_pos - I824X_START_ACTIVE_SCAN;
			}

            m_y_beam_pos = machine().primary_screen->vpos() - m_start_vpos;
        }
    }

    m_o2_vdc.reg[offset] = data;
}

WRITE8_MEMBER(odyssey2_state::odyssey2_lum_w)
{
	m_lum = data;
}

READ8_MEMBER(odyssey2_state::odyssey2_t1_r)
{
	if ( machine().primary_screen->vpos() > m_start_vpos && machine().primary_screen->vpos() < m_start_vblank )
	{
		if ( machine().primary_screen->hpos() >= I824X_START_ACTIVE_SCAN && machine().primary_screen->hpos() < I824X_END_ACTIVE_SCAN )
		{
			return 1;
		}
	}
	return 0;
}

static TIMER_CALLBACK( i824x_scanline_callback )
{
	odyssey2_state *state = machine.driver_data<odyssey2_state>();
	UINT8	collision_map[160];
	int		vpos = machine.primary_screen->vpos();

	if ( vpos < state->m_start_vpos )
		return;

	if ( vpos == state->m_start_vpos )
	{
		state->m_control_status &= ~0x08;
	}

	if ( vpos < state->m_start_vblank )
	{
		rectangle rect;
		//static const int sprite_width[4] = { 8, 8, 8, 8 };
		int i;

		state->m_control_status &= ~ 0x01;

		/* Draw a line */
		rect.set(I824X_START_ACTIVE_SCAN, I824X_END_ACTIVE_SCAN - 1, vpos, vpos);
		state->m_tmp_bitmap.fill(( (state->m_o2_vdc.s.color >> 3) & 0x7 ) | ( ( state->m_lum << 3 ) ^ 0x08 ), rect );

		/* Clear collision map */
		memset( collision_map, 0, sizeof( collision_map ) );

		/* Display grid if enabled */
		if ( state->m_o2_vdc.s.control & 0x08 )
		{
			UINT16	color = ( state->m_o2_vdc.s.color & 7 ) | ( ( state->m_o2_vdc.s.color >> 3 ) & 0x08 ) | ( ( state->m_lum << 3 ) ^ 0x08 );
			int		x_grid_offset = 8;
			int 	y_grid_offset = 24;
			int		width = 16;
			int		height = 24;
			int		w = ( state->m_o2_vdc.s.control & 0x80 ) ? width : 2;
			int		j, k, y;

			/* Draw horizontal part of grid */
			for ( j = 1, y = 0; y < 9; y++, j <<= 1 )
			{
				if ( y_grid_offset + y * height <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y_grid_offset + y * height + 3 )
				{
					for ( i = 0; i < 9; i++ )
					{
						if ( ( state->m_o2_vdc.s.hgrid[0][i] & j ) || ( state->m_o2_vdc.s.hgrid[1][i] & ( j >> 8 ) ) )
						{
							for ( k = 0; k < width + 2; k++ )
							{
								int px = x_grid_offset + i * width + k;
								collision_map[ px ] |= COLLISION_HORIZ_GRID_DOTS;
								state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + px ) = color;
							}
						}
					}
				}
			}

			/* Draw vertical part of grid */
			for( j = 1, y = 0; y < 8; y++, j <<= 1 )
			{
				if ( y_grid_offset + y * height <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y_grid_offset + ( y + 1 ) * height )
				{
					for ( i = 0; i < 10; i++ )
					{
						if ( state->m_o2_vdc.s.vgrid[i] & j )
						{
							for ( k = 0; k < w; k++ )
							{
								int px = x_grid_offset + i * width + k;

								/* Check if we collide with an already drawn source object */
								if ( collision_map[ px ] & state->m_o2_vdc.s.collision )
								{
									state->m_collision_status |= COLLISION_VERTICAL_GRID;
								}
								/* Check if an already drawn object would collide with us */
								if ( COLLISION_VERTICAL_GRID & state->m_o2_vdc.s.collision && collision_map[ px ] )
								{
									state->m_collision_status |= collision_map[ px ];
								}
								collision_map[ px ] |= COLLISION_VERTICAL_GRID;
								state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + px ) = color;
							}
						}
					}
				}
			}
		}

		/* Display objects if enabled */
		if ( state->m_o2_vdc.s.control & 0x20 )
		{
			/* Regular foreground objects */
			for ( i = 0; i < ARRAY_LENGTH( state->m_o2_vdc.s.foreground ); i++ )
			{
				int	y = state->m_o2_vdc.s.foreground[i].y;
				int	height = 8 - ( ( ( y >> 1 ) + state->m_o2_vdc.s.foreground[i].ptr ) & 7 );

				if ( y <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y + height * 2 )
				{
					UINT16	color = 16 + ( ( state->m_o2_vdc.s.foreground[i].color & 0x0E ) >> 1 );
					int		offset = ( state->m_o2_vdc.s.foreground[i].ptr | ( ( state->m_o2_vdc.s.foreground[i].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( vpos - state->m_start_vpos - y ) >> 1 );
					UINT8	chr = ((char*)o2_shape)[ offset & 0x1FF ];
					int		x = state->m_o2_vdc.s.foreground[i].x;
					UINT8	m;

					for ( m = 0x80; m > 0; m >>= 1, x++ )
					{
						if ( chr & m )
						{
							if ( x >= 0 && x < 160 )
							{
								/* Check if we collide with an already drawn source object */
								if ( collision_map[ x ] & state->m_o2_vdc.s.collision )
								{
									state->m_collision_status |= COLLISION_CHARACTERS;
								}
								/* Check if an already drawn object would collide with us */
								if ( COLLISION_CHARACTERS & state->m_o2_vdc.s.collision && collision_map[ x ] )
								{
									state->m_collision_status |= collision_map[ x ];
								}
								collision_map[ x ] |= COLLISION_CHARACTERS;
								state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + x ) = color;
							}
						}
					}
				}
			}

			/* Quad objects */
			for ( i = 0; i < ARRAY_LENGTH( state->m_o2_vdc.s.quad ); i++ )
			{
				int y = state->m_o2_vdc.s.quad[i].single[0].y;
				int height = 8;

				if ( y <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y + height * 2 )
				{
					int	x = state->m_o2_vdc.s.quad[i].single[0].x;
					int j;

					// Charaecter height is always determined by the height of the 4th character
					int char_height = 8 - ( ( ( y >> 1 ) + state->m_o2_vdc.s.quad[i].single[3].ptr ) & 7 );

					for ( j = 0; j < ARRAY_LENGTH( state->m_o2_vdc.s.quad[0].single ); j++, x += 8 )
					{


						if ( y <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y + char_height * 2 )
						{

						UINT16 color = 16 + ( ( state->m_o2_vdc.s.quad[i].single[j].color & 0x0E ) >> 1 );


							int	offset = ( state->m_o2_vdc.s.quad[i].single[j].ptr | ( ( state->m_o2_vdc.s.quad[i].single[j].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( vpos - state->m_start_vpos - y ) >> 1 );

							UINT8	chr = ((char*)o2_shape)[ offset & 0x1FF ];

							UINT8	m;
							for ( m = 0x80; m > 0; m >>= 1, x++ )
							{
								if ( chr & m )
								{
									if ( x >= 0 && x < 160 )
									{
										/* Check if we collide with an already drawn source object */
										if ( collision_map[ x ] & state->m_o2_vdc.s.collision )
										{
											state->m_collision_status |= COLLISION_CHARACTERS;
										}
										/* Check if an already drawn object would collide with us */
										if ( COLLISION_CHARACTERS & state->m_o2_vdc.s.collision && collision_map[ x ] )
										{
											state->m_collision_status |= collision_map[ x ];
										}
										collision_map[ x ] |= COLLISION_CHARACTERS;
										state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + x ) = color;
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
			for ( i = 0; i < ARRAY_LENGTH( state->m_o2_vdc.s.sprites ); i++ )
			{
				int y = state->m_o2_vdc.s.sprites[i].y;
				int height = 8;
				if ( state->m_o2_vdc.s.sprites[i].color & 4 )
				{
					/* Zoomed sprite */
					//sprite_width[i] = 16;
					if ( y <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y + height * 4 )
					{
						UINT16 color = 16 + ( ( state->m_o2_vdc.s.sprites[i].color >> 3 ) & 0x07 );
						UINT8	chr = state->m_o2_vdc.s.shape[i][ ( ( vpos - state->m_start_vpos - y ) >> 2 ) ];
						int		x = state->m_o2_vdc.s.sprites[i].x;
						UINT8	m;

						for ( m = 0x01; m > 0; m <<= 1, x += 2 )
						{
							if ( chr & m )
							{
								if ( x >= 0 && x < 160 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ x ] & state->m_o2_vdc.s.collision )
									{
										state->m_collision_status |= ( 1 << i );
									}
									/* Check if an already drawn object would collide with us */
									if ( ( 1 << i ) & state->m_o2_vdc.s.collision && collision_map[ x ] )
									{
										state->m_collision_status |= collision_map[ x ];
									}
									collision_map[ x ] |= ( 1 << i );
									state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + x ) = color;
								}
								if ( x >= -1 && x < 159 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ x ] & state->m_o2_vdc.s.collision )
									{
										state->m_collision_status |= ( 1 << i );
									}
									/* Check if an already drawn object would collide with us */
									if ( ( 1 << i ) & state->m_o2_vdc.s.collision && collision_map[ x ] )
									{
										state->m_collision_status |= collision_map[ x ];
									}
									collision_map[ x ] |= ( 1 << i );
									state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + x + 1 ) = color;
								}
							}
						}
					}
				}
				else
				{
					/* Regular sprite */
					if ( y <= ( vpos - state->m_start_vpos ) && ( vpos - state->m_start_vpos ) < y + height * 2 )
					{
						UINT16 color = 16 + ( ( state->m_o2_vdc.s.sprites[i].color >> 3 ) & 0x07 );
						UINT8	chr = state->m_o2_vdc.s.shape[i][ ( ( vpos - state->m_start_vpos - y ) >> 1 ) ];
						int		x = state->m_o2_vdc.s.sprites[i].x;
						UINT8	m;

						for ( m = 0x01; m > 0; m <<= 1, x++ )
						{
							if ( chr & m )
							{
								if ( x >= 0 && x < 160 )
								{
									/* Check if we collide with an already drawn source object */
									if ( collision_map[ x ] & state->m_o2_vdc.s.collision )
									{
										state->m_collision_status |= ( 1 << i );
									}
									/* Check if an already drawn object would collide with us */
									if ( ( 1 << i ) & state->m_o2_vdc.s.collision && collision_map[ x ] )
									{
										state->m_collision_status |= collision_map[ x ];
									}
									collision_map[ x ] |= ( 1 << i );
									state->m_tmp_bitmap.pix16(vpos, I824X_START_ACTIVE_SCAN + x ) = color;
								}
							}
						}
					}
				}
			}
		}
	}

	/* Check for start of VBlank */
	if ( vpos == state->m_start_vblank )
	{
		state->m_control_status |= 0x08;
		if ( ! state->m_iff )
		{
			machine.device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
			state->m_iff = 1;
		}
	}
}

static TIMER_CALLBACK( i824x_hblank_callback )
{
	odyssey2_state *state = machine.driver_data<odyssey2_state>();
	int vpos = machine.primary_screen->vpos();

	if ( vpos < state->m_start_vpos - 1 )
		return;

	if ( vpos < state->m_start_vblank - 1 )
	{
		state->m_control_status |= 0x01;
	}
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void odyssey2_state::video_start()
{
	screen_device *screen = machine().first_screen();

	memset(m_o2_vdc.reg, 0, 0x100);

	m_o2_snd_shift[0] = m_o2_snd_shift[1] = 0;
	m_x_beam_pos = 0;
	m_y_beam_pos = 0;
	m_control_status = 0;
	m_collision_status = 0;
	m_iff = 0;
	m_start_vpos = 0;
	m_start_vblank = 0;
	m_lum = 0;

	m_o2_snd_shift[0] = machine().sample_rate() / 983;
	m_o2_snd_shift[1] = machine().sample_rate() / 3933;

	m_start_vpos = I824X_START_Y;
	m_start_vblank = I824X_START_Y + I824X_SCREEN_HEIGHT;
	m_control_status = 0;
	m_iff = 0;

	screen->register_screen_bitmap(m_tmp_bitmap);

	m_i824x_line_timer = machine().scheduler().timer_alloc(FUNC(i824x_scanline_callback));
	m_i824x_line_timer->adjust( machine().primary_screen->time_until_pos(1, I824X_START_ACTIVE_SCAN ), 0,  machine().primary_screen->scan_period() );

	m_i824x_hblank_timer = machine().scheduler().timer_alloc(FUNC(i824x_hblank_callback));
	m_i824x_hblank_timer->adjust( machine().primary_screen->time_until_pos(1, I824X_END_ACTIVE_SCAN + 18 ), 0, machine().primary_screen->scan_period() );
}

/***************************************************************************

  Refresh the video screen

***************************************************************************/

UINT32 odyssey2_state::screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, m_tmp_bitmap, 0, 0, 0, 0, cliprect );

	return 0;
}

static DEVICE_START( odyssey2_sound )
{
	odyssey2_state *state = device->machine().driver_data<odyssey2_state>();
	state->m_sh_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/(I824X_LINE_CLOCKS*4), 0, odyssey2_sh_update );
}


STREAM_UPDATE( odyssey2_sh_update )
{
	odyssey2_state *state = device->machine().driver_data<odyssey2_state>();
	UINT32 old_signal, signal;
	int ii;
	int period;
	stream_sample_t *buffer = outputs[0];

	/* Generate the signal */
	old_signal = signal = state->m_o2_vdc.s.shift3 | (state->m_o2_vdc.s.shift2 << 8) | (state->m_o2_vdc.s.shift1 << 16);

	if( state->m_o2_vdc.s.sound & 0x80 )	/* Sound is enabled */
	{
		for( ii = 0; ii < samples; ii++, buffer++ )
		{
			*buffer = 0;
			*buffer = signal & 0x1;
			period = (state->m_o2_vdc.s.sound & 0x20) ? 1 : 4;
			if( ++state->m_sh_count >= period )
			{
				state->m_sh_count = 0;
				signal >>= 1;
				/* Loop sound */
				signal |= *buffer << 23;
				/* Check if noise should be applied */
				if ( state->m_o2_vdc.s.sound & 0x10 )
				{
					/* Noise tap is on bits 0 and 5 and fed back to bits 15 (and 23!) */
					UINT32 new_bit = ( ( old_signal ) ^ ( old_signal >> 5 ) ) & 0x01;
					signal = ( old_signal & 0xFF0000 ) | ( ( old_signal & 0xFFFF ) >> 1 ) | ( new_bit << 15 ) | ( new_bit << 23 );
				}
				state->m_o2_vdc.s.shift3 = signal & 0xFF;
				state->m_o2_vdc.s.shift2 = ( signal >> 8 ) & 0xFF;
				state->m_o2_vdc.s.shift1 = ( signal >> 16 ) & 0xFF;
				old_signal = signal;
			}

			/* Throw an interrupt if enabled */
			if( state->m_o2_vdc.s.control & 0x4 )
			{
				device->machine().device("maincpu")->execute().set_input_line(1, HOLD_LINE); /* Is this right? */
			}

			/* Adjust volume */
			*buffer *= state->m_o2_vdc.s.sound & 0xf;
			/* Pump the volume up */
			*buffer <<= 10;
		}
	}
	else
	{
		/* Sound disabled, so clear the buffer */
		for( ii = 0; ii < samples; ii++, buffer++ )
			*buffer = 0;
	}
}

/*
    Thomson EF9340/EF9341 extra chips in the g7400
 */

#ifdef UNUSED_FUNCTION
INLINE UINT16 ef9341_get_c_addr( odyssey2_state *state )
{
	if ( ( state->m_ef9341.Y & 0x0C ) == 0x0C )
	{
		return 0x318 | ( ( state->m_ef9341.X & 0x38 ) << 2 ) | ( state->m_ef9341.X & 0x07 );
	}
	if ( state->m_ef9341.X & 0x20 )
	{
		return 0x300 | ( ( state->m_ef9341.Y & 0x07 ) << 5 ) | ( state->m_ef9341.Y & 0x18 ) | ( state->m_ef9341.X & 0x07 );
	}
	return ( state->m_ef9341.Y << 5 ) | state->m_ef9341.X;
}

INLINE void ef9341_inc_c( odyssey2_state *state )
{
	state->m_ef9341.X++;
	if ( state->m_ef9341.X >= 40 )
	{
		state->m_ef9341.Y = ( state->m_ef9341.Y + 1 ) % 24;
	}
}

void odyssey2_ef9341_w( running_machine &machine, int command, int b, UINT8 data )
{
	odyssey2_state *state = machine.driver_data<odyssey2_state>();
	logerror("ef9341 %s write, t%s, data %02X\n", command ? "command" : "data", b ? "B" : "A", data );

	if ( command )
	{
		if ( b )
		{
			state->m_ef9341.TB = data;
			state->m_ef9341.busy = 0x80;
			switch( state->m_ef9341.TB & 0xE0 )
			{
			case 0x00:	/* Begin row */
				state->m_ef9341.X = 0;
				state->m_ef9341.Y = state->m_ef9341.TA & 0x1F;
				break;
			case 0x20:	/* Load Y */
				state->m_ef9341.Y = state->m_ef9341.TA & 0x1F;
				break;
			case 0x40:	/* Load X */
				state->m_ef9341.X = state->m_ef9341.TA & 0x3F;
				break;
			case 0x60:	/* INC C */
				ef9341_inc_c(state);
				break;
			case 0x80:	/* Load M */
				state->m_ef9341.M = state->m_ef9341.TA;
				break;
			case 0xA0:	/* Load R */
				state->m_ef9341.R = state->m_ef9341.TA;
				break;
			case 0xC0:	/* Load Y0 */
				state->m_ef9341.Y0 = state->m_ef9341.TA & 0x3F;
				break;
			}
			state->m_ef9341.busy = 0;
		}
		else
		{
			state->m_ef9341.TA = data;
		}
	}
	else
	{
		if ( b )
		{
			state->m_ef9341.TB = data;
			state->m_ef9341.busy = 0x80;
			switch ( state->m_ef9341.M & 0xE0 )
			{
			case 0x00:	/* Write */
				state->m_ef9341.ram[ ef9341_get_c_addr(state) ] = state->m_ef9341.TB;
				ef9341_inc_c(state);
				break;
			case 0x20:	/* Read */
				logerror("ef9341 unimplemented data action %02X\n", state->m_ef9341.M & 0xE0 );
				ef9341_inc_c(state);
				break;
			case 0x40:	/* Write without increment */
			case 0x60:	/* Read without increment */
			case 0x80:	/* Write slice */
			case 0xA0:	/* Read slice */
				logerror("ef9341 unimplemented data action %02X\n", state->m_ef9341.M & 0xE0 );
				break;
			}
			state->m_ef9341.busy = 0;
		}
		else
		{
			state->m_ef9341.TA = data;
		}
	}
}

UINT8 odyssey2_ef9341_r( running_machine &machine, int command, int b )
{
	odyssey2_state *state = machine.driver_data<odyssey2_state>();
	UINT8	data = 0xFF;

	logerror("ef9341 %s read, t%s\n", command ? "command" : "data", b ? "B" : "A" );
	if ( command )
	{
		if ( b )
		{
			data = 0xFF;
		}
		else
		{
			data = state->m_ef9341.busy;
		}
	}
	else
	{
		if ( b )
		{
			data = state->m_ef9341.TB;
			state->m_ef9341.busy = 0x80;
		}
		else
		{
			data = state->m_ef9341.TA;
		}
	}
	return data;
}
#endif

const device_type ODYSSEY2 = &device_creator<odyssey2_sound_device>;

odyssey2_sound_device::odyssey2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ODYSSEY2, "P8244/P8245", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void odyssey2_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void odyssey2_sound_device::device_start()
{
	DEVICE_START_NAME( odyssey2_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void odyssey2_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


