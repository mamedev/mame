/* from Andrew Scott (ascott@utkux.utcc.utk.edu) */

/*
  updated by BUT
  - corrected music tempo (not confirmed Satan of Saturn and clone)
  - adjusted music freq (except Satan of Saturn and clone)
  - adjusted music waveform
  - support playing flag for music channel 0
  - support HD38880 speech by samples
*/


#include "emu.h"
#include "sound/sn76477.h"
#include "sound/samples.h"
#include "includes/snk6502.h"
#include "sound/discrete.h"


#ifndef M_LN2
#define M_LN2		0.69314718055994530942
#endif

#define TONE_VOLUME	50
#define CHANNELS	3

#define SAMPLE_RATE	(48000)
#define FRAC_BITS	16
#define FRAC_ONE	(1 << FRAC_BITS)
#define FRAC_MASK	(FRAC_ONE - 1)

typedef struct tone
{
	int	mute;
	int	offset;
	int	base;
	int	mask;
	INT32	sample_rate;
	INT32	sample_step;
	INT32	sample_cur;
	INT16	form[16];
} TONE;

typedef struct _snk6502_sound_state snk6502_sound_state;
struct _snk6502_sound_state
{
	TONE m_tone_channels[CHANNELS];
	INT32 m_tone_clock_expire;
	INT32 m_tone_clock;
	sound_stream * m_tone_stream;

	samples_device *m_samples;
	UINT8 *m_ROM;
	int m_Sound0StopOnRollover;
	UINT8 m_LastPort1;

	int m_hd38880_cmd;
	UINT32 m_hd38880_addr;
	int m_hd38880_data_bytes;
	double m_hd38880_speed;
};

static const char *const sasuke_sample_names[] =
{
	"*sasuke",

	// SN76477 and discrete
	"hit",
	"boss_start",
	"shot",
	"boss_attack",

	0
};

const samples_interface sasuke_samples_interface =
{
	4,	/* 4 channels */
	sasuke_sample_names
};

static const char *const vanguard_sample_names[] =
{
	"*vanguard",

	// SN76477 and discrete
	"fire",
	"explsion",

	// HD38880 speech
	"vg_voi-0",
	"vg_voi-1",
	"vg_voi-2",
	"vg_voi-3",
	"vg_voi-4",
	"vg_voi-5",
	"vg_voi-6",
	"vg_voi-7",
	"vg_voi-8",
	"vg_voi-9",
	"vg_voi-a",
	"vg_voi-b",
	"vg_voi-c",
	"vg_voi-d",
	"vg_voi-e",
	"vg_voi-f",

	0
};

const samples_interface vanguard_samples_interface =
{
	3,	/* 3 channel */
	vanguard_sample_names
};

static const char *const fantasy_sample_names[] =
{
	"*fantasy",

	// HD38880 speech
	"ft_voi-0",
	"ft_voi-1",
	"ft_voi-2",
	"ft_voi-3",
	"ft_voi-4",
	"ft_voi-5",
	"ft_voi-6",
	"ft_voi-7",
	"ft_voi-8",
	"ft_voi-9",
	"ft_voi-a",
	"ft_voi-b",

	0
};

const samples_interface fantasy_samples_interface =
{
	1,	/* 1 channel */
	fantasy_sample_names
};


