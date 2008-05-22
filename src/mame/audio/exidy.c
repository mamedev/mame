/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "driver.h"
#include "rescap.h"
#include "streams.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/5220intf.h"
#include "sound/custom.h"
#include "exidy.h"



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
#define CVSD_Z80_CLOCK 			(CRYSTAL_OSC / 2)
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

/* IRQ variable */
static UINT8 riot_irq_state;

/* 6532 variables */
static emu_timer *riot_timer;
static UINT8 riot_irq_flag;
static UINT8 riot_timer_irq_enable;
static UINT8 riot_PA7_irq_enable;
static UINT8 riot_porta_data;
static UINT8 riot_porta_ddr;
static UINT8 riot_portb_data;
static UINT8 riot_portb_ddr;
static int riot_clock_divisor;
static UINT8 riot_state;

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
static struct sh6840_timer_channel sh6840_timer[3];
static INT16 sh6840_volume[3];
static UINT8 sh6840_MSB;
static UINT8 sh6840_LFSR_oldxor = 0; /* should be saved in savestate */
static UINT32 sh6840_LFSR_0 = 0xffffffff;/* ditto */
static UINT32 sh6840_LFSR_1 = 0xffffffff;/* ditto */
static UINT32 sh6840_LFSR_2 = 0xffffffff;/* ditto */
static UINT32 sh6840_LFSR_3 = 0xffffffff;/* ditto */
static UINT32 sh6840_clocks_per_sample;
static UINT32 sh6840_clock_count;

static UINT8 exidy_sfxctrl;

/* 8253 variables */
struct sh8253_timer_channel
{
	UINT8	clstate;
	UINT8	enable;
	UINT16	count;
	UINT32	step;
	UINT32	fraction;
};
static struct sh8253_timer_channel sh8253_timer[3];
static int has_sh8253;

/* 5220/CVSD variables */
static int has_mc3417;
static int has_tms5220;

/* sound streaming variables */
static sound_stream *exidy_stream;
static double freq_to_step;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_CALLBACK( riot_interrupt );



/*************************************
 *
 *  Interrupt generation helper
 *
 *************************************/

static void update_irq_state(running_machine *machine, /* unused */ int state)
{
	cpunum_set_input_line(machine, 1, M6502_IRQ_LINE, (pia_get_irq_b(1) | riot_irq_state) ? ASSERT_LINE : CLEAR_LINE);
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

INLINE int sh6840_update_noise(int clocks)
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
		newxor = (sh6840_LFSR_3 ^ sh6840_LFSR_2) >> 31; /* high bits of 3 and 2 xored is new xor */
		sh6840_LFSR_3 <<= 1;
		sh6840_LFSR_3 |= sh6840_LFSR_2 >> 31;
		sh6840_LFSR_2 <<= 1;
		sh6840_LFSR_2 |= sh6840_LFSR_1 >> 31;
		sh6840_LFSR_1 <<= 1;
		sh6840_LFSR_1 |= sh6840_LFSR_0 >> 31;
		sh6840_LFSR_0 <<= 1;
		sh6840_LFSR_0 |= newxor ^ sh6840_LFSR_oldxor;
		sh6840_LFSR_oldxor = newxor;
		/*printf("LFSR: %4x, %4x, %4x, %4x\n", sh6840_LFSR_3, sh6840_LFSR_2, sh6840_LFSR_1, sh6840_LFSR_0);*/
		/* if we clocked 0->1, that will serve as an external clock */
		if ((sh6840_LFSR_2 & 0x03) == 0x01) /* tap is at 96th bit */
		{
			noise_clocks++;
		}
	}
	return noise_clocks;
}



/*************************************
 *
 *  Core sound generation
 *
 *************************************/

