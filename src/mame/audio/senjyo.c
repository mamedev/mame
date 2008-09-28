#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "sound/samples.h"


/* single tone generator */
#define SINGLE_LENGTH 10000
#define SINGLE_DIVIDER 8

static INT16 *_single;
static int single_rate = 1000;
static int single_volume = 0;
static const device_config *z80pio;


static void senjyo_z80pio_reset(int which)
{
	z80pio_reset( z80pio );
}

static int senjyo_z80pio_irq_state(int which)
{
	return z80pio_irq_state( z80pio );
}

static int senjyo_z80pio_irq_ack(int which)
{
	return z80pio_irq_ack( z80pio );
}

static void senjyo_z80pio_irq_reti(int which)
{
	z80pio_irq_reti( z80pio );
}

const struct z80_irq_daisy_chain senjyo_daisy_chain[] =
{
	{ z80ctc_reset, z80ctc_irq_state, z80ctc_irq_ack, z80ctc_irq_reti , 0 }, /* device 0 = CTC_0 , high priority */
	{ senjyo_z80pio_reset, senjyo_z80pio_irq_state, senjyo_z80pio_irq_ack, senjyo_z80pio_irq_reti , 0 }, /* device 1 = PIO_0 , low  priority */
	{ 0,0,0,0,-1} 	   /* end mark */
};


/* z80 pio */
static void pio_interrupt(running_machine *machine, int state)
{
	cpunum_set_input_line(machine, 1, 0, state);
}

const z80pio_interface senjyo_pio_intf =
{
	pio_interrupt,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/* z80 ctc */
static void ctc_interrupt (running_machine *machine, int state)
{
	cpunum_set_input_line(machine, 1, 0, state);
}

static z80ctc_interface ctc_intf =
{
	0,               /* clock (filled in from the CPU 0 clock */
	NOTIMER_2,       /* timer disables */
	ctc_interrupt,   /* interrupt handler */
	z80ctc_0_trg1_w, /* ZC/TO0 callback */
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
	attotime period = z80ctc_getperiod (0, 2);
	if (attotime_compare(period, attotime_zero) != 0 )
		single_rate = ATTOSECONDS_TO_HZ(period.attoseconds);
	else
		single_rate = 0;

	sample_set_freq(0,single_rate);
}


void senjyo_sh_start(void)
{
    int i;

	/* z80 ctc init */
	ctc_intf.baseclock = cpunum_get_clock(1);
	z80ctc_init (0, &ctc_intf);

	z80pio = device_list_find_by_tag( Machine->config->devicelist, Z80PIO, "z80pio" );

	_single = (INT16 *)auto_malloc(SINGLE_LENGTH*2);

	for (i = 0;i < SINGLE_LENGTH;i++)		/* freq = ctc2 zco / 8 */
		_single[i] = ((i/SINGLE_DIVIDER)&0x01)*127*256;

	/* CTC2 single tone generator */
	sample_set_volume(0,0);
	sample_start_raw(0,_single,SINGLE_LENGTH,single_rate,1);

	timer_pulse(video_screen_get_frame_period(Machine->primary_screen), NULL, 0, senjyo_sh_update);
}
