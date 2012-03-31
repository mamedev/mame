/*

-Galaxy Force
-Run Away
--------------------------
Dai San Wakusei Meteor
(c)1979 Sun Electronics

SIV-01-B
TVG_13.6     [8e8b40b1]
TVG_14.7     [d48cbabe]
TVG_15.8     [cf44bd60]
TVG_16.9     [ae723f56]
--------------------------
-Warp 1


Dumped by Chack'n
01/04/2009

Written by Hau
02/18/2009
12/14/2010

Discrete by Andy
11/11/2009


Driver Notes:

- Two player games are automatically displayed in cocktail mode.
  Is this by design (a cocktail only romset)?

- Discrete audio needs adding to replace hardcoded samples

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "machine/rescap.h"
#include "sound/sn76477.h"

#define USE_SAMPLES		(1)


class dai3wksi_state : public driver_device
{
public:
	dai3wksi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video */
	UINT8 *     m_dai3wksi_videoram;
	size_t      m_dai3wksi_videoram_size;
	int         m_dai3wksi_flipscreen;
	int         m_dai3wksi_redscreen;
	int         m_dai3wksi_redterop;

	/* sound */
	UINT8       m_port_last1;
	UINT8       m_port_last2;
	int         m_enabled_sound;
	int         m_sound3_counter;
};


/*************************************
 *
 *  Video system
 *
 *************************************/

static const UINT8 vr_prom1[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

static const UINT8 vr_prom2[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};


static void dai3wksi_get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i <= 7; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	}
}


static SCREEN_UPDATE_RGB32( dai3wksi )
{
	dai3wksi_state *state = screen.machine().driver_data<dai3wksi_state>();
	offs_t offs;
	pen_t pens[8];

	dai3wksi_get_pens(pens);

	for (offs = 0; offs < state->m_dai3wksi_videoram_size; offs++)
	{
		offs_t i;

		UINT8 x = offs << 2;
		UINT8 y = offs >> 6;
		UINT8 data = state->m_dai3wksi_videoram[offs];
		UINT8 color;
		int value = (x >> 2) + ((y >> 5) << 6) + 64 * 8 * (state->m_dai3wksi_redterop ? 1 : 0);

		if (state->m_dai3wksi_redscreen)
		{
			color = 0x02;
		}
		else
		{
			if (input_port_read(screen.machine(), "IN2") & 0x03)
				color = vr_prom2[value];
			else
				color = vr_prom1[value];
		}

		for (i = 0; i <= 3; i++)
		{
			pen_t pen = (data & (1 << i)) ? pens[color] : pens[0];

			if (state->m_dai3wksi_flipscreen)
				bitmap.pix32(255-y, 255-x) = pen;
			else
				bitmap.pix32(y, x) = pen;

			x++;
		}
	}

	return 0;
}


/*************************************
 *
 *  Audio system
 *
 *************************************/

#define SAMPLE_SOUND1		0
#define SAMPLE_SOUND2		1
#define SAMPLE_SOUND3_1		2
#define SAMPLE_SOUND3_2		3
#define SAMPLE_SOUND4		4
#define SAMPLE_SOUND5		5
#define SAMPLE_SOUND6_1		6
#define SAMPLE_SOUND6_2		7

#define CHANNEL_SOUND1		0
#define CHANNEL_SOUND2		1
#define CHANNEL_SOUND3		2
#define CHANNEL_SOUND4		3
#define CHANNEL_SOUND5		4
#define CHANNEL_SOUND6		5


#if (USE_SAMPLES)
static WRITE8_HANDLER( dai3wksi_audio_1_w )
{
	dai3wksi_state *state = space->machine().driver_data<dai3wksi_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");
	UINT8 rising_bits = data & ~state->m_port_last1;

	state->m_enabled_sound = data & 0x80;

	if ((rising_bits & 0x20) && state->m_enabled_sound)
	{
		if (data & 0x04)
			samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5);
		else
			samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5, true);
	}
	if (!(data & 0x20) && (state->m_port_last1 & 0x20))
		samples->stop(CHANNEL_SOUND5);

	state->m_port_last1 = data;
}

static WRITE8_HANDLER( dai3wksi_audio_2_w )
{
	dai3wksi_state *state = space->machine().driver_data<dai3wksi_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");
	UINT8 rising_bits = data & ~state->m_port_last2;

	state->m_dai3wksi_flipscreen = data & 0x10;
	state->m_dai3wksi_redscreen  = ~data & 0x20;
	state->m_dai3wksi_redterop   = data & 0x40;

	if (state->m_enabled_sound)
	{
		if (rising_bits & 0x01) samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1);
		if (rising_bits & 0x02) samples->start(CHANNEL_SOUND2, SAMPLE_SOUND2);
		if (rising_bits & 0x08) samples->start(CHANNEL_SOUND4, SAMPLE_SOUND4);
		if (rising_bits & 0x04)
		{
			if (!state->m_sound3_counter)
				samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_1);
			else
				samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_2);

			state->m_sound3_counter ^= 1;
		}
	}

	state->m_port_last2 = data;
}

