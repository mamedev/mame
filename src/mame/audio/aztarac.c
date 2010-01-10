/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/aztarac.h"

static int sound_status;

READ16_HANDLER( aztarac_sound_r )
{
    return sound_status & 0x01;
}

WRITE16_HANDLER( aztarac_sound_w )
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		soundlatch_w(space, offset, data);
		sound_status ^= 0x21;
		if (sound_status & 0x20)
			cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
	}
}

READ8_HANDLER( aztarac_snd_command_r )
{
    sound_status |= 0x01;
    sound_status &= ~0x20;
    return soundlatch_r(space,offset);
}

READ8_HANDLER( aztarac_snd_status_r )
{
    return sound_status & ~0x01;
}

WRITE8_HANDLER( aztarac_snd_status_w )
{
    sound_status &= ~0x10;
}

INTERRUPT_GEN( aztarac_snd_timed_irq )
{
    sound_status ^= 0x10;

    if (sound_status & 0x10)
        cpu_set_input_line(device,0,HOLD_LINE);
}