const sn76477_interface sasuke_sn76477_intf_1 =
{
	RES_K(470),		/*  4  noise_res     */
	RES_K(150),		/*  5  filter_res    */
	CAP_P(4700),	/*  6  filter_cap    */
	RES_K(22),		/*  7  decay_res     */
	CAP_U(10),		/*  8  attack_decay_cap  */
	RES_K(10),		/* 10  attack_res    */
	RES_K(100),		/* 11  amplitude_res     */
	RES_K(47),		/* 12  feedback_res      */
	0 /* NC */,		/* 16  vco_voltage   */
	0 /* NC */,		/* 17  vco_cap       */
	0 /* NC */,		/* 18  vco_res       */
	0 /* NC */,		/* 19  pitch_voltage     */
	RES_K(10),		/* 20  slf_res       */
	0 /* NC */,		/* 21  slf_cap       */
	CAP_U(2.2),		/* 23  oneshot_cap   */
	RES_K(100),		/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	0,			    /* 26  mixer A           */
	1,			    /* 25  mixer B           */
	0,			    /* 27  mixer C           */
	1,			    /* 1   envelope 1        */
	0,			    /* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// ic48     GND: 2,22,26,27,28  +5V: 1,15,25
};

const sn76477_interface sasuke_sn76477_intf_2 =
{
	RES_K(340),		/*  4  noise_res     */
	RES_K(47),		/*  5  filter_res    */
	CAP_P(100),		/*  6  filter_cap    */
	RES_K(470),		/*  7  decay_res     */
	CAP_U(4.7),		/*  8  attack_decay_cap  */
	RES_K(10),		/* 10  attack_res    */
	RES_K(100),		/* 11  amplitude_res     */
	RES_K(47),		/* 12  feedback_res      */
	0 /* NC */,		/* 16  vco_voltage   */
	CAP_P(220),		/* 17  vco_cap       */
	RES_K(1000),	/* 18  vco_res       */
	0 /* NC */,		/* 19  pitch_voltage     */
	RES_K(220),		/* 20  slf_res       */
	0 /* NC */,		/* 21  slf_cap       */
	CAP_U(22),		/* 23  oneshot_cap   */
	RES_K(47),		/* 24  oneshot_res   */
	1,			    /* 22  vco                    */
	0,			    /* 26  mixer A           */
	1,			    /* 25  mixer B           */
	0,			    /* 27  mixer C           */
	1,			    /* 1   envelope 1        */
	1,			    /* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// ic51     GND: 2,26,27        +5V: 1,15,22,25,28
};

const sn76477_interface sasuke_sn76477_intf_3 =
{
	RES_K(330),		/*  4  noise_res     */
	RES_K(47),		/*  5  filter_res    */
	CAP_P(100),		/*  6  filter_cap    */
	RES_K(1),		/*  7  decay_res     */
	0 /* NC */,		/*  8  attack_decay_cap  */
	RES_K(1),		/* 10  attack_res    */
	RES_K(100),		/* 11  amplitude_res     */
	RES_K(47),		/* 12  feedback_res      */
	0 /* NC */,		/* 16  vco_voltage   */
	CAP_P(1000),	/* 17  vco_cap       */
	RES_K(1000),	/* 18  vco_res       */
	0 /* NC */,		/* 19  pitch_voltage     */
	RES_K(10),		/* 20  slf_res       */
	CAP_U(1),		/* 21  slf_cap       */
	CAP_U(2.2),		/* 23  oneshot_cap   */
	RES_K(150),		/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	1,			    /* 26  mixer A           */
	1,			    /* 25  mixer B           */
	0,			    /* 27  mixer C           */
	1,			    /* 1   envelope 1        */
	0,			    /* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// ic52     GND: 2,22,27,28     +5V: 1,15,25,26
};

const sn76477_interface satansat_sn76477_intf =
{
	RES_K(470),		/*  4  noise_res     */
	RES_M(1.5),		/*  5  filter_res    */
	CAP_P(220),		/*  6  filter_cap    */
	0,				/*  7  decay_res     */
	0,				/*  8  attack_decay_cap  */
	0,				/* 10  attack_res    */
	RES_K(47),		/* 11  amplitude_res     */
	RES_K(47),		/* 12  feedback_res      */
	0,				/* 16  vco_voltage   */
	0,				/* 17  vco_cap       */
	0,				/* 18  vco_res       */
	0,				/* 19  pitch_voltage     */
	0,				/* 20  slf_res       */
	0,				/* 21  slf_cap       */
	0,				/* 23  oneshot_cap   */
	0,				/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	0,			    /* 26  mixer A           */
	1,			    /* 25  mixer B           */
	0,			    /* 27  mixer C           */
	1,			    /* 1   envelope 1        */
	1,			    /* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// ???      GND: 2,26,27        +5V: 15,25
};

const sn76477_interface vanguard_sn76477_intf_1 =
{
	RES_K(470),		/*  4  noise_res     */
	RES_M(1.5),		/*  5  filter_res    */
	CAP_P(220),		/*  6  filter_cap    */
	0,				/*  7  decay_res     */
	0,				/*  8  attack_decay_cap  */
	0,				/* 10  attack_res    */
	RES_K(47),		/* 11  amplitude_res     */
	RES_K(4.7),		/* 12  feedback_res      */
	0,				/* 16  vco_voltage   */
	0,				/* 17  vco_cap       */
	0,				/* 18  vco_res       */
	0,				/* 19  pitch_voltage     */
	0,				/* 20  slf_res       */
	0,				/* 21  slf_cap       */
	0,				/* 23  oneshot_cap   */
	0,				/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	0,				/* 26  mixer A           */
	1,				/* 25  mixer B           */
	0,				/* 27  mixer C           */
	1,				/* 1   envelope 1        */
	1,				/* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// SHOT A   GND: 2,9,26,27  +5V: 15,25
};

const sn76477_interface vanguard_sn76477_intf_2 =
{
	RES_K(10),		/*  4  noise_res     */
	RES_K(30),		/*  5  filter_res    */
	0,				/*  6  filter_cap    */
	0,				/*  7  decay_res     */
	0,				/*  8  attack_decay_cap  */
	0,				/* 10  attack_res    */
	RES_K(47),		/* 11  amplitude_res     */
	RES_K(4.7),		/* 12  feedback_res      */
	0,				/* 16  vco_voltage   */
	0,				/* 17  vco_cap       */
	0,				/* 18  vco_res       */
	0,				/* 19  pitch_voltage     */
	0,				/* 20  slf_res       */
	0,				/* 21  slf_cap       */
	0,				/* 23  oneshot_cap   */
	0,				/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	0,				/* 26  mixer A           */
	1,				/* 25  mixer B           */
	0,				/* 27  mixer C           */
	0,				/* 1   envelope 1        */
	1,				/* 28  envelope 2        */
	1			    /* 9   enable (variable)      */

	// SHOT B   GND: 1,2,26,27  +5V: 15,25,28
};

