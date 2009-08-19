/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "streams.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/6532riot.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"
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
static const device_config *riot;

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
static UINT32 sh6840_LFSR_0;/* ditto */
static UINT32 sh6840_LFSR_1;/* ditto */
static UINT32 sh6840_LFSR_2;/* ditto */
static UINT32 sh6840_LFSR_3;/* ditto */
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
 *  Interrupt generation helper
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( update_irq_state )
{
	const device_config *pia = devtag_get_device(device->machine, "pia1");
	cputag_set_input_line(device->machine, "audiocpu", M6502_IRQ_LINE, (pia6821_get_irq_b(pia) | riot_irq_state) ? ASSERT_LINE : CLEAR_LINE);
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
 *  6840 state saving
 *
 *************************************/

static void sh6840_register_state_globals(running_machine *machine)
{
    state_save_register_global_array(machine, sh6840_volume);
    state_save_register_global(machine, sh6840_MSB);
    state_save_register_global(machine, sh6840_LFSR_oldxor);
    state_save_register_global(machine, sh6840_LFSR_0);
    state_save_register_global(machine, sh6840_LFSR_1);
    state_save_register_global(machine, sh6840_LFSR_2);
    state_save_register_global(machine, sh6840_LFSR_3);
    state_save_register_global(machine, sh6840_clock_count);
    state_save_register_global(machine, exidy_sfxctrl);
    state_save_register_global(machine, sh6840_timer[0].cr);
    state_save_register_global(machine, sh6840_timer[0].state);
    state_save_register_global(machine, sh6840_timer[0].leftovers);
    state_save_register_global(machine, sh6840_timer[0].timer);
    state_save_register_global(machine, sh6840_timer[0].clocks);
    state_save_register_global(machine, sh6840_timer[0].counter.w);
    state_save_register_global(machine, sh6840_timer[1].cr);
    state_save_register_global(machine, sh6840_timer[1].state);
    state_save_register_global(machine, sh6840_timer[1].leftovers);
    state_save_register_global(machine, sh6840_timer[1].timer);
    state_save_register_global(machine, sh6840_timer[1].clocks);
    state_save_register_global(machine, sh6840_timer[1].counter.w);
    state_save_register_global(machine, sh6840_timer[2].cr);
    state_save_register_global(machine, sh6840_timer[2].state);
    state_save_register_global(machine, sh6840_timer[2].leftovers);
    state_save_register_global(machine, sh6840_timer[2].timer);
    state_save_register_global(machine, sh6840_timer[2].clocks);
    state_save_register_global(machine, sh6840_timer[2].counter.w);
}



/*************************************
 *
 *  Core sound generation
 *
 *************************************/

static STREAM_UPDATE( exidy_stream_update )
{
	int noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
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

static DEVICE_START( common_sh_start )
{
	int sample_rate = SH8253_CLOCK;

	sh6840_clocks_per_sample = (int)((double)SH6840_CLOCK / (double)sample_rate * (double)(1 << 24));

	/* allocate the stream */
	exidy_stream = stream_create(device, 0, 1, sample_rate, NULL, exidy_stream_update);

    sh6840_register_state_globals(device->machine);
}

static DEVICE_START( exidy_sound )
{
	/* indicate no additional hardware */
	has_sh8253  = FALSE;
	has_tms5220 = FALSE;
	has_mc3417 = FALSE;

	DEVICE_START_CALL(common_sh_start);
}


/*************************************
 *
 *  Audio reset routines
 *
 *************************************/

static DEVICE_RESET( common_sh_reset )
{
	/* 6840 */
	memset(sh6840_timer, 0, sizeof(sh6840_timer));
	sh6840_MSB = 0;
	sh6840_volume[0] = 0;
	sh6840_volume[1] = 0;
	sh6840_volume[2] = 0;
    sh6840_clock_count = 0;
	exidy_sfxctrl = 0;

	/* LFSR */
	sh6840_LFSR_oldxor = 0;
	sh6840_LFSR_0 = 0xffffffff;
	sh6840_LFSR_1 = 0xffffffff;
	sh6840_LFSR_2 = 0xffffffff;
	sh6840_LFSR_3 = 0xffffffff;
}

static DEVICE_RESET( exidy_sound )
{
	DEVICE_RESET_CALL(common_sh_reset);
}


DEVICE_GET_INFO( exidy_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(exidy_sound);	break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(exidy_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Exidy SFX");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


/*************************************
 *
 *  6532 interface
 *
 *************************************/

static void r6532_irq(const device_config *device, int state)
{
	riot_irq_state = (state == ASSERT_LINE) ? 1 : 0;
	update_irq_state(device, 0);
}


static void r6532_porta_w(const device_config *device, UINT8 newdata, UINT8 olddata)
{
	if (has_mc3417)
		cputag_set_input_line(device->machine, "cvsdcpu", INPUT_LINE_RESET, (newdata & 0x10) ? CLEAR_LINE : ASSERT_LINE);
}


static void r6532_portb_w(const device_config *device, UINT8 newdata, UINT8 olddata)
{
	const device_config *tms = devtag_get_device(device->machine, "tms");
	if (device != NULL)
	{
		if ((olddata & 0x01) && !(newdata & 0x01))
		{
			riot6532_porta_in_set(riot, tms5220_status_r(tms, 0), 0xff);
			logerror("(%f)%s:TMS5220 status read = %02X\n", attotime_to_double(timer_get_time(device->machine)), cpuexec_describe_context(device->machine), tms5220_status_r(tms, 0));
		}
		if ((olddata & 0x02) && !(newdata & 0x02))
		{
			logerror("(%f)%s:TMS5220 data write = %02X\n", attotime_to_double(timer_get_time(device->machine)), cpuexec_describe_context(device->machine), riot6532_porta_out_get(riot));
			tms5220_data_w(tms, 0, riot6532_porta_out_get(riot));
		}
	}
}


static UINT8 r6532_portb_r(const device_config *device, UINT8 olddata)
{
	UINT8 newdata = olddata;
	if (has_tms5220)
	{
		const device_config *tms = devtag_get_device(device->machine, "tms");
		newdata &= ~0x0c;
		if (!tms5220_ready_r(tms)) newdata |= 0x04;
		if (!tms5220_int_r(tms)) newdata |= 0x08;
	}
	return newdata;
}


static const riot6532_interface r6532_interface =
{
	NULL,					/* port A read handler */
	r6532_portb_r,			/* port B read handler */
	r6532_porta_w,			/* port A write handler */
	r6532_portb_w,			/* port B write handler */
	r6532_irq				/* IRQ callback */
};



/*************************************
 *
 *  8253 state saving
 *
 *************************************/


static void sh8253_register_state_globals(running_machine *machine)
{
    state_save_register_global(machine, sh8253_timer[0].clstate);
    state_save_register_global(machine, sh8253_timer[0].enable);
    state_save_register_global(machine, sh8253_timer[0].count);
    state_save_register_global(machine, sh8253_timer[0].step);
    state_save_register_global(machine, sh8253_timer[0].fraction);
    state_save_register_global(machine, sh8253_timer[1].clstate);
    state_save_register_global(machine, sh8253_timer[1].enable);
    state_save_register_global(machine, sh8253_timer[1].count);
    state_save_register_global(machine, sh8253_timer[1].step);
    state_save_register_global(machine, sh8253_timer[1].fraction);
    state_save_register_global(machine, sh8253_timer[2].clstate);
    state_save_register_global(machine, sh8253_timer[2].enable);
    state_save_register_global(machine, sh8253_timer[2].count);
    state_save_register_global(machine, sh8253_timer[2].step);
    state_save_register_global(machine, sh8253_timer[2].fraction);
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
	logerror("%04X:exidy_sh6840_r - unexpected read", cpu_get_pc(space->cpu));
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

static const pia6821_interface venture_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("pia1", pia6821_portb_w),		/* port A out */
	DEVCB_DEVICE_HANDLER("pia1", pia6821_porta_w),		/* port B out */
	DEVCB_DEVICE_HANDLER("pia1", pia6821_cb1_w),		/* line CA2 out */
	DEVCB_DEVICE_HANDLER("pia1", pia6821_ca1_w),		/* port CB2 out */
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
	DEVCB_DEVICE_HANDLER("pia0", pia6821_portb_w),		/* port A out */
	DEVCB_DEVICE_HANDLER("pia0", pia6821_porta_w),		/* port B out */
	DEVCB_DEVICE_HANDLER("pia0", pia6821_cb1_w),		/* line CA2 out */
	DEVCB_DEVICE_HANDLER("pia0", pia6821_ca1_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_LINE(update_irq_state)		/* IRQB */
};


static DEVICE_START( venture_common_sh_start )
{
	running_machine *machine = device->machine;

	DEVICE_START_CALL(common_sh_start);

	riot = devtag_get_device(machine, "riot");

	has_sh8253  = TRUE;
	has_tms5220 = FALSE;

	/* determine which sound hardware is installed */
	has_mc3417 = (devtag_get_device(device->machine, "cvsd") != NULL);

	/* 8253 */
	freq_to_step = (double)(1 << 24) / (double)SH8253_CLOCK;

    state_save_register_global(machine, riot_irq_state);
    sh8253_register_state_globals(device->machine);
}


static DEVICE_START( venture_sound )
{
	DEVICE_START_CALL(venture_common_sh_start);
}


static DEVICE_RESET( venture_sound )
{
	DEVICE_RESET_CALL(common_sh_reset);

	/* PIA */
	devtag_reset(device->machine, "pia0");
	devtag_reset(device->machine, "pia1");

	/* 6532 */
	device_reset(riot);

	/* 8253 */
	memset(sh8253_timer, 0, sizeof(sh8253_timer));
}


static DEVICE_GET_INFO( venture_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(venture_sound);	break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(venture_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Exidy SFX+PSG");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


#define SOUND_EXIDY_VENTURE DEVICE_GET_INFO_NAME( venture_sound )


static ADDRESS_MAP_START( venture_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0780) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0780) AM_DEVREADWRITE("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x07fc) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x07fc) AM_READWRITE(exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(exidy_sound_filter_w)
	AM_RANGE(0x2800, 0x2807) AM_MIRROR(0x07f8) AM_READWRITE(exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x07fc) AM_WRITE(exidy_sfxctrl_w)
	AM_RANGE(0x5800, 0x7fff) AM_ROM
ADDRESS_MAP_END


MACHINE_DRIVER_START( venture_audio )

	MDRV_CPU_ADD("audiocpu", M6502, 3579545/4)
	MDRV_CPU_PROGRAM_MAP(venture_audio_map)

	MDRV_RIOT6532_ADD("riot", SH6532_CLOCK, r6532_interface)

	MDRV_PIA6821_ADD("pia0", venture_pia0_intf)
	MDRV_PIA6821_ADD("pia1", venture_pia1_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("custom", EXIDY_VENTURE, 0)
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
		hc55516_digit_w(devtag_get_device(space->machine, "cvsd"), data & 1);

	if (!(offset & 0x20))
		riot6532_portb_in_set(riot, data & 1, 0xff);
}


static READ8_HANDLER( mtrap_voiceio_r )
{
	if (!(offset & 0x80))
	{
		UINT8 porta = riot6532_porta_out_get(riot);
		UINT8 data = (porta & 0x06) >> 1;
		data |= (porta & 0x01) << 2;
		data |= (porta & 0x08);
		return data;
	}

	if (!(offset & 0x40))
		return hc55516_clock_state_r(devtag_get_device(space->machine, "cvsd")) << 7;

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

	MDRV_CPU_ADD("cvsdcpu", Z80, CVSD_Z80_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cvsd_map)
	MDRV_CPU_IO_MAP(cvsd_iomap)

	/* audio hardware */
	MDRV_SOUND_ADD("cvsd", MC3417, CVSD_CLOCK)
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
	const device_config *pia1 = devtag_get_device(space->machine, "pia1");
	UINT8 ret = pia6821_get_output_b(pia1);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound response read = %02X\n", cpu_get_previouspc(space->cpu), ret);

	pia6821_cb1_w(pia1, 0, 0);

	return ret;
}


READ8_HANDLER( victory_sound_status_r )
{
	const device_config *pia1 = devtag_get_device(space->machine, "pia1");
	UINT8 ret = (pia6821_ca1_r(pia1, 0) << 7) | (pia6821_cb1_r(pia1, 0) << 6);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound status read = %02X\n", cpu_get_previouspc(space->cpu), ret);

	return ret;
}


static TIMER_CALLBACK( delayed_command_w )
{
	const device_config *pia1 = devtag_get_device(machine, "pia1");
	pia6821_set_input_a(pia1, param, 0);
	pia6821_ca1_w(pia1, 0, 0);
}

WRITE8_HANDLER( victory_sound_command_w )
{
	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound command = %02X\n", cpu_get_previouspc(space->cpu), data);

	timer_call_after_resynch(space->machine, NULL, data, delayed_command_w);
}


static WRITE8_DEVICE_HANDLER( victory_sound_irq_clear_w )
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound IRQ clear = %02X\n", cpuexec_describe_context(device->machine), data);

	if (!data) pia6821_ca1_w(device, 0, 1);
}


static WRITE8_DEVICE_HANDLER( victory_main_ack_w )
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound Main ACK W = %02X\n", cpuexec_describe_context(device->machine), data);

	if (victory_sound_response_ack_clk && !data)
		pia6821_cb1_w(device, 0, 1);

	victory_sound_response_ack_clk = data;
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
	DEVCB_HANDLER(victory_sound_irq_clear_w),	/* line CA2 out */
	DEVCB_HANDLER(victory_main_ack_w),			/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_LINE(update_irq_state)				/* IRQB */
};



static DEVICE_START( victory_sound )
{
	state_save_register_global(device->machine, victory_sound_response_ack_clk);

	DEVICE_START_CALL(venture_common_sh_start);
	has_tms5220 = TRUE;
}


static DEVICE_RESET( victory_sound )
{
	const device_config *pia1 = devtag_get_device(device->machine, "pia1");

	DEVICE_RESET_CALL(common_sh_reset);
	device_reset(pia1);
	device_reset(riot);
	memset(sh8253_timer, 0, sizeof(sh8253_timer));

	/* the flip-flop @ F4 is reset */
	victory_sound_response_ack_clk = 0;
	pia6821_cb1_w(pia1, 0, 1);

	/* these two lines shouldn't be needed, but it avoids the log entry
       as the sound CPU checks port A before the main CPU ever writes to it */
	pia6821_set_input_a(pia1, 0, 0);
	pia6821_ca1_w(pia1, 0, 1);
}


static DEVICE_GET_INFO( victory_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(victory_sound);	break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(victory_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Exidy SFX+PSG+Speech");		break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


#define SOUND_EXIDY_VICTORY DEVICE_GET_INFO_NAME( victory_sound )


static ADDRESS_MAP_START( victory_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0f00) AM_RAM
	AM_RANGE(0x1000, 0x107f) AM_MIRROR(0x0f80) AM_DEVREADWRITE("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x0ffc) AM_READWRITE(exidy_sh8253_r, exidy_sh8253_w)
	AM_RANGE(0x4000, 0x4fff) AM_NOP
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x0ff8) AM_READWRITE(exidy_sh6840_r, exidy_sh6840_w)
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x0ffc) AM_WRITE(exidy_sfxctrl_w)
	AM_RANGE(0x7000, 0xafff) AM_NOP
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


MACHINE_DRIVER_START( victory_audio )

	MDRV_CPU_ADD("audiocpu", M6502, VICTORY_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(victory_audio_map)

	MDRV_RIOT6532_ADD("riot", SH6532_CLOCK, r6532_interface)
	MDRV_PIA6821_ADD("pia1", victory_pia1_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("custom", EXIDY_VICTORY, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("tms", TMS5220, 640000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
