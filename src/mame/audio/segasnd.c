/***************************************************************************

    segasnd.c

    Sound boards for early Sega G-80 based games.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/sp0250.h"
#include "segasnd.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SPEECH_MASTER_CLOCK	3120000

#define USB_MASTER_CLOCK	6000000
#define USB_2MHZ_CLOCK		(USB_MASTER_CLOCK/3)
#define USB_PCS_CLOCK		(USB_2MHZ_CLOCK/2)
#define USB_GOS_CLOCK		(USB_2MHZ_CLOCK/16/4)
#define MM5837_CLOCK		100000

#define SAMPLE_RATE			(USB_2MHZ_CLOCK/8)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _filter_state filter_state;
struct _filter_state
{
	double				capval;				/* current capacitor value */
	double				exponent;			/* constant exponent */
};


typedef struct _timer8253_channel timer8253_channel;
struct _timer8253_channel
{
	UINT8				holding;			/* holding until counts written? */
	UINT8				latchmode;			/* latching mode */
	UINT8				latchtoggle;		/* latching state */
	UINT8				clockmode;			/* clocking mode */
	UINT8				bcdmode;			/* BCD mode? */
	UINT8				output;				/* current output value */
	UINT8				lastgate;			/* previous gate value */
	UINT8				gate;				/* current gate value */
	UINT8				subcount;			/* subcount (2MHz clocks per input clock) */
	UINT16				count;				/* initial count */
	UINT16				remain;				/* current down counter value */
};


typedef struct _timer8253 timer8253;
struct _timer8253
{
	timer8253_channel	chan[3];			/* three channels' worth of information */
	double				env[3];				/* envelope value for each channel */
	filter_state		chan_filter[2];		/* filter states for the first two channels */
	filter_state		gate1;				/* first RC filter state */
	filter_state		gate2;				/* second RC filter state */
	UINT8				config;				/* configuration for this timer */
};


typedef struct _usb_state usb_state;
struct _usb_state
{
	sound_stream *		stream;				/* output stream */
	device_t *cpu;				/* CPU index of the 8035 */
	device_t *maincpu;
	UINT8				in_latch;			/* input latch */
	UINT8				out_latch;			/* output latch */
	UINT8				last_p2_value;		/* current P2 output value */
	UINT8 *				program_ram;		/* pointer to program RAM */
	UINT8 *				work_ram;			/* pointer to work RAM */
	UINT8				work_ram_bank;		/* currently selected work RAM bank */
	UINT8				t1_clock;			/* T1 clock value */
	UINT8				t1_clock_mask;		/* T1 clock mask (configured via jumpers) */
	timer8253			timer_group[3];		/* 3 groups of timers */
	UINT8				timer_mode[3];		/* mode control for each group */
	UINT32				noise_shift;
	UINT8				noise_state;
	UINT8				noise_subcount;
	double				gate_rc1_exp[2];
	double				gate_rc2_exp[2];
	filter_state		final_filter;
	filter_state		noise_filters[5];
};

