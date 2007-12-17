/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "aztarac.h"

static int sound_status;

READ16_HANDLER( aztarac_sound_r )
{
    return sound_status & 0x01;
}

WRITE16_HANDLER( aztarac_sound_w )
{
	if (ACCESSING_LSB)
	{
		data &= 0xff;
		soundlatch_w(offset, data);
		sound_status ^= 0x21;
		if (sound_status & 0x20)
			cpunum_set_input_line(1, 0, HOLD_LINE);
	}
}

READ8_HANDLER( aztarac_snd_command_r )
{
    sound_status |= 0x01;
    sound_status &= ~0x20;
    return soundlatch_r(offset);
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
        cpunum_set_input_line(1,0,HOLD_LINE);
}


