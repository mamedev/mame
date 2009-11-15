/***************************************************************************

    Cinemat/Leland driver

    Leland sound hardware
    driver by Aaron Giles and Paul Leaman

    -------------------------------------------------------------------

    1st generation sound hardware was controlled by the master Z80.
    It drove an AY-8910/AY-8912 pair for music. It also had two DACs
    that were driven by the video refresh. At the end of each scanline
    there are 8-bit DAC samples that can be enabled via the output
    ports on the AY-8910. The DACs run at a fixed frequency of 15.3kHz,
    since they are clocked once each scanline.

    -------------------------------------------------------------------

    2nd generation sound hardware was used in Redline Racer. It
    consisted of an 80186 microcontroller driving 8 8-bit DACs. The
    frequency of the DACs were controlled by one of 3 Intel 8254
    programmable interval timers (PITs):

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            3       8254 PIT 2 output 0
            4       8254 PIT 2 output 1
            5-7     8254 PIT 3 output 0

    The clock outputs for each DAC can be read, and are polled to
    determine when data should be updated on the chips. The 80186's
    two DMA channels are generally used to drive the first two DACs,
    with the remaining 6 DACs being fed manually via polling.

    -------------------------------------------------------------------

    3rd generation sound hardware appeared in the football games
    (Quarterback, AAFB) and the later games up through Pigout. This
    variant is closely based on the Redline Racer sound system, but
    they took out two of the DACs and replaced them with a higher
    resolution (10-bit) DAC. The driving clocks have been rearranged
    a bit, and the number of PITs reduced from 3 to 2:

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            3       8254 PIT 2 output 0
            4       8254 PIT 2 output 1
            5       8254 PIT 2 output 2
            10-bit  80186 timer 0

    Like the 2nd generation board, the first two DACs are driven via
    the DMA channels, and the remaining 5 DACs are polled.

    -------------------------------------------------------------------

    4th generation sound hardware showed up in Ataxx, Indy Heat, and
    World Soccer Finals. For this variant, they removed one more PIT
    and 3 of the 8-bit DACs, and added a YM2151 music chip and an
    externally-fed 8-bit DAC.

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            10-bit  80186 timer 0
            ext     80186 timer 1

    The externally driven DACs have registers for a start/stop address
    and triggers to control the clocking.

***************************************************************************/

#include "driver.h"
#include "streams.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "leland.h"
#include "sound/2151intf.h"


#define OUTPUT_RATE			50000


/*************************************
 *
 *  1st generation sound
 *
 *************************************/

#define DAC_BUFFER_SIZE		1024
#define DAC_BUFFER_MASK		(DAC_BUFFER_SIZE - 1)

static UINT8 *dac_buffer[2];
static UINT32 dac_bufin[2];
static UINT32 dac_bufout[2];

static sound_stream * dac_stream;

static STREAM_UPDATE( leland_update )
{
	stream_sample_t *buffer = outputs[0];
	int dacnum;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));
	for (dacnum = 0; dacnum < 2; dacnum++)
	{
		int bufout = dac_bufout[dacnum];
		int count = (dac_bufin[dacnum] - bufout) & DAC_BUFFER_MASK;

		if (count > 300)
		{
			UINT8 *base = dac_buffer[dacnum];
			int i;

			for (i = 0; i < samples && count > 0; i++, count--)
			{
				buffer[i] += ((INT16)base[bufout] - 0x80) * 0x40;
				bufout = (bufout + 1) & DAC_BUFFER_MASK;
			}
			dac_bufout[dacnum] = bufout;
		}
	}
}


static DEVICE_START( leland_sound )
{
	/* reset globals */
	dac_buffer[0] = dac_buffer[1] = NULL;
	dac_bufin[0]  = dac_bufin[1]  = 0;
	dac_bufout[0] = dac_bufout[1] = 0;

	/* allocate the stream */
	dac_stream = stream_create(device, 0, 1, 256*60, NULL, leland_update);

	/* allocate memory */
	dac_buffer[0] = auto_alloc_array(device->machine, UINT8, DAC_BUFFER_SIZE);
	dac_buffer[1] = auto_alloc_array(device->machine, UINT8, DAC_BUFFER_SIZE);
}