/* SP0250-based speech board */
typedef struct _speech_state speech_state;
struct _speech_state
{
	UINT8 latch, t0, p2, drq;
	UINT8 *speech;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE void configure_filter(filter_state *state, double r, double c)
{
	state->capval = 0;
	state->exponent = 1.0 - exp(-1.0 / (r * c * SAMPLE_RATE));
}


INLINE double step_rc_filter(filter_state *state, double input)
{
	state->capval += (input - state->capval) * state->exponent;
	return state->capval;
}


INLINE double step_cr_filter(filter_state *state, double input)
{
	double result = (input - state->capval);
	state->capval += (input - state->capval) * state->exponent;
	return result;
}



/***************************************************************************
    SPEECH BOARD
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(SEGASPEECH, speech_sound);

INLINE speech_state *get_safe_speech(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SEGASPEECH);

	return (speech_state *)downcast<legacy_device_base *>(device)->token();
}

static DEVICE_START( speech_sound )
{
	speech_state *state = get_safe_speech(device);

	state->speech = device->machine().region("speech")->base();
}


DEVICE_GET_INFO( speech_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(speech_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(speech_sound);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Sega Speech Sound Board");	break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(SEGASPEECH, speech_sound);



/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/



static READ8_DEVICE_HANDLER( speech_t0_r )
{
	speech_state *state = get_safe_speech(device);

	return state->t0;
}

static READ8_DEVICE_HANDLER( speech_t1_r )
{
	speech_state *state = get_safe_speech(device);

	return state->drq;
}

static READ8_DEVICE_HANDLER( speech_p1_r )
{
	speech_state *state = get_safe_speech(device);

	return state->latch & 0x7f;
}

static READ8_DEVICE_HANDLER( speech_rom_r )
{
	speech_state *state = get_safe_speech(device);

	return state->speech[0x100 * (state->p2 & 0x3f) + offset];
}

static WRITE8_DEVICE_HANDLER( speech_p1_w )
{
	speech_state *state = get_safe_speech(device);

	if (!(data & 0x80))
		state->t0 = 0;
}

static WRITE8_DEVICE_HANDLER( speech_p2_w )
{
	speech_state *state = get_safe_speech(device);

	state->p2 = data;
}



/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/

static void speech_drq_w(device_t *device, int level)
{
	device_t *speech = device->machine().device("segaspeech");
	speech_state *state = get_safe_speech(speech);

	state->drq = (level == ASSERT_LINE);
}



/*************************************
 *
 *  External access
 *
 *************************************/

static TIMER_CALLBACK( delayed_speech_w )
{
	int data = param;
	speech_state *state = (speech_state *)ptr;
	UINT8 old = state->latch;

	/* all 8 bits are latched */
	state->latch = data;

	/* the high bit goes directly to the INT line */
	cputag_set_input_line(machine, "audiocpu", 0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	/* a clock on the high bit clocks a 1 into T0 */
	if (!(old & 0x80) && (data & 0x80))
		state->t0 = 1;
}


WRITE8_DEVICE_HANDLER( sega_speech_data_w )
{
	speech_state *state = get_safe_speech(device);

	device->machine().scheduler().synchronize(FUNC(delayed_speech_w), data, state);
}


WRITE8_DEVICE_HANDLER( sega_speech_control_w )
{
	LOG(("Speech control = %X\n", data));
}



/*************************************
 *
 *  Speech board address maps
 *
 *************************************/

static ADDRESS_MAP_START( speech_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( speech_portmap, AS_IO, 8, driver_device )
	AM_RANGE(0x00, 0xff) AM_DEVREAD_LEGACY("segaspeech", speech_rom_r)
	AM_RANGE(0x00, 0xff) AM_DEVWRITE_LEGACY("speech", sp0250_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVREADWRITE_LEGACY("segaspeech", speech_p1_r, speech_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_DEVWRITE_LEGACY("segaspeech", speech_p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_DEVREAD_LEGACY("segaspeech", speech_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_DEVREAD_LEGACY("segaspeech", speech_t1_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Speech board sound interfaces
 *
 *************************************/

static const struct sp0250_interface sp0250_interface =
{
	speech_drq_w
};



/*************************************
 *
 *  Speech board machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( sega_speech_board )

	/* CPU for the speech board */
	MCFG_CPU_ADD("audiocpu", I8035, SPEECH_MASTER_CLOCK)		/* divide by 15 in CPU */
	MCFG_CPU_PROGRAM_MAP(speech_map)
	MCFG_CPU_IO_MAP(speech_portmap)

	/* sound hardware */
	MCFG_SOUND_ADD("segaspeech", SEGASPEECH, 0)
	MCFG_SOUND_ADD("speech", SP0250, SPEECH_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(sp0250_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************
    UNIVERSAL SOUND BOARD
***************************************************************************/

static DECLARE_LEGACY_SOUND_DEVICE(SEGAUSB, usb_sound);

INLINE usb_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SEGAUSB);

	return (usb_state *)downcast<legacy_device_base *>(device)->token();
}

/*************************************
 *
 *  Initialization/reset
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( increment_t1_clock )
{
	device_t *usbsnd = timer.machine().device("usbsnd");
	usb_state *usb = get_safe_token(usbsnd);

	/* only increment if it is not being forced clear */
	if (!(usb->last_p2_value & 0x80))
		usb->t1_clock++;
}


static DEVICE_RESET( usb_sound )
{
	usb_state *usb = get_safe_token(device);

	/* halt the USB CPU at reset time */
	device_set_input_line(usb->cpu, INPUT_LINE_RESET, ASSERT_LINE);

	/* start the clock timer */
	usb->t1_clock_mask = 0x10;
}



/*************************************
 *
 *  External access
 *
 *************************************/

READ8_DEVICE_HANDLER( sega_usb_status_r )
{
	usb_state *usb = get_safe_token(device);

	LOG(("%04X:usb_data_r = %02X\n", cpu_get_pc(usb->maincpu), (usb->out_latch & 0x81) | (usb->in_latch & 0x7e)));

	device_adjust_icount(usb->maincpu, -200);

	/* only bits 0 and 7 are controlled by the I8035; the remaining */
	/* bits 1-6 reflect the current input latch values */
	return (usb->out_latch & 0x81) | (usb->in_latch & 0x7e);
}


static TIMER_CALLBACK( delayed_usb_data_w )
{
	usb_state *usb = (usb_state *)ptr;
	int data = param;

	/* look for rising/falling edges of bit 7 to control the RESET line */
	device_set_input_line(usb->cpu, INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	/* if the CLEAR line is set, the low 7 bits of the input are ignored */
	if ((usb->last_p2_value & 0x40) == 0)
		data &= ~0x7f;

	/* update the effective input latch */
	usb->in_latch = data;
}


WRITE8_DEVICE_HANDLER( sega_usb_data_w )
{
	usb_state *usb = get_safe_token(device);

	LOG(("%04X:usb_data_w = %02X\n", cpu_get_pc(usb->maincpu), data));
	device->machine().scheduler().synchronize(FUNC(delayed_usb_data_w), data, usb);

	/* boost the interleave so that sequences can be sent */
	device->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(250));
}


READ8_DEVICE_HANDLER( sega_usb_ram_r )
{
	usb_state *usb = get_safe_token(device);

	return usb->program_ram[offset];
}


WRITE8_DEVICE_HANDLER( sega_usb_ram_w )
{
	usb_state *usb = get_safe_token(device);

	if (usb->in_latch & 0x80)
		usb->program_ram[offset] = data;
	else
		LOG(("%04X:sega_usb_ram_w(%03X) = %02X while /LOAD disabled\n", cpu_get_pc(usb->maincpu), offset, data));
}



/*************************************
 *
 *  I8035 port accesses
 *
 *************************************/

static READ8_DEVICE_HANDLER( usb_p1_r )
{
	usb_state *usb = get_safe_token(device);

	/* bits 0-6 are inputs and map to bits 0-6 of the input latch */
	if ((usb->in_latch & 0x7f) != 0)
		LOG(("%03X: P1 read = %02X\n", cpu_get_pc(usb->maincpu), usb->in_latch & 0x7f));
	return usb->in_latch & 0x7f;
}


static WRITE8_DEVICE_HANDLER( usb_p1_w )
{
	usb_state *usb = get_safe_token(device);

	/* bit 7 maps to bit 0 on the output latch */
	usb->out_latch = (usb->out_latch & 0xfe) | (data >> 7);
	LOG(("%03X: P1 write = %02X\n", cpu_get_pc(usb->maincpu), data));
}


static WRITE8_DEVICE_HANDLER( usb_p2_w )
{
	usb_state *usb = get_safe_token(device);
	UINT8 old = usb->last_p2_value;
	usb->last_p2_value = data;

	/* low 2 bits control the bank of work RAM we are addressing */
	usb->work_ram_bank = data & 3;

	/* bit 6 controls the "ready" bit output to the host */
	/* it also clears the input latch from the host (active low) */
	usb->out_latch = ((data & 0x40) << 1) | (usb->out_latch & 0x7f);
	if ((data & 0x40) == 0)
		usb->in_latch = 0;

	/* bit 7 controls the reset on the upper counter at U33 */
	if ((old & 0x80) && !(data & 0x80))
		usb->t1_clock = 0;

	LOG(("%03X: P2 write -> bank=%d ready=%d clock=%d\n", cpu_get_pc(usb->maincpu), data & 3, (data >> 6) & 1, (data >> 7) & 1));
}


static READ8_DEVICE_HANDLER( usb_t1_r )
{
	usb_state *usb = get_safe_token(device);

	/* T1 returns 1 based on the value of the T1 clock; the exact */
	/* pattern is determined by one or more jumpers on the board. */
	return (usb->t1_clock & usb->t1_clock_mask) != 0;
}



/*************************************
 *
 *  Sound generation
 *
 *************************************/

INLINE void clock_channel(timer8253_channel *ch)
{
	UINT8 lastgate = ch->lastgate;

	/* update the gate */
	ch->lastgate = ch->gate;

	/* if we're holding, skip */
	if (ch->holding)
		return;

	/* switch off the clock mode */
	switch (ch->clockmode)
	{
		/* oneshot; waits for trigger to restart */
		case 1:
			if (!lastgate && ch->gate)
			{
				ch->output = 0;
				ch->remain = ch->count;
			}
			else
			{
				if (--ch->remain == 0)
					ch->output = 1;
			}
			break;

		/* square wave: counts down by 2 and toggles output */
		case 3:
			ch->remain = (ch->remain - 1) & ~1;
			if (ch->remain == 0)
			{
				ch->output ^= 1;
				ch->remain = ch->count;
			}
			break;
	}
}


static STREAM_UPDATE( usb_stream_update )
{
	usb_state *usb = get_safe_token(device);
	stream_sample_t *dest = outputs[0];

	/* iterate over samples */
	while (samples--)
	{
		double noiseval;
		double sample = 0;
		int group;
		int step;


		/*----------------
            Noise Source
          ----------------

                         RC
           MM5837 ---> FILTER ---> CR FILTER ---> 3.2x AMP ---> NOISE
                       LADDER
       */

		/* update the noise source */
		for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= usb->noise_subcount; step -= usb->noise_subcount)
		{
			usb->noise_shift = (usb->noise_shift << 1) | (((usb->noise_shift >> 13) ^ (usb->noise_shift >> 16)) & 1);
			usb->noise_state = (usb->noise_shift >> 16) & 1;
			usb->noise_subcount = USB_2MHZ_CLOCK / MM5837_CLOCK;
		}
		usb->noise_subcount -= step;

		/* update the filtered noise value -- this is just an approximation to the pink noise filter */
		/* being applied on the PCB, but it sounds pretty close */
		usb->noise_filters[0].capval = 0.99765 * usb->noise_filters[0].capval + usb->noise_state * 0.0990460;
		usb->noise_filters[1].capval = 0.96300 * usb->noise_filters[1].capval + usb->noise_state * 0.2965164;
		usb->noise_filters[2].capval = 0.57000 * usb->noise_filters[2].capval + usb->noise_state * 1.0526913;
		noiseval = usb->noise_filters[0].capval + usb->noise_filters[1].capval + usb->noise_filters[2].capval + usb->noise_state * 0.1848;

		/* final output goes through a CR filter; the scaling factor is arbitrary to get the noise to the */
		/* correct relative volume */
		noiseval = step_cr_filter(&usb->noise_filters[4], noiseval);
		noiseval *= 0.075;

		/* there are 3 identical groups of circuits, each with its own 8253 */
		for (group = 0; group < 3; group++)
		{
			timer8253 *g = &usb->timer_group[group];
			double chan0, chan1, chan2, mix;


			/*-------------
                Channel 0
              -------------

                8253        CR                   AD7524
                OUT0 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
            */

			/* channel 0 clocks with the PCS clock */
			for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= g->chan[0].subcount; step -= g->chan[0].subcount)
			{
				g->chan[0].subcount = USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				g->chan[0].gate = 1;
				clock_channel(&g->chan[0]);
			}
			g->chan[0].subcount -= step;

			/* channel 0 is mixed in with a resistance of 100k */
			chan0 = step_cr_filter(&g->chan_filter[0], g->chan[0].output) * g->env[0] * (1.0/100.0);


			/*-------------
                Channel 1
              -------------

                8253        CR                   AD7524
                OUT1 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
            */

			/* channel 1 clocks with the PCS clock */
			for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= g->chan[1].subcount; step -= g->chan[1].subcount)
			{
				g->chan[1].subcount = USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				g->chan[1].gate = 1;
				clock_channel(&g->chan[1]);
			}
			g->chan[1].subcount -= step;

			/* channel 1 is mixed in with a resistance of 100k */
			chan1 = step_cr_filter(&g->chan_filter[1], g->chan[1].output) * g->env[1] * (1.0/100.0);


			/*-------------
                Channel 2
              -------------

              If timer_mode == 0:

                           SWITCHED                                  AD7524
                NOISE --->    RC   ---> 1.56x AMP ---> INVERTER --->  VRef ---> 33k ---> mix
                            FILTERS

              If timer mode == 1:

                                         AD7524                                    SWITCHED
                NOISE ---> INVERTER --->  VRef ---> 33k ---> mix ---> INVERTER --->   RC   ---> 1.56x AMP ---> finalmix
                                                                                    FILTERS
            */

			/* channel 2 clocks with the 2MHZ clock and triggers with the GOS clock */
			for (step = 0; step < USB_2MHZ_CLOCK / SAMPLE_RATE; step++)
			{
				if (g->chan[2].subcount-- == 0)
				{
					g->chan[2].subcount = USB_2MHZ_CLOCK / USB_GOS_CLOCK / 2 - 1;
					g->chan[2].gate = !g->chan[2].gate;
				}
				clock_channel(&g->chan[2]);
			}

			/* the exponents for the gate filters are determined by channel 2's output */
			g->gate1.exponent = usb->gate_rc1_exp[g->chan[2].output];
			g->gate2.exponent = usb->gate_rc2_exp[g->chan[2].output];

			/* based on the envelope mode, we do one of two things with source 2 */
			if (g->config == 0)
			{
				chan2 = step_rc_filter(&g->gate2, step_rc_filter(&g->gate1, noiseval)) * -1.56 * g->env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
			}
			else
			{
				chan2 = -noiseval * g->env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
				mix = step_rc_filter(&g->gate2, step_rc_filter(&g->gate1, -mix)) * 1.56;
			}

			/* accumulate the sample */
			sample += mix;
		}


		/*-------------
            Final mix
          -------------

          INPUTS
          EQUAL ---> 1.2x INVERTER ---> CR FILTER ---> out
          WEIGHT

       */
       *dest++ = 4000 * step_cr_filter(&usb->final_filter, sample);
	}
}


static DEVICE_START( usb_sound )
{
	usb_state *usb = get_safe_token(device);
	running_machine &machine = device->machine();
	filter_state temp;
	int tchan, tgroup;

	/* find the CPU we are associated with */
	usb->maincpu = machine.device("maincpu");
	assert(usb->maincpu != NULL);
	usb->cpu = machine.device("usbcpu");
	assert(usb->cpu != NULL);

	/* allocate RAM */
	usb->program_ram = (UINT8 *)machine.memory().shared("pgmram")->ptr();
	usb->work_ram = auto_alloc_array(machine, UINT8, 0x400);

	/* create a sound stream */
	usb->stream = device->machine().sound().stream_alloc(*device, 0, 1, SAMPLE_RATE, NULL, usb_stream_update);

	/* initialize state */
	usb->noise_shift = 0x15555;

	for (tgroup = 0; tgroup < 3; tgroup++)
	{
		timer8253 *g = &usb->timer_group[tgroup];
		configure_filter(&g->chan_filter[0], 10e3, 1e-6);
		configure_filter(&g->chan_filter[1], 10e3, 1e-6);
		configure_filter(&g->gate1, 100e3, 0.01e-6);
		configure_filter(&g->gate2, 2 * 100e3, 0.01e-6);
	}

	configure_filter(&temp, 100e3, 0.01e-6);
	usb->gate_rc1_exp[0] = temp.exponent;
	configure_filter(&temp, 1e3, 0.01e-6);
	usb->gate_rc1_exp[1] = temp.exponent;
	configure_filter(&temp, 2 * 100e3, 0.01e-6);
	usb->gate_rc2_exp[0] = temp.exponent;
	configure_filter(&temp, 2 * 1e3, 0.01e-6);
	usb->gate_rc2_exp[1] = temp.exponent;

	configure_filter(&usb->noise_filters[0], 2.7e3 + 2.7e3, 1.0e-6);
	configure_filter(&usb->noise_filters[1], 2.7e3 + 1e3, 0.30e-6);
	configure_filter(&usb->noise_filters[2], 2.7e3 + 270, 0.15e-6);
	configure_filter(&usb->noise_filters[3], 2.7e3 + 0, 0.082e-6);
	configure_filter(&usb->noise_filters[4], 33e3, 0.1e-6);

	configure_filter(&usb->final_filter, 100e3, 4.7e-6);

	/* register for save states */
	state_save_register_item(machine, "usb", NULL, 0, usb->in_latch);
	state_save_register_item(machine, "usb", NULL, 0, usb->out_latch);
	state_save_register_item(machine, "usb", NULL, 0, usb->last_p2_value);
	state_save_register_item(machine, "usb", NULL, 0, usb->work_ram_bank);
	state_save_register_item(machine, "usb", NULL, 0, usb->t1_clock);

	for (tgroup = 0; tgroup < 3; tgroup++)
	{
		timer8253 *group = &usb->timer_group[tgroup];
		for (tchan = 0; tchan < 3; tchan++)
		{
			timer8253_channel *channel = &group->chan[tchan];
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->holding);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->latchmode);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->latchtoggle);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->clockmode);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->bcdmode);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->output);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->lastgate);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->gate);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->subcount);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->count);
			state_save_register_item(machine, "usb", NULL, tgroup * 3 + tchan, channel->remain);
		}
		state_save_register_item_array(machine, "usb", NULL, tgroup, group->env);
		state_save_register_item(machine, "usb", NULL, tgroup, group->chan_filter[0].capval);
		state_save_register_item(machine, "usb", NULL, tgroup, group->chan_filter[1].capval);
		state_save_register_item(machine, "usb", NULL, tgroup, group->gate1.capval);
		state_save_register_item(machine, "usb", NULL, tgroup, group->gate2.capval);
		state_save_register_item(machine, "usb", NULL, tgroup, group->config);
	}

	state_save_register_item_array(machine, "usb", NULL, 0, usb->timer_mode);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_shift);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_state);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_subcount);
	state_save_register_item(machine, "usb", NULL, 0, usb->final_filter.capval);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_filters[0].capval);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_filters[1].capval);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_filters[2].capval);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_filters[3].capval);
	state_save_register_item(machine, "usb", NULL, 0, usb->noise_filters[4].capval);
}