const sn76477_interface fantasy_sn76477_intf =
{
	RES_K(470),		/*  4  noise_res     */
	RES_M(1.5),		/*  5  filter_res    */
	CAP_P(220),		/*  6  filter_cap    */
	0,				/*  7  decay_res     */
	0,				/*  8  attack_decay_cap  */
	0,				/* 10  attack_res    */
	RES_K(470),		/* 11  amplitude_res     */
	RES_K(4.7),		/* 12  feedback_res      */
	0,				/* 16  vco_voltage   */
	0,				/* 17  vco_cap       */
	0,				/* 18  vco_res       */
	0,				/* 19  pitch_voltage     */
	0,				/* 20  slf_res       */
	0,				/* 21  slf_cap       */
	0,				/* 23  oneshot_cap   */
	0,				/* 24  oneshot_res   */
	0,			    /* 22  vco                    */
	0,			    /* 26  mixer A           */
	1,			    /* 25  mixer B           */
	0,			    /* 27  mixer C           */
	/* schematic does not show pin 1 grounded, but it must be. */
	/* otherwise it is using the VCO for the envelope, but the VCO is not hooked up */
	0,			    /* 1   envelope 1        */
	1,			    /* 28  envelope 2        */
	0			    /* 9   enable (variable)      */

	// BOMB     GND:    2,9,26,27       +5V: 15,25
};


/************************************************************************
 * fantasy Sound System Analog emulation
 * July 2008, D. Renaud
 ************************************************************************/

static const discrete_op_amp_filt_info fantasy_filter =
{
	RES_K(10.5), 0, RES_K(33), 0, RES_K(470), CAP_U(.01), CAP_U(.01), 0, 0, 12, -12
};

#define FANTASY_BOMB_EN				NODE_01
#define FANTASY_NOISE_STREAM_IN		NODE_02
#define FANTASY_NOISE_LOGIC			NODE_03

DISCRETE_SOUND_START( fantasy )

	DISCRETE_INPUT_LOGIC (FANTASY_BOMB_EN)
	DISCRETE_INPUT_STREAM(FANTASY_NOISE_STREAM_IN, 0)

	/* This is not the perfect way to discharge, but it is good enough for now */
	/* it does not take into acount that there is no discharge when noise is low */
	DISCRETE_RCDISC2(NODE_10, FANTASY_BOMB_EN, 0, RES_K(10) + RES_K(33), DEFAULT_TTL_V_LOGIC_1 - 0.5, RES_K(1), CAP_U(1))
	DISCRETE_CLAMP(FANTASY_NOISE_LOGIC, FANTASY_NOISE_STREAM_IN, 0, 1)
	DISCRETE_SWITCH(NODE_11, 1, FANTASY_NOISE_LOGIC, 0, NODE_10)

	DISCRETE_OP_AMP_FILTER(NODE_20, 1, NODE_11, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &fantasy_filter)
	DISCRETE_RCFILTER(NODE_21, NODE_20, RES_K(22), CAP_U(.01))
	DISCRETE_RCFILTER(NODE_22, NODE_21, RES_K(22) +  RES_K(22), CAP_P(2200))
	DISCRETE_RCFILTER(NODE_23, NODE_22, RES_K(22) + RES_K(22) +  RES_K(22), CAP_U(.001))

	DISCRETE_OUTPUT(NODE_23, 32760.0/12)