DEVICE_GET_INFO( leland_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(leland_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Leland DAC");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


void leland_dac_update(int dacnum, UINT8 sample)
{
	UINT8 *buffer = dac_buffer[dacnum];
	int bufin = dac_bufin[dacnum];

	/* skip if nothing */
	if (!buffer)
		return;

	/* copy data from VRAM */
	buffer[bufin] = sample;
	bufin = (bufin + 1) & DAC_BUFFER_MASK;

	/* update the buffer */
	dac_bufin[dacnum] = bufin;
}



/*************************************
 *
 *  2nd-4th generation sound
 *
 *************************************/

#define LOG_INTERRUPTS		0
#define LOG_DMA				0
#define LOG_SHORTAGES		0
#define LOG_TIMER			0
#define LOG_COMM			0
#define LOG_PORTS			0
#define LOG_DAC				0
#define LOG_EXTERN			0
#define LOG_PIT				0


/* according to the Intel manual, external interrupts are not latched */
/* however, I cannot get this system to work without latching them */
#define LATCH_INTS	1

#define DAC_VOLUME_SCALE	4


static sound_stream * dma_stream;
static sound_stream * nondma_stream;
static sound_stream * extern_stream;

static UINT8 has_ym2151;
static UINT8 is_redline;

static UINT8 last_control;
static UINT8 clock_active;
static UINT8 clock_tick;

static UINT16 sound_command;
static UINT8 sound_response;

static UINT32 ext_start;
static UINT32 ext_stop;
static UINT8 ext_active;
static UINT8 *ext_base;

struct mem_state
{
	UINT16	lower;
	UINT16	upper;
	UINT16	middle;
	UINT16	middle_size;
	UINT16	peripheral;
};

struct timer_state
{
	UINT16	control;
	UINT16	maxA;
	UINT16	maxB;
	UINT16	count;
	emu_timer *int_timer;
	emu_timer *time_timer;
	UINT8	time_timer_active;
	attotime last_time;
};

struct dma_state
{
	UINT32	source;
	UINT32	dest;
	UINT16	count;
	UINT16	control;
	UINT8	finished;
	emu_timer *finish_timer;
};

struct intr_state
{
	UINT8	pending;
	UINT16	ack_mask;
	UINT16	priority_mask;
	UINT16	in_service;
	UINT16	request;
	UINT16	status;
	UINT16	poll_status;
	UINT16	timer;
	UINT16	dma[2];
	UINT16	ext[4];
};

static struct i80186_state
{
	struct timer_state	timer[3];
	struct dma_state	dma[2];
	struct intr_state	intr;
	struct mem_state	mem;
} i80186;


#define DAC_BUFFER_SIZE			1024
#define DAC_BUFFER_SIZE_MASK	(DAC_BUFFER_SIZE - 1)
static struct dac_state
{
	INT16	value;
	INT16	volume;
	UINT32	frequency;
	UINT32	step;
	UINT32	fraction;

	INT16	buffer[DAC_BUFFER_SIZE];
	UINT32	bufin;
	UINT32	bufout;
	UINT32	buftarget;
} dac[8];

static struct counter_state
{
	emu_timer *timer;
	INT32 count;
	UINT8 mode;
	UINT8 readbyte;
	UINT8 writebyte;
} counter[9];

static void set_dac_frequency(int which, int frequency);

static READ16_HANDLER( peripheral_r );
static WRITE16_HANDLER( peripheral_w );



/*************************************
 *
 *  Manual DAC sound generation
 *
 *************************************/

static STREAM_UPDATE( leland_80186_dac_update )
{
	stream_sample_t *buffer = outputs[0];
	int i, j, start, stop;

	if (LOG_SHORTAGES) logerror("----\n");

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* if we're redline racer, we have more DACs */
	if (!is_redline)
		start = 2, stop = 7;
	else
		start = 0, stop = 8;

	/* loop over manual DAC channels */
	for (i = start; i < stop; i++)
	{
		struct dac_state *d = &dac[i];
		int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

		/* if we have data, process it */
		if (count > 0)
		{
			INT16 *base = d->buffer;
			int source = d->bufout;
			int frac = d->fraction;
			int step = d->step;

			/* sample-rate convert to the output frequency */
			for (j = 0; j < samples && count > 0; j++)
			{
				buffer[j] += base[source];
				frac += step;
				source += frac >> 24;
				count -= frac >> 24;
				frac &= 0xffffff;
				source &= DAC_BUFFER_SIZE_MASK;
			}

			if (LOG_SHORTAGES && j < samples)
				logerror("DAC #%d short by %d/%d samples\n", i, samples - j, samples);

			/* update the DAC state */
			d->fraction = frac;
			d->bufout = source;
		}

		/* update the clock status */
		if (count < d->buftarget)
			clock_active |= 1 << i;
	}
}



/*************************************
 *
 *  DMA-based DAC sound generation
 *
 *************************************/

static STREAM_UPDATE( leland_80186_dma_update )
{
	const address_space *dmaspace = (const address_space *)param;
	stream_sample_t *buffer = outputs[0];
	int i, j;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* loop over DMA buffers */
	for (i = 0; i < 2; i++)
	{
		struct dma_state *d = &i80186.dma[i];

		/* check for enabled DMA */
		if (d->control & 0x0002)
		{
			/* make sure the parameters meet our expectations */
			if ((d->control & 0xfe00) != 0x1600)
			{
				logerror("Unexpected DMA control %02X\n", d->control);
			}
			else if (!is_redline && ((d->dest & 1) || (d->dest & 0x3f) > 0x0b))
			{
				logerror("Unexpected DMA destination %02X\n", d->dest);
			}
			else if (is_redline && (d->dest & 0xf000) != 0x4000 && (d->dest & 0xf000) != 0x5000)
			{
				logerror("Unexpected DMA destination %02X\n", d->dest);
			}

			/* otherwise, we're ready for liftoff */
			else
			{
				int source = d->source;
				int count = d->count;
				int which, frac, step, volume;

				/* adjust for redline racer */
				if (!is_redline)
					which = (d->dest & 0x3f) / 2;
				else
					which = (d->dest >> 9) & 7;

				frac = dac[which].fraction;
				step = dac[which].step;
				volume = dac[which].volume;

				/* sample-rate convert to the output frequency */
				for (j = 0; j < samples && count > 0; j++)
				{
					buffer[j] += ((int)memory_read_byte(dmaspace, source) - 0x80) * volume;
					frac += step;
					source += frac >> 24;
					count -= frac >> 24;
					frac &= 0xffffff;
				}

				/* update the DMA state */
				if (count > 0)
				{
					d->source = source;
					d->count = count;
				}
				else
				{
					/* let the timer callback actually mark the transfer finished */
					d->source = source + count - 1;
					d->count = 1;
					d->finished = 1;
				}

				if (LOG_DMA) logerror("DMA Generated %d samples - new count = %04X, source = %04X\n", j, d->count, d->source);

				/* update the DAC state */
				dac[which].fraction = frac;
			}
		}
	}
}



/*************************************
 *
 *  Externally-driven DAC sound generation
 *
 *************************************/

static STREAM_UPDATE( leland_80186_extern_update )
{
	stream_sample_t *buffer = outputs[0];
	struct dac_state *d = &dac[7];
	int count = ext_stop - ext_start;
	int j;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* if we have data, process it */
	if (count > 0 && ext_active)
	{
		int source = ext_start;
		int frac = d->fraction;
		int step = d->step;

		/* sample-rate convert to the output frequency */
		for (j = 0; j < samples && count > 0; j++)
		{
			buffer[j] += ((INT16)ext_base[source] - 0x80) * d->volume;
			frac += step;
			source += frac >> 24;
			count -= frac >> 24;
			frac &= 0xffffff;
		}

		/* update the DAC state */
		d->fraction = frac;
		ext_start = source;
	}
}



/*************************************
 *
 *  Sound initialization
 *
 *************************************/

static TIMER_CALLBACK( internal_timer_int );
static TIMER_CALLBACK( dma_timer_callback );

static DEVICE_START( common_sh_start )
{
	running_machine *machine = device->machine;
	const address_space *dmaspace = memory_find_address_space(cputag_get_cpu(machine, "audiocpu"), ADDRESS_SPACE_PROGRAM);
	int i;

	/* determine which sound hardware is installed */
	has_ym2151 = (devtag_get_device(device->machine, "ymsnd") != NULL);

	/* allocate separate streams for the DMA and non-DMA DACs */
	dma_stream = stream_create(device, 0, 1, OUTPUT_RATE, (void *)dmaspace, leland_80186_dma_update);
	nondma_stream = stream_create(device, 0, 1, OUTPUT_RATE, NULL, leland_80186_dac_update);

	/* if we have a 2151, install an externally driven DAC stream */
	if (has_ym2151)
	{
		ext_base = memory_region(machine, "dac");
		extern_stream = stream_create(device, 0, 1, OUTPUT_RATE, NULL, leland_80186_extern_update);
	}

	/* create timers here so they stick around */
	i80186.timer[0].int_timer = timer_alloc(machine, internal_timer_int, NULL);
	i80186.timer[1].int_timer = timer_alloc(machine, internal_timer_int, NULL);
	i80186.timer[2].int_timer = timer_alloc(machine, internal_timer_int, NULL);
	i80186.timer[0].time_timer = timer_alloc(machine, NULL, NULL);
	i80186.timer[1].time_timer = timer_alloc(machine, NULL, NULL);
	i80186.timer[2].time_timer = timer_alloc(machine, NULL, NULL);
	i80186.dma[0].finish_timer = timer_alloc(machine, dma_timer_callback, NULL);
	i80186.dma[1].finish_timer = timer_alloc(machine, dma_timer_callback, NULL);

	for (i = 0; i < 9; i++)
		counter[i].timer = timer_alloc(machine, NULL, NULL);
}

static DEVICE_START( leland_80186_sound )
{
	is_redline = 0;
	DEVICE_START_CALL(common_sh_start);
}

static DEVICE_START( redline_80186_sound )
{
	is_redline = 1;
	DEVICE_START_CALL(common_sh_start);
}


DEVICE_GET_INFO( leland_80186_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(leland_80186_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Leland 80186 DAC");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEVICE_GET_INFO( redline_80186_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(redline_80186_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Redline Racer 80186 DAC");		break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


static void leland_80186_reset(void)
{
	struct i80186_state oldstate = i80186;
	emu_timer *counter_timer[9];
	int i;

	/* reset the i80186 state, but save the timers */
	memset(&i80186, 0, sizeof(i80186));
	i80186.timer[0].int_timer = oldstate.timer[0].int_timer;
	i80186.timer[1].int_timer = oldstate.timer[1].int_timer;
	i80186.timer[2].int_timer = oldstate.timer[2].int_timer;
	i80186.timer[0].time_timer = oldstate.timer[0].time_timer;
	i80186.timer[1].time_timer = oldstate.timer[1].time_timer;
	i80186.timer[2].time_timer = oldstate.timer[2].time_timer;
	i80186.dma[0].finish_timer = oldstate.dma[0].finish_timer;
	i80186.dma[1].finish_timer = oldstate.dma[1].finish_timer;

	/* reset the interrupt state */
	i80186.intr.priority_mask	= 0x0007;
	i80186.intr.timer 			= 0x000f;
	i80186.intr.dma[0]			= 0x000f;
	i80186.intr.dma[1]			= 0x000f;
	i80186.intr.ext[0]			= 0x000f;
	i80186.intr.ext[1]			= 0x000f;
	i80186.intr.ext[2]			= 0x000f;
	i80186.intr.ext[3]			= 0x000f;

	/* reset the DAC and counter states as well */
	memset(&dac, 0, sizeof(dac));
	for (i = 0; i < 9; i++)
		counter_timer[i] = counter[i].timer;
	memset(&counter, 0, sizeof(counter));
	for (i = 0; i < 9; i++)
		counter[i].timer = counter_timer[i];
}


void leland_80186_sound_init(void)
{
	/* reset the 80186 registers */
	leland_80186_reset();

	/* reset our internal stuff */
	last_control = 0xf8;
	clock_active = 0;

	/* reset the external DAC */
	ext_start = 0;
	ext_stop = 0;
	ext_active = 0;
}



/*************************************
 *
 *  80186 interrupt controller
 *
 *************************************/

static IRQ_CALLBACK( int_callback )
{
	if (LOG_INTERRUPTS) logerror("(%f) **** Acknowledged interrupt vector %02X\n", attotime_to_double(timer_get_time(device->machine)), i80186.intr.poll_status & 0x1f);

	/* clear the interrupt */
	cpu_set_info(device, CPUINFO_INT_INPUT_STATE + 0, CLEAR_LINE);
	i80186.intr.pending = 0;

	/* clear the request and set the in-service bit */
#if LATCH_INTS
	i80186.intr.request &= ~i80186.intr.ack_mask;
#else
	i80186.intr.request &= ~(i80186.intr.ack_mask & 0x0f);
#endif
	i80186.intr.in_service |= i80186.intr.ack_mask;
	if (i80186.intr.ack_mask == 0x0001)
	{
		switch (i80186.intr.poll_status & 0x1f)
		{
			case 0x08:	i80186.intr.status &= ~0x01;	break;
			case 0x12:	i80186.intr.status &= ~0x02;	break;
			case 0x13:	i80186.intr.status &= ~0x04;	break;
		}
	}
	i80186.intr.ack_mask = 0;

	/* a request no longer pending */
	i80186.intr.poll_status &= ~0x8000;

	/* return the vector */
	return i80186.intr.poll_status & 0x1f;
}


static void update_interrupt_state(running_machine *machine)
{
	int i, j, new_vector = 0;

	if (LOG_INTERRUPTS) logerror("update_interrupt_status: req=%02X stat=%02X serv=%02X\n", i80186.intr.request, i80186.intr.status, i80186.intr.in_service);

	/* loop over priorities */
	for (i = 0; i <= i80186.intr.priority_mask; i++)
	{
		/* note: by checking 4 bits, we also verify that the mask is off */
		if ((i80186.intr.timer & 15) == i)
		{
			/* if we're already servicing something at this level, don't generate anything new */
			if (i80186.intr.in_service & 0x01)
				return;

			/* if there's something pending, generate an interrupt */
			if (i80186.intr.status & 0x07)
			{
				if (i80186.intr.status & 1)
					new_vector = 0x08;
				else if (i80186.intr.status & 2)
					new_vector = 0x12;
				else if (i80186.intr.status & 4)
					new_vector = 0x13;
				else
					popmessage("Invalid timer interrupt!");

				/* set the clear mask and generate the int */
				i80186.intr.ack_mask = 0x0001;
				goto generate_int;
			}
		}

		/* check DMA interrupts */
		for (j = 0; j < 2; j++)
			if ((i80186.intr.dma[j] & 15) == i)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (i80186.intr.in_service & (0x04 << j))
					return;

				/* if there's something pending, generate an interrupt */
				if (i80186.intr.request & (0x04 << j))
				{
					new_vector = 0x0a + j;

					/* set the clear mask and generate the int */
					i80186.intr.ack_mask = 0x0004 << j;
					goto generate_int;
				}
			}

		/* check external interrupts */
		for (j = 0; j < 4; j++)
			if ((i80186.intr.ext[j] & 15) == i)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (i80186.intr.in_service & (0x10 << j))
					return;

				/* if there's something pending, generate an interrupt */
				if (i80186.intr.request & (0x10 << j))
				{
					/* otherwise, generate an interrupt for this request */
					new_vector = 0x0c + j;

					/* set the clear mask and generate the int */
					i80186.intr.ack_mask = 0x0010 << j;
					goto generate_int;
				}
			}
	}
	return;

generate_int:
	/* generate the appropriate interrupt */
	i80186.intr.poll_status = 0x8000 | new_vector;
	if (!i80186.intr.pending)
		cputag_set_input_line(machine, "audiocpu", 0, ASSERT_LINE);
	i80186.intr.pending = 1;
	if (LOG_INTERRUPTS) logerror("(%f) **** Requesting interrupt vector %02X\n", attotime_to_double(timer_get_time(machine)), new_vector);
}


static void handle_eoi(running_machine *machine, int data)
{
	int i, j;

	/* specific case */
	if (!(data & 0x8000))
	{
		/* turn off the appropriate in-service bit */
		switch (data & 0x1f)
		{
			case 0x08:	i80186.intr.in_service &= ~0x01;	break;
			case 0x12:	i80186.intr.in_service &= ~0x01;	break;
			case 0x13:	i80186.intr.in_service &= ~0x01;	break;
			case 0x0a:	i80186.intr.in_service &= ~0x04;	break;
			case 0x0b:	i80186.intr.in_service &= ~0x08;	break;
			case 0x0c:	i80186.intr.in_service &= ~0x10;	break;
			case 0x0d:	i80186.intr.in_service &= ~0x20;	break;
			case 0x0e:	i80186.intr.in_service &= ~0x40;	break;
			case 0x0f:	i80186.intr.in_service &= ~0x80;	break;
			default:	logerror("%s:ERROR - 80186 EOI with unknown vector %02X\n", cpuexec_describe_context(machine), data & 0x1f);
		}
		if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for vector %02X\n", attotime_to_double(timer_get_time(machine)), data & 0x1f);
	}

	/* non-specific case */
	else
	{
		/* loop over priorities */
		for (i = 0; i <= 7; i++)
		{
			/* check for in-service timers */
			if ((i80186.intr.timer & 7) == i && (i80186.intr.in_service & 0x01))
			{
				i80186.intr.in_service &= ~0x01;
				if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for timer\n", attotime_to_double(timer_get_time(machine)));
				return;
			}

			/* check for in-service DMA interrupts */
			for (j = 0; j < 2; j++)
				if ((i80186.intr.dma[j] & 7) == i && (i80186.intr.in_service & (0x04 << j)))
				{
					i80186.intr.in_service &= ~(0x04 << j);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for DMA%d\n", attotime_to_double(timer_get_time(machine)), j);
					return;
				}

			/* check external interrupts */
			for (j = 0; j < 4; j++)
				if ((i80186.intr.ext[j] & 7) == i && (i80186.intr.in_service & (0x10 << j)))
				{
					i80186.intr.in_service &= ~(0x10 << j);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for INT%d\n", attotime_to_double(timer_get_time(machine)), j);
					return;
				}
		}
	}
}



/*************************************
 *
 *  80186 internal timers
 *
 *************************************/

static TIMER_CALLBACK( internal_timer_int )
{
	int which = param;
	struct timer_state *t = &i80186.timer[which];

	if (LOG_TIMER) logerror("Hit interrupt callback for timer %d\n", which);

	/* set the max count bit */
	t->control |= 0x0020;

	/* request an interrupt */
	if (t->control & 0x2000)
	{
		i80186.intr.status |= 0x01 << which;
		update_interrupt_state(machine);
		if (LOG_TIMER) logerror("  Generating timer interrupt\n");
	}

	/* if we're continuous, reset */
	if (t->control & 0x0001)
	{
		int count = t->maxA ? t->maxA : 0x10000;
		timer_adjust_oneshot(t->int_timer, attotime_mul(ATTOTIME_IN_HZ(2000000), count), which);
		if (LOG_TIMER) logerror("  Repriming interrupt\n");
	}
	else
		timer_adjust_oneshot(t->int_timer, attotime_never, which);
}


static void internal_timer_sync(int which)
{
	struct timer_state *t = &i80186.timer[which];

	/* if we have a timing timer running, adjust the count */
	if (t->time_timer_active)
	{
		attotime current_time = timer_timeelapsed(t->time_timer);
		int net_clocks = attotime_to_double(attotime_mul(attotime_sub(current_time, t->last_time), 2000000));
		t->last_time = current_time;

		/* set the max count bit if we passed the max */
		if ((int)t->count + net_clocks >= t->maxA)
			t->control |= 0x0020;

		/* set the new count */
		if (t->maxA != 0)
			t->count = (t->count + net_clocks) % t->maxA;
		else
			t->count = t->count + net_clocks;
	}
}


static void internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control)
{
	struct timer_state *t = &i80186.timer[which];
	int update_int_timer = 0;

	/* if we have a new count and we're on, update things */
	if (new_count != -1)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = 1;
		}
		t->count = new_count;
	}

	/* if we have a new max and we're on, update things */
	if (new_maxA != -1 && new_maxA != t->maxA)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = 1;
		}
		t->maxA = new_maxA;
		if (new_maxA == 0) new_maxA = 0x10000;

		/* redline racer controls nothing externally? */
		if (is_redline)
			;

		/* on the common board, timer 0 controls the 10-bit DAC frequency */
		else if (which == 0)
			set_dac_frequency(6, 2000000 / new_maxA);

		/* timer 1 controls the externally driven DAC on Indy Heat/WSF */
		else if (which == 1 && has_ym2151)
			set_dac_frequency(7, 2000000 / (new_maxA * 2));
	}

	/* if we have a new max and we're on, update things */
	if (new_maxB != -1 && new_maxB != t->maxB)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = 1;
		}
		t->maxB = new_maxB;
		if (new_maxB == 0) new_maxB = 0x10000;

		/* timer 1 controls the externally driven DAC on Indy Heat/WSF */
		/* they alternate the use of maxA and maxB in a way that makes no */
		/* sense according to the 80186 documentation! */
		if (which == 1 && has_ym2151)
			set_dac_frequency(7, 2000000 / (new_maxB * 2));
	}

	/* handle control changes */
	if (new_control != -1)
	{
		int diff;

		/* merge back in the bits we don't modify */
		new_control = (new_control & ~0x1fc0) | (t->control & 0x1fc0);

		/* handle the /INH bit */
		if (!(new_control & 0x4000))
			new_control = (new_control & ~0x8000) | (t->control & 0x8000);
		new_control &= ~0x4000;

		/* check for control bits we don't handle */
		diff = new_control ^ t->control;
		if (diff & 0x001c)
			logerror("ERROR! - unsupported timer mode %04X\n", new_control);

		/* if we have real changes, update things */
		if (diff != 0)
		{
			/* if we're going off, make sure our timers are gone */
			if ((diff & 0x8000) && !(new_control & 0x8000))
			{
				/* compute the final count */
				internal_timer_sync(which);

				/* nuke the timer and force the interrupt timer to be recomputed */
				timer_adjust_oneshot(t->time_timer, attotime_never, which);
				t->time_timer_active = 0;
				update_int_timer = 1;
			}

			/* if we're going on, start the timers running */
			else if ((diff & 0x8000) && (new_control & 0x8000))
			{
				/* start the timing */
				timer_adjust_oneshot(t->time_timer, attotime_never, which);
				t->time_timer_active = 1;
				update_int_timer = 1;
			}

			/* if something about the interrupt timer changed, force an update */
			if (!(diff & 0x8000) && (diff & 0x2000))
			{
				internal_timer_sync(which);
				update_int_timer = 1;
			}
		}

		/* set the new control register */
		t->control = new_control;
	}

	/* update the interrupt timer */

	/* kludge: the YM2151 games sometimes crank timer 1 really high, and leave interrupts */
	/* enabled, even though the handler for timer 1 does nothing. To alleviate this, we */
	/* just ignore it */
	if (!has_ym2151 || which != 1)
		if (update_int_timer)
		{
			if ((t->control & 0x8000) && (t->control & 0x2000))
			{
				int diff = t->maxA - t->count;
				if (diff <= 0) diff += 0x10000;
				timer_adjust_oneshot(t->int_timer, attotime_mul(ATTOTIME_IN_HZ(2000000), diff), which);
				if (LOG_TIMER) logerror("Set interrupt timer for %d\n", which);
			}
			else
				timer_adjust_oneshot(t->int_timer, attotime_never, which);
		}
}



