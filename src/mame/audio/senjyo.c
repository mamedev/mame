#include "driver.h"
#include "deprecat.h"
#include "sound/samples.h"
#include "includes/senjyo.h"


/* single tone generator */
#define SINGLE_LENGTH 10000
#define SINGLE_DIVIDER 8

static INT16 *_single;
static int single_rate = 1000;
static int single_volume = 0;


const z80_daisy_chain senjyo_daisy_chain[] =
{
	{ Z80CTC, "z80ctc" },
	{ Z80PIO, "z80pio" },
	{ NULL }
};


/* z80 pio */
static void daisy_interrupt(const device_config *device, int state)
{
	cputag_set_input_line(device->machine, "sub", 0, state);
}

const z80pio_interface senjyo_pio_intf =
{
	daisy_interrupt,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/* z80 ctc */
const z80ctc_interface senjyo_ctc_intf =
{
	"sub",		 	/* clock from sub CPU */
	0,               /* clock (filled in from the CPU 0 clock */
	NOTIMER_2,       /* timer disables */
	daisy_interrupt, /* interrupt handler */
	z80ctc_trg1_w, 	 /* ZC/TO0 callback */
	0,               /* ZC/TO1 callback */
	0                /* ZC/TO2 callback */
};


WRITE8_HANDLER( senjyo_volume_w )
{
	single_volume = data & 0x0f;
	sample_set_volume(0,single_volume / 15.0);
}


static TIMER_CALLBACK( senjyo_sh_update )
{
	/* ctc2 timer single tone generator frequency */
	attotime period = z80ctc_getperiod (devtag_get_device(machine, Z80CTC, "z80ctc"), 2);
	if (attotime_compare(period, attotime_zero) != 0 )
		single_rate = ATTOSECONDS_TO_HZ(period.attoseconds);
	else
		single_rate = 0;

	sample_set_freq(0,single_rate);
}


void senjyo_sh_start(void)
{
    int i;

	_single = (INT16 *)auto_malloc(SINGLE_LENGTH*2);

	for (i = 0;i < SINGLE_LENGTH;i++)		/* freq = ctc2 zco / 8 */
		_single[i] = ((i/SINGLE_DIVIDER)&0x01)*127*256;

	/* CTC2 single tone generator */
	sample_set_volume(0,0);
	sample_start_raw(0,_single,SINGLE_LENGTH,single_rate,1);

	timer_pulse(Machine, video_screen_get_frame_period(Machine->primary_screen), NULL, 0, senjyo_sh_update);
}