static WRITE8_HANDLER( dai3wksi_audio_3_w )
{
	dai3wksi_state *state = space->machine().driver_data<dai3wksi_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	if (state->m_enabled_sound)
	{
		if (data & 0x40)
			samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_1);
		else if (data & 0x80)
			samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_2);
	}
}


static const char *const dai3wksi_sample_names[] =
{
	"*dai3wksi",
	"1",
	"2",
	"3",
	"3-2",
	"4",
	"5",
	"6",
	"6-2",
	0
};


static const samples_interface dai3wksi_samples_interface =
{
	6,	/* 6 channels */
	dai3wksi_sample_names
};

#else

static WRITE8_HANDLER( dai3wksi_audio_1_w )
{
	device_t *ic79 = space->machine().device("ic79");

	space->machine().sound().system_enable(data & 0x80);

	sn76477_enable_w(ic79, (~data >> 5) & 0x01);		/* invader movement enable */
	sn76477_envelope_1_w(ic79, (~data >> 2) & 0x01);	/* invader movement envelope control*/
}

static WRITE8_HANDLER( dai3wksi_audio_2_w )
{

	dai3wksi_state *state = space->machine().driver_data<dai3wksi_state>();
	device_t *ic77 = space->machine().device("ic77");
	device_t *ic78 = space->machine().device("ic78");
	device_t *ic80 = space->machine().device("ic80");

	state->m_dai3wksi_flipscreen =  data & 0x10;
	state->m_dai3wksi_redscreen  = ~data & 0x20;
	state->m_dai3wksi_redterop   =  data & 0x40;

	sn76477_enable_w(ic77, (~data >> 0) & 0x01);	/* ship movement */
	sn76477_enable_w(ic78, (~data >> 1) & 0x01);	/* danger text */
	/* ic76 - invader hit  (~data >> 2) & 0x01 */
	sn76477_enable_w(ic80, (~data >> 3) & 0x01);	/* planet explosion */
}

static WRITE8_HANDLER( dai3wksi_audio_3_w )
{
	device_t *ic81 = space->machine().device("ic81");

	sn76477_enable_w(ic81, (~data >> 2) & 0x01);	/* player shoot enable */
	sn76477_vco_w(ic81, (~data >> 3) & 0x01);		/* player shoot vco control */
}


/* Invader Hit */
static const sn76477_interface dai3wksi_sn76477_ic76 =
{
	0,				/*  4 noise_res (N/C)        */
	0,				/*  5 filter_res (N/C)       */
	0,				/*  6 filter_cap (N/C)       */
	RES_K(4.7),		/*  7 decay_res              */
	CAP_U(0.1),		/*  8 attack_decay_cap       */
	RES_K(4.7), 	/* 10 attack_res             */
	RES_K(150),		/* 11 amplitude_res          */
	RES_K(47),		/* 12 feedback_res           */
	0,				/* 16 vco_voltage (variable) */
	CAP_U(0.022),	/* 17 vco_cap                */
	RES_K(33),		/* 18 vco_res                */
	5.0,			/* 19 pitch_voltage          */
	0,				/* 20 slf_res (N/C)          */
	0,				/* 21 slf_cap (N/C)          */
	0,				/* 23 oneshot_cap (N/C)      */
	0,				/* 24 oneshot_res (N/C)      */
	0,				/* 22 vco                    */
	0,				/* 26 mixer A                */
	0,				/* 25 mixer B                */
	0,				/* 27 mixer C                */
	0,				/* 1  envelope 1             */
	0,				/* 28 envelope 2             */
	0				/* 9  enable                 */
};


