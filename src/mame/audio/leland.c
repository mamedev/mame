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

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "includes/leland.h"
#include "sound/2151intf.h"
#include "sound/dac.h"


#define OUTPUT_RATE			50000

#define DAC_BUFFER_SIZE		1024
#define DAC_BUFFER_SIZE_MASK		(DAC_BUFFER_SIZE - 1)

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

struct i80186_state
{
	device_t *cpu;
	struct timer_state	timer[3];
	struct dma_state	dma[2];
	struct intr_state	intr;
	struct mem_state	mem;
};

struct dac_state
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
};

struct counter_state
{
	emu_timer *timer;
	INT32 count;
	UINT8 mode;
	UINT8 readbyte;
	UINT8 writebyte;
};

typedef struct _leland_sound_state leland_sound_state;
struct _leland_sound_state
{
	/* 1st gen */
	UINT8 *m_dac_buffer[2];
	UINT32 m_dac_bufin[2];
	UINT32 m_dac_bufout[2];
	sound_stream *m_dac_stream;

	/* 2nd+ gen */
	sound_stream *m_dma_stream;
	sound_stream *m_nondma_stream;
	sound_stream *m_extern_stream;

	UINT8 m_has_ym2151;
	UINT8 m_is_redline;

	UINT8 m_last_control;
	UINT8 m_clock_active;
	UINT8 m_clock_tick;

	UINT16 m_sound_command;
	UINT8 m_sound_response;

	UINT32 m_ext_start;
	UINT32 m_ext_stop;
	UINT8 m_ext_active;
	UINT8 *m_ext_base;

	struct i80186_state m_i80186;
	struct dac_state m_dac[8];
	struct counter_state m_counter[9];
};


/*************************************
 *
 *  1st generation sound
 *
 *************************************/

INLINE leland_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == LELAND || device->type() == LELAND_80186 || device->type() == REDLINE_80186);

	return (leland_sound_state *)downcast<leland_sound_device *>(device)->token();
}

static STREAM_UPDATE( leland_update )
{
	leland_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	int dacnum;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));
	for (dacnum = 0; dacnum < 2; dacnum++)
	{
		int bufout = state->m_dac_bufout[dacnum];
		int count = (state->m_dac_bufin[dacnum] - bufout) & DAC_BUFFER_SIZE_MASK;

		if (count > 300)
		{
			UINT8 *base = state->m_dac_buffer[dacnum];
			int i;

			for (i = 0; i < samples && count > 0; i++, count--)
			{
				buffer[i] += ((INT16)base[bufout] - 0x80) * 0x40;
				bufout = (bufout + 1) & DAC_BUFFER_SIZE_MASK;
			}
			state->m_dac_bufout[dacnum] = bufout;
		}
	}
}


static DEVICE_START( leland_sound )
{
	leland_sound_state *state = get_safe_token(device);

	/* reset globals */
	state->m_dac_buffer[0] = state->m_dac_buffer[1] = NULL;
	state->m_dac_bufin[0]  = state->m_dac_bufin[1]  = 0;
	state->m_dac_bufout[0] = state->m_dac_bufout[1] = 0;

	/* allocate the stream */
	state->m_dac_stream = device->machine().sound().stream_alloc(*device, 0, 1, 256*60, NULL, leland_update);

	/* allocate memory */
	state->m_dac_buffer[0] = auto_alloc_array(device->machine(), UINT8, DAC_BUFFER_SIZE);
	state->m_dac_buffer[1] = auto_alloc_array(device->machine(), UINT8, DAC_BUFFER_SIZE);
}


void leland_dac_update(device_t *device, int dacnum, UINT8 sample)
{
	leland_sound_state *state = get_safe_token(device);
	UINT8 *buffer = state->m_dac_buffer[dacnum];
	int bufin = state->m_dac_bufin[dacnum];

	/* skip if nothing */
	if (!buffer)
		return;

	/* copy data from VRAM */
	buffer[bufin] = sample;
	bufin = (bufin + 1) & DAC_BUFFER_SIZE_MASK;

	/* update the buffer */
	state->m_dac_bufin[dacnum] = bufin;
}



/*************************************
 *
 *  2nd-4th generation sound
 *
 *************************************/

static void set_dac_frequency(leland_sound_state *state, int which, int frequency);

static READ16_DEVICE_HANDLER( peripheral_r );
static WRITE16_DEVICE_HANDLER( peripheral_w );



/*************************************
 *
 *  Manual DAC sound generation
 *
 *************************************/

static STREAM_UPDATE( leland_80186_dac_update )
{
	leland_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	int i, j, start, stop;

	if (LOG_SHORTAGES) logerror("----\n");

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* if we're redline racer, we have more DACs */
	if (!state->m_is_redline)
		start = 2, stop = 7;
	else
		start = 0, stop = 8;

	/* loop over manual DAC channels */
	for (i = start; i < stop; i++)
	{
		struct dac_state *d = &state->m_dac[i];
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
			state->m_clock_active |= 1 << i;
	}
}



/*************************************
 *
 *  DMA-based DAC sound generation
 *
 *************************************/