/*************************************
 *
 *  80186 internal DMA
 *
 *************************************/

static TIMER_CALLBACK( dma_timer_callback )
{
	int which = param;
	struct dma_state *d = &i80186.dma[which];

	/* force an update and see if we're really done */
	stream_update(dma_stream);

	/* complete the status update */
	d->control &= ~0x0002;
	d->source += d->count;
	d->count = 0;

	/* check for interrupt generation */
	if (d->control & 0x0100)
	{
		if (LOG_DMA) logerror("DMA%d timer callback - requesting interrupt: count = %04X, source = %04X\n", which, d->count, d->source);
		i80186.intr.request |= 0x04 << which;
		update_interrupt_state(machine);
	}
}


static void update_dma_control(int which, int new_control)
{
	struct dma_state *d = &i80186.dma[which];
	int diff;

	/* handle the CHG bit */
	if (!(new_control & 0x0004))
		new_control = (new_control & ~0x0002) | (d->control & 0x0002);
	new_control &= ~0x0004;

	/* check for control bits we don't handle */
	diff = new_control ^ d->control;
	if (diff & 0x6811)
		logerror("ERROR! - unsupported DMA mode %04X\n", new_control);

	/* if we're going live, set a timer */
	if ((diff & 0x0002) && (new_control & 0x0002))
	{
		/* make sure the parameters meet our expectations */
		if ((new_control & 0xfe00) != 0x1600)
		{
			logerror("Unexpected DMA control %02X\n", new_control);
		}
		else if (!is_redline && ((d->dest & 1) || (d->dest & 0x3f) > 0x0b))
		{
			logerror("Unexpected DMA destination %02X\n", d->dest);
		}
		else if (is_redline && (d->dest & 0xf000) != 0x4000 && (d->dest & 0xf000) != 0x5000)
		{
			logerror("Unexpected DMA destination %02X\n", d->dest);
		}

		/* otherwise, set a timer */
		else
		{
			int count = d->count;
			int dacnum;

			/* adjust for redline racer */
			if (!is_redline)
				dacnum = (d->dest & 0x3f) / 2;
			else
			{
				dacnum = (d->dest >> 9) & 7;
				dac[dacnum].volume = (d->dest & 0x1fe) / 2 / DAC_VOLUME_SCALE;
			}

			if (LOG_DMA) logerror("Initiated DMA %d - count = %04X, source = %04X, dest = %04X\n", which, d->count, d->source, d->dest);

			d->finished = 0;
			timer_adjust_oneshot(d->finish_timer, attotime_mul(ATTOTIME_IN_HZ(dac[dacnum].frequency), count), which);
		}
	}

	/* set the new control register */
	d->control = new_control;
}



