/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/aztarac.h"


READ16_HANDLER( aztarac_sound_r )
{
	aztarac_state *state = space->machine().driver_data<aztarac_state>();
    return state->m_sound_status & 0x01;
}

WRITE16_HANDLER( aztarac_sound_w )
{
	aztarac_state *state = space->machine().driver_data<aztarac_state>();
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		state->soundlatch_w(*space, offset, data);
		state->m_sound_status ^= 0x21;
		if (state->m_sound_status & 0x20)
			cputag_set_input_line(space->machine(), "audiocpu", 0, HOLD_LINE);
	}
}

READ8_HANDLER( aztarac_snd_command_r )
{
	aztarac_state *state = space->machine().driver_data<aztarac_state>();
    state->m_sound_status |= 0x01;
    state->m_sound_status &= ~0x20;
    return state->soundlatch_r(*space,offset);
}

READ8_HANDLER( aztarac_snd_status_r )
{
	aztarac_state *state = space->machine().driver_data<aztarac_state>();
    return state->m_sound_status & ~0x01;
}

WRITE8_HANDLER( aztarac_snd_status_w )
{
	aztarac_state *state = space->machine().driver_data<aztarac_state>();
    state->m_sound_status &= ~0x10;
}

INTERRUPT_GEN( aztarac_snd_timed_irq )
{
	aztarac_state *state = device->machine().driver_data<aztarac_state>();
    state->m_sound_status ^= 0x10;

    if (state->m_sound_status & 0x10)
        device_set_input_line(device,0,HOLD_LINE);
}