DISCRETE_SOUND_END

INLINE snk6502_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == SNK6502);

	return (snk6502_sound_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE void validate_tone_channel(snk6502_sound_state *state, int channel)
{
	TONE *tone_channels = state->m_tone_channels;

	if (!tone_channels[channel].mute)
	{
		UINT8 romdata = state->m_ROM[tone_channels[channel].base + tone_channels[channel].offset];

		if (romdata != 0xff)
			tone_channels[channel].sample_step = tone_channels[channel].sample_rate / (256 - romdata);
		else
			tone_channels[channel].sample_step = 0;
	}
}

static STREAM_UPDATE( snk6502_tone_update )
{
	stream_sample_t *buffer = outputs[0];
	snk6502_sound_state *state = get_safe_token(device);
	TONE *tone_channels = state->m_tone_channels;
	int i;

	for (i = 0; i < CHANNELS; i++)
		validate_tone_channel(state, i);

	while (samples-- > 0)
	{
		INT32 data = 0;

		for (i = 0; i < CHANNELS; i++)
		{
			TONE *voice = &tone_channels[i];
			INT16 *form = voice->form;

			if (!voice->mute && voice->sample_step)
			{
				int cur_pos = voice->sample_cur + voice->sample_step;
				int prev = form[(voice->sample_cur >> FRAC_BITS) & 15];
				int cur = form[(cur_pos >> FRAC_BITS) & 15];

				/* interpolate */
				data += ((INT32)prev * (FRAC_ONE - (cur_pos & FRAC_MASK))
				        + (INT32)cur * (cur_pos & FRAC_MASK)) >> FRAC_BITS;

				voice->sample_cur = cur_pos;
			}
		}

		*buffer++ = data;

		state->m_tone_clock += FRAC_ONE;
		if (state->m_tone_clock >= state->m_tone_clock_expire)
		{
			for (i = 0; i < CHANNELS; i++)
			{
				tone_channels[i].offset++;
				tone_channels[i].offset &= tone_channels[i].mask;

				validate_tone_channel(state, i);
			}

			if (tone_channels[0].offset == 0 && state->m_Sound0StopOnRollover)
				tone_channels[0].mute = 1;

			state->m_tone_clock -= state->m_tone_clock_expire;
		}

	}
}


static void sasuke_build_waveform(snk6502_sound_state *state, int mask)
{
	TONE *tone_channels = state->m_tone_channels;
	int bit0, bit1, bit2, bit3;
	int base;
	int i;

	mask &= 7;

	//logerror("0: wave form = %d\n", mask);
	bit0 = bit1 = bit3 = 0;
	bit2 = 1;

	if (mask & 1)
		bit0 = 1;
	if (mask & 2)
		bit1 = 1;
	if (mask & 4)
		bit3 = 1;

	base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (i = 0; i < 16; i++)
	{
		int data = 0;

		if (i & 1)
			data += bit0;
		if (i & 2)
			data += bit1;
		if (i & 4)
			data += bit2;
		if (i & 8)
			data += bit3;

		//logerror(" %3d\n", data);
		tone_channels[0].form[i] = data - base;
	}

	for (i = 0; i < 16; i++)
		tone_channels[0].form[i] *= 65535 / 16;
}

static void satansat_build_waveform(snk6502_sound_state *state, int mask)
{
	TONE *tone_channels = state->m_tone_channels;
	int bit0, bit1, bit2, bit3;
	int base;
	int i;

	mask &= 7;

	//logerror("1: wave form = %d\n", mask);
	bit0 = bit1 = bit2 = 1;
	bit3 = 0;

	if (mask & 1)
		bit3 = 1;

	base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (i = 0; i < 16; i++)
	{
		int data = 0;

		if (i & 1)
			data += bit0;
		if (i & 2)
			data += bit1;
		if (i & 4)
			data += bit2;
		if (i & 8)
			data += bit3;

		//logerror(" %3d\n", data);
		tone_channels[1].form[i] = data - base;
	}

	for (i = 0; i < 16; i++)
		tone_channels[1].form[i] *= 65535 / 16;
}

static void build_waveform(snk6502_sound_state *state, int channel, int mask)
{
	TONE *tone_channels = state->m_tone_channels;
	int bit0, bit1, bit2, bit3;
	int base;
	int i;

	mask &= 15;

	//logerror("%d: wave form = %d\n", channel, mask);
	bit0 = bit1 = bit2 = bit3 = 0;

	// bit 3
	if (mask & (1 | 2))
		bit3 = 8;
	else if (mask & 4)
		bit3 = 4;
	else if (mask & 8)
		bit3 = 2;

	// bit 2
	if (mask & 4)
		bit2 = 8;
	else if (mask & (2 | 8))
		bit2 = 4;

	// bit 1
	if (mask & 8)
		bit1 = 8;
	else if (mask & 4)
		bit1 = 4;
	else if (mask & 2)
		bit1 = 2;

	// bit 0
	bit0 = bit1 / 2;

	if (bit0 + bit1 + bit2 + bit3 < 16)
	{
		bit0 *= 2;
		bit1 *= 2;
		bit2 *= 2;
		bit3 *= 2;
	}

	base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (i = 0; i < 16; i++)
	{
		/* special channel for fantasy */
		if (channel == 2)
		{
			tone_channels[channel].form[i] = (i & 8) ? 7 : -8;
		}
		else
		{
			int data = 0;

			if (i & 1)
				data += bit0;
			if (i & 2)
				data += bit1;
			if (i & 4)
				data += bit2;
			if (i & 8)
				data += bit3;

			//logerror(" %3d\n", data);
			tone_channels[channel].form[i] = data - base;
		}
	}

	for (i = 0; i < 16; i++)
		tone_channels[channel].form[i] *= 65535 / 160;
}

void snk6502_set_music_freq(running_machine &machine, int freq)
{
	device_t *device = machine.device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	TONE *tone_channels = state->m_tone_channels;

	int i;

	for (i = 0; i < CHANNELS; i++)
	{
		tone_channels[i].mute = 1;
		tone_channels[i].offset = 0;
		tone_channels[i].base = i * 0x800;
		tone_channels[i].mask = 0xff;
		tone_channels[i].sample_step = 0;
		tone_channels[i].sample_cur = 0;
		tone_channels[i].sample_rate = (double)(freq * 8) / SAMPLE_RATE * FRAC_ONE;

		build_waveform(state, i, 1);
	}
}

void snk6502_set_music_clock(running_machine &machine, double clock_time)
{
	device_t *device = machine.device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);

	state->m_tone_clock_expire = clock_time * SAMPLE_RATE * FRAC_ONE;
	state->m_tone_clock = 0;
}

