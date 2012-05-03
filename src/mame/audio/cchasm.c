/***************************************************************************

    Cinematronics Cosmic Chasm hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "includes/cchasm.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


WRITE8_MEMBER(cchasm_state::cchasm_reset_coin_flag_w)
{
	if (m_coin_flag)
	{
		m_coin_flag = 0;
		z80ctc_trg0_w(m_ctc, m_coin_flag);
	}
}

INPUT_CHANGED_MEMBER(cchasm_state::cchasm_set_coin_flag )
{
	if (!newval && !m_coin_flag)
	{
		m_coin_flag = 1;
		z80ctc_trg0_w(m_ctc, m_coin_flag);
	}
}

READ8_MEMBER(cchasm_state::cchasm_coin_sound_r)
{
	UINT8 coin = (ioport("IN3")->read() >> 4) & 0x7;
	return m_sound_flags | (m_coin_flag << 3) | coin;
}

READ8_MEMBER(cchasm_state::cchasm_soundlatch2_r)
{
	m_sound_flags &= ~0x80;
	z80ctc_trg2_w(m_ctc, 0);
	return soundlatch2_byte_r(space, offset);
}

WRITE8_MEMBER(cchasm_state::cchasm_soundlatch4_w)
{
	m_sound_flags |= 0x40;
	soundlatch4_byte_w(space, offset, data);
	cputag_set_input_line(machine(), "maincpu", 1, HOLD_LINE);
}

WRITE16_MEMBER(cchasm_state::cchasm_io_w)
{
	//static int led;

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (offset & 0xf)
		{
		case 0:
			soundlatch_byte_w(space, offset, data);
			break;
		case 1:
			m_sound_flags |= 0x80;
			soundlatch2_byte_w(space, offset, data);
			z80ctc_trg2_w(m_ctc, 1);
			cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
			break;
		case 2:
			//led = data;
			break;
		}
	}
}

READ16_MEMBER(cchasm_state::cchasm_io_r)
{
	switch (offset & 0xf)
	{
	case 0x0:
		return soundlatch3_byte_r(space, offset) << 8;
	case 0x1:
		m_sound_flags &= ~0x40;
		return soundlatch4_byte_r(space,offset) << 8;
	case 0x2:
		return (m_sound_flags| (ioport("IN3")->read() & 0x07) | 0x08) << 8;
	case 0x5:
		return ioport("IN2")->read() << 8;
	case 0x8:
		return ioport("IN1")->read() << 8;
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
