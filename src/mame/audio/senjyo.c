#include "emu.h"
#include "sound/samples.h"
#include "includes/senjyo.h"


/* single tone generator */
#define SINGLE_LENGTH 10000
#define SINGLE_DIVIDER 8

static INT16 *_single;
static int single_rate = 0;
static int single_volume = 0;


const z80_daisy_chain senjyo_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ NULL }
};


/* z80 pio */

const z80pio_interface senjyo_pio_intf =
{
	DEVCB_CPU_INPUT_LINE("sub", INPUT_LINE_IRQ0),
	DEVCB_NULL,
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


WRITE8_HANDLER( senjyo_volume_w )
{
	const device_config *samples = devtag_get_device(space->machine, "samples");
	single_volume = data & 0x0f;
	sample_set_volume(samples,0,single_volume / 15.0);
}


static TIMER_CALLBACK( senjyo_sh_update )
{
	const device_config *samples = devtag_get_device(machine, "samples");

	/* ctc2 timer single tone generator frequency */
	attotime period = z80ctc_getperiod (devtag_get_device(machine, "z80ctc"), 2);
	if (attotime_compare(period, attotime_zero) != 0 )
		single_rate = ATTOSECONDS_TO_HZ(period.attoseconds);
	else
		single_rate = 0;

	sample_set_freq(samples, 0,single_rate);
}


SAMPLES_START( senjyo_sh_start )
{
	running_machine *machine = device->machine;
	int i;

	_single = auto_alloc_array(machine, INT16, SINGLE_LENGTH);

	for (i = 0;i < SINGLE_LENGTH;i++)		/* freq = ctc2 zco / 8 */
		_single[i] = ((i/SINGLE_DIVIDER)&0x01)*127*256;

	/* CTC2 single tone generator */
	single_rate = 1000;
	single_volume = 0;
	sample_set_volume(device,0,single_volume / 15.0);
	sample_start_raw(device,0,_single,SINGLE_LENGTH,single_rate,1);

	timer_pulse(machine, video_screen_get_frame_period(machine->primary_screen), NULL, 0, senjyo_sh_update);
}
