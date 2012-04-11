#include "emu.h"
#include "sound/samples.h"
#include "includes/senjyo.h"


/* single tone generator */
#define SINGLE_LENGTH 10000
#define SINGLE_DIVIDER 8

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
	NOTIMER_2,       /* timer disables */
	DEVCB_CPU_INPUT_LINE("sub", INPUT_LINE_IRQ0), /* interrupt handler */
	DEVCB_LINE(z80ctc_trg1_w),	/* ZC/TO0 callback */
	DEVCB_NULL,					/* ZC/TO1 callback */
	DEVCB_NULL					/* ZC/TO2 callback */
};


WRITE8_MEMBER(senjyo_state::senjyo_volume_w)
{
	samples_device *samples = machine().device<samples_device>("samples");

	m_single_volume = data & 0x0f;
	samples->set_volume(0, m_single_volume / 15.0);
}


static TIMER_CALLBACK( senjyo_sh_update )
{
	samples_device *samples = machine.device<samples_device>("samples");
	senjyo_state *state = machine.driver_data<senjyo_state>();

	/* ctc2 timer single tone generator frequency */
	z80ctc_device *ctc = machine.device<z80ctc_device>("z80ctc");
	attotime period = ctc->period(2);
	if (period != attotime::zero)
		state->m_single_rate = ATTOSECONDS_TO_HZ(period.attoseconds);
	else
		state->m_single_rate = 0;

	samples->set_frequency(0, state->m_single_rate);
}


SAMPLES_START( senjyo_sh_start )
{
	running_machine &machine = device.machine();
	senjyo_state *state = machine.driver_data<senjyo_state>();
	int i;

	state->m_single_data = auto_alloc_array(machine, INT16, SINGLE_LENGTH);

	for (i = 0;i < SINGLE_LENGTH;i++)		/* freq = ctc2 zco / 8 */
		state->m_single_data[i] = ((i/SINGLE_DIVIDER)&0x01)*127*256;

	/* CTC2 single tone generator */
	state->m_single_rate = 1000;
	state->m_single_volume = 0;
	device.set_volume(0, state->m_single_volume / 15.0);
	device.start_raw(0, state->m_single_data, SINGLE_LENGTH, state->m_single_rate, true);

	machine.scheduler().timer_pulse(machine.primary_screen->frame_period(), FUNC(senjyo_sh_update));
}
