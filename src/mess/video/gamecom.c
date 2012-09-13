
#include "includes/gamecom.h"

#define Y_PIXELS 200


static TIMER_CALLBACK( gamecom_scanline )
	{
	gamecom_state *state = machine.driver_data<gamecom_state>();
	// draw line
	if ( state->m_scanline == 0 )
		state->m_base_address = ( state->m_p_ram[SM8521_LCDC] & 0x40 ) ? 0x2000 : 0x0000;

	if ( ~state->m_p_ram[SM8521_LCDC] & 0x80 )
	{
		rectangle rec(0, Y_PIXELS - 1, state->m_scanline, state->m_scanline);
		state->m_bitmap.fill(0, rec );
		return;
	}
	else
	{
		UINT8 *line = &state->m_p_videoram[ state->m_base_address + 40 * state->m_scanline ];
		int	pal[4];
		int	i;

		switch( state->m_p_ram[SM8521_LCDC] & 0x30 )
		{
		case 0x00:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 2;
			pal[3] = 0;
			break;
		case 0x10:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 1;
			pal[3] = 0;
			break;
		case 0x20:
			pal[0] = 4;
			pal[1] = 3;
			pal[2] = 1;
			pal[3] = 0;
			break;
		case 0x30:
			pal[0] = 4;
			pal[1] = 2;
			pal[2] = 1;
			pal[3] = 0;
			break;
		}
		for( i = 0; i < 40; i++ )
		{
			UINT8 p = line[i];
			state->m_bitmap.pix16(i * 4 + 0, state->m_scanline) = pal[ ( p >> 6 ) & 3 ];
			state->m_bitmap.pix16(i * 4 + 1, state->m_scanline) = pal[ ( p >> 4 ) & 3 ];
			state->m_bitmap.pix16(i * 4 + 2, state->m_scanline) = pal[ ( p >> 2 ) & 3 ];
			state->m_bitmap.pix16(i * 4 + 3, state->m_scanline) = pal[ ( p      ) & 3 ];
		}
	}

	state->m_scanline = ( state->m_scanline + 1 ) % Y_PIXELS;
}

void gamecom_state::video_start()
{
	m_scanline_timer = machine().scheduler().timer_alloc(FUNC(gamecom_scanline));
	m_scanline_timer->adjust( machine().primary_screen->time_until_pos(0 ), 0, machine().primary_screen->scan_period() );
	machine().primary_screen->register_screen_bitmap(m_bitmap);
}