static STREAM_UPDATE( leland_80186_dma_update )
{
	leland_sound_state *state = get_safe_token(device);
	address_space *dmaspace = (address_space *)param;
	stream_sample_t *buffer = outputs[0];
	int i, j;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* loop over DMA buffers */
	for (i = 0; i < 2; i++)
	{
		struct dma_state *d = &state->m_i80186.dma[i];

		/* check for enabled DMA */
		if (d->control & 0x0002)
		{
			/* make sure the parameters meet our expectations */
			if ((d->control & 0xfe00) != 0x1600)
			{
				logerror("Unexpected DMA control %02X\n", d->control);
			}
			else if (!state->m_is_redline && ((d->dest & 1) || (d->dest & 0x3f) > 0x0b))
			{
				logerror("Unexpected DMA destination %02X\n", d->dest);
			}
			else if (state->m_is_redline && (d->dest & 0xf000) != 0x4000 && (d->dest & 0xf000) != 0x5000)
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
				if (!state->m_is_redline)
					which = (d->dest & 0x3f) / 2;
				else
					which = (d->dest >> 9) & 7;

				frac = state->m_dac[which].fraction;
				step = state->m_dac[which].step;
				volume = state->m_dac[which].volume;

				/* sample-rate convert to the output frequency */
				for (j = 0; j < samples && count > 0; j++)
				{
					buffer[j] += ((int)dmaspace->read_byte(source) - 0x80) * volume;
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
				state->m_dac[which].fraction = frac;
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
	leland_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	struct dac_state *d = &state->m_dac[7];
	int count = state->m_ext_stop - state->m_ext_start;
	int j;

	/* reset the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	/* if we have data, process it */
	if (count > 0 && state->m_ext_active)
	{
		int source = state->m_ext_start;
		int frac = d->fraction;
		int step = d->step;

		/* sample-rate convert to the output frequency */
		for (j = 0; j < samples && count > 0; j++)
		{
			buffer[j] += ((INT16)state->m_ext_base[source] - 0x80) * d->volume;
			frac += step;
			source += frac >> 24;
			count -= frac >> 24;
			frac &= 0xffffff;
		}

		/* update the DAC state */
		d->fraction = frac;
		state->m_ext_start = source;
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
	leland_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	address_space *dmaspace = machine.device("audiocpu")->memory().space(AS_PROGRAM);
	int i;

	/* determine which sound hardware is installed */
	state->m_has_ym2151 = (device->machine().device("ymsnd") != NULL);

	/* allocate separate streams for the DMA and non-DMA DACs */
	state->m_dma_stream = device->machine().sound().stream_alloc(*device, 0, 1, OUTPUT_RATE, (void *)dmaspace, leland_80186_dma_update);
	state->m_nondma_stream = device->machine().sound().stream_alloc(*device, 0, 1, OUTPUT_RATE, NULL, leland_80186_dac_update);

	/* if we have a 2151, install an externally driven DAC stream */
	if (state->m_has_ym2151)
	{
		state->m_ext_base = machine.root_device().memregion("dac")->base();
		state->m_extern_stream = device->machine().sound().stream_alloc(*device, 0, 1, OUTPUT_RATE, NULL, leland_80186_extern_update);
	}

	/* create timers here so they stick around */
	state->m_i80186.cpu = &dmaspace->device();
	state->m_i80186.timer[0].int_timer = machine.scheduler().timer_alloc(FUNC(internal_timer_int), device);
	state->m_i80186.timer[1].int_timer = machine.scheduler().timer_alloc(FUNC(internal_timer_int), device);
	state->m_i80186.timer[2].int_timer = machine.scheduler().timer_alloc(FUNC(internal_timer_int), device);
	state->m_i80186.timer[0].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i80186.timer[1].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i80186.timer[2].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i80186.dma[0].finish_timer = machine.scheduler().timer_alloc(FUNC(dma_timer_callback), device);
	state->m_i80186.dma[1].finish_timer = machine.scheduler().timer_alloc(FUNC(dma_timer_callback), device);

	for (i = 0; i < 9; i++)
		state->m_counter[i].timer = machine.scheduler().timer_alloc(FUNC_NULL);
}

static DEVICE_START( leland_80186_sound )
{
	leland_sound_state *state = get_safe_token(device);
	state->m_is_redline = 0;
	DEVICE_START_CALL(common_sh_start);
}

static DEVICE_RESET( leland_80186_sound );

static DEVICE_START( redline_80186_sound )
{
	leland_sound_state *state = get_safe_token(device);
	state->m_is_redline = 1;
	DEVICE_START_CALL(common_sh_start);
}


const device_type LELAND = &device_creator<leland_sound_device>;

leland_sound_device::leland_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LELAND, "Leland DAC", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(leland_sound_state));
}

leland_sound_device::leland_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(leland_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void leland_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void leland_sound_device::device_start()
{
	DEVICE_START_NAME( leland_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void leland_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type LELAND_80186 = &device_creator<leland_80186_sound_device>;

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: leland_sound_device(mconfig, LELAND_80186, "Leland 80186 DAC", tag, owner, clock)
{
}

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: leland_sound_device(mconfig, type, name, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void leland_80186_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void leland_80186_sound_device::device_start()
{
	DEVICE_START_NAME( leland_80186_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void leland_80186_sound_device::device_reset()
{
	DEVICE_RESET_NAME( leland_80186_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void leland_80186_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type REDLINE_80186 = &device_creator<redline_80186_sound_device>;

redline_80186_sound_device::redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: leland_80186_sound_device(mconfig, REDLINE_80186, "Redline Racer 80186 DAC", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void redline_80186_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void redline_80186_sound_device::device_start()
{
	DEVICE_START_NAME( redline_80186_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void redline_80186_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}




static void leland_80186_reset(device_t *device)
{
	leland_sound_state *state = get_safe_token(device);
	struct i80186_state oldstate = state->m_i80186;
	emu_timer *counter_timer[9];
	int i;

	/* reset the i80186 state, but save the timers */
	memset(&state->m_i80186, 0, sizeof(state->m_i80186));
	state->m_i80186.cpu = oldstate.cpu;
	state->m_i80186.timer[0].int_timer = oldstate.timer[0].int_timer;
	state->m_i80186.timer[1].int_timer = oldstate.timer[1].int_timer;
	state->m_i80186.timer[2].int_timer = oldstate.timer[2].int_timer;
	state->m_i80186.timer[0].time_timer = oldstate.timer[0].time_timer;
	state->m_i80186.timer[1].time_timer = oldstate.timer[1].time_timer;
	state->m_i80186.timer[2].time_timer = oldstate.timer[2].time_timer;
	state->m_i80186.dma[0].finish_timer = oldstate.dma[0].finish_timer;
	state->m_i80186.dma[1].finish_timer = oldstate.dma[1].finish_timer;

	/* reset the interrupt state */
	state->m_i80186.intr.priority_mask	= 0x0007;
	state->m_i80186.intr.timer			= 0x000f;
	state->m_i80186.intr.dma[0]			= 0x000f;
	state->m_i80186.intr.dma[1]			= 0x000f;
	state->m_i80186.intr.ext[0]			= 0x000f;
	state->m_i80186.intr.ext[1]			= 0x000f;
	state->m_i80186.intr.ext[2]			= 0x000f;
	state->m_i80186.intr.ext[3]			= 0x000f;

	/* reset the DAC and counter states as well */
	memset(&state->m_dac, 0, sizeof(state->m_dac));
	for (i = 0; i < 9; i++)
		counter_timer[i] = state->m_counter[i].timer;
	memset(&state->m_counter, 0, sizeof(state->m_counter));
	for (i = 0; i < 9; i++)
		state->m_counter[i].timer = counter_timer[i];
}


static DEVICE_RESET( leland_80186_sound )
{
	leland_sound_state *state = get_safe_token(device);

	/* reset the 80186 registers */
	leland_80186_reset(device);

	/* reset our internal stuff */
	state->m_last_control = 0xf8;
	state->m_clock_active = 0;

	/* reset the external DAC */
	state->m_ext_start = 0;
	state->m_ext_stop = 0;
	state->m_ext_active = 0;
}



/*************************************
 *
 *  80186 interrupt controller
 *
 *************************************/

static IRQ_CALLBACK( int_callback )
{
	leland_sound_state *state = get_safe_token(device->machine().device("custom"));
	if (LOG_INTERRUPTS) logerror("(%f) **** Acknowledged interrupt vector %02X\n", device->machine().time().as_double(), state->m_i80186.intr.poll_status & 0x1f);

	/* clear the interrupt */
	device_set_input_line(state->m_i80186.cpu, 0, CLEAR_LINE);
	state->m_i80186.intr.pending = 0;

	/* clear the request and set the in-service bit */
#if LATCH_INTS
	state->m_i80186.intr.request &= ~state->m_i80186.intr.ack_mask;
#else
	state->m_i80186.intr.request &= ~(state->m_i80186.intr.ack_mask & 0x0f);
#endif
	state->m_i80186.intr.in_service |= state->m_i80186.intr.ack_mask;
	if (state->m_i80186.intr.ack_mask == 0x0001)
	{
		switch (state->m_i80186.intr.poll_status & 0x1f)
		{
			case 0x08:	state->m_i80186.intr.status &= ~0x01;	break;
			case 0x12:	state->m_i80186.intr.status &= ~0x02;	break;
			case 0x13:	state->m_i80186.intr.status &= ~0x04;	break;
		}
	}
	state->m_i80186.intr.ack_mask = 0;

	/* a request no longer pending */
	state->m_i80186.intr.poll_status &= ~0x8000;

	/* return the vector */
	return state->m_i80186.intr.poll_status & 0x1f;
}


static void update_interrupt_state(device_t *device)
{
	leland_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	int i, j, new_vector = 0;

	if (LOG_INTERRUPTS) logerror("update_interrupt_status: req=%02X stat=%02X serv=%02X\n", state->m_i80186.intr.request, state->m_i80186.intr.status, state->m_i80186.intr.in_service);

	/* loop over priorities */
	for (i = 0; i <= state->m_i80186.intr.priority_mask; i++)
	{
		/* note: by checking 4 bits, we also verify that the mask is off */
		if ((state->m_i80186.intr.timer & 15) == i)
		{
			/* if we're already servicing something at this level, don't generate anything new */
			if (state->m_i80186.intr.in_service & 0x01)
				return;

			/* if there's something pending, generate an interrupt */
			if (state->m_i80186.intr.status & 0x07)
			{
				if (state->m_i80186.intr.status & 1)
					new_vector = 0x08;
				else if (state->m_i80186.intr.status & 2)
					new_vector = 0x12;
				else if (state->m_i80186.intr.status & 4)
					new_vector = 0x13;
				else
					popmessage("Invalid timer interrupt!");

				/* set the clear mask and generate the int */
				state->m_i80186.intr.ack_mask = 0x0001;
				goto generate_int;
			}
		}

		/* check DMA interrupts */
		for (j = 0; j < 2; j++)
			if ((state->m_i80186.intr.dma[j] & 15) == i)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (state->m_i80186.intr.in_service & (0x04 << j))
					return;

				/* if there's something pending, generate an interrupt */
				if (state->m_i80186.intr.request & (0x04 << j))
				{
					new_vector = 0x0a + j;

					/* set the clear mask and generate the int */
					state->m_i80186.intr.ack_mask = 0x0004 << j;
					goto generate_int;
				}
			}

		/* check external interrupts */
		for (j = 0; j < 4; j++)
			if ((state->m_i80186.intr.ext[j] & 15) == i)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (state->m_i80186.intr.in_service & (0x10 << j))
					return;

				/* if there's something pending, generate an interrupt */
				if (state->m_i80186.intr.request & (0x10 << j))
				{
					/* otherwise, generate an interrupt for this request */
					new_vector = 0x0c + j;

					/* set the clear mask and generate the int */
					state->m_i80186.intr.ack_mask = 0x0010 << j;
					goto generate_int;
				}
			}
	}
	return;

generate_int:
	/* generate the appropriate interrupt */
	state->m_i80186.intr.poll_status = 0x8000 | new_vector;
	if (!state->m_i80186.intr.pending)
		machine.device("audiocpu")->execute().set_input_line(0, ASSERT_LINE);
	state->m_i80186.intr.pending = 1;
	if (LOG_INTERRUPTS) logerror("(%f) **** Requesting interrupt vector %02X\n", machine.time().as_double(), new_vector);
}


static void handle_eoi(device_t *device, int data)
{
	leland_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	int i, j;

	/* specific case */
	if (!(data & 0x8000))
	{
		/* turn off the appropriate in-service bit */
		switch (data & 0x1f)
		{
			case 0x08:	state->m_i80186.intr.in_service &= ~0x01;	break;
			case 0x12:	state->m_i80186.intr.in_service &= ~0x01;	break;
			case 0x13:	state->m_i80186.intr.in_service &= ~0x01;	break;
			case 0x0a:	state->m_i80186.intr.in_service &= ~0x04;	break;
			case 0x0b:	state->m_i80186.intr.in_service &= ~0x08;	break;
			case 0x0c:	state->m_i80186.intr.in_service &= ~0x10;	break;
			case 0x0d:	state->m_i80186.intr.in_service &= ~0x20;	break;
			case 0x0e:	state->m_i80186.intr.in_service &= ~0x40;	break;
			case 0x0f:	state->m_i80186.intr.in_service &= ~0x80;	break;
			default:	logerror("%s:ERROR - 80186 EOI with unknown vector %02X\n", machine.describe_context(), data & 0x1f);
		}
		if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for vector %02X\n", machine.time().as_double(), data & 0x1f);
	}

	/* non-specific case */
	else
	{
		/* loop over priorities */
		for (i = 0; i <= 7; i++)
		{
			/* check for in-service timers */
			if ((state->m_i80186.intr.timer & 7) == i && (state->m_i80186.intr.in_service & 0x01))
			{
				state->m_i80186.intr.in_service &= ~0x01;
				if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for timer\n", machine.time().as_double());
				return;
			}

			/* check for in-service DMA interrupts */
			for (j = 0; j < 2; j++)
				if ((state->m_i80186.intr.dma[j] & 7) == i && (state->m_i80186.intr.in_service & (0x04 << j)))
				{
					state->m_i80186.intr.in_service &= ~(0x04 << j);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for DMA%d\n", machine.time().as_double(), j);
					return;
				}

			/* check external interrupts */
			for (j = 0; j < 4; j++)
				if ((state->m_i80186.intr.ext[j] & 7) == i && (state->m_i80186.intr.in_service & (0x10 << j)))
				{
					state->m_i80186.intr.in_service &= ~(0x10 << j);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for INT%d\n", machine.time().as_double(), j);
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
	device_t *device = (device_t *)ptr;
	leland_sound_state *state = get_safe_token(device);
	int which = param;
	struct timer_state *t = &state->m_i80186.timer[which];

	if (LOG_TIMER) logerror("Hit interrupt callback for timer %d\n", which);

	/* set the max count bit */
	t->control |= 0x0020;

	/* request an interrupt */
	if (t->control & 0x2000)
	{
		state->m_i80186.intr.status |= 0x01 << which;
		update_interrupt_state(device);
		if (LOG_TIMER) logerror("  Generating timer interrupt\n");
	}

	/* if we're continuous, reset */
	if (t->control & 0x0001)
	{
		int count = t->maxA ? t->maxA : 0x10000;
		t->int_timer->adjust(attotime::from_hz(2000000) * count, which);
		if (LOG_TIMER) logerror("  Repriming interrupt\n");
	}
	else
		t->int_timer->adjust(attotime::never, which);
}


static void internal_timer_sync(leland_sound_state *state, int which)
{
	struct timer_state *t = &state->m_i80186.timer[which];

	/* if we have a timing timer running, adjust the count */
	if (t->time_timer_active)
	{
		attotime current_time = t->time_timer->elapsed();
		int net_clocks = ((current_time - t->last_time) * 2000000).as_double();
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


static void internal_timer_update(leland_sound_state *state, int which, int new_count, int new_maxA, int new_maxB, int new_control)
{
	struct timer_state *t = &state->m_i80186.timer[which];
	int update_int_timer = 0;

	/* if we have a new count and we're on, update things */
	if (new_count != -1)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(state, which);
			update_int_timer = 1;
		}
		t->count = new_count;
	}

	/* if we have a new max and we're on, update things */
	if (new_maxA != -1 && new_maxA != t->maxA)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(state, which);
			update_int_timer = 1;
		}
		t->maxA = new_maxA;
		if (new_maxA == 0) new_maxA = 0x10000;

		/* redline racer controls nothing externally? */
		if (state->m_is_redline)
			;

		/* on the common board, timer 0 controls the 10-bit DAC frequency */
		else if (which == 0)
			set_dac_frequency(state, 6, 2000000 / new_maxA);

		/* timer 1 controls the externally driven DAC on Indy Heat/WSF */
		else if (which == 1 && state->m_has_ym2151)
			set_dac_frequency(state, 7, 2000000 / (new_maxA * 2));
	}

	/* if we have a new max and we're on, update things */
	if (new_maxB != -1 && new_maxB != t->maxB)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(state, which);
			update_int_timer = 1;
		}
		t->maxB = new_maxB;
		if (new_maxB == 0) new_maxB = 0x10000;

		/* timer 1 controls the externally driven DAC on Indy Heat/WSF */
		/* they alternate the use of maxA and maxB in a way that makes no */
		/* sense according to the 80186 documentation! */
		if (which == 1 && state->m_has_ym2151)
			set_dac_frequency(state, 7, 2000000 / (new_maxB * 2));
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
				internal_timer_sync(state, which);

				/* nuke the timer and force the interrupt timer to be recomputed */
				t->time_timer->adjust(attotime::never, which);
				t->time_timer_active = 0;
				update_int_timer = 1;
			}

			/* if we're going on, start the timers running */
			else if ((diff & 0x8000) && (new_control & 0x8000))
			{
				/* start the timing */
				t->time_timer->adjust(attotime::never, which);
				t->time_timer_active = 1;
				update_int_timer = 1;
			}

			/* if something about the interrupt timer changed, force an update */
			if (!(diff & 0x8000) && (diff & 0x2000))
			{
				internal_timer_sync(state, which);
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
	if (!state->m_has_ym2151 || which != 1)
		if (update_int_timer)
		{
			if ((t->control & 0x8000) && (t->control & 0x2000))
			{
				int diff = t->maxA - t->count;
				if (diff <= 0) diff += 0x10000;
				t->int_timer->adjust(attotime::from_hz(2000000) * diff, which);
				if (LOG_TIMER) logerror("Set interrupt timer for %d\n", which);
			}
			else
				t->int_timer->adjust(attotime::never, which);
		}
}



/*************************************
 *
 *  80186 internal DMA
 *
 *************************************/

static TIMER_CALLBACK( dma_timer_callback )
{
	device_t *device = (device_t *)ptr;
	leland_sound_state *state = get_safe_token(device);
	int which = param;
	struct dma_state *d = &state->m_i80186.dma[which];

	/* force an update and see if we're really done */
	state->m_dma_stream->update();

	/* complete the status update */
	d->control &= ~0x0002;
	d->source += d->count;
	d->count = 0;

	/* check for interrupt generation */
	if (d->control & 0x0100)
	{
		if (LOG_DMA) logerror("DMA%d timer callback - requesting interrupt: count = %04X, source = %04X\n", which, d->count, d->source);
		state->m_i80186.intr.request |= 0x04 << which;
		update_interrupt_state(device);
	}
}


static void update_dma_control(leland_sound_state *state, int which, int new_control)
{
	struct dma_state *d = &state->m_i80186.dma[which];
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
		else if (!state->m_is_redline && ((d->dest & 1) || (d->dest & 0x3f) > 0x0b))
		{
			logerror("Unexpected DMA destination %02X\n", d->dest);
		}
		else if (state->m_is_redline && (d->dest & 0xf000) != 0x4000 && (d->dest & 0xf000) != 0x5000)
		{
			logerror("Unexpected DMA destination %02X\n", d->dest);
		}

		/* otherwise, set a timer */
		else
		{
			int count = d->count;
			int dacnum;

			/* adjust for redline racer */
			if (!state->m_is_redline)
				dacnum = (d->dest & 0x3f) / 2;
			else
			{
				dacnum = (d->dest >> 9) & 7;
				state->m_dac[dacnum].volume = (d->dest & 0x1fe) / 2 / DAC_VOLUME_SCALE;
			}

			if (LOG_DMA) logerror("Initiated DMA %d - count = %04X, source = %04X, dest = %04X\n", which, d->count, d->source, d->dest);

			d->finished = 0;
			d->finish_timer->adjust(attotime::from_hz(state->m_dac[dacnum].frequency) * count, which);
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

static READ16_DEVICE_HANDLER( i80186_internal_port_r )
{
	leland_sound_state *state = get_safe_token(device);
	int temp, which;

	switch (offset)
	{
		case 0x22/2:
			logerror("%05X:ERROR - read from 80186 EOI\n", state->m_i80186.cpu->safe_pc());
			break;

		case 0x24/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll\n", state->m_i80186.cpu->safe_pc());
			if (state->m_i80186.intr.poll_status & 0x8000)
				int_callback(state->m_i80186.cpu, 0);
			return state->m_i80186.intr.poll_status;

		case 0x26/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll status\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.poll_status;

		case 0x28/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt mask\n", state->m_i80186.cpu->safe_pc());
			temp  = (state->m_i80186.intr.timer  >> 3) & 0x01;
			temp |= (state->m_i80186.intr.dma[0] >> 1) & 0x04;
			temp |= (state->m_i80186.intr.dma[1] >> 0) & 0x08;
			temp |= (state->m_i80186.intr.ext[0] << 1) & 0x10;
			temp |= (state->m_i80186.intr.ext[1] << 2) & 0x20;
			temp |= (state->m_i80186.intr.ext[2] << 3) & 0x40;
			temp |= (state->m_i80186.intr.ext[3] << 4) & 0x80;
			return temp;

		case 0x2a/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt priority mask\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.priority_mask;

		case 0x2c/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt in-service\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.in_service;

		case 0x2e/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt request\n", state->m_i80186.cpu->safe_pc());
			temp = state->m_i80186.intr.request & ~0x0001;
			if (state->m_i80186.intr.status & 0x0007)
				temp |= 1;
			return temp;

		case 0x30/2:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt status\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.status;

		case 0x32/2:
			if (LOG_PORTS) logerror("%05X:read 80186 timer interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.timer;

		case 0x34/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 0 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.dma[0];

		case 0x36/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 1 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.dma[1];

		case 0x38/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 0 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.ext[0];

		case 0x3a/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 1 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.ext[1];

		case 0x3c/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 2 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.ext[2];

		case 0x3e/2:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 3 interrupt control\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.intr.ext[3];

		case 0x50/2:
		case 0x58/2:
		case 0x60/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d count\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			if (ACCESSING_BITS_0_7)
				internal_timer_sync(state, which);
			return state->m_i80186.timer[which].count;

		case 0x52/2:
		case 0x5a/2:
		case 0x62/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d max A\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			return state->m_i80186.timer[which].maxA;

		case 0x54/2:
		case 0x5c/2:
			logerror("%05X:read 80186 Timer %d max B\n", state->m_i80186.cpu->safe_pc(), (offset/2 - 0x50) / 4);
			which = (offset - 0x50/2) / 4;
			return state->m_i80186.timer[which].maxB;

		case 0x56/2:
		case 0x5e/2:
		case 0x66/2:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d control\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4);
			which = (offset - 0x50/2) / 4;
			return state->m_i80186.timer[which].control;

		case 0xa0/2:
			if (LOG_PORTS) logerror("%05X:read 80186 upper chip select\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.mem.upper;

		case 0xa2/2:
			if (LOG_PORTS) logerror("%05X:read 80186 lower chip select\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.mem.lower;

		case 0xa4/2:
			if (LOG_PORTS) logerror("%05X:read 80186 peripheral chip select\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.mem.peripheral;

		case 0xa6/2:
			if (LOG_PORTS) logerror("%05X:read 80186 middle chip select\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.mem.middle;

		case 0xa8/2:
			if (LOG_PORTS) logerror("%05X:read 80186 middle P chip select\n", state->m_i80186.cpu->safe_pc());
			return state->m_i80186.mem.middle_size;

		case 0xc0/2:
		case 0xd0/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower source address\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].source;

		case 0xc2/2:
		case 0xd2/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper source address\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].source >> 16;

		case 0xc4/2:
		case 0xd4/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower dest address\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].dest;

		case 0xc6/2:
		case 0xd6/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper dest address\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].dest >> 16;

		case 0xc8/2:
		case 0xd8/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d transfer count\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].count;

		case 0xca/2:
		case 0xda/2:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d control\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			return state->m_i80186.dma[which].control;

		default:
			logerror("%05X:read 80186 port %02X\n", state->m_i80186.cpu->safe_pc(), offset*2);
			break;
	}
	return 0x00;
}



/*************************************
 *
 *  80186 internal I/O writes
 *
 *************************************/

static WRITE16_DEVICE_HANDLER( i80186_internal_port_w )
{
	leland_sound_state *state = get_safe_token(device);
	int temp, which;

	/* handle partials */
	if (!ACCESSING_BITS_8_15)
		data = (i80186_internal_port_r(device, offset, 0xff00) & 0xff00) | (data & 0x00ff);
	else if (!ACCESSING_BITS_0_7)
		data = (i80186_internal_port_r(device, offset, 0x00ff) & 0x00ff) | (data & 0xff00);

	switch (offset)
	{
		case 0x22/2:
			if (LOG_PORTS) logerror("%05X:80186 EOI = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			handle_eoi(device, 0x8000);
			update_interrupt_state(device);
			break;

		case 0x24/2:
			logerror("%05X:ERROR - write to 80186 interrupt poll = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			break;

		case 0x26/2:
			logerror("%05X:ERROR - write to 80186 interrupt poll status = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			break;

		case 0x28/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt mask = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.timer  = (state->m_i80186.intr.timer  & ~0x08) | ((data << 3) & 0x08);
			state->m_i80186.intr.dma[0] = (state->m_i80186.intr.dma[0] & ~0x08) | ((data << 1) & 0x08);
			state->m_i80186.intr.dma[1] = (state->m_i80186.intr.dma[1] & ~0x08) | ((data << 0) & 0x08);
			state->m_i80186.intr.ext[0] = (state->m_i80186.intr.ext[0] & ~0x08) | ((data >> 1) & 0x08);
			state->m_i80186.intr.ext[1] = (state->m_i80186.intr.ext[1] & ~0x08) | ((data >> 2) & 0x08);
			state->m_i80186.intr.ext[2] = (state->m_i80186.intr.ext[2] & ~0x08) | ((data >> 3) & 0x08);
			state->m_i80186.intr.ext[3] = (state->m_i80186.intr.ext[3] & ~0x08) | ((data >> 4) & 0x08);
			update_interrupt_state(device);
			break;

		case 0x2a/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt priority mask = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.priority_mask = data & 0x0007;
			update_interrupt_state(device);
			break;

		case 0x2c/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt in-service = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.in_service = data & 0x00ff;
			update_interrupt_state(device);
			break;

		case 0x2e/2:
			if (LOG_PORTS) logerror("%05X:80186 interrupt request = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.request = (state->m_i80186.intr.request & ~0x00c0) | (data & 0x00c0);
			update_interrupt_state(device);
			break;

		case 0x30/2:
			if (LOG_PORTS) logerror("%05X:WARNING - wrote to 80186 interrupt status = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.status = (state->m_i80186.intr.status & ~0x8000) | (data & 0x8000);
			state->m_i80186.intr.status = (state->m_i80186.intr.status & ~0x0007) | (data & 0x0007);
			update_interrupt_state(device);
			break;

		case 0x32/2:
			if (LOG_PORTS) logerror("%05X:80186 timer interrupt contol = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.timer = data & 0x000f;
			break;

		case 0x34/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA 0 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.dma[0] = data & 0x000f;
			break;

		case 0x36/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA 1 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.dma[1] = data & 0x000f;
			break;

		case 0x38/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 0 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.ext[0] = data & 0x007f;
			break;

		case 0x3a/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 1 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.ext[1] = data & 0x007f;
			break;

		case 0x3c/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 2 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.ext[2] = data & 0x001f;
			break;

		case 0x3e/2:
			if (LOG_PORTS) logerror("%05X:80186 INT 3 interrupt control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.intr.ext[3] = data & 0x001f;
			break;

		case 0x50/2:
		case 0x58/2:
		case 0x60/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d count = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(state, which, data, -1, -1, -1);
			break;

		case 0x52/2:
		case 0x5a/2:
		case 0x62/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max A = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(state, which, -1, data, -1, -1);
			break;

		case 0x54/2:
		case 0x5c/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max B = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(state, which, -1, -1, data, -1);
			break;

		case 0x56/2:
		case 0x5e/2:
		case 0x66/2:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0x50/2) / 4, data, mem_mask);
			which = (offset - 0x50/2) / 4;
			internal_timer_update(state, which, -1, -1, -1, data);
			break;

		case 0xa0/2:
			if (LOG_PORTS) logerror("%05X:80186 upper chip select = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.mem.upper = data | 0xc038;
			break;

		case 0xa2/2:
			if (LOG_PORTS) logerror("%05X:80186 lower chip select = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.mem.lower = (data & 0x3fff) | 0x0038;
			break;

		case 0xa4/2:
			if (LOG_PORTS) logerror("%05X:80186 peripheral chip select = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.mem.peripheral = data | 0x0038;
			break;

		case 0xa6/2:
			if (LOG_PORTS) logerror("%05X:80186 middle chip select = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.mem.middle = data | 0x01f8;
			break;

		case 0xa8/2:
			if (LOG_PORTS) logerror("%05X:80186 middle P chip select = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);
			state->m_i80186.mem.middle_size = data | 0x8038;

			temp = (state->m_i80186.mem.peripheral & 0xffc0) << 4;
			if (state->m_i80186.mem.middle_size & 0x0040)
			{
				state->m_i80186.cpu->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(*device, temp, temp + 0x2ff, FUNC(peripheral_r), FUNC(peripheral_w));
			}
			else
			{
				temp &= 0xffff;
				state->m_i80186.cpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*device, temp, temp + 0x2ff, FUNC(peripheral_r), FUNC(peripheral_w));
			}

			/* we need to do this at a time when the 80186 context is swapped in */
			/* this register is generally set once at startup and never again, so it's a good */
			/* time to set it up */
			device_set_irq_callback(state->m_i80186.cpu, int_callback);
			break;

		case 0xc0/2:
		case 0xd0/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower source address = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			state->m_i80186.dma[which].source = (state->m_i80186.dma[which].source & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0xc2/2:
		case 0xd2/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper source address = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			state->m_i80186.dma[which].source = (state->m_i80186.dma[which].source & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0xc4/2:
		case 0xd4/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower dest address = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			state->m_i80186.dma[which].dest = (state->m_i80186.dma[which].dest & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0xc6/2:
		case 0xd6/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper dest address = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			state->m_i80186.dma[which].dest = (state->m_i80186.dma[which].dest & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0xc8/2:
		case 0xd8/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d transfer count = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			state->m_i80186.dma[which].count = data;
			break;

		case 0xca/2:
		case 0xda/2:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d control = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), (offset - 0xc0/2) / 8, data, mem_mask);
			which = (offset - 0xc0/2) / 8;
			state->m_dma_stream->update();
			update_dma_control(state, which, data);
			break;

		case 0xfe/2:
			if (LOG_PORTS) logerror("%05X:80186 relocation register = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), data, mem_mask);

			/* we assume here there that this doesn't happen too often */
			/* plus, we can't really remove the old memory range, so we also assume that it's */
			/* okay to leave us mapped where we were */
			temp = (data & 0x0fff) << 8;
			if (data & 0x1000)
			{
				state->m_i80186.cpu->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(*device, temp, temp + 0xff, FUNC(i80186_internal_port_r), FUNC(i80186_internal_port_w));
			}
			else
			{
				temp &= 0xffff;
				state->m_i80186.cpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*device, temp, temp + 0xff, FUNC(i80186_internal_port_r), FUNC(i80186_internal_port_w));
			}
/*          popmessage("Sound CPU reset");*/
			break;

		default:
			logerror("%05X:80186 port %02X = %04X & %04X\n", state->m_i80186.cpu->safe_pc(), offset*2, data, mem_mask);
			break;
	}
}



/*************************************
 *
 *  8254 PIT accesses
 *
 *************************************/

INLINE void counter_update_count(struct counter_state *ctr)
{
	/* only update if the timer is running */
	if (ctr->timer)
	{
		/* determine how many 2MHz cycles are remaining */
		int count = (ctr->timer->remaining() * 2000000).as_double();
		ctr->count = (count < 0) ? 0 : count;
	}
}


static READ16_DEVICE_HANDLER( pit8254_r )
{
	leland_sound_state *state = get_safe_token(device);
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
			ctr = &state->m_counter[which];

			/* update the count */
			counter_update_count(ctr);

			/* return the LSB */
			if (ctr->readbyte == 0)
			{
				ctr->readbyte = 1;
				return ctr->count & 0xff;
			}

			/* write the MSB and reset the counter */
			else
			{
				ctr->readbyte = 0;
				return (ctr->count >> 8) & 0xff;
			}
			break;
	}
	return 0;
}


static WRITE16_DEVICE_HANDLER( pit8254_w )
{
	leland_sound_state *state = get_safe_token(device);
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
			ctr = &state->m_counter[which];

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
				ctr->timer->adjust(attotime::never);

				if (LOG_PIT) logerror("PIT counter %d set to %d (%d Hz)\n", which, ctr->count, 4000000 / ctr->count);

				/* set the frequency of the associated DAC */
				if (!state->m_is_redline)
					set_dac_frequency(state, which, 4000000 / ctr->count);
				else
				{
					if (which < 5)
						set_dac_frequency(state, which, 7000000 / ctr->count);
					else if (which == 6)
					{
						set_dac_frequency(state, 5, 7000000 / ctr->count);
						set_dac_frequency(state, 6, 7000000 / ctr->count);
						set_dac_frequency(state, 7, 7000000 / ctr->count);
					}
				}
			}
			break;

		case 3:
			/* determine which counter */
			if ((data & 0xc0) == 0xc0) break;
			which = (which * 3) + (data >> 6);
			ctr = &state->m_counter[which];

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

WRITE8_DEVICE_HANDLER( leland_80186_control_w )
{
	leland_sound_state *state = get_safe_token(device);

	/* see if anything changed */
	int diff = (state->m_last_control ^ data) & 0xf8;
	if (!diff)
		return;
	state->m_last_control = data;

	if (LOG_COMM)
	{
		logerror("%04X:80186 control = %02X", state->m_i80186.cpu->safe_pcbase(), data);
		if (!(data & 0x80)) logerror("  /RESET");
		if (!(data & 0x40)) logerror("  ZNMI");
		if (!(data & 0x20)) logerror("  INT0");
		if (!(data & 0x10)) logerror("  /TEST");
		if (!(data & 0x08)) logerror("  INT1");
		logerror("\n");
	}

	/* /RESET */
	device->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, data & 0x80  ? CLEAR_LINE : ASSERT_LINE);

	/* /NMI */
/*  If the master CPU doesn't get a response by the time it's ready to send
    the next command, it uses an NMI to force the issue; unfortunately, this
    seems to really screw up the sound system. It turns out it's better to
    just wait for the original interrupt to occur naturally */
/*  device->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_NMI, data & 0x40  ? CLEAR_LINE : ASSERT_LINE);*/

	/* INT0 */
	if (data & 0x20)
	{
		if (!LATCH_INTS) state->m_i80186.intr.request &= ~0x10;
	}
	else if (state->m_i80186.intr.ext[0] & 0x10)
		state->m_i80186.intr.request |= 0x10;
	else if (diff & 0x20)
		state->m_i80186.intr.request |= 0x10;

	/* INT1 */
	if (data & 0x08)
	{
		if (!LATCH_INTS) state->m_i80186.intr.request &= ~0x20;
	}
	else if (state->m_i80186.intr.ext[1] & 0x10)
		state->m_i80186.intr.request |= 0x20;
	else if (diff & 0x08)
		state->m_i80186.intr.request |= 0x20;

	/* handle reset here */
	if ((diff & 0x80) && (data & 0x80))
		leland_80186_reset(device);

	update_interrupt_state(device);
}



/*************************************
 *
 *  Sound command handling
 *
 *************************************/

static TIMER_CALLBACK( command_lo_sync )
{
	device_t *device = (device_t *)ptr;
	leland_sound_state *state = get_safe_token(device);
	if (LOG_COMM) logerror("%s:Write sound command latch lo = %02X\n", machine.describe_context(), param);
	state->m_sound_command = (state->m_sound_command & 0xff00) | param;
}


WRITE8_DEVICE_HANDLER( leland_80186_command_lo_w )
{
	device->machine().scheduler().synchronize(FUNC(command_lo_sync), data, device);
}


WRITE8_DEVICE_HANDLER( leland_80186_command_hi_w )
{
	leland_sound_state *state = get_safe_token(device);
	if (LOG_COMM) logerror("%04X:Write sound command latch hi = %02X\n", state->m_i80186.cpu->safe_pcbase(), data);
	state->m_sound_command = (state->m_sound_command & 0x00ff) | (data << 8);
}


static READ16_DEVICE_HANDLER( main_to_sound_comm_r )
{
	leland_sound_state *state = get_safe_token(device);
	if (LOG_COMM) logerror("%05X:Read sound command latch = %02X\n", state->m_i80186.cpu->safe_pc(), state->m_sound_command);
	return state->m_sound_command;
}




/*************************************
 *
 *  Sound response handling
 *
 *************************************/

static TIMER_CALLBACK( delayed_response_r )
{
	device_t *device = (device_t *)ptr;
	leland_sound_state *state = get_safe_token(device);
	cpu_device *master = machine.device<cpu_device>("master");
	int checkpc = param;
	int pc = master->pc();
	int oldaf = master->state_int(Z80_AF);

	/* This is pretty cheesy, but necessary. Since the CPUs run in round-robin order,
       synchronizing on the write to this register from the slave side does nothing.
       In order to make sure the master CPU get the real response, we synchronize on
       the read. However, the value we returned the first time around may not be
       accurate, so after the system has synced up, we go back into the master CPUs
       state and put the proper value into the A register. */
	if (pc == checkpc)
	{
		if (LOG_COMM) logerror("(Updated sound response latch to %02X)\n", state->m_sound_response);

		oldaf = (oldaf & 0x00ff) | (state->m_sound_response << 8);
		master->set_state_int(Z80_AF, oldaf);
	}
	else
		logerror("ERROR: delayed_response_r - current PC = %04X, checkPC = %04X\n", pc, checkpc);
}


READ8_DEVICE_HANDLER( leland_80186_response_r )
{
	leland_sound_state *state = get_safe_token(device);
	offs_t pc = state->m_i80186.cpu->safe_pcbase();

	if (LOG_COMM) logerror("%04X:Read sound response latch = %02X\n", pc, state->m_sound_response);

	/* synchronize the response */
	device->machine().scheduler().synchronize(FUNC(delayed_response_r), pc + 2, device);
	return state->m_sound_response;
}


static WRITE16_DEVICE_HANDLER( sound_to_main_comm_w )
{
	leland_sound_state *state = get_safe_token(device);
	if (LOG_COMM) logerror("%05X:Write sound response latch = %02X\n", state->m_i80186.cpu->safe_pc(), data);
	state->m_sound_response = data;
}



/*************************************
 *
 *  Low-level DAC I/O
 *
 *************************************/

static void set_dac_frequency(leland_sound_state *state, int which, int frequency)
{
	struct dac_state *d = &state->m_dac[which];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

	/* set the frequency of the associated DAC */
	d->frequency = frequency;
	d->step = (int)((double)frequency * (double)(1 << 24) / (double)OUTPUT_RATE);

	/* also determine the target buffer size */
	d->buftarget = d->frequency / 60 + 50;
	if (d->buftarget > DAC_BUFFER_SIZE - 1)
		d->buftarget = DAC_BUFFER_SIZE - 1;

	/* reevaluate the count */
	if (count > d->buftarget)
		state->m_clock_active &= ~(1 << which);
	else if (count < d->buftarget)
		state->m_clock_active |= 1 << which;

	if (LOG_DAC) logerror("DAC %d frequency = %d, step = %08X\n", which, d->frequency, d->step);
}


static WRITE16_DEVICE_HANDLER( dac_w )
{
	leland_sound_state *state = get_safe_token(device);
	int which = offset;
	struct dac_state *d = &state->m_dac[which];

	/* handle value changes */
	if (ACCESSING_BITS_0_7)
	{
		int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

		/* set the new value */
		d->value = (INT16)(UINT8)data - 0x80;
		if (LOG_DAC) logerror("%05X:DAC %d value = %02X\n", state->m_i80186.cpu->safe_pc(), offset, (UINT8)data);

		/* if we haven't overflowed the buffer, add the value value to it */
		if (count < DAC_BUFFER_SIZE - 1)
		{
			/* if this is the first byte, sync the stream */
			if (count == 0)
				state->m_nondma_stream->update();

			/* prescale by the volume */
			d->buffer[d->bufin] = d->value * d->volume;
			d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

			/* update the clock status */
			if (++count > d->buftarget)
				state->m_clock_active &= ~(1 << which);
		}
	}

	/* handle volume changes */
	if (ACCESSING_BITS_8_15)
	{
		d->volume = ((data >> 8) ^ 0x00) / DAC_VOLUME_SCALE;
		if (LOG_DAC) logerror("%05X:DAC %d volume = %02X\n", state->m_i80186.cpu->safe_pc(), offset, data);
	}
}


static WRITE16_DEVICE_HANDLER( redline_dac_w )
{
	leland_sound_state *state = get_safe_token(device);
	int which = offset / 0x100;
	struct dac_state *d = &state->m_dac[which];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;

	/* set the new value */
	d->value = (INT16)(UINT8)data - 0x80;

	/* if we haven't overflowed the buffer, add the value value to it */
	if (count < DAC_BUFFER_SIZE - 1)
	{
		/* if this is the first byte, sync the stream */
		if (count == 0)
			state->m_nondma_stream->update();

		/* prescale by the volume */
		d->buffer[d->bufin] = d->value * d->volume;
		d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

		/* update the clock status */
		if (++count > d->buftarget)
			state->m_clock_active &= ~(1 << which);
	}

	/* update the volume */
	d->volume = (offset & 0xff) / DAC_VOLUME_SCALE;
	if (LOG_DAC) logerror("%05X:DAC %d value = %02X, volume = %02X\n", state->m_i80186.cpu->safe_pc(), which, data, (offset & 0x1fe) / 2);
}


static WRITE16_DEVICE_HANDLER( dac_10bit_w )
{
	leland_sound_state *state = get_safe_token(device);
	struct dac_state *d = &state->m_dac[6];
	int count = (d->bufin - d->bufout) & DAC_BUFFER_SIZE_MASK;
	int data16;

	/* warning: this assumes all port writes here are word-sized */
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	data16 = data;

	/* set the new value */
	d->value = (INT16)data16 - 0x200;
	if (LOG_DAC) logerror("%05X:DAC 10-bit value = %02X\n", state->m_i80186.cpu->safe_pc(), data16);

	/* if we haven't overflowed the buffer, add the value value to it */
	if (count < DAC_BUFFER_SIZE - 1)
	{
		/* if this is the first byte, sync the stream */
		if (count == 0)
			state->m_nondma_stream->update();

		/* prescale by the volume */
		d->buffer[d->bufin] = d->value * (0xff / DAC_VOLUME_SCALE / 2);
		d->bufin = (d->bufin + 1) & DAC_BUFFER_SIZE_MASK;

		/* update the clock status */
		if (++count > d->buftarget)
			state->m_clock_active &= ~0x40;
	}
}


static WRITE16_DEVICE_HANDLER( ataxx_dac_control )
{
	leland_sound_state *state = get_safe_token(device);

	/* handle common offsets */
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if (ACCESSING_BITS_0_7)
				dac_w(device, offset, data, 0x00ff);
			return;

		case 0x03:
			dac_w(device, 0, ((data << 13) & 0xe000) | ((data << 10) & 0x1c00) | ((data << 7) & 0x0300), 0xff00);
			dac_w(device, 2, ((data << 10) & 0xe000) | ((data <<  7) & 0x1c00) | ((data << 4) & 0x0300), 0xff00);
			dac_w(device, 4, ((data <<  8) & 0xc000) | ((data <<  6) & 0x3000) | ((data << 4) & 0x0c00) | ((data << 2) & 0x0300), 0xff00);
			return;
	}

	/* if we have a YM2151 (and an external DAC), handle those offsets */
	if (state->m_has_ym2151)
	{
		state->m_extern_stream->update();
		switch (offset)
		{
			case 0x04:
				state->m_ext_active = 1;
				if (LOG_EXTERN) logerror("External DAC active\n");
				return;

			case 0x05:
				state->m_ext_active = 0;
				if (LOG_EXTERN) logerror("External DAC inactive\n");
				return;

			case 0x06:
				state->m_ext_start >>= 4;
				COMBINE_DATA(&state->m_ext_start);
				state->m_ext_start <<= 4;
				if (LOG_EXTERN) logerror("External DAC start = %05X\n", state->m_ext_start);
				return;

			case 0x07:
				state->m_ext_stop >>= 4;
				COMBINE_DATA(&state->m_ext_stop);
				state->m_ext_stop <<= 4;
				if (LOG_EXTERN) logerror("External DAC stop = %05X\n", state->m_ext_stop);
				return;

			case 0x21:
				dac_w(device, offset - 0x21 + 7, data, mem_mask);
				return;
		}
	}
	logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", state->m_i80186.cpu->safe_pc(), 5, offset, data);
}



/*************************************
 *
 *  Peripheral chip dispatcher
 *
 *************************************/

static READ16_DEVICE_HANDLER( peripheral_r )
{
	leland_sound_state *state = get_safe_token(device);
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 0:
			/* we have to return 0 periodically so that they handle interrupts */
			if ((++state->m_clock_tick & 7) == 0)
				return 0;

			/* if we've filled up all the active channels, we can give this CPU a reset */
			/* until the next interrupt */
			if (!state->m_is_redline)
				return ((state->m_clock_active >> 1) & 0x3e);
			else
				return ((state->m_clock_active << 1) & 0x7e);

		case 1:
			return main_to_sound_comm_r(device, offset, mem_mask);

		case 2:
			return pit8254_r(device, offset, mem_mask);

		case 3:
			if (!state->m_has_ym2151)
				return pit8254_r(device, offset | 0x40, mem_mask);
			else
				return ym2151_r(device->machine().device("ymsnd"), offset);

		case 4:
			if (state->m_is_redline)
				return pit8254_r(device, offset | 0x80, mem_mask);
			else
				logerror("%05X:Unexpected peripheral read %d/%02X\n", state->m_i80186.cpu->safe_pc(), select, offset*2);
			break;

		default:
			logerror("%05X:Unexpected peripheral read %d/%02X\n", state->m_i80186.cpu->safe_pc(), select, offset*2);
			break;
	}
	return 0xffff;
}


static WRITE16_DEVICE_HANDLER( peripheral_w )
{
	leland_sound_state *state = get_safe_token(device);
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 1:
			sound_to_main_comm_w(device, offset, data, mem_mask);
			break;

		case 2:
			pit8254_w(device, offset, data, mem_mask);
			break;

		case 3:
			if (!state->m_has_ym2151)
				pit8254_w(device, offset | 0x40, data, mem_mask);
			else
				ym2151_w(device->machine().device("ymsnd"), offset, data);
			break;

		case 4:
			if (state->m_is_redline)
				pit8254_w(device, offset | 0x80, data, mem_mask);
			else
				dac_10bit_w(device, offset, data, mem_mask);
			break;

		case 5:	/* Ataxx/WSF/Indy Heat only */
			ataxx_dac_control(device, offset, data, mem_mask);
			break;

		default:
			logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", state->m_i80186.cpu->safe_pc(), select, offset, data);
			break;
	}
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( ataxx_80186_control_w )
{
	/* compute the bit-shuffled variants of the bits and then write them */
	int modified =	((data & 0x01) << 7) |
					((data & 0x02) << 5) |
					((data & 0x04) << 3) |
					((data & 0x08) << 1);
	leland_80186_control_w(device, offset, modified);
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

ADDRESS_MAP_START( leland_80186_map_program, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x00000, 0x03fff) AM_MIRROR(0x1c000) AM_RAM
	AM_RANGE(0x20000, 0xfffff) AM_ROM
ADDRESS_MAP_END


ADDRESS_MAP_START( ataxx_80186_map_io, AS_IO, 16, driver_device )
	AM_RANGE(0xff00, 0xffff) AM_DEVREADWRITE_LEGACY("custom", i80186_internal_port_r, i80186_internal_port_w)
ADDRESS_MAP_END


ADDRESS_MAP_START( redline_80186_map_io, AS_IO, 16, driver_device )
	AM_RANGE(0x6000, 0x6fff) AM_DEVWRITE_LEGACY("custom", redline_dac_w)
	AM_RANGE(0xff00, 0xffff) AM_DEVREADWRITE_LEGACY("custom", i80186_internal_port_r, i80186_internal_port_w)
ADDRESS_MAP_END


ADDRESS_MAP_START( leland_80186_map_io, AS_IO, 16, driver_device )
	AM_RANGE(0x0000, 0x000b) AM_DEVWRITE_LEGACY("custom", dac_w)
	AM_RANGE(0x0080, 0x008b) AM_DEVWRITE_LEGACY("custom", dac_w)
	AM_RANGE(0x00c0, 0x00cb) AM_DEVWRITE_LEGACY("custom", dac_w)
	AM_RANGE(0xff00, 0xffff) AM_DEVREADWRITE_LEGACY("custom", i80186_internal_port_r, i80186_internal_port_w)
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
