/***************************************************************************

  galaxy.c

  Functions to emulate the video hardware of the Galaksija.

  20/05/2008 - Real video implementation by Miodrag Milanovic
  01/03/2008 - Update by Miodrag Milanovic to make Galaksija video work with new SVN code
***************************************************************************/

#include "emu.h"
#include "includes/galaxy.h"
#include "cpu/z80/z80.h"


static TIMER_CALLBACK( gal_video )
{
	galaxy_state *state = machine.driver_data<galaxy_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	int y, x;
	if (state->m_interrupts_enabled == TRUE)
	{
		UINT8 *gfx = state->memregion("gfx1")->base();
		UINT8 dat = (state->m_latch_value & 0x3c) >> 2;
		if ((state->m_gal_cnt >= 48 * 2) && (state->m_gal_cnt < 48 * 210))  // display on screen just state->m_first 208 lines
		{
			UINT8 mode = (state->m_latch_value >> 1) & 1; // bit 2 latch represents mode
			UINT16 addr = (cpu_get_reg(machine.device("maincpu"), Z80_I) << 8) | cpu_get_reg(machine.device("maincpu"), Z80_R) | ((state->m_latch_value & 0x80) ^ 0x80);
			if (mode == 0)
			{
				// Text mode
				if (state->m_first == 0 && (cpu_get_reg(machine.device("maincpu"), Z80_R) & 0x1f) == 0)
				{
					// Due to a fact that on real processor latch value is set at
					// the end of last cycle we need to skip dusplay of double
					// state->m_first char in each row
					state->m_code = 0x00;
					state->m_first = 1;
				}
				else
				{
					state->m_code = space->read_byte(addr) & 0xbf;
					state->m_code += (state->m_code & 0x80) >> 1;
					state->m_code = gfx[(state->m_code & 0x7f) +(dat << 7 )] ^ 0xff;
					state->m_first = 0;
				}
				y = state->m_gal_cnt / 48 - 2;
				x = (state->m_gal_cnt % 48) * 8;

				state->m_bitmap.pix16(y, x ) = (state->m_code >> 0) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 1) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 2) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 3) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 4) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 5) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 6) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 7) & 1;
			}
			else
			{ // Graphics mode
				if (state->m_first < 4 && (cpu_get_reg(machine.device("maincpu"), Z80_R) & 0x1f) == 0)
				{
					// Due to a fact that on real processor latch value is set at
					// the end of last cycle we need to skip dusplay of 4 times
					// state->m_first char in each row
					state->m_code = 0x00;
					state->m_first++;
				}
				else
				{
					state->m_code = space->read_byte(addr) ^ 0xff;
					state->m_first = 0;
				}
				y = state->m_gal_cnt / 48 - 2;
				x = (state->m_gal_cnt % 48) * 8;

				/* hack - until calc of R is fixed in Z80 */
				if (x == 11 * 8 && y == 0)
				{
					state->m_start_addr = addr;
				}
				if ((x / 8 >= 11) && (x / 8 < 44))
				{
					state->m_code = space->read_byte(state->m_start_addr + y * 32 + (state->m_gal_cnt % 48) - 11) ^ 0xff;
				}
				else
				{
					state->m_code = 0x00;
				}
				/* end of hack */

				state->m_bitmap.pix16(y, x ) = (state->m_code >> 0) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 1) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 2) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 3) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 4) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 5) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 6) & 1; x++;
				state->m_bitmap.pix16(y, x ) = (state->m_code >> 7) & 1;
			}
		}
		state->m_gal_cnt++;
	}
}

void galaxy_set_timer(running_machine &machine)
{
	galaxy_state *state = machine.driver_data<galaxy_state>();
	state->m_gal_cnt = 0;
	state->m_gal_video_timer->adjust(attotime::zero, 0, attotime::from_hz(6144000 / 8));
}

VIDEO_START( galaxy )
{
	galaxy_state *state = machine.driver_data<galaxy_state>();
	state->m_gal_cnt = 0;

	state->m_gal_video_timer = machine.scheduler().timer_alloc(FUNC(gal_video));
	state->m_gal_video_timer->adjust(attotime::zero, 0, attotime::never);

	machine.primary_screen->register_screen_bitmap(state->m_bitmap);
}

SCREEN_UPDATE_IND16( galaxy )
{
	galaxy_state *state = screen.machine().driver_data<galaxy_state>();
	state->m_gal_video_timer->adjust(attotime::zero, 0, attotime::never);
	if (state->m_interrupts_enabled == FALSE)
	{
		const rectangle black_area(0, 384 - 1, 0, 208 - 1);
		state->m_bitmap.fill(0, black_area);
	}
	state->m_interrupts_enabled = FALSE;
	copybitmap(bitmap, state->m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

