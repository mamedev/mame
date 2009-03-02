/***************************************************************************

    Cinematronics Cosmic Chasm hardware

***************************************************************************/

#include "driver.h"
#include "streams.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "cchasm.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

static int sound_flags;
static const device_config *ctc;

READ8_HANDLER( cchasm_coin_sound_r )
{
	UINT8 coin = (input_port_read(space->machine, "IN3") >> 4) & 0x7;
	if (coin != 0x7) coin |= 0x8;
	return sound_flags | coin;
}

READ8_HANDLER( cchasm_soundlatch2_r )
{
	sound_flags &= ~0x80;
	z80ctc_trg2_w(ctc, 0, 0);
	return soundlatch2_r(space, offset);
}

WRITE8_HANDLER( cchasm_soundlatch4_w )
{
	sound_flags |= 0x40;
	soundlatch4_w(space, offset, data);
	cpu_set_input_line(space->machine->cpu[0], 1, HOLD_LINE);
}

WRITE16_HANDLER( cchasm_io_w )
{
    static int led;

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (offset & 0xf)
		{
		case 0:
			soundlatch_w (space, offset, data);
			break;
		case 1:
			sound_flags |= 0x80;
			soundlatch2_w (space, offset, data);
			z80ctc_trg2_w (ctc, 0, 1);
			cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE);
			break;
		case 2:
			led = data;
			break;
		}
	}
}

READ16_HANDLER( cchasm_io_r )
{
	switch (offset & 0xf)
	{
	case 0x0:
		return soundlatch3_r (space, offset) << 8;
	case 0x1:
		sound_flags &= ~0x40;
		return soundlatch4_r (space,offset) << 8;
	case 0x2:
		return (sound_flags| (input_port_read(space->machine, "IN3") & 0x07) | 0x08) << 8;
	case 0x5:
		return input_port_read(space->machine, "IN2") << 8;
	case 0x8:
		return input_port_read(space->machine, "IN1") << 8;
	default:
		return 0xff << 8;
	}
}

static int channel_active[2];
static int output[2];

static void ctc_interrupt (const device_config *device, int state)
{
	cpu_set_input_line(device->machine->cpu[1], 0, state);
}

static WRITE8_DEVICE_HANDLER( ctc_timer_1_w )
{
	if (data) /* rising edge */
	{
		output[0] ^= 0x7f;
		channel_active[0] = 1;
		dac_data_w(devtag_get_device(device->machine, "dac1"), output[0]);
	}
}

static WRITE8_DEVICE_HANDLER( ctc_timer_2_w )
{
	if (data) /* rising edge */
	{
		output[1] ^= 0x7f;
		channel_active[1] = 1;
		dac_data_w(devtag_get_device(device->machine, "dac2"), output[0]);
	}
}

z80ctc_interface cchasm_ctc_intf =
{
	0,               /* timer disables */
	ctc_interrupt,   /* interrupt handler */
	0,               /* ZC/TO0 callback */
	ctc_timer_1_w,     /* ZC/TO1 callback */
	ctc_timer_2_w      /* ZC/TO2 callback */
};

static TIMER_CALLBACK( cchasm_sh_update )
{
    if ((input_port_read(machine, "IN3") & 0x70) != 0x70)
        z80ctc_trg0_w (ctc, 0, 1);
}

SOUND_START( cchasm )
{
    sound_flags = 0;
    output[0] = 0; output[1] = 0;

	ctc = devtag_get_device(machine, "ctc");

	timer_pulse(machine, video_screen_get_frame_period(machine->primary_screen), NULL, 0, cchasm_sh_update);
}