/*************************************
 *
 *  80186 internal I/O reads
 *
 *************************************/

static READ16_HANDLER( i80186_internal_port_r )
{
	int temp, which;

	switch (offset)
	{
		case 0x22/2:
			logerror("%05X:ERROR - read from 80186 EOI\n", cpu_get_pc(space->cpu));
			break;

		case 0x24/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll\n", cpu_get_pc(space->cpu));
			if (i80186.intr.poll_status & 0x8000)
				int_callback(space->cpu, 0);
			return i80186.intr.poll_status;

		case 0x26/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll status\n", cpu_get_pc(space->cpu));
			return i80186.intr.poll_status;

		case 0x28/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt mask\n", cpu_get_pc(space->cpu));
			temp  = (i80186.intr.timer  >> 3) & 0x01;
			temp |= (i80186.intr.dma[0] >> 1) & 0x04;
			temp |= (i80186.intr.dma[1] >> 0) & 0x08;
			temp |= (i80186.intr.ext[0] << 1) & 0x10;
			temp |= (i80186.intr.ext[1] << 2) & 0x20;
			temp |= (i80186.intr.ext[2] << 3) & 0x40;
			temp |= (i80186.intr.ext[3] << 4) & 0x80;
			return temp;

		case 0x2a/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt priority mask\n", cpu_get_pc(space->cpu));
			return i80186.intr.priority_mask;

		case 0x2c/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt in-service\n", cpu_get_pc(space->cpu));
			return i80186.intr.in_service;

		case 0x2e/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt request\n", cpu_get_pc(space->cpu));
			temp = i80186.intr.request & ~0x0001;
			if (i80186.intr.status & 0x0007)
				temp |= 1;
			return temp;

		case 0x30/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt status\n", cpu_get_pc(space->cpu));
			return i80186.intr.status;

		case 0x32/2:
			if (LOG_PORTS) logerror("%05X:read 80186 timer interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.timer;

		case 0x34/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 0 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.dma[0];

		case 0x36/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 1 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.dma[1];

		case 0x38/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 0 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.ext[0];

		case 0x3a/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 1 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.ext[1];

		case 0x3c/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 2 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.ext[2];

		case 0x3e/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 3 interrupt control\n", cpu_get_pc(space->cpu));
			return i80186.intr.ext[3];

		case 0x50/2:
		case 0x58/2:
		case 0x60/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d count\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			if (ACCESSING_BITS_0_7)
				internal_timer_sync(which);
			return i80186.timer[which].count;

		case 0x52/2:
		case 0x5a/2:
		case 0x62/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d max A\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			return i80186.timer[which].maxA;

		case 0x54/2:
		case 0x5c/2:
			logerror("%05X:read 80186 Timer %d max B\n", cpu_get_pc(space->cpu), (offset/2 - 0x50) / 4);
			which = (offset - 0x50/2) / 4;
			return i80186.timer[which].maxB;

		case 0x56/2:
		case 0x5e/2:
		case 0x66/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d control\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			return i80186.timer[which].control;

		case 0xa0/2:
			if (LOG_PORTS) logerror("%05X:read 80186 upper chip select\n", cpu_get_pc(space->cpu));
			return i80186.mem.upper;

		case 0xa2/2:
			if (LOG_PORTS) logerror("%05X:read 80186 lower chip select\n", cpu_get_pc(space->cpu));
			return i80186.mem.lower;

		case 0xa4/2:
			if (LOG_PORTS) logerror("%05X:read 80186 peripheral chip select\n", cpu_get_pc(space->cpu));
			return i80186.mem.peripheral;

		case 0xa6/2:
			if (LOG_PORTS) logerror("%05X:read 80186 middle chip select\n", cpu_get_pc(space->cpu));
			return i80186.mem.middle;

		case 0xa8/2:
			if (LOG_PORTS) logerror("%05X:read 80186 middle P chip select\n", cpu_get_pc(space->cpu));
			return i80186.mem.middle_size;

		case 0xc0/2:
		case 0xd0/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower source address\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].source;

		case 0xc2/2:
		case 0xd2/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper source address\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].source >> 16;

		case 0xc4/2:
		case 0xd4/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower dest address\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].dest;

		case 0xc6/2:
		case 0xd6/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper dest address\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].dest >> 16;

		case 0xc8/2:
		case 0xd8/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d transfer count\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].count;

		case 0xca/2:
		case 0xda/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d control\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			return i80186.dma[which].control;

		default:
			logerror("%05X:read 80186 port %02X\n", cpu_get_pc(space->cpu), offset*2);
			break;
	}
	return 0x00;
}



