/***************************************************************************

    Cinematronics Cosmic Chasm hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "includes/cchasm.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


WRITE8_HANDLER( cchasm_reset_coin_flag_w )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	if (state->m_coin_flag)
	{
		state->m_coin_flag = 0;
		z80ctc_trg0_w(state->m_ctc, state->m_coin_flag);
	}
}

INPUT_CHANGED( cchasm_set_coin_flag )
{
	cchasm_state *state = field.machine().driver_data<cchasm_state>();
	if (!newval && !state->m_coin_flag)
	{
		state->m_coin_flag = 1;
		z80ctc_trg0_w(state->m_ctc, state->m_coin_flag);
	}
}

READ8_HANDLER( cchasm_coin_sound_r )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	UINT8 coin = (input_port_read(space->machine(), "IN3") >> 4) & 0x7;
	return state->m_sound_flags | (state->m_coin_flag << 3) | coin;
}

READ8_HANDLER( cchasm_soundlatch2_r )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	state->m_sound_flags &= ~0x80;
	z80ctc_trg2_w(state->m_ctc, 0);
	return state->soundlatch2_r(*space, offset);
}

WRITE8_HANDLER( cchasm_soundlatch4_w )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	state->m_sound_flags |= 0x40;
	state->soundlatch4_w(*space, offset, data);
	cputag_set_input_line(space->machine(), "maincpu", 1, HOLD_LINE);
}

WRITE16_HANDLER( cchasm_io_w )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	//static int led;

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (offset & 0xf)
		{
		case 0:
			state->soundlatch_w(*space, offset, data);
			break;
		case 1:
			state->m_sound_flags |= 0x80;
			state->soundlatch2_w(*space, offset, data);
			z80ctc_trg2_w(state->m_ctc, 1);
			cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
			break;
		case 2:
			//led = data;
			break;
		}
	}
}

READ16_HANDLER( cchasm_io_r )
{
	cchasm_state *state = space->machine().driver_data<cchasm_state>();
	switch (offset & 0xf)
	{
	case 0x0:
		return state->soundlatch3_r(*space, offset) << 8;
	case 0x1:
		state->m_sound_flags &= ~0x40;
		return state->soundlatch4_r(*space,offset) << 8;
	case 0x2:
		return (state->m_sound_flags| (input_port_read(space->machine(), "IN3") & 0x07) | 0x08) << 8;
	case 0x5:
		return input_port_read(space->machine(), "IN2") << 8;
	case 0x8:
		return input_port_read(space->machine(), "IN1") << 8;
	default:
		return 0xff << 8;
	}
}


static WRITE_LINE_DEVICE_HANDLER( ctc_timer_1_w )
{
	cchasm_state *drvstate = device->machine().driver_data<cchasm_state>();
	if (state) /* rising edge */
	{
		drvstate->m_output[0] ^= 0x7f;
		drvstate->m_channel_active[0] = 1;
		dac_data_w(device->machine().device("dac1"), drvstate->m_output[0]);
	}
}

static WRITE_LINE_DEVICE_HANDLER( ctc_timer_2_w )
{
	cchasm_state *drvstate = device->machine().driver_data<cchasm_state>();
	if (state) /* rising edge */
	{
		drvstate->m_output[1] ^= 0x7f;
		drvstate->m_channel_active[1] = 1;
		dac_data_w(device->machine().device("dac2"), drvstate->m_output[0]);
	}
}

Z80CTC_INTERFACE( cchasm_ctc_intf )
{
	0,               /* timer disables */
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),   /* interrupt handler */
	DEVCB_NULL,					/* ZC/TO0 callback */
	DEVCB_LINE(ctc_timer_1_w),	/* ZC/TO1 callback */
	DEVCB_LINE(ctc_timer_2_w)	/* ZC/TO2 callback */
};

SOUND_START( cchasm )
{
	cchasm_state *state = machine.driver_data<cchasm_state>();
	state->m_coin_flag = 0;
	state->m_sound_flags = 0;
	state->m_output[0] = 0; state->m_output[1] = 0;

	state->m_ctc = machine.device("ctc");
}
