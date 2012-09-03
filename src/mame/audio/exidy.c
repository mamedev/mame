/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/6532riot.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"
#include "audio/exidy.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CRYSTAL_OSC				(XTAL_3_579545MHz)
#define SH8253_CLOCK			(CRYSTAL_OSC / 2)
#define SH6840_CLOCK			(CRYSTAL_OSC / 4)
#define SH6532_CLOCK			(CRYSTAL_OSC / 4)
#define CVSD_CLOCK				(1.0 / (0.693 * (RES_K(2.4) + 2.0 * RES_K(20)) * CAP_P(2200)))
#define CVSD_Z80_CLOCK			(CRYSTAL_OSC / 2)
#define BASE_VOLUME				(32767 / 6)

enum
{
	RIOT_IDLE,
	RIOT_COUNT,
	RIOT_POST_COUNT
};



/*************************************
 *
 *  Local variables
 *
 *************************************/

/* 6840 variables */
struct sh6840_timer_channel
{
	UINT8	cr;
	UINT8	state;
	UINT8	leftovers;
	UINT16	timer;
	UINT32	clocks;
	union
	{
#ifdef LSB_FIRST
		struct { UINT8 l, h; } b;
#else
		struct { UINT8 h, l; } b;
#endif
		UINT16 w;
	} counter;
};

struct sh8253_timer_channel
{
	UINT8	clstate;
	UINT8	enable;
	UINT16	count;
	UINT32	step;
	UINT32	fraction;
};

typedef struct _exidy_sound_state exidy_sound_state;
struct _exidy_sound_state
{
	device_t *m_maincpu;

	/* IRQ variable */
	UINT8 m_riot_irq_state;

	/* 6532 variables */
	device_t *m_riot;

	struct sh6840_timer_channel m_sh6840_timer[3];
	INT16 m_sh6840_volume[3];
	UINT8 m_sh6840_MSB_latch;
	UINT8 m_sh6840_LSB_latch;
	UINT8 m_sh6840_LFSR_oldxor;
	UINT32 m_sh6840_LFSR_0;
	UINT32 m_sh6840_LFSR_1;
	UINT32 m_sh6840_LFSR_2;
	UINT32 m_sh6840_LFSR_3;
	UINT32 m_sh6840_clocks_per_sample;
	UINT32 m_sh6840_clock_count;

	UINT8 m_sfxctrl;

	/* 8253 variables */
	struct sh8253_timer_channel m_sh8253_timer[3];
	int m_has_sh8253;

	/* 5220/CVSD variables */
	device_t *m_cvsd;
	device_t *m_tms;
	pia6821_device *m_pia1;

	/* sound streaming variables */
	sound_stream *m_stream;
	double m_freq_to_step;

	UINT8 m_victory_sound_response_ack_clk;	/* 7474 @ F4 */
};


INLINE exidy_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == EXIDY || device->type() == EXIDY_VENTURE || device->type() == EXIDY_VICTORY);

	return (exidy_sound_state *)downcast<exidy_sound_device *>(device)->token();
}

/*************************************
 *
 *  Interrupt generation helper
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( update_irq_state )
{
	exidy_sound_state *sndstate = get_safe_token(device);
	cputag_set_input_line(device->machine(), "audiocpu", M6502_IRQ_LINE, (sndstate->m_pia1->irq_b_state() | sndstate->m_riot_irq_state) ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  6840 clock counting helper
 *
 *************************************/

INLINE void sh6840_apply_clock(struct sh6840_timer_channel *t, int clocks)
{
	/* dual 8-bit case */
	if (t->cr & 0x04)
	{
		/* handle full decrements */
		while (clocks > t->counter.b.l)
		{
			clocks -= t->counter.b.l + 1;
			t->counter.b.l = t->timer;

			/* decrement MSB */
			if (!t->counter.b.h--)
			{
				t->state = 0;
				t->counter.w = t->timer;
			}

			/* state goes high when MSB is 0 */
			else if (!t->counter.b.h)
			{
				t->state = 1;
				t->clocks++;
			}
		}

		/* subtract off the remainder */
		t->counter.b.l -= clocks;
	}

	/* 16-bit case */
	else
	{
		/* handle full decrements */
		while (clocks > t->counter.w)
		{
			clocks -= t->counter.w + 1;
			t->state ^= 1;
			t->clocks += t->state;
			t->counter.w = t->timer;
		}

		/* subtract off the remainder */
		t->counter.w -= clocks;
	}
}