/*************************************
 *
 *  80186 internal I/O writes
 *
 *************************************/

static WRITE16_HANDLER( i80186_internal_port_w )
{
	int temp, which;

	/* handle partials */
	if (!ACCESSING_BITS_8_15)
		data = (i80186_internal_port_r(space, offset, 0xff00) & 0xff00) | (data & 0x00ff);
	else if (!ACCESSING_BITS_0_7)
		data = (i80186_internal_port_r(space, offset, 0x00ff) & 0x00ff) | (data & 0xff00);

	switch (offset)
	{
		case 0x22/2:
			if (LOG_PORTS) logerror("%05X:80186 EOI = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			handle_eoi(space->machine, 0x8000);
			update_interrupt_state(space->machine);
			break;

		case 0x24/2:
			logerror("%05X:ERROR - write to 80186 interrupt poll = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			break;

		case 0x26/2:
			logerror("%05X:ERROR - write to 80186 interrupt poll status = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			break;

		case 0x28/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt mask = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.timer  = (i80186.intr.timer  & ~0x08) | ((data << 3) & 0x08);
			i80186.intr.dma[0] = (i80186.intr.dma[0] & ~0x08) | ((data << 1) & 0x08);
			i80186.intr.dma[1] = (i80186.intr.dma[1] & ~0x08) | ((data << 0) & 0x08);
			i80186.intr.ext[0] = (i80186.intr.ext[0] & ~0x08) | ((data >> 1) & 0x08);
			i80186.intr.ext[1] = (i80186.intr.ext[1] & ~0x08) | ((data >> 2) & 0x08);
			i80186.intr.ext[2] = (i80186.intr.ext[2] & ~0x08) | ((data >> 3) & 0x08);
			i80186.intr.ext[3] = (i80186.intr.ext[3] & ~0x08) | ((data >> 4) & 0x08);
			update_interrupt_state(space->machine);
			break;

		case 0x2a/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt priority mask = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.priority_mask = data & 0x0007;
			update_interrupt_state(space->machine);
			break;

		case 0x2c/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt in-service = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.in_service = data & 0x00ff;
			update_interrupt_state(space->machine);
			break;

		case 0x2e/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt request = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.request = (i80186.intr.request & ~0x00c0) | (data & 0x00c0);
			update_interrupt_state(space->machine);
			break;

		case 0x30/2:
			if (LOG_PORTS) logerror("%05X:WARNING - wrote to 80186 interrupt status = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.status = (i80186.intr.status & ~0x8000) | (data & 0x8000);
			i80186.intr.status = (i80186.intr.status & ~0x0007) | (data & 0x0007);
			update_interrupt_state(space->machine);
			break;

		case 0x32/2:
			if (LOG_PORTS) logerror("%05X:80186 timer interrupt contol = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.timer = data & 0x000f;
			break;

		case 0x34/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA 0 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.dma[0] = data & 0x000f;
			break;

		case 0x36/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA 1 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.dma[1] = data & 0x000f;
			break;

		case 0x38/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 0 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.ext[0] = data & 0x007f;
			break;

		case 0x3a/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 1 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.ext[1] = data & 0x007f;
			break;

		case 0x3c/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 2 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.ext[2] = data & 0x001f;
			break;

		case 0x3e/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 3 interrupt control = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.intr.ext[3] = data & 0x001f;
			break;

		case 0x50/2:
		case 0x58/2:
		case 0x60/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d count = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(which, data, -1, -1, -1);
			break;

		case 0x52/2:
		case 0x5a/2:
		case 0x62/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max A = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(which, -1, data, -1, -1);
			break;

		case 0x54/2:
		case 0x5c/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max B = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(which, -1, -1, data, -1);
			break;

		case 0x56/2:
		case 0x5e/2:
		case 0x66/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d control = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(which, -1, -1, -1, data);
			break;

		case 0xa0/2:
			if (LOG_PORTS) logerror("%05X:80186 upper chip select = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.mem.upper = data | 0xc038;
			break;

		case 0xa2/2:
			if (LOG_PORTS) logerror("%05X:80186 lower chip select = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.mem.lower = (data & 0x3fff) | 0x0038;
			break;

		case 0xa4/2:
			if (LOG_PORTS) logerror("%05X:80186 peripheral chip select = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.mem.peripheral = data | 0x0038;
			break;

		case 0xa6/2:
			if (LOG_PORTS) logerror("%05X:80186 middle chip select = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.mem.middle = data | 0x01f8;
			break;

		case 0xa8/2:
			if (LOG_PORTS) logerror("%05X:80186 middle P chip select = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);
			i80186.mem.middle_size = data | 0x8038;

			temp = (i80186.mem.peripheral & 0xffc0) << 4;
			if (i80186.mem.middle_size & 0x0040)
				memory_install_readwrite16_handler(cpu_get_address_space(space->cpu, ADDRESS_SPACE_PROGRAM), temp, temp + 0x2ff, 0, 0, peripheral_r, peripheral_w);
			else
			{
				temp &= 0xffff;
				memory_install_readwrite16_handler(cpu_get_address_space(space->cpu, ADDRESS_SPACE_IO), temp, temp + 0x2ff, 0, 0, peripheral_r, peripheral_w);
			}

			/* we need to do this at a time when the 80186 context is swapped in */
			/* this register is generally set once at startup and never again, so it's a good */
			/* time to set it up */
			cpu_set_irq_callback(space->cpu, int_callback);
			break;

		case 0xc0/2:
		case 0xd0/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower source address = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			i80186.dma[which].source = (i80186.dma[which].source & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0xc2/2:
		case 0xd2/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper source address = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			i80186.dma[which].source = (i80186.dma[which].source & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0xc4/2:
		case 0xd4/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower dest address = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			i80186.dma[which].dest = (i80186.dma[which].dest & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0xc6/2:
		case 0xd6/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper dest address = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			i80186.dma[which].dest = (i80186.dma[which].dest & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0xc8/2:
		case 0xd8/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d transfer count = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			i80186.dma[which].count = data;
			break;

		case 0xca/2:
		case 0xda/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d control = %04X & %04X\n", cpu_get_pc(space->cpu), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			stream_update(dma_stream);
			update_dma_control(which, data);
			break;

		case 0xfe/2:
			if (LOG_PORTS) logerror("%05X:80186 relocation register = %04X & %04X\n", cpu_get_pc(space->cpu), data, mem_mask);

			/* we assume here there that this doesn't happen too often */
			/* plus, we can't really remove the old memory range, so we also assume that it's */
			/* okay to leave us mapped where we were */
			temp = (data & 0x0fff) << 8;
			if (data & 0x1000)
				memory_install_readwrite16_handler(cpu_get_address_space(space->cpu, ADDRESS_SPACE_PROGRAM), temp, temp + 0xff, 0, 0, i80186_internal_port_r, i80186_internal_port_w);
			else
			{
				temp &= 0xffff;
				memory_install_readwrite16_handler(cpu_get_address_space(space->cpu, ADDRESS_SPACE_IO), temp, temp + 0xff, 0, 0, i80186_internal_port_r, i80186_internal_port_w);
			}
/*          popmessage("Sound CPU reset");*/
			break;

		default:
			logerror("%05X:80186 port %02X = %04X & %04X\n", cpu_get_pc(space->cpu), offset*2, data, mem_mask);
			break;
	}
}



/*************************************
 *
 *  8254 PIT accesses
 *
 *************************************/

INLINE void counter_update_count(int which)
{
	/* only update if the timer is running */
	if (counter[which].timer)
	{
		/* determine how many 2MHz cycles are remaining */
		int count = attotime_to_double(attotime_mul(timer_timeleft(counter[which].timer), 2000000));
		counter[which].count = (count < 0) ? 0 : count;
	}
}


static READ16_HANDLER( pit8254_r )
{
	struct counter_state *ctr;
	int which = offset / 0x40;
	int reg = offset & 3;

	/* switch off the register */
	switch (reg)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = (which * 3) + reg;
			ctr = &counter[which];

			/* update the count */
			counter_update_count(which);

			/* return the LSB */
			if (counter[which].readbyte == 0)
			{
				counter[which].readbyte = 1;
				return counter[which].count & 0xff;
			}

			/* write the MSB and reset the counter */
			else
			{
				counter[which].readbyte = 0;
				return (counter[which].count >> 8) & 0xff;
			}
			break;
	}
	return 0;
}


static WRITE16_HANDLER( pit8254_w )
{
	struct counter_state *ctr;
	int which = offset / 0x40;
	int reg = offset & 3;

	/* ignore odd offsets */
	if (!ACCESSING_BITS_0_7)
		return;
	data &= 0xff;

	/* switch off the register */
	switch (reg)
	{
		case 0:
		case 1:
		case 2:
			/* warning: assumes LSB/MSB addressing and no latching! */
			which = (which * 3) + reg;
			ctr = &counter[which];

			/* write the LSB */
			if (ctr->writebyte == 0)
			{
				ctr->count = (ctr->count & 0xff00) | (data & 0x00ff);
				ctr->writebyte = 1;
			}

			/* write the MSB and reset the counter */
			else
			{
				ctr->count = (ctr->count & 0x00ff) | ((data << 8) & 0xff00);
				ctr->writebyte = 0;

				/* treat 0 as $10000 */
				if (ctr->count == 0) ctr->count = 0x10000;

				/* reset/start the timer */
				timer_adjust_oneshot(ctr->timer, attotime_never, 0);

				if (LOG_PIT) logerror("PIT counter %d set to %d (%d Hz)\n", which, ctr->count, 4000000 / ctr->count);

				/* set the frequency of the associated DAC */
				if (!is_redline)
					set_dac_frequency(which, 4000000 / ctr->count);
				else
				{
					if (which < 5)
						set_dac_frequency(which, 7000000 / ctr->count);
					else if (which == 6)
					{
						set_dac_frequency(5, 7000000 / ctr->count);
						set_dac_frequency(6, 7000000 / ctr->count);
						set_dac_frequency(7, 7000000 / ctr->count);
					}
				}
			}
			break;

		case 3:
			/* determine which counter */
			if ((data & 0xc0) == 0xc0) break;
			which = (which * 3) + (data >> 6);
			ctr = &counter[which];

			/* set the mode */
			ctr->mode = (data >> 1) & 7;
			break;
	}
}



/*************************************
 *
 *  External 80186 control
 *
 *************************************/

WRITE8_HANDLER( leland_80186_control_w )
{
	/* see if anything changed */
	int diff = (last_control ^ data) & 0xf8;
	if (!diff)
		return;
	last_control = data;

	if (LOG_COMM)
	{
		logerror("%04X:80186 control = %02X", cpu_get_previouspc(space->cpu), data);
		if (!(data & 0x80)) logerror("  /RESET");
		if (!(data & 0x40)) logerror("  ZNMI");
		if (!(data & 0x20)) logerror("  INT0");
		if (!(data & 0x10)) logerror("  /TEST");
		if (!(data & 0x08)) logerror("  INT1");
		logerror("\n");
	}

	/* /RESET */
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, data & 0x80  ? CLEAR_LINE : ASSERT_LINE);

	/* /NMI */
/*  If the master CPU doesn't get a response by the time it's ready to send
    the next command, it uses an NMI to force the issue; unfortunately, this
    seems to really screw up the sound system. It turns out it's better to
    just wait for the original interrupt to occur naturally */
/*  cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, data & 0x40  ? CLEAR_LINE : ASSERT_LINE);*/

	/* INT0 */
	if (data & 0x20)
	{
		if (!LATCH_INTS) i80186.intr.request &= ~0x10;
	}
	else if (i80186.intr.ext[0] & 0x10)
		i80186.intr.request |= 0x10;
	else if (diff & 0x20)
		i80186.intr.request |= 0x10;

	/* INT1 */
	if (data & 0x08)
	{
		if (!LATCH_INTS) i80186.intr.request &= ~0x20;
	}
	else if (i80186.intr.ext[1] & 0x10)
		i80186.intr.request |= 0x20;
	else if (diff & 0x08)
		i80186.intr.request |= 0x20;

	/* handle reset here */
	if ((diff & 0x80) && (data & 0x80))
		leland_80186_reset();

	update_interrupt_state(space->machine);
}



/*************************************
 *
 *  Sound command handling
 *
 *************************************/

static TIMER_CALLBACK( command_lo_sync )
{
	if (LOG_COMM) logerror("%s:Write sound command latch lo = %02X\n", cpuexec_describe_context(machine), param);
	sound_command = (sound_command & 0xff00) | param;
}


WRITE8_HANDLER( leland_80186_command_lo_w )
{
	timer_call_after_resynch(space->machine, NULL, data, command_lo_sync);
}


WRITE8_HANDLER( leland_80186_command_hi_w )
{
	if (LOG_COMM) logerror("%04X:Write sound command latch hi = %02X\n", cpu_get_previouspc(space->cpu), data);
	sound_command = (sound_command & 0x00ff) | (data << 8);
}


static READ16_HANDLER( main_to_sound_comm_r )
{
	if (LOG_COMM) logerror("%05X:Read sound command latch = %02X\n", cpu_get_pc(space->cpu), sound_command);
	return sound_command;
}




/*************************************
 *
 *  Sound response handling
 *
 *************************************/

static TIMER_CALLBACK( delayed_response_r )
{
	int checkpc = param;
	int pc = cpu_get_reg(cputag_get_cpu(machine, "master"), Z80_PC);
	int oldaf = cpu_get_reg(cputag_get_cpu(machine, "master"), Z80_AF);

	/* This is pretty cheesy, but necessary. Since the CPUs run in round-robin order,
       synchronizing on the write to this register from the slave side does nothing.
       In order to make sure the master CPU get the real response, we synchronize on
       the read. However, the value we returned the first time around may not be
       accurate, so after the system has synced up, we go back into the master CPUs
       state and put the proper value into the A register. */
	if (pc == checkpc)
	{
		if (LOG_COMM) logerror("(Updated sound response latch to %02X)\n", sound_response);

		oldaf = (oldaf & 0x00ff) | (sound_response << 8);
		cpu_set_reg(cputag_get_cpu(machine, "master"), Z80_AF, oldaf);
	}
	else
		logerror("ERROR: delayed_response_r - current PC = %04X, checkPC = %04X\n", pc, checkpc);
}


READ8_HANDLER( leland_80186_response_r )
{
	if (LOG_COMM) logerror("%04X:Read sound response latch = %02X\n", cpu_get_previouspc(space->cpu), sound_response);

	/* synchronize the response */
	timer_call_after_resynch(space->machine, NULL, cpu_get_previouspc(space->cpu) + 2, delayed_response_r);
	return sound_response;
}


static WRITE16_HANDLER( sound_to_main_comm_w )
{
	if (LOG_COMM) logerror("%05X:Write sound response latch = %02X\n", cpu_get_pc(space->cpu), data);
	sound_response = data;
}



/*************************************
 *
 *  Low-level DAC I/O
 *
 *************************************/

static void set_dac_frequency(int which, int frequency)
{
	struct dac_state *d = &dac[which];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

	/* set the frequency of the associated DAC */
	d->frequency = frequency;
	d->step = (int)((double)frequency * (double)(1 << 24) / (double)OUTPUT_RATE);

	/* also determine the target buffer size */
	d->buftarget = dac[which].frequency / 60 + 50;
	if (d->buftarget > DAC_BUFFER_SIZE - 1)
		d->buftarget = DAC_BUFFER_SIZE - 1;

	/* reevaluate the count */
	if (count > d->buftarget)
		clock_active &= ~(1 << which);
	else if (count < d->buftarget)
		clock_active |= 1 << which;

	if (LOG_DAC) logerror("DAC %d frequency = %d, step = %08X\n", which, d->frequency, d->step);
}


static WRITE16_HANDLER( dac_w )
{
	int which = offset;
	struct dac_state *d = &dac[which];

	/* handle value changes */
	if (ACCESSING_BITS_0_7)
	{
		int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

		/* set the new value */
		d->value = (INT16)(UINT8)data - 0x80;
		if (LOG_DAC) logerror("%05X:DAC %d value = %02X\n", cpu_get_pc(space->cpu), offset, (UINT8)data);

		/* if we haven't overflowed the buffer, add the value value to it */
		if (count < DAC_BUFFER_SIZE - 1)
		{
			/* if this is the first byte, sync the stream */
			if (count == 0)
				stream_update(nondma_stream);

			/* prescale by the volume */
			d->buffer[d->bufin] = d->value * d->volume;
			d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

			/* update the clock status */
			if (++count > d->buftarget)
				clock_active &= ~(1 << which);
		}
	}

	/* handle volume changes */
	if (ACCESSING_BITS_8_15)
	{
		d->volume = ((data >> 8) ^ 0x00) / DAC_VOLUME_SCALE;
		if (LOG_DAC) logerror("%05X:DAC %d volume = %02X\n", cpu_get_pc(space->cpu), offset, data);
	}
}


static WRITE16_HANDLER( redline_dac_w )
{
	int which = offset / 0x100;
	struct dac_state *d = &dac[which];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

	/* set the new value */
	d->value = (INT16)(UINT8)data - 0x80;

	/* if we haven't overflowed the buffer, add the value value to it */
	if (count < DAC_BUFFER_SIZE - 1)
	{
		/* if this is the first byte, sync the stream */
		if (count == 0)
			stream_update(nondma_stream);

		/* prescale by the volume */
		d->buffer[d->bufin] = d->value * d->volume;
		d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

		/* update the clock status */
		if (++count > d->buftarget)
			clock_active &= ~(1 << which);
	}

	/* update the volume */
	d->volume = (offset & 0xff) / DAC_VOLUME_SCALE;
	if (LOG_DAC) logerror("%05X:DAC %d value = %02X, volume = %02X\n", cpu_get_pc(space->cpu), which, data, (offset & 0x1fe) / 2);
}


static WRITE16_HANDLER( dac_10bit_w )
{
	struct dac_state *d = &dac[6];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;
	int data16;

	/* warning: this assumes all port writes here are word-sized */
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	data16 = data;

	/* set the new value */
	d->value = (INT16)data16 - 0x200;
	if (LOG_DAC) logerror("%05X:DAC 10-bit value = %02X\n", cpu_get_pc(space->cpu), data16);

	/* if we haven't overflowed the buffer, add the value value to it */
	if (count < DAC_BUFFER_SIZE - 1)
	{
		/* if this is the first byte, sync the stream */
		if (count == 0)
			stream_update(nondma_stream);

		/* prescale by the volume */
		d->buffer[d->bufin] = d->value * (0xff / DAC_VOLUME_SCALE / 2);
		d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

		/* update the clock status */
		if (++count > d->buftarget)
			clock_active &= ~0x40;
	}
}


static WRITE16_HANDLER( ataxx_dac_control )
{
	/* handle common offsets */
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if (ACCESSING_BITS_0_7)
				dac_w(space, offset, data, 0x00ff);
			return;

		case 0x03:
			dac_w(space, 0, ((data << 13) & 0xe000) | ((data << 10) & 0x1c00) | ((data << 7) & 0x0300), 0xff00);
			dac_w(space, 2, ((data << 10) & 0xe000) | ((data <<  7) & 0x1c00) | ((data << 4) & 0x0300), 0xff00);
			dac_w(space, 4, ((data <<  8) & 0xc000) | ((data <<  6) & 0x3000) | ((data << 4) & 0x0c00) | ((data << 2) & 0x0300), 0xff00);
			return;
	}

	/* if we have a YM2151 (and an external DAC), handle those offsets */
	if (has_ym2151)
	{
		stream_update(extern_stream);
		switch (offset)
		{
			case 0x04:
				ext_active = 1;
				if (LOG_EXTERN) logerror("External DAC active\n");
				return;

			case 0x05:
				ext_active = 0;
				if (LOG_EXTERN) logerror("External DAC inactive\n");
				return;

			case 0x06:
				ext_start >>= 4;
				COMBINE_DATA(&ext_start);
				ext_start <<= 4;
				if (LOG_EXTERN) logerror("External DAC start = %05X\n", ext_start);
				return;

			case 0x07:
				ext_stop >>= 4;
				COMBINE_DATA(&ext_stop);
				ext_stop <<= 4;
				if (LOG_EXTERN) logerror("External DAC stop = %05X\n", ext_stop);
				return;

			case 0x21:
				dac_w(space, offset - 0x21 + 7, data, mem_mask);
				return;
		}
	}
	logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", cpu_get_pc(space->cpu), 5, offset, data);
}



/*************************************
 *
 *  Peripheral chip dispatcher
 *
 *************************************/

static READ16_HANDLER( peripheral_r )
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 0:
			/* we have to return 0 periodically so that they handle interrupts */
			if ((++clock_tick & 7) == 0)
				return 0;

			/* if we've filled up all the active channels, we can give this CPU a reset */
			/* until the next interrupt */
			if (!is_redline)
				return ((clock_active >> 1) & 0x3e);
			else
				return ((clock_active << 1) & 0x7e);

		case 1:
			return main_to_sound_comm_r(space, offset, mem_mask);

		case 2:
			return pit8254_r(space, offset, mem_mask);

		case 3:
			if (!has_ym2151)
				return pit8254_r(space, offset | 0x40, mem_mask);
			else
				return ym2151_r(devtag_get_device(space->machine, "ymsnd"), offset);

		case 4:
			if (is_redline)
				return pit8254_r(space, offset | 0x80, mem_mask);
			else
				logerror("%05X:Unexpected peripheral read %d/%02X\n", cpu_get_pc(space->cpu), select, offset*2);
			break;

		default:
			logerror("%05X:Unexpected peripheral read %d/%02X\n", cpu_get_pc(space->cpu), select, offset*2);
			break;
	}
	return 0xffff;
}


static WRITE16_HANDLER( peripheral_w )
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 1:
			sound_to_main_comm_w(space, offset, data, mem_mask);
			break;

		case 2:
			pit8254_w(space, offset, data, mem_mask);
			break;

		case 3:
			if (!has_ym2151)
				pit8254_w(space, offset | 0x40, data, mem_mask);
			else
				ym2151_w(devtag_get_device(space->machine, "ymsnd"), offset, data);
			break;

		case 4:
			if (is_redline)
				pit8254_w(space, offset | 0x80, data, mem_mask);
			else
				dac_10bit_w(space, offset, data, mem_mask);
			break;

		case 5:	/* Ataxx/WSF/Indy Heat only */
			ataxx_dac_control(space, offset, data, mem_mask);
			break;

		default:
			logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", cpu_get_pc(space->cpu), select, offset, data);
			break;
	}
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

WRITE8_HANDLER( ataxx_80186_control_w )
{
	/* compute the bit-shuffled variants of the bits and then write them */
	int modified = 	((data & 0x01) << 7) |
					((data & 0x02) << 5) |
					((data & 0x04) << 3) |
					((data & 0x08) << 1);
	leland_80186_control_w(space, offset, modified);
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

ADDRESS_MAP_START( leland_80186_map_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_MIRROR(0x1c000) AM_RAM
	AM_RANGE(0x20000, 0xfffff) AM_ROM
ADDRESS_MAP_END


ADDRESS_MAP_START( ataxx_80186_map_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0xff00, 0xffff) AM_READWRITE(i80186_internal_port_r, i80186_internal_port_w)
ADDRESS_MAP_END


ADDRESS_MAP_START( redline_80186_map_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(redline_dac_w)
	AM_RANGE(0xff00, 0xffff) AM_READWRITE(i80186_internal_port_r, i80186_internal_port_w)
ADDRESS_MAP_END


ADDRESS_MAP_START( leland_80186_map_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0000, 0x000b) AM_WRITE(dac_w)
	AM_RANGE(0x0080, 0x008b) AM_WRITE(dac_w)
	AM_RANGE(0x00c0, 0x00cb) AM_WRITE(dac_w)
	AM_RANGE(0xff00, 0xffff) AM_READWRITE(i80186_internal_port_r, i80186_internal_port_w)
ADDRESS_MAP_END


/************************************************************************

Memory configurations:

    Redline Racer:
        FFDF7:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDF7:80186 lower chip select = 00FC        -> 00000-00FFF, 4k long
        FFDF7:80186 peripheral chip select = 013C   -> 01000, 01080, 01100, 01180, 01200, 01280, 01300
        FFDF7:80186 middle chip select = 81FC       -> 80000-C0000, 64k chunks, 256k total
        FFDF7:80186 middle P chip select = A0FC

    Quarterback, Team Quarterback, AAFB, Super Offroad, Track Pack, Pigout, Viper:
        FFDFA:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDFA:80186 peripheral chip select = 203C   -> 20000, 20080, 20100, 20180, 20200, 20280, 20300
        FFDFA:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFDFA:80186 middle P chip select = C0FC

    Ataxx, Indy Heat, World Soccer Finals:
        FFD9D:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFD9D:80186 peripheral chip select = 043C   -> 04000, 04080, 04100, 04180, 04200, 04280, 04300
        FFD9D:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFD9D:80186 middle P chip select = C0BC

************************************************************************/