static DEVICE_START( snk6502_sound )
{
	snk6502_sound_state *state = get_safe_token(device);

	state->m_samples = device->machine().device<samples_device>("samples");
	state->m_ROM = device->machine().region("snk6502")->base();

	// adjusted
	snk6502_set_music_freq(device->machine(), 43000);

	// 38.99 Hz update (according to schematic)
	snk6502_set_music_clock(device->machine(), M_LN2 * (RES_K(18) * 2 + RES_K(1)) * CAP_U(1));

	state->m_tone_stream = device->machine().sound().stream_alloc(*device, 0, 1, SAMPLE_RATE, NULL, snk6502_tone_update);
}

DEVICE_GET_INFO( snk6502_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(snk6502_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(snk6502_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "snk6502 Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}

int snk6502_music0_playing(running_machine &machine)
{
	device_t *device = machine.device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	TONE *tone_channels = state->m_tone_channels;

	return tone_channels[0].mute;
}


WRITE8_HANDLER( sasuke_sound_w )
{
	device_t *device = space->machine().device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	samples_device *samples = state->m_samples;
	TONE *tone_channels = state->m_tone_channels;

	switch (offset)
	{
	case 0:
		/*
            bit description

            0   hit (ic52)
            1   boss start (ic51)
            2   shot
            3   boss attack (ic48?)
            4   ??
            5
            6
            7   reset counter
        */

		if ((~data & 0x01) && (state->m_LastPort1 & 0x01))
			samples->start(0, 0);
		if ((~data & 0x02) && (state->m_LastPort1 & 0x02))
			samples->start(1, 1);
		if ((~data & 0x04) && (state->m_LastPort1 & 0x04))
			samples->start(2, 2);
		if ((~data & 0x08) && (state->m_LastPort1 & 0x08))
			samples->start(3, 3);

		if ((data & 0x80) && (~state->m_LastPort1 & 0x80))
		{
			tone_channels[0].offset = 0;
			tone_channels[0].mute = 0;
		}

		if ((~data & 0x80) && (state->m_LastPort1 & 0x80))
			tone_channels[0].mute = 1;

		state->m_LastPort1 = data;
		break;

	case 1:
		/*
            bit description

            0
            1   wave form
            2   wave form
            3   wave form
            4   MUSIC A8
            5   MUSIC A9
            6   MUSIC A10
            7
        */

		/* select tune in ROM based on sound command byte */
		tone_channels[0].base = 0x0000 + ((data & 0x70) << 4);
		tone_channels[0].mask = 0xff;

		state->m_Sound0StopOnRollover = 1;

		/* bit 1-3 sound0 waveform control */
		sasuke_build_waveform(state, (data & 0x0e) >> 1);
		break;
	}
}

WRITE8_HANDLER( satansat_sound_w )
{
	device_t *device = space->machine().device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	samples_device *samples = state->m_samples;
	TONE *tone_channels = state->m_tone_channels;

	switch (offset)
	{
	case 0:
		/*
            bit description

        */

		/* bit 0 = analog sound trigger */

		/* bit 1 = to 76477 */

		/* bit 2 = analog sound trigger */
		if (data & 0x04 && !(state->m_LastPort1 & 0x04))
			samples->start(0, 1);

		if (data & 0x08)
		{
			tone_channels[0].mute = 1;
			tone_channels[0].offset = 0;
		}

		/* bit 4-6 sound0 waveform control */
		sasuke_build_waveform(state, (data & 0x70) >> 4);

		/* bit 7 sound1 waveform control */
		satansat_build_waveform(state, (data & 0x80) >> 7);

		state->m_LastPort1 = data;
		break;
	case 1:
		/*
            bit description

        */

		/* select tune in ROM based on sound command byte */
		tone_channels[0].base = 0x0000 + ((data & 0x0e) << 7);
		tone_channels[0].mask = 0xff;
		tone_channels[1].base = 0x0800 + ((data & 0x60) << 4);
		tone_channels[1].mask = 0x1ff;

		state->m_Sound0StopOnRollover = 1;

		if (data & 0x01)
			tone_channels[0].mute = 0;

		if (data & 0x10)
			tone_channels[1].mute = 0;
		else
		{
			tone_channels[1].mute = 1;
			tone_channels[1].offset = 0;
		}

		/* bit 7 = ? */
		break;
	}
}

WRITE8_HANDLER( vanguard_sound_w )
{
	device_t *device = space->machine().device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	samples_device *samples = state->m_samples;
	TONE *tone_channels = state->m_tone_channels;

	switch (offset)
	{
	case 0:
		/*
            bit description

            0   MUSIC A10
            1   MUSIC A9
            2   MUSIC A8
            3   LS05 PORT 1
            4   LS04 PORT 2
            5   SHOT A
            6   SHOT B
            7   BOMB
        */

		/* select musical tune in ROM based on sound command byte */
		tone_channels[0].base = ((data & 0x07) << 8);
		tone_channels[0].mask = 0xff;

		state->m_Sound0StopOnRollover = 1;

		/* play noise samples requested by sound command byte */
		/* SHOT A */
		if (data & 0x20 && !(state->m_LastPort1 & 0x20))
			samples->start(1, 0);
		else if (!(data & 0x20) && state->m_LastPort1 & 0x20)
			samples->stop(1);

		/* BOMB */
		if (data & 0x80 && !(state->m_LastPort1 & 0x80))
			samples->start(2, 1);

		if (data & 0x08)
		{
			tone_channels[0].mute = 1;
			tone_channels[0].offset = 0;
		}

		if (data & 0x10)
		{
			tone_channels[0].mute = 0;
		}

		/* SHOT B */
		sn76477_enable_w(space->machine().device("sn76477.2"), (data & 0x40) ? 0 : 1);

		state->m_LastPort1 = data;
		break;
	case 1:
		/*
            bit description

            0   MUSIC A10
            1   MUSIC A9
            2   MUSIC A8
            3   LS04 PORT 3
            4   EXTP A (HD38880 external pitch control A)
            5   EXTP B (HD38880 external pitch control B)
            6
            7
        */

		/* select tune in ROM based on sound command byte */
		tone_channels[1].base = 0x0800 + ((data & 0x07) << 8);
		tone_channels[1].mask = 0xff;

		if (data & 0x08)
			tone_channels[1].mute = 0;
		else
		{
			tone_channels[1].mute = 1;
			tone_channels[1].offset = 0;
		}
		break;
	case 2:
		/*
            bit description

            0   AS 1    (sound0 waveform)
            1   AS 2    (sound0 waveform)
            2   AS 4    (sound0 waveform)
            3   AS 3    (sound0 waveform)
            4   AS 5    (sound1 waveform)
            5   AS 6    (sound1 waveform)
            6   AS 7    (sound1 waveform)
            7   AS 8    (sound1 waveform)
        */

		build_waveform(state, 0, (data & 0x3) | ((data & 4) << 1) | ((data & 8) >> 1));
		build_waveform(state, 1, data >> 4);
	}
}

WRITE8_HANDLER( fantasy_sound_w )
{
	device_t *device = space->machine().device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	TONE *tone_channels = state->m_tone_channels;

	switch (offset)
	{
	case 0:
		/*
            bit description

            0   MUSIC A10
            1   MUSIC A9
            2   MUSIC A8
            3   LS04 PART 1
            4   LS04 PART 2
            5
            6
            7   BOMB
        */

		/* select musical tune in ROM based on sound command byte */
		tone_channels[0].base = 0x0000 + ((data & 0x07) << 8);
		tone_channels[0].mask = 0xff;

		state->m_Sound0StopOnRollover = 0;

		if (data & 0x08)
			tone_channels[0].mute = 0;
		else
		{
			tone_channels[0].offset = tone_channels[0].base;
			tone_channels[0].mute = 1;
		}

		if (data & 0x10)
			tone_channels[2].mute = 0;
		else
		{
			tone_channels[2].offset = 0;
			tone_channels[2].mute = 1;
		}

		/* BOMB */
		discrete_sound_w(space->machine().device("discrete"), FANTASY_BOMB_EN, data & 0x80);

		state->m_LastPort1 = data;
		break;
	case 1:
		/*
            bit description

            0   MUSIC A10
            1   MUSIC A9
            2   MUSIC A8
            3   LS04 PART 3
            4   EXT PA (HD38880 external pitch control A)
            5   EXT PB (HD38880 external pitch control B)
            6
            7
        */

		/* select tune in ROM based on sound command byte */
		tone_channels[1].base = 0x0800 + ((data & 0x07) << 8);
		tone_channels[1].mask = 0xff;

		if (data & 0x08)
			tone_channels[1].mute = 0;
		else
		{
			tone_channels[1].mute = 1;
			tone_channels[1].offset = 0;
		}
		break;
	case 2:
		/*
            bit description

            0   AS 1    (sound0 waveform)
            1   AS 3    (sound0 waveform)
            2   AS 2    (sound0 waveform)
            3   AS 4    (sound0 waveform)
            4   AS 5    (sound1 waveform)
            5   AS 6    (sound1 waveform)
            6   AS 7    (sound1 waveform)
            7   AS 8    (sound1 waveform)
        */

		build_waveform(state, 0, (data & 0x9) | ((data & 2) << 1) | ((data & 4) >> 1));
		build_waveform(state, 1, data >> 4);
		break;
	case 3:
		/*
            bit description

            0   BC 1
            1   BC 2
            2   BC 3
            3   MUSIC A10
            4   MUSIC A9
            5   MUSIC A8
            6
            7   INV
        */

		/* select tune in ROM based on sound command byte */
		tone_channels[2].base = 0x1000 + ((data & 0x70) << 4);
		tone_channels[2].mask = 0xff;

		snk6502_flipscreen_w(space, 0, data);
		break;
	}
}


/*
  Hitachi HD38880 speech synthesizer chip

    I heard that this chip uses PARCOR coefficients but I don't know ROM data format.
    How do I generate samples?
*/


/* HD38880 command */
#define	HD38880_ADSET	2
#define	HD38880_READ	3
#define	HD38880_INT1	4
#define	HD38880_INT2	6
#define	HD38880_SYSPD	8
#define	HD38880_STOP	10
#define	HD38880_CONDT	11
#define	HD38880_START	12
#define	HD38880_SSTART	14

/* HD38880 control bits */
#define HD38880_CTP	0x10
#define HD38880_CMV	0x20
#define HD68880_SYBS	0x0f


static void snk6502_speech_w(running_machine &machine, UINT8 data, const UINT16 *table, int start)
{
	/*
        bit description
        0   SYBS1
        1   SYBS2
        2   SYBS3
        3   SYBS4
        4   CTP
        5   CMV
        6
        7
    */

	device_t *device = machine.device("snk6502");
	snk6502_sound_state *state = get_safe_token(device);
	samples_device *samples = state->m_samples;

	if ((data & HD38880_CTP) && (data & HD38880_CMV))
	{
		data &= HD68880_SYBS;

		switch (state->m_hd38880_cmd)
		{
		case 0:
			switch (data)
			{
			case HD38880_START:
				logerror("speech: START\n");

				if (state->m_hd38880_data_bytes == 5 && !samples->playing(0))
				{
					int i;

					for (i = 0; i < 16; i++)
					{
						if (table[i] && table[i] == state->m_hd38880_addr)
						{
							samples->start(0, start + i);
							break;
						}
					}
				}
				break;

			case HD38880_SSTART:
				logerror("speech: SSTART\n");
				break;

			case HD38880_STOP:
				samples->stop(0);
				logerror("speech: STOP\n");
				break;

			case HD38880_SYSPD:
				state->m_hd38880_cmd = data;
				break;

			case HD38880_CONDT:
				logerror("speech: CONDT\n");
				break;

			case HD38880_ADSET:
				state->m_hd38880_cmd = data;
				state->m_hd38880_addr = 0;
				state->m_hd38880_data_bytes = 0;
				break;

			case HD38880_READ:
				logerror("speech: READ\n");
				break;

			case HD38880_INT1:
				state->m_hd38880_cmd = data;
				break;

			case HD38880_INT2:
				state->m_hd38880_cmd = data;
				break;

			case 0:
				// ignore it
				break;

			default:
				logerror("speech: unknown command: 0x%x\n", data);
			}
			break;

		case HD38880_INT1:
			logerror("speech: INT1: 0x%x\n", data);

			if (data & 8)
				logerror("speech:   triangular waveform\n");
			else
				logerror("speech:   impulse waveform\n");

			logerror("speech:   %sable losing effect of vocal tract\n", data & 4 ? "en" : "dis");

			if ((data & 2) && (data & 8))
				logerror("speech:   use external pitch control\n");

			state->m_hd38880_cmd = 0;
			break;

		case HD38880_INT2:
			logerror("speech: INT2: 0x%x\n", data);

			logerror("speech:   %d bits / frame\n", data & 8 ? 48 : 96);
			logerror("speech:   %d ms / frame\n", data & 4 ? 20 : 10);
			logerror("speech:   %sable repeat\n", data & 2 ? "en" : "dis");
			logerror("speech:   %d operations\n", ((data & 8) == 0) || (data & 1) ? 10 : 8);

			state->m_hd38880_cmd = 0;
			break;

		case HD38880_SYSPD:
			state->m_hd38880_speed = ((double)(data + 1)) / 10.0;
			logerror("speech: SYSPD: %1.1f\n", state->m_hd38880_speed);
			state->m_hd38880_cmd = 0;
			break;

		case HD38880_ADSET:
			state->m_hd38880_addr |= (data << (state->m_hd38880_data_bytes++ * 4));
			if (state->m_hd38880_data_bytes == 5)
			{
				logerror("speech: ADSET: 0x%05x\n", state->m_hd38880_addr);
				state->m_hd38880_cmd = 0;
			}
			break;
		}
	}
}


/*
 vanguard/fantasy speech

 ROM data format (INT2 = 0xf):
  48 bits / frame
  20 ms / frame
  enable repeat
  10 operations
*/

WRITE8_HANDLER( vanguard_speech_w )
{
	static const UINT16 vanguard_table[16] =
	{
		0x04000,
		0x04325,
		0x044a2,
		0x045b7,
		0x046ee,
		0x04838,
		0x04984,
		0x04b01,
		0x04c38,
		0x04de6,
		0x04f43,
		0x05048,
		0x05160,
		0x05289,
		0x0539e,
		0x054ce
	};

	snk6502_speech_w(space->machine(), data, vanguard_table, 2);
}

WRITE8_HANDLER( fantasy_speech_w )
{
	static const UINT16 fantasy_table[16] =
	{
		0x04000,
		0x04297,
		0x044b6,
		0x04682,
		0x04927,
		0x04be0,
		0x04cc2,
		0x04e36,
		0x05000,
		0x05163,
		0x052c9,
		0x053fd,
		0,
		0,
		0,
		0
	};

	snk6502_speech_w(space->machine(), data, fantasy_table, 0);
}


DEFINE_LEGACY_SOUND_DEVICE(SNK6502, snk6502_sound);