static void exidy_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	int noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (length--)
	{
		struct sh6840_timer_channel *t;
		struct sh8253_timer_channel *c;
		int clocks_this_sample;
		INT16 sample = 0;

		/* determine how many 6840 clocks this sample */
		sh6840_clock_count += sh6840_clocks_per_sample;
		clocks_this_sample = sh6840_clock_count >> 24;
		sh6840_clock_count &= (1 << 24) - 1;

		/* skip if nothing enabled */
		if ((sh6840_timer[0].cr & 0x01) == 0)
		{
			int noise_clocks_this_sample = 0;
			UINT32 chan0_clocks;

			/* generate E-clocked noise if configured to do so */
			if (noisy && !(exidy_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(clocks_this_sample);

			/* handle timer 0 if enabled */
			t = &sh6840_timer[0];
			chan0_clocks = t->clocks;
			if (t->cr & 0x80)
			{
				int clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
				sh6840_apply_clock(t, clocks);
				if (t->state && !(exidy_sfxctrl & 0x02))
					sample += sh6840_volume[0];
			}

			/* generate channel 0-clocked noise if configured to do so */
			if (noisy && (exidy_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(t->clocks - chan0_clocks);

			/* handle timer 1 if enabled */
			t = &sh6840_timer[1];
			if (t->cr & 0x80)
			{
				int clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
				sh6840_apply_clock(t, clocks);
				if (t->state)
					sample += sh6840_volume[1];
			}

			/* handle timer 2 if enabled */
			t = &sh6840_timer[2];
			if (t->cr & 0x80)
			{
				int clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;

				/* prescale */
				if (t->cr & 0x01)
				{
					clocks += t->leftovers;
					t->leftovers = clocks % 8;
					clocks /= 8;
				}
				sh6840_apply_clock(t, clocks);
				if (t->state)
					sample += sh6840_volume[2];
			}
		}

		/* music (if present) */
		if (has_sh8253)
		{
			/* music channel 0 */
			c = &sh8253_timer[0];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 1 */
			c = &sh8253_timer[1];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 2 */
			c = &sh8253_timer[2];
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

void *exidy_sh6840_sh_start(int clock, const struct CustomSound_interface *config)
{
	int sample_rate = SH8253_CLOCK;

	sh6840_clocks_per_sample = (int)((double)SH6840_CLOCK / (double)sample_rate * (double)(1 << 24));

	/* allocate the stream */
	exidy_stream = stream_create(0, 1, sample_rate, NULL, exidy_stream_update);

	/* indicate no additional hardware */
	has_sh8253  = FALSE;
	has_tms5220 = FALSE;
	has_mc3417 = FALSE;

	return auto_malloc(1);
}



/*************************************
 *
 *  Audio reset routines
 *
 *************************************/

void exidy_sh6840_sh_reset(void *token)
{
	/* 6840 */
	memset(sh6840_timer, 0, sizeof(sh6840_timer));
	sh6840_MSB = 0;
	sh6840_volume[0] = 0;
	sh6840_volume[1] = 0;
	sh6840_volume[2] = 0;
	exidy_sfxctrl = 0;

	/* LFSR */
	sh6840_LFSR_oldxor = 0;
	sh6840_LFSR_0 = 0xffffffff;
	sh6840_LFSR_1 = 0xffffffff;
	sh6840_LFSR_2 = 0xffffffff;
	sh6840_LFSR_3 = 0xffffffff;
}



/*************************************
 *
 *  6532 RIOT timer callback
 *
 *************************************/

static TIMER_CALLBACK( riot_interrupt )
{
	/* if we're doing the initial interval counting... */
	if (riot_state == RIOT_COUNT)
	{
		/* generate the IRQ */
		riot_irq_flag |= 0x80;
		riot_irq_state = riot_timer_irq_enable;
		update_irq_state(machine, 0);

		/* now start counting clock cycles down */
		riot_state = RIOT_POST_COUNT;
		timer_adjust_oneshot(riot_timer, attotime_mul(ATTOTIME_IN_HZ(SH6532_CLOCK), 0xff), 0);
	}

	/* if not, we are done counting down */
	else
	{
		riot_state = RIOT_IDLE;
		timer_adjust_oneshot(riot_timer, attotime_never, 0);
	}
}



/*************************************
 *
 *  6532 RIOT write handler
 *
 *************************************/

static WRITE8_HANDLER( exidy_shriot_w )
{
	/* I/O is done if A2 == 0 */
	if ((offset & 0x04) == 0)
	{
		switch (offset & 0x03)
		{
			case 0:	/* port A */
				if (has_mc3417)
					cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
				riot_porta_data = (riot_porta_data & ~riot_porta_ddr) | (data & riot_porta_ddr);
				break;

			case 1:	/* port A DDR */
				riot_porta_ddr = data;
				break;

			case 2:	/* port B */
				if (has_tms5220)
				{
					if (!(data & 0x01) && (riot_portb_data & 0x01))
					{
						riot_porta_data = tms5220_status_r(machine, 0);
						logerror("(%f)%04X:TMS5220 status read = %02X\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), riot_porta_data);
					}
					if (!(data & 0x02) && (riot_portb_data & 0x02))
					{
						logerror("(%f)%04X:TMS5220 data write = %02X\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), riot_porta_data);
						tms5220_data_w(machine, 0, riot_porta_data);
					}
				}
				riot_portb_data = (riot_portb_data & ~riot_portb_ddr) | (data & riot_portb_ddr);
				break;

			case 3:	/* port B DDR */
				riot_portb_ddr = data;
				break;
		}
	}

	/* PA7 edge detect control if A2 == 1 and A4 == 0 */
	else if ((offset & 0x10) == 0)
	{
		riot_PA7_irq_enable = offset & 0x03;
	}

	/* timer enable if A2 == 1 and A4 == 1 */
	else
	{
		static const int divisors[4] = { 1, 8, 64, 1024 };

		/* make sure the IRQ state is clear */
		if (riot_state != RIOT_COUNT)
			riot_irq_flag &= ~0x80;
		riot_irq_state = 0;
		update_irq_state(machine, 0);

		/* set the enable from the offset */
		riot_timer_irq_enable = (offset & 0x08) ? 1 : 0;

		/* set a new timer */
		riot_clock_divisor = divisors[offset & 0x03];
		timer_adjust_oneshot(riot_timer, attotime_mul(ATTOTIME_IN_HZ(SH6532_CLOCK), data * riot_clock_divisor), 0);
		riot_state = RIOT_COUNT;
	}
}



/*************************************
 *
 *  6532 RIOT read handler
 *
 *************************************/

static READ8_HANDLER( exidy_shriot_r )
{
	/* I/O is done if A2 == 0 */
	if ((offset & 0x04) == 0)
	{
		switch (offset & 0x03)
		{
			case 0x00:	/* port A */
				return riot_porta_data;

			case 0x01:	/* port A DDR */
				return riot_porta_ddr;

			case 0x02:	/* port B */
				if (has_tms5220)
				{
					riot_portb_data &= ~0x0c;
					if (!tms5220_ready_r()) riot_portb_data |= 0x04;
					if (!tms5220_int_r()) riot_portb_data |= 0x08;
				}
				return riot_portb_data;

			case 0x03:	/* port B DDR */
				return riot_portb_ddr;
		}
	}

	/* interrupt flags are read if A2 == 1 and A0 == 1 */
	else if (offset & 0x01)
	{
		int temp = riot_irq_flag;
		riot_irq_flag = 0;
		riot_irq_state = 0;
		update_irq_state(machine, 0);
		return temp;
	}

	/* timer count is read if A2 == 1 and A0 == 0 */
	else
	{
		/* set the enable from the offset */
		riot_timer_irq_enable = offset & 0x08;

		/* compute the timer based on the current state */
		switch (riot_state)
		{
			case RIOT_IDLE:
				return 0x00;

			case RIOT_COUNT:
				return attotime_to_double(timer_timeleft(riot_timer)) * SH6532_CLOCK / riot_clock_divisor;

			case RIOT_POST_COUNT:
				return attotime_to_double(timer_timeleft(riot_timer)) * SH6532_CLOCK;
		}
	}

	logerror("Undeclared RIOT read: %x  PC:%x\n",offset,activecpu_get_pc());
	return 0xff;
}



/*************************************
 *
 *  8253 timer handlers
 *
 *************************************/

static WRITE8_HANDLER( exidy_sh8253_w )
{
	int chan;

	stream_update(exidy_stream);

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			chan = offset;
			if (!sh8253_timer[chan].clstate)
			{
				sh8253_timer[chan].clstate = 1;
				sh8253_timer[chan].count = (sh8253_timer[chan].count & 0xff00) | (data & 0x00ff);
			}
			else
			{
				sh8253_timer[chan].clstate = 0;
				sh8253_timer[chan].count = (sh8253_timer[chan].count & 0x00ff) | ((data << 8) & 0xff00);
				if (sh8253_timer[chan].count)
					sh8253_timer[chan].step = freq_to_step * (double)SH8253_CLOCK / (double)sh8253_timer[chan].count;
				else
					sh8253_timer[chan].step = 0;
			}
			break;

		case 3:
			chan = (data & 0xc0) >> 6;
			sh8253_timer[chan].enable = ((data & 0x0e) != 0);
			break;
	}
}


static READ8_HANDLER( exidy_sh8253_r )
{
	logerror("8253(R): %x\n",offset);

	return 0;
}



/*************************************
 *
 *  6840 timer handlers
 *
 *************************************/

static READ8_HANDLER( exidy_sh6840_r )
{
	fatalerror("exidy_sh6840_r - unexpected read");

	return 0;
}


WRITE8_HANDLER( exidy_sh6840_w )
{
	/* force an update of the stream */
	stream_update(exidy_stream);

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
			sh6840_MSB = data;
			break;

		/* offsets 3/5/7 write to the LSB controls */
		case 3:
		case 5:
		case 7:
		{
			/* latch the timer value */
			int ch = (offset - 3) / 2;
			sh6840_timer[ch].timer = (sh6840_MSB << 8) | (data & 0xff);

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

WRITE8_HANDLER( exidy_sfxctrl_w )
{
	stream_update(exidy_stream);

	switch (offset)
	{
		case 0:
			exidy_sfxctrl = data;
			break;

		case 1:
		case 2:
		case 3:
			sh6840_volume[offset - 1] = ((data & 7) * BASE_VOLUME) / 7;
			break;
	}
}



/*************************************
 *
 *  Sound filter control
 *
 *************************************/

static WRITE8_HANDLER( exidy_sound_filter_w )
{
	logerror("exidy_sound_filter_w = %02X\n", data);
}



/*************************************
 *
 *  Venture, etc.
 *
 *************************************/

static const pia6821_interface venture_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_1_portb_w, pia_1_porta_w, pia_1_cb1_w, pia_1_ca1_w,
	/*irqs   : A/B             */ 0, 0
};


static const pia6821_interface venture_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_0_portb_w, pia_0_porta_w, pia_0_cb1_w, pia_0_ca1_w,
	/*irqs   : A/B             */ 0, update_irq_state
};


static void *venture_common_sh_start(int clock, const struct CustomSound_interface *config, int _has_tms5220)
{
	int i;

	void *ret = exidy_sh6840_sh_start(clock, config);

	has_sh8253  = TRUE;
	has_tms5220 = _has_tms5220;

	/* determine which sound hardware is installed */
	has_mc3417 = FALSE;
	for (i = 0; i < MAX_SOUND; i++)
	{
		if (Machine->config->sound[i].type == SOUND_MC3417)
			has_mc3417 = TRUE;
	}

	/* 6532 */
    riot_timer = timer_alloc(riot_interrupt, NULL);

	/* 8253 */
	freq_to_step = (double)(1 << 24) / (double)SH8253_CLOCK;

	return ret;
}


static void *venture_sh_start(int clock, const struct CustomSound_interface *config)
{
	pia_config(0, &venture_pia_0_intf);
	pia_config(1, &venture_pia_1_intf);

	return venture_common_sh_start(clock, config, FALSE);
}


static void venture_sh_reset(void *token)
{
	exidy_sh6840_sh_reset(token);

	/* PIA */
	pia_reset();

	/* 6532 */
	riot_irq_flag = 0;
	riot_timer_irq_enable = 0;
	riot_porta_data = 0xff;
	riot_portb_data = 0xff;
	riot_clock_divisor = 1;
	riot_state = RIOT_IDLE;

	/* 8253 */
	memset(sh8253_timer, 0, sizeof(sh8253_timer));
}


static const struct CustomSound_interface venture_custom_interface =
{
	venture_sh_start,
	0,
	venture_sh_reset
};


static ADDRESS_MAP_START( venture_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0780) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0780) AM_READWRITE(exidy_shriot_r, exidy_shriot_w)
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x07fc) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x07fc) AM_READWRITE(exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(exidy_sound_filter_w)
	AM_RANGE(0x2800, 0x2807) AM_MIRROR(0x07f8) AM_READWRITE(exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x07fc) AM_WRITE(exidy_sfxctrl_w)
	AM_RANGE(0x5800, 0x7fff) AM_ROM
ADDRESS_MAP_END


MACHINE_DRIVER_START( venture_audio )

	MDRV_CPU_ADD(M6502, 3579545/4)
	MDRV_CPU_PROGRAM_MAP(venture_audio_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CUSTOM, 0)
	MDRV_SOUND_CONFIG(venture_custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  CVSD sound for Mouse Trap
 *
 *************************************/

static WRITE8_HANDLER( mtrap_voiceio_w )
{
	if (!(offset & 0x10))
		hc55516_digit_w(0, data & 1);

	if (!(offset & 0x20))
		riot_portb_data = data & 1;
}


static READ8_HANDLER( mtrap_voiceio_r )
{
	if (!(offset & 0x80))
	{
		int data = (riot_porta_data & 0x06) >> 1;
		data |= (riot_porta_data & 0x01) << 2;
		data |= (riot_porta_data & 0x08);

		return data;
	}

	if (!(offset & 0x40))
		return hc55516_clock_state_r(0) << 7;

	return 0;
}


static ADDRESS_MAP_START( cvsd_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvsd_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(mtrap_voiceio_r, mtrap_voiceio_w)
ADDRESS_MAP_END


MACHINE_DRIVER_START( mtrap_cvsd_audio )

	MDRV_CPU_ADD(Z80, CVSD_Z80_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cvsd_map,0)
	MDRV_CPU_IO_MAP(cvsd_iomap,0)

	/* audio hardware */
	MDRV_SOUND_ADD(MC3417, CVSD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END



/*************************************
 *
 *  Victory
 *
 *************************************/

#define VICTORY_AUDIO_CPU_CLOCK		(XTAL_3_579545MHz / 4)
#define VICTORY_LOG_SOUND			0


static UINT8 victory_sound_response_ack_clk;	/* 7474 @ F4 */


READ8_HANDLER( victory_sound_response_r )
{
	UINT8 ret = pia_get_output_b(1);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound response read = %02X\n", activecpu_get_previouspc(), ret);

	pia_set_input_cb1(1, 0);

	return ret;
}


READ8_HANDLER( victory_sound_status_r )
{
	UINT8 ret = (pia_get_input_ca1(1) << 7) | (pia_get_input_cb1(1) << 6);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound status read = %02X\n", activecpu_get_previouspc(), ret);

	return ret;
}


static TIMER_CALLBACK( delayed_command_w )
{
	pia_set_input_a(1, param, 0);
	pia_set_input_ca1(1, 0);
}

WRITE8_HANDLER( victory_sound_command_w )
{
	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound command = %02X\n", activecpu_get_previouspc(), data);

	timer_call_after_resynch(NULL, data, delayed_command_w);
}


static WRITE8_HANDLER( victory_sound_irq_clear_w )
{
	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound IRQ clear = %02X\n", activecpu_get_previouspc(), data);

	if (!data) pia_set_input_ca1(1, 1);
}


static WRITE8_HANDLER( victory_main_ack_w )
{
	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound Main ACK W = %02X\n", activecpu_get_previouspc(), data);

	if (victory_sound_response_ack_clk && !data)
		pia_set_input_cb1(1, 1);

	victory_sound_response_ack_clk = data;
}


static const pia6821_interface victory_pia_e5_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, victory_sound_irq_clear_w, victory_main_ack_w,
	/*irqs   : A/B             */ 0, update_irq_state
};


static void *victory_sh_start(int clock, const struct CustomSound_interface *config)
{
	pia_config(1, &victory_pia_e5_intf);

	state_save_register_global(victory_sound_response_ack_clk);

	return venture_common_sh_start(clock, config, TRUE);
}


static void victory_sh_reset(void *token)
{
	venture_sh_reset(token);

	/* the flip-flop @ F4 is reset */
	victory_sound_response_ack_clk = 0;
	pia_set_input_cb1(1, 1);

	/* these two lines shouldn't be needed, but it avoids the log entry
       as the sound CPU checks port A before the main CPU ever writes to it */
	pia_set_input_a(1, 0, 0);
	pia_set_input_ca1(1, 1);
}


static const struct CustomSound_interface victory_custom_interface =
{
	victory_sh_start,
	0,
	victory_sh_reset,
};


static ADDRESS_MAP_START( victory_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0f00) AM_RAM
	AM_RANGE(0x1000, 0x107f) AM_MIRROR(0x0f80) AM_READWRITE(exidy_shriot_r, exidy_shriot_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x0ffc) AM_READWRITE(exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x4000, 0x4fff) AM_NOP
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x0ff8) AM_READWRITE(exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x0ffc) AM_WRITE(exidy_sfxctrl_w)
	AM_RANGE(0x7000, 0xafff) AM_NOP
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


MACHINE_DRIVER_START( victory_audio )

	MDRV_CPU_ADD(M6502, VICTORY_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(victory_audio_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CUSTOM, 0)
	MDRV_SOUND_CONFIG(victory_custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(TMS5220, 640000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