static DEVICE_GET_INFO( usb_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(usb_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(usb_sound);		break;
		case DEVINFO_FCT_RESET:							info->start = DEVICE_RESET_NAME(usb_sound);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Sega Universal Sound Board");	break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(SEGAUSB, usb_sound);



/*************************************
 *
 *  USB timer and envelope controls
 *
 *************************************/

static void timer_w(usb_state *usb, int which, UINT8 offset, UINT8 data)
{
	timer8253 *g = &usb->timer_group[which];
	timer8253_channel *ch;
	int was_holding;

	usb->stream->update();

	/* switch off the offset */
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			ch = &g->chan[offset];
			was_holding = ch->holding;

			/* based on the latching mode */
			switch (ch->latchmode)
			{
				case 1:	/* low word only */
					ch->count = data;
					ch->holding = FALSE;
					break;

				case 2:	/* high word only */
					ch->count = data << 8;
					ch->holding = FALSE;
					break;

				case 3:	/* low word followed by high word */
					if (ch->latchtoggle == 0)
					{
						ch->count = (ch->count & 0xff00) | (data & 0x00ff);
						ch->latchtoggle = 1;
					}
					else
					{
						ch->count = (ch->count & 0x00ff) | (data << 8);
						ch->holding = FALSE;
						ch->latchtoggle = 0;
					}
					break;
			}

			/* if we're not holding, load the initial count for some modes */
			if (was_holding && !ch->holding)
				ch->remain = 1;
			break;

		case 3:
			/* break out the components */
			if (((data & 0xc0) >> 6) < 3)
			{
				ch = &g->chan[(data & 0xc0) >> 6];

				/* extract the bits */
				ch->holding = TRUE;
				ch->latchmode = (data >> 4) & 3;
				ch->clockmode = (data >> 1) & 7;
				ch->bcdmode = (data >> 0) & 1;
				ch->latchtoggle = 0;
				ch->output = (ch->clockmode == 1);
			}
			break;
	}
}


static void env_w(usb_state *usb, int which, UINT8 offset, UINT8 data)
{
	timer8253 *g = &usb->timer_group[which];

	usb->stream->update();

	if (offset < 3)
		g->env[offset] = (double)data;
	else
		g->config = data & 1;
}



/*************************************
 *
 *  USB work RAM access
 *
 *************************************/

static READ8_DEVICE_HANDLER( usb_workram_r )
{
	usb_state *usb = get_safe_token(device);

	offset += 256 * usb->work_ram_bank;
	return usb->work_ram[offset];
}


static WRITE8_DEVICE_HANDLER( usb_workram_w )
{
	usb_state *usb = get_safe_token(device);

	offset += 256 * usb->work_ram_bank;
	usb->work_ram[offset] = data;

	/* writes to the low 32 bytes go to various controls */
	switch (offset & ~3)
	{
		case 0x00:	/* CTC0 */
			timer_w(usb, 0, offset & 3, data);
			break;

		case 0x04:	/* ENV0 */
			env_w(usb, 0, offset & 3, data);
			break;

		case 0x08:	/* CTC1 */
			timer_w(usb, 1, offset & 3, data);
			break;

		case 0x0c:	/* ENV1 */
			env_w(usb, 1, offset & 3, data);
			break;

		case 0x10:	/* CTC2 */
			timer_w(usb, 2, offset & 3, data);
			break;

		case 0x14:	/* ENV2 */
			env_w(usb, 2, offset & 3, data);
			break;
	}
}



/*************************************
 *
 *  USB address maps
 *
 *************************************/

static ADDRESS_MAP_START( usb_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("pgmram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( usb_map_rom, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( usb_portmap, AS_IO, 8, driver_device )
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE_LEGACY("usbsnd", usb_workram_r, usb_workram_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVREADWRITE_LEGACY("usbsnd", usb_p1_r, usb_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_DEVWRITE_LEGACY("usbsnd", usb_p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_DEVREAD_LEGACY("usbsnd", usb_t1_r)
ADDRESS_MAP_END



/*************************************
 *
 *  USB machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( sega_universal_sound_board )

	/* CPU for the usb board */
	MCFG_CPU_ADD("usbcpu", I8035, USB_MASTER_CLOCK)		/* divide by 15 in CPU */
	MCFG_CPU_PROGRAM_MAP(usb_map)
	MCFG_CPU_IO_MAP(usb_portmap)

	MCFG_TIMER_ADD_PERIODIC("usb_timer", increment_t1_clock, attotime::from_hz(USB_2MHZ_CLOCK / 256))

	/* sound hardware */
	MCFG_SOUND_ADD("usbsnd", SEGAUSB, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( sega_universal_sound_board_rom, sega_universal_sound_board )

	/* CPU for the usb board */
	MCFG_CPU_MODIFY("usbcpu")
	MCFG_CPU_PROGRAM_MAP(usb_map_rom)
MACHINE_CONFIG_END