/* Ship Movement */
static const sn76477_interface dai3wksi_sn76477_ic77 =
{
	0,				/*  4 noise_res (N/C)      */
	0,				/*  5 filter_res (N/C)     */
	0,				/*  6 filter_cap (N/C)     */
	RES_K(4.7),		/*  7 decay_res            */
	CAP_U(0.1),		/*  8 attack_decay_cap     */
	RES_K(4.7), 	/* 10 attack_res           */
	RES_K(150),		/* 11 amplitude_res        */
	RES_K(47),		/* 12 feedback_res         */
	0,				/* 16 vco_voltage (N/C)    */
	0,				/* 17 vco_cap (N/C)        */
	0,				/* 18 vco_res (N/C)        */
	0,				/* 19 pitch_voltage        */
	RES_K(200),		/* 20 slf_res              */
	CAP_U(0.0022),	/* 21 slf_cap              */
	CAP_U(10),		/* 23 oneshot_cap          */
	RES_K(4.7),		/* 24 oneshot_res          */
	5,				/* 22 vco                  */
	5,				/* 26 mixer A              */
	0,				/* 25 mixer B              */
	0,				/* 27 mixer C              */
	5,				/* 1  envelope 1           */
	0,				/* 28 envelope 2           */
	1				/* 9  enable (variable)    */
};


/* Danger */
static const sn76477_interface dai3wksi_sn76477_ic78 =
{
	RES_K(47),		/*  4 noise_res            */
	0,				/*  5 filter_res (N/C)     */
	0,				/*  6 filter_cap (N/C)     */
	RES_K(200),		/*  7 decay_res            */
	CAP_U(0.1),		/*  8 attack_decay_cap     */
	RES_K(4.7), 	/* 10 attack_res           */
	RES_K(150),		/* 11 amplitude_res        */
	RES_K(47),		/* 12 feedback_res         */
	0,				/* 16 vco_voltage (N/C)    */
	CAP_U(0.47),	/* 17 vco_cap              */
	RES_K(75),		/* 18 vco_res              */
	5.0,			/* 19 pitch_voltage        */
	RES_K(47),		/* 20 slf_res              */
	CAP_N(1),		/* 21 slf_cap              */
	CAP_U(10),		/* 23 oneshot_cap          */
	RES_K(22),		/* 24 oneshot_res          */
	5,				/* 22 vco                  */
	0,				/* 26 mixer A              */
	0,				/* 25 mixer B              */
	0,				/* 27 mixer C              */
	5,				/* 1  envelope 1           */
	0,				/* 28 envelope 2           */
	1				/* 9  enable (variable)    */
};


/* Invader Marching Noise */
static const sn76477_interface dai3wksi_sn76477_ic79 =
{
	0,				/*  4 noise_res (N/C)      */
	0,				/*  5 filter_res (N/C)     */
	0,				/*  6 filter_cap (N/C)     */
	RES_K(56),		/*  7 decay_res            */
	CAP_U(0.1),		/*  8 attack_decay_cap     */
	RES_K(4.7), 	/* 10 attack_res           */
	RES_K(150),		/* 11 amplitude_res        */
	RES_K(47),		/* 12 feedback_res         */
	0,				/* 16 vco_voltage (N/C)    */
	CAP_U(0.01),	/* 17 vco_cap              */
	RES_K(100),		/* 18 vco_res              */
	5.0,			/* 19 pitch_voltage        */
	RES_K(150),		/* 20 slf_res              */
	CAP_N(1),		/* 21 slf_cap              */
	CAP_U(10),		/* 23 oneshot_cap          */
	RES_K(22),		/* 24 oneshot_res          */
	5,				/* 22 vco                  */
	0,				/* 26 mixer A              */
	0,				/* 25 mixer B              */
	0,				/* 27 mixer C              */
	5,				/* 1  envelope 1 (variable)*/
	5,				/* 28 envelope 2           */
	1				/* 9  enable (variable)    */
};


/* Big Planet Explosion */
static const sn76477_interface dai3wksi_sn76477_ic80 =
{
	RES_K(47),		/*  4 noise_res            */
	RES_K(330),		/*  5 filter_res           */
	CAP_P(470),		/*  6 filter_cap           */
	RES_M(2),		/*  7 decay_res            */
	CAP_U(1),		/*  8 attack_decay_cap     */
	RES_K(4.7), 	/* 10 attack_res           */
	RES_K(150),		/* 11 amplitude_res        */
	RES_K(47),		/* 12 feedback_res         */
	0,				/* 16 vco_voltage (N/C)    */
	0,				/* 17 vco_cap (N/C)        */
	0,				/* 18 vco_res (N/C)        */
	5.0,			/* 19 pitch_voltage        */
	0,				/* 20 slf_res (N/C)        */
	0,				/* 21 slf_cap (N/C)        */
	CAP_U(10),		/* 23 oneshot_cap          */
	RES_K(55),		/* 24 oneshot_res          */
	5,				/* 22 vco                  */
	0,				/* 26 mixer A              */
	5,				/* 25 mixer B              */
	0,				/* 27 mixer C              */
	5,				/* 1  envelope 1           */
	0,				/* 28 envelope 2           */
	1				/* 9  enable (variable)    */
};


