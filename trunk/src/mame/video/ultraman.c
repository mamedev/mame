#include "emu.h"
#include "video/konicdev.h"
#include "includes/ultraman.h"

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ultraman_sprite_callback( running_machine &machine, int *code, int *color, int *priority, int *shadow )
{
	ultraman_state *state = machine.driver_data<ultraman_state>();

	*priority = (*color & 0x80) >> 7;
	*color = state->m_sprite_colorbase + ((*color & 0x7e) >> 1);
	*shadow = 0;
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void ultraman_zoom_callback_0(running_machine &machine, int *code, int *color, int *flags )
{
	ultraman_state *state = machine.driver_data<ultraman_state>();
	*code |= ((*color & 0x07) << 8) | (state->m_bank0 << 11);
	*color = state->m_zoom_colorbase[0] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_1(running_machine &machine, int *code, int *color, int *flags )
{
	ultraman_state *state = machine.driver_data<ultraman_state>();
	*code |= ((*color & 0x07) << 8) | (state->m_bank1 << 11);
	*color = state->m_zoom_colorbase[1] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_2(running_machine &machine, int *code, int *color, int *flags )
{
	ultraman_state *state = machine.driver_data<ultraman_state>();
	*code |= ((*color & 0x07) << 8) | (state->m_bank2 << 11);
	*color = state->m_zoom_colorbase[2] + ((*color & 0xf8) >> 3);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ultraman )
{
	ultraman_state *state = machine.driver_data<ultraman_state>();
	state->m_sprite_colorbase = 192;
	state->m_zoom_colorbase[0] = 0;
	state->m_zoom_colorbase[1] = 64;
	state->m_zoom_colorbase[2] = 128;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(ultraman_state::ultraman_gfxctrl_w)
{

	if (ACCESSING_BITS_0_7)
	{
		/*  bit 0: enable wraparound for scr #1
            bit 1: msb of code for scr #1
            bit 2: enable wraparound for scr #2
            bit 3: msb of code for scr #2
            bit 4: enable wraparound for scr #3
            bit 5: msb of code for scr #3
            bit 6: coin counter 1
            bit 7: coin counter 2 */

		k051316_wraparound_enable(m_k051316_1, data & 0x01);

		if (m_bank0 != ((data & 0x02) >> 1))
		{
			m_bank0 = (data & 0x02) >> 1;
			machine().tilemap().mark_all_dirty();	/* should mark only zoom0 */
		}

		k051316_wraparound_enable(m_k051316_2, data & 0x04);

		if (m_bank1 != ((data & 0x08) >> 3))
		{
			m_bank1 = (data & 0x08) >> 3;
			machine().tilemap().mark_all_dirty();	/* should mark only zoom1 */
		}

		k051316_wraparound_enable(m_k051316_3, data & 0x10);

		if (m_bank2 != ((data & 0x20) >> 5))
		{
			m_bank2 = (data & 0x20) >> 5;
			machine().tilemap().mark_all_dirty();	/* should mark only zoom2 */
		}

		coin_counter_w(machine(), 0, data & 0x40);
		coin_counter_w(machine(), 1, data & 0x80);
	}
}



/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( ultraman )
{
	ultraman_state *state = screen.machine().driver_data<ultraman_state>();

	k051316_zoom_draw(state->m_k051316_3, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->m_k051316_2, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(state->m_k051960, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->m_k051316_1, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(state->m_k051960, bitmap, cliprect, 1, 1);
	return 0;
}