/*************************************
 *
 *  Noise generation helper
 *
 *************************************/

INLINE int sh6840_update_noise(exidy_sound_state *state, int clocks)
{
	UINT32 newxor;
	int noise_clocks = 0;
	int i;

	/* loop over clocks */
	for (i = 0; i < clocks; i++)
	{
		/* shift the LFSR. its a LOOOONG LFSR, so we need
        * four longs to hold it all!
        * first we grab new sample, then shift the high bits,
        * then the low ones; finally or in the result and see if we've
        * had a 0->1 transition */
		newxor = (state->m_sh6840_LFSR_3 ^ state->m_sh6840_LFSR_2) >> 31; /* high bits of 3 and 2 xored is new xor */
		state->m_sh6840_LFSR_3 <<= 1;
		state->m_sh6840_LFSR_3 |= state->m_sh6840_LFSR_2 >> 31;
		state->m_sh6840_LFSR_2 <<= 1;
		state->m_sh6840_LFSR_2 |= state->m_sh6840_LFSR_1 >> 31;
		state->m_sh6840_LFSR_1 <<= 1;
		state->m_sh6840_LFSR_1 |= state->m_sh6840_LFSR_0 >> 31;
		state->m_sh6840_LFSR_0 <<= 1;
		state->m_sh6840_LFSR_0 |= newxor ^ state->m_sh6840_LFSR_oldxor;
		state->m_sh6840_LFSR_oldxor = newxor;
		/*printf("LFSR: %4x, %4x, %4x, %4x\n", sh6840_LFSR_3, sh6840_LFSR_2, sh6840_LFSR_1, sh6840_LFSR_0);*/
		/* if we clocked 0->1, that will serve as an external clock */
		if ((state->m_sh6840_LFSR_2 & 0x03) == 0x01) /* tap is at 96th bit */
		{
			noise_clocks++;
		}
	}
	return noise_clocks;
}



/*************************************
 *
 *  6840 state saving
 *
 *************************************/

static void sh6840_register_state_globals(device_t *device)
{
	exidy_sound_state *state = get_safe_token(device);

	device->save_item(NAME(state->m_sh6840_volume));
	device->save_item(NAME(state->m_sh6840_MSB_latch));
	device->save_item(NAME(state->m_sh6840_LSB_latch));
	device->save_item(NAME(state->m_sh6840_LFSR_oldxor));
	device->save_item(NAME(state->m_sh6840_LFSR_0));
	device->save_item(NAME(state->m_sh6840_LFSR_1));
	device->save_item(NAME(state->m_sh6840_LFSR_2));
	device->save_item(NAME(state->m_sh6840_LFSR_3));
	device->save_item(NAME(state->m_sh6840_clock_count));
	device->save_item(NAME(state->m_sfxctrl));
	device->save_item(NAME(state->m_sh6840_timer[0].cr));
	device->save_item(NAME(state->m_sh6840_timer[0].state));
	device->save_item(NAME(state->m_sh6840_timer[0].leftovers));
	device->save_item(NAME(state->m_sh6840_timer[0].timer));
	device->save_item(NAME(state->m_sh6840_timer[0].clocks));
	device->save_item(NAME(state->m_sh6840_timer[0].counter.w));
	device->save_item(NAME(state->m_sh6840_timer[1].cr));
	device->save_item(NAME(state->m_sh6840_timer[1].state));
	device->save_item(NAME(state->m_sh6840_timer[1].leftovers));
	device->save_item(NAME(state->m_sh6840_timer[1].timer));
	device->save_item(NAME(state->m_sh6840_timer[1].clocks));
	device->save_item(NAME(state->m_sh6840_timer[1].counter.w));
	device->save_item(NAME(state->m_sh6840_timer[2].cr));
	device->save_item(NAME(state->m_sh6840_timer[2].state));
	device->save_item(NAME(state->m_sh6840_timer[2].leftovers));
	device->save_item(NAME(state->m_sh6840_timer[2].timer));
	device->save_item(NAME(state->m_sh6840_timer[2].clocks));
	device->save_item(NAME(state->m_sh6840_timer[2].counter.w));
}