/* Plane Shoot noise */
static const sn76477_interface dai3wksi_sn76477_ic81 =
{
	0,				/*  4 noise_res (N/C)      */
	0,				/*  5 filter_res (N/C)     */
	0,				/*  6 filter_cap (N/C)     */
	RES_K(200),		/*  7 decay_res            */
	CAP_U(10),		/*  8 attack_decay_cap     */
	RES_K(4.7), 	/* 10 attack_res           */
	RES_K(150),		/* 11 amplitude_res        */
	RES_K(47),		/* 12 feedback_res         */
	2.5,			/* 16 vco_voltage    */
	CAP_U(0.01),	/* 17 vco_cap              */
	RES_K(100),		/* 18 vco_res              */
	5.0,			/* 19 pitch_voltage        */
	RES_K(100),		/* 20 slf_res              */
	CAP_N(0.47),	/* 21 slf_cap              */
	CAP_U(10),		/* 23 oneshot_cap          */
	RES_K(6.8),		/* 24 oneshot_res          */
	0,				/* 22 vco (variable)       */
	0,				/* 26 mixer A              */
	5,				/* 25 mixer B              */
	5,				/* 27 mixer C              */
	5,				/* 1  envelope 1           */
	0,				/* 28 envelope 2           */
	1				/* 9  enable (variable)    */
};

#endif



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, dai3wksi_state )
	AM_RANGE(0x0000, 0x1bff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x24ff) AM_MIRROR(0x100) AM_READ_PORT("IN0")
	AM_RANGE(0x2800, 0x28ff) AM_MIRROR(0x100) AM_READ_PORT("IN1")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(dai3wksi_audio_1_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(dai3wksi_audio_2_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(dai3wksi_audio_3_w)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_BASE_SIZE_MEMBER(dai3wksi_state, m_dai3wksi_videoram, m_dai3wksi_videoram_size)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( dai3wksi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x00, "DIPSW #7" )						PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW #8" )						PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW #1" )						PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW #2" )						PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_START( dai3wksi )
{
	dai3wksi_state *state = machine.driver_data<dai3wksi_state>();

	/* Set up save state */
	state->save_item(NAME(state->m_dai3wksi_flipscreen));
	state->save_item(NAME(state->m_dai3wksi_redscreen));
	state->save_item(NAME(state->m_dai3wksi_redterop));
	state->save_item(NAME(state->m_port_last1));
	state->save_item(NAME(state->m_port_last2));
	state->save_item(NAME(state->m_enabled_sound));
	state->save_item(NAME(state->m_sound3_counter));
}

static MACHINE_RESET( dai3wksi )
{
	dai3wksi_state *state = machine.driver_data<dai3wksi_state>();

	state->m_port_last1 = 0;
	state->m_port_last2 = 0;
	state->m_enabled_sound = 0;
	state->m_sound3_counter = 0;
}


static MACHINE_CONFIG_START( dai3wksi, dai3wksi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10MHz/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(dai3wksi)
	MCFG_MACHINE_RESET(dai3wksi)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(4, 251, 8, 247)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_STATIC(dai3wksi)

	MCFG_SPEAKER_STANDARD_MONO("mono")

#if (USE_SAMPLES)
	MCFG_SAMPLES_ADD("samples", dai3wksi_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
#else
	MCFG_SOUND_ADD("ic76", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic76)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_SOUND_ADD("ic77", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic77)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_SOUND_ADD("ic78", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic78)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_SOUND_ADD("ic79", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic79)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_SOUND_ADD("ic80", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic80)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_SOUND_ADD("ic81", SN76477, 0)
	MCFG_SOUND_CONFIG(dai3wksi_sn76477_ic81)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)
#endif
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dai3wksi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_13.6",  0x0000, 0x0800, CRC(8e8b40b1) SHA1(25b9223486dd348ea302e8e8f1d47c804a88b142) )
	ROM_LOAD( "tvg_14.7",  0x0800, 0x0800, CRC(d48cbabe) SHA1(64b571cd778fc7d67a5fa998a0defd36c04f111f) )
	ROM_LOAD( "tvg_15.8",  0x1000, 0x0800, CRC(cf44bd60) SHA1(61e0b3f9c4a1f9da1de57fb8276d4fc9e43b8f24) )
	ROM_LOAD( "tvg_16.9",  0x1800, 0x0400, CRC(ae723f56) SHA1(c25c27d6144533b2b2a888bfa8dbf48ed8d8b09a) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, dai3wksi, 0, dai3wksi, dai3wksi, 0, ROT270, "Sun Electronics", "Dai San Wakusei Meteor (Japan)", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
