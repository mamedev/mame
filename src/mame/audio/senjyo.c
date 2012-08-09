#include "emu.h"
#include "includes/senjyo.h"


const z80_daisy_config senjyo_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ NULL }
};


/* z80 pio */

static READ8_DEVICE_HANDLER( pio_pa_r )
{
	senjyo_state *state = device->machine().driver_data<senjyo_state>();

	return state->m_sound_cmd;
}

Z80PIO_INTERFACE( senjyo_pio_intf )
{
	DEVCB_CPU_INPUT_LINE("sub", INPUT_LINE_IRQ0),
	DEVCB_HANDLER(pio_pa_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* z80 ctc */
Z80CTC_INTERFACE( senjyo_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("sub", INPUT_LINE_IRQ0), /* interrupt handler */
	DEVCB_DEVICE_LINE_MEMBER("z80ctc", z80ctc_device, trg1),	/* ZC/TO0 callback */
	DEVCB_NULL,													/* ZC/TO1 callback */
	DEVCB_DRIVER_LINE_MEMBER(senjyo_state, sound_line_clock)	/* ZC/TO2 callback */
};

WRITE_LINE_MEMBER(senjyo_state::sound_line_clock)
{
	if (state != 0)
	{
		m_dac->write_signed16(2184 * 2 * ((m_sound_state & 8) ? m_single_volume : 0));
		m_sound_state++;
	}
}

WRITE8_MEMBER(senjyo_state::senjyo_volume_w)
{
	m_single_volume = data & 0x0f;
}