/*************************************
 *
 *  Core sound generation
 *
 *************************************/

static STREAM_UPDATE( exidy_stream_update )
{
	exidy_sound_state *state = get_safe_token(device);
	struct sh6840_timer_channel *sh6840_timer = state->m_sh6840_timer;

	/* hack to skip the expensive lfsr noise generation unless at least one of the 3 channels actually depends on it being generated */
	int noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
	{
		struct sh6840_timer_channel *t;
		struct sh8253_timer_channel *c;
		int clocks_this_sample;
		int clocks;
		INT16 sample = 0;

		/* determine how many 6840 clocks this sample */
		state->m_sh6840_clock_count += state->m_sh6840_clocks_per_sample;
		clocks_this_sample = state->m_sh6840_clock_count >> 24;
		state->m_sh6840_clock_count &= (1 << 24) - 1;

		/* skip if nothing enabled */
		if ((sh6840_timer[0].cr & 0x01) == 0)
		{
			int noise_clocks_this_sample = 0;
			UINT32 chan0_clocks;

			/* generate E-clocked noise if configured to do so */
			if (noisy && !(state->m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(state, clocks_this_sample);

			/* handle timer 0 if enabled */
			t = &sh6840_timer[0];
			chan0_clocks = t->clocks;
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && !(state->m_sfxctrl & 0x02) && (t->cr & 0x80))
				sample += state->m_sh6840_volume[0];

			/* generate channel 0-clocked noise if configured to do so */
			if (noisy && (state->m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(state, t->clocks - chan0_clocks);

			/* handle timer 1 if enabled */
			t = &sh6840_timer[1];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += state->m_sh6840_volume[1];

			/* handle timer 2 if enabled */
			t = &sh6840_timer[2];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			/* prescale */
			if (t->cr & 0x01)
			{
				clocks += t->leftovers;
				t->leftovers = clocks % 8;
				clocks /= 8;
			}
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += state->m_sh6840_volume[2];
		}

		/* music (if present) */
		if (state->m_has_sh8253)
		{
			/* music channel 0 */
			c = &state->m_sh8253_timer[0];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 1 */
			c = &state->m_sh8253_timer[1];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 2 */
			c = &state->m_sh8253_timer[2];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}
		}

		/* stash */
		*buffer++ = sample;
	}
}



/*************************************
 *
 *  Audio startup routines
 *
 *************************************/

static DEVICE_START( common_sh_start )
{
	exidy_sound_state *state = get_safe_token(device);
	int sample_rate = SH8253_CLOCK;

	state->m_sh6840_clocks_per_sample = (int)((double)SH6840_CLOCK / (double)sample_rate * (double)(1 << 24));

	/* allocate the stream */
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, sample_rate, NULL, exidy_stream_update);
	state->m_maincpu = device->machine().device("maincpu");

	sh6840_register_state_globals(device);
}

static DEVICE_START( exidy_sound )
{
	exidy_sound_state *state = get_safe_token(device);

	/* indicate no additional hardware */
	state->m_has_sh8253  = FALSE;
	state->m_tms = NULL;
	state->m_cvsd = NULL;

	DEVICE_START_CALL(common_sh_start);
}

const device_type EXIDY = &device_creator<exidy_sound_device>;

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EXIDY, "Exidy SFX", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(exidy_sound_state));
}

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(exidy_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void exidy_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sound_device::device_start()
{
	DEVICE_START_NAME( exidy_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

static DEVICE_RESET( exidy_sound );

void exidy_sound_device::device_reset()
{
	DEVICE_RESET_NAME( exidy_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}



/*************************************
 *
 *  Audio reset routines
 *
 *************************************/

static DEVICE_RESET( common_sh_reset )
{
	exidy_sound_state *state = get_safe_token(device);

	/* 6840 */
	memset(state->m_sh6840_timer, 0, sizeof(state->m_sh6840_timer));
	state->m_sh6840_MSB_latch = 0;
	state->m_sh6840_LSB_latch = 0;
	state->m_sh6840_volume[0] = 0;
	state->m_sh6840_volume[1] = 0;
	state->m_sh6840_volume[2] = 0;
	state->m_sh6840_clock_count = 0;
	state->m_sfxctrl = 0;

	/* LFSR */
	state->m_sh6840_LFSR_oldxor = 0;
	state->m_sh6840_LFSR_0 = 0xffffffff;
	state->m_sh6840_LFSR_1 = 0xffffffff;
	state->m_sh6840_LFSR_2 = 0xffffffff;
	state->m_sh6840_LFSR_3 = 0xffffffff;
}

static DEVICE_RESET( exidy_sound )
{
	DEVICE_RESET_CALL(common_sh_reset);
}


/*************************************
 *
 *  6532 interface
 *
 *************************************/

static void r6532_irq(device_t *device, int state)
{
	exidy_sound_state *sndstate = get_safe_token(device);
	sndstate->m_riot_irq_state = (state == ASSERT_LINE) ? 1 : 0;
	update_irq_state(device, 0);
}


static WRITE8_DEVICE_HANDLER( r6532_porta_w )
{
	exidy_sound_state *state = get_safe_token(device);
	if (state->m_cvsd != NULL)
		cputag_set_input_line(device->machine(), "cvsdcpu", INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	if (state->m_tms != NULL)
	{
		logerror("(%f)%s:TMS5220 data write = %02X\n", device->machine().time().as_double(), device->machine().describe_context(), riot6532_porta_out_get(state->m_riot));
		tms5220_data_w(state->m_tms, 0, data);
	}
}

static READ8_DEVICE_HANDLER( r6532_porta_r )
{
	exidy_sound_state *state = get_safe_token(device);
	if (state->m_tms != NULL)
	{
		logerror("(%f)%s:TMS5220 status read = %02X\n", device->machine().time().as_double(), device->machine().describe_context(), tms5220_status_r(state->m_tms, 0));
		return tms5220_status_r(state->m_tms, 0);
	}
	else
		return 0xff;
}

static WRITE8_DEVICE_HANDLER( r6532_portb_w )
{
	exidy_sound_state *state = get_safe_token(device);
	if (state->m_tms != NULL)
	{
		tms5220_rsq_w(state->m_tms, data & 0x01);
		tms5220_wsq_w(state->m_tms, (data >> 1) & 0x01);
	}
}


static READ8_DEVICE_HANDLER( r6532_portb_r )
{
	exidy_sound_state *state = get_safe_token(device);
	UINT8 newdata = riot6532_portb_in_get(state->m_riot);
	if (state->m_tms != NULL)
	{
		newdata &= ~0x0c;
		if (tms5220_readyq_r(state->m_tms)) newdata |= 0x04;
		if (tms5220_intq_r(state->m_tms)) newdata |= 0x08;
	}
	return newdata;
}


static const riot6532_interface r6532_interface =
{
	DEVCB_DEVICE_HANDLER("custom", r6532_porta_r),	/* port A read handler */
	DEVCB_DEVICE_HANDLER("custom", r6532_portb_r),	/* port B read handler */
	DEVCB_DEVICE_HANDLER("custom", r6532_porta_w),	/* port A write handler */
	DEVCB_DEVICE_HANDLER("custom", r6532_portb_w),	/* port B write handler */
	DEVCB_DEVICE_LINE("custom", r6532_irq)			/* IRQ callback */
};



/*************************************
 *
 *  8253 state saving
 *
 *************************************/


static void sh8253_register_state_globals(device_t *device)
{
	exidy_sound_state *state = get_safe_token(device);

	device->save_item(NAME(state->m_sh8253_timer[0].clstate));
	device->save_item(NAME(state->m_sh8253_timer[0].enable));
	device->save_item(NAME(state->m_sh8253_timer[0].count));
	device->save_item(NAME(state->m_sh8253_timer[0].step));
	device->save_item(NAME(state->m_sh8253_timer[0].fraction));
	device->save_item(NAME(state->m_sh8253_timer[1].clstate));
	device->save_item(NAME(state->m_sh8253_timer[1].enable));
	device->save_item(NAME(state->m_sh8253_timer[1].count));
	device->save_item(NAME(state->m_sh8253_timer[1].step));
	device->save_item(NAME(state->m_sh8253_timer[1].fraction));
	device->save_item(NAME(state->m_sh8253_timer[2].clstate));
	device->save_item(NAME(state->m_sh8253_timer[2].enable));
	device->save_item(NAME(state->m_sh8253_timer[2].count));
	device->save_item(NAME(state->m_sh8253_timer[2].step));
	device->save_item(NAME(state->m_sh8253_timer[2].fraction));
}

/*************************************
 *
 *  8253 timer handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( exidy_sh8253_w )
{
	exidy_sound_state *state = get_safe_token(device);
	int chan;

	state->m_stream->update();

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			chan = offset;
			if (!state->m_sh8253_timer[chan].clstate)
			{
				state->m_sh8253_timer[chan].clstate = 1;
				state->m_sh8253_timer[chan].count = (state->m_sh8253_timer[chan].count & 0xff00) | (data & 0x00ff);
			}
			else
			{
				state->m_sh8253_timer[chan].clstate = 0;
				state->m_sh8253_timer[chan].count = (state->m_sh8253_timer[chan].count & 0x00ff) | ((data << 8) & 0xff00);
				if (state->m_sh8253_timer[chan].count)
					state->m_sh8253_timer[chan].step = state->m_freq_to_step * (double)SH8253_CLOCK / (double)state->m_sh8253_timer[chan].count;
				else
					state->m_sh8253_timer[chan].step = 0;
			}
			break;

		case 3:
			chan = (data & 0xc0) >> 6;
			state->m_sh8253_timer[chan].enable = ((data & 0x0e) != 0);
			break;
	}
}


static READ8_DEVICE_HANDLER( exidy_sh8253_r )
{
	logerror("8253(R): %x\n",offset);

	return 0;
}



/*************************************
 *
 *  6840 timer handlers
 *
 *************************************/

READ8_DEVICE_HANDLER( exidy_sh6840_r )
{
	exidy_sound_state *state = get_safe_token(device);

	/* force an update of the stream */
	state->m_stream->update();

	switch (offset)
	{
		/* offset 0: Motorola datasheet says it isn't used, Hitachi datasheet says it reads as 0s always*/
		case 0:
		return 0;
		/* offset 1 reads the status register: bits 2 1 0 correspond to ints on channels 2,1,0, and bit 7 is an 'OR' of bits 2,1,0 */
		case 1:
		logerror("%04X:exidy_sh6840_r - unexpected read, status register is TODO!\n", cpu_get_pc(state->m_maincpu));
		return 0;
		/* offsets 2,4,6 read channel 0,1,2 MSBs and latch the LSB*/
		case 2: case 4: case 6:
		state->m_sh6840_LSB_latch = state->m_sh6840_timer[((offset<<1)-1)].counter.b.l;
		return state->m_sh6840_timer[((offset<<1)-1)].counter.b.h;
		/* offsets 3,5,7 read the LSB latch*/
		default: /* case 3,5,7 */
		return state->m_sh6840_LSB_latch;
	}
}


WRITE8_DEVICE_HANDLER( exidy_sh6840_w )
{
	exidy_sound_state *state = get_safe_token(device);
	struct sh6840_timer_channel *sh6840_timer = state->m_sh6840_timer;

	/* force an update of the stream */
	state->m_stream->update();

	switch (offset)
	{
		/* offset 0 writes to either channel 0 control or channel 2 control */
		case 0:
			if (sh6840_timer[1].cr & 0x01)
				sh6840_timer[0].cr = data;
			else
				sh6840_timer[2].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel %d configured for mode %d", (sh6840_timer[1].cr & 0x01) ? 0 : 2, (data >> 3) & 7);
			break;

		/* offset 1 writes to channel 1 control */
		case 1:
			sh6840_timer[1].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel 1 configured for mode %d", (data >> 3) & 7);
			break;

		/* offsets 2/4/6 write to the common MSB latch */
		case 2:
		case 4:
		case 6:
			state->m_sh6840_MSB_latch = data;
			break;

		/* offsets 3/5/7 write to the LSB controls */
		case 3:
		case 5:
		case 7:
		{
			/* latch the timer value */
			int ch = (offset - 3) / 2;
			sh6840_timer[ch].timer = (state->m_sh6840_MSB_latch << 8) | (data & 0xff);

			/* if CR4 is clear, the value is loaded immediately */
			if (!(sh6840_timer[ch].cr & 0x10))
				sh6840_timer[ch].counter.w = sh6840_timer[ch].timer;
			break;
		}
	}
}



/*************************************
 *
 *  External sound effect controls
 *
 *************************************/

WRITE8_DEVICE_HANDLER( exidy_sfxctrl_w )
{
	exidy_sound_state *state = get_safe_token(device);

	state->m_stream->update();

	switch (offset)
	{
		case 0:
			state->m_sfxctrl = data;
			break;

		case 1:
		case 2:
		case 3:
			state->m_sh6840_volume[offset - 1] = ((data & 7) * BASE_VOLUME) / 7;
			break;
	}
}



/*************************************
 *
 *  Sound filter control
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( exidy_sound_filter_w )
{
	logerror("exidy_sound_filter_w = %02X\n", data);
}



/*************************************
 *
 *  Venture, etc.
 *
 *************************************/

static const pia6821_interface venture_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_MEMBER("pia1", pia6821_device, portb_w),		/* port A out */
	DEVCB_DEVICE_MEMBER("pia1", pia6821_device, porta_w),		/* port B out */
	DEVCB_DEVICE_LINE_MEMBER("pia1", pia6821_device, cb1_w),		/* line CA2 out */
	DEVCB_DEVICE_LINE_MEMBER("pia1", pia6821_device, ca1_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


static const pia6821_interface venture_pia1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_MEMBER("pia0", pia6821_device, portb_w),		/* port A out */
	DEVCB_DEVICE_MEMBER("pia0", pia6821_device, porta_w),		/* port B out */
	DEVCB_DEVICE_LINE_MEMBER("pia0", pia6821_device, cb1_w),		/* line CA2 out */
	DEVCB_DEVICE_LINE_MEMBER("pia0", pia6821_device, ca1_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_DEVICE_LINE("custom", update_irq_state)		/* IRQB */
};


static DEVICE_START( venture_common_sh_start )
{
	exidy_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();

	DEVICE_START_CALL(common_sh_start);

	state->m_riot = machine.device("riot");

	state->m_has_sh8253  = TRUE;
	state->m_tms = NULL;
	state->m_pia1 = device->machine().device<pia6821_device>("pia1");

	/* determine which sound hardware is installed */
	state->m_cvsd = device->machine().device("cvsd");

	/* 8253 */
	state->m_freq_to_step = (double)(1 << 24) / (double)SH8253_CLOCK;

	device->save_item(NAME(state->m_riot_irq_state));
	sh8253_register_state_globals(device);
}


static DEVICE_START( venture_sound )
{
	DEVICE_START_CALL(venture_common_sh_start);
}


static DEVICE_RESET( venture_sound )
{
	exidy_sound_state *state = get_safe_token(device);

	DEVICE_RESET_CALL(common_sh_reset);

	/* PIA */
	devtag_reset(device->machine(), "pia0");
	devtag_reset(device->machine(), "pia1");

	/* 6532 */
	state->m_riot->reset();

	/* 8253 */
	memset(state->m_sh8253_timer, 0, sizeof(state->m_sh8253_timer));
}


const device_type EXIDY_VENTURE = &device_creator<venture_sound_device>;

venture_sound_device::venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: exidy_sound_device(mconfig, EXIDY_VENTURE, "Exidy SFX+PSG", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void venture_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void venture_sound_device::device_start()
{
	DEVICE_START_NAME( venture_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void venture_sound_device::device_reset()
{
	DEVICE_RESET_NAME( venture_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void venture_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}




static ADDRESS_MAP_START( venture_audio_map, AS_PROGRAM, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0780) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0780) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x07fc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x07fc) AM_DEVREADWRITE_LEGACY("custom", exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x2000, 0x27ff) AM_DEVWRITE_LEGACY("custom", exidy_sound_filter_w)
	AM_RANGE(0x2800, 0x2807) AM_MIRROR(0x07f8) AM_DEVREADWRITE_LEGACY("custom", exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x07fc) AM_DEVWRITE_LEGACY("custom", exidy_sfxctrl_w)
	AM_RANGE(0x5800, 0x7fff) AM_ROM
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( venture_audio )

	MCFG_CPU_ADD("audiocpu", M6502, 3579545/4)
	MCFG_CPU_PROGRAM_MAP(venture_audio_map)

	MCFG_RIOT6532_ADD("riot", SH6532_CLOCK, r6532_interface)

	MCFG_PIA6821_ADD("pia0", venture_pia0_intf)
	MCFG_PIA6821_ADD("pia1", venture_pia1_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", EXIDY_VENTURE, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  CVSD sound for Mouse Trap
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( mtrap_voiceio_w )
{
	exidy_sound_state *state = get_safe_token(device);

	if (!(offset & 0x10))
		hc55516_digit_w(state->m_cvsd, data & 1);

	if (!(offset & 0x20))
		riot6532_portb_in_set(state->m_riot, data & 1, 0xff);
}


static READ8_DEVICE_HANDLER( mtrap_voiceio_r )
{
	exidy_sound_state *state = get_safe_token(device);

	if (!(offset & 0x80))
	{
		UINT8 porta = riot6532_porta_out_get(state->m_riot);
		UINT8 data = (porta & 0x06) >> 1;
		data |= (porta & 0x01) << 2;
		data |= (porta & 0x08);
		return data;
	}

	if (!(offset & 0x40))
		return hc55516_clock_state_r(state->m_cvsd) << 7;

	return 0;
}


static ADDRESS_MAP_START( cvsd_map, AS_PROGRAM, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvsd_iomap, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE_LEGACY("custom", mtrap_voiceio_r, mtrap_voiceio_w)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( mtrap_cvsd_audio )

	MCFG_CPU_ADD("cvsdcpu", Z80, CVSD_Z80_CLOCK)
	MCFG_CPU_PROGRAM_MAP(cvsd_map)
	MCFG_CPU_IO_MAP(cvsd_iomap)

	/* audio hardware */
	MCFG_SOUND_ADD("cvsd", MC3417, CVSD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



/*************************************
 *
 *  Victory
 *
 *************************************/

#define VICTORY_AUDIO_CPU_CLOCK		(XTAL_3_579545MHz / 4)
#define VICTORY_LOG_SOUND			0



READ8_DEVICE_HANDLER( victory_sound_response_r )
{
	exidy_sound_state *state = get_safe_token(device);
	UINT8 ret = state->m_pia1->b_output();

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound response read = %02X\n", cpu_get_previouspc(state->m_maincpu), ret);

	state->m_pia1->cb1_w(0);

	return ret;
}


READ8_DEVICE_HANDLER( victory_sound_status_r )
{
	exidy_sound_state *state = get_safe_token(device);
	UINT8 ret = (state->m_pia1->ca1_r() << 7) | (state->m_pia1->cb1_r() << 6);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound status read = %02X\n", cpu_get_previouspc(state->m_maincpu), ret);

	return ret;
}


static TIMER_CALLBACK( delayed_command_w )
{
	pia6821_device *pia1 = (pia6821_device *)ptr;
	pia1->set_a_input(param, 0);
	pia1->ca1_w(0);
}

WRITE8_DEVICE_HANDLER( victory_sound_command_w )
{
	exidy_sound_state *state = get_safe_token(device);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound command = %02X\n", cpu_get_previouspc(state->m_maincpu), data);

	device->machine().scheduler().synchronize(FUNC(delayed_command_w), data, state->m_pia1);
}


static WRITE8_DEVICE_HANDLER( victory_sound_irq_clear_w )
{
	exidy_sound_state *state = get_safe_token(device);

	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound IRQ clear = %02X\n", device->machine().describe_context(), data);

	if (!data) state->m_pia1->ca1_w(1);
}


static WRITE8_DEVICE_HANDLER( victory_main_ack_w )
{
	exidy_sound_state *state = get_safe_token(device);

	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound Main ACK W = %02X\n", device->machine().describe_context(), data);

	if (state->m_victory_sound_response_ack_clk && !data)
		state->m_pia1->cb1_w(1);

	state->m_victory_sound_response_ack_clk = data;
}


static const pia6821_interface victory_pia1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_DEVICE_HANDLER("custom", victory_sound_irq_clear_w),	/* line CA2 out */
	DEVCB_DEVICE_HANDLER("custom", victory_main_ack_w),			/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_DEVICE_LINE("custom", update_irq_state)				/* IRQB */
};



static DEVICE_START( victory_sound )
{
	exidy_sound_state *state = get_safe_token(device);

	device->save_item(NAME(state->m_victory_sound_response_ack_clk));

	DEVICE_START_CALL(venture_common_sh_start);
	state->m_tms = device->machine().device("tms");
}


static DEVICE_RESET( victory_sound )
{
	exidy_sound_state *state = get_safe_token(device);
	pia6821_device *pia1 = state->m_pia1;

	DEVICE_RESET_CALL(common_sh_reset);
	pia1->reset();
	state->m_riot->reset();
	memset(state->m_sh8253_timer, 0, sizeof(state->m_sh8253_timer));

	/* the flip-flop @ F4 is reset */
	state->m_victory_sound_response_ack_clk = 0;
	pia1->cb1_w(1);

	/* these two lines shouldn't be needed, but it avoids the log entry
       as the sound CPU checks port A before the main CPU ever writes to it */
	pia1->set_a_input(0, 0);
	pia1->ca1_w(1);
}


const device_type EXIDY_VICTORY = &device_creator<victory_sound_device>;

victory_sound_device::victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: exidy_sound_device(mconfig, EXIDY_VICTORY, "Exidy SFX+PSG+Speech", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void victory_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victory_sound_device::device_start()
{
	DEVICE_START_NAME( victory_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victory_sound_device::device_reset()
{
	DEVICE_RESET_NAME( victory_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void victory_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}




static ADDRESS_MAP_START( victory_audio_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0f00) AM_RAM
	AM_RANGE(0x1000, 0x107f) AM_MIRROR(0x0f80) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE_LEGACY("custom", exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x4000, 0x4fff) AM_NOP
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x0ff8) AM_DEVREADWRITE_LEGACY("custom", exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x0ffc) AM_DEVWRITE_LEGACY("custom", exidy_sfxctrl_w)
	AM_RANGE(0x7000, 0xafff) AM_NOP
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( victory_audio )

	MCFG_CPU_ADD("audiocpu", M6502, VICTORY_AUDIO_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(victory_audio_map)

	MCFG_RIOT6532_ADD("riot", SH6532_CLOCK, r6532_interface)
	MCFG_PIA6821_ADD("pia1", victory_pia1_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", EXIDY_VICTORY, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("tms", TMS5220, 640000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
