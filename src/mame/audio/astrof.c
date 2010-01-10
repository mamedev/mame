/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "includes/astrof.h"


/*************************************
 *
 *  Astro Fighter
 *
 *************************************/

#define SAMPLE_FIRE		 0
#define SAMPLE_EKILLED	 1
#define SAMPLE_WAVE		 2
#define SAMPLE_BOSSFIRE  6
#define SAMPLE_FUEL		 7
#define SAMPLE_DEATH	 8
#define SAMPLE_BOSSHIT	 9
#define SAMPLE_BOSSKILL  10

#define CHANNEL_FIRE	  0
#define CHANNEL_EXPLOSION 1
#define CHANNEL_WAVE      2   /* background humm */
#define CHANNEL_BOSSFIRE  2	  /* there is no background humm on the boss level */
#define CHANNEL_FUEL	  3


WRITE8_HANDLER( astrof_audio_1_w )
{
	astrof_state *state = (astrof_state *)space->machine->driver_data;
	UINT8 rising_bits = data & ~state->port_1_last;

	if (state->astrof_death_playing)
		state->astrof_death_playing = sample_playing(state->samples, CHANNEL_EXPLOSION);

	if (state->astrof_bosskill_playing)
		state->astrof_bosskill_playing = sample_playing(state->samples, CHANNEL_EXPLOSION);

	/* D2 - explosion */
	if (rising_bits & 0x04)
	{
		/* I *know* that the effect select port will be written shortly
           after this one, so this works */
		state->astrof_start_explosion = 1;
	}

	/* D0,D1,D3 - background noise */
	if ((data & 0x08) && (~state->port_1_last & 0x08))
	{
		int sample = SAMPLE_WAVE + (data & 3);
		sample_start(state->samples, CHANNEL_WAVE, sample, 1);
	}

	if ((~data & 0x08) && (state->port_1_last & 0x08))
		sample_stop(state->samples, CHANNEL_WAVE);

	/* D4 - boss laser */
	if ((rising_bits & 0x10) && !state->astrof_bosskill_playing)
		sample_start(state->samples, CHANNEL_BOSSFIRE, SAMPLE_BOSSFIRE, 0);

	/* D5 - fire */
	if ((rising_bits & 0x20) && !state->astrof_bosskill_playing)
		sample_start(state->samples, CHANNEL_FIRE, SAMPLE_FIRE, 0);

	/* D6 - don't know. Probably something to do with the explosion sounds */

	/* D7 - sound enable bit */
	sound_global_enable(space->machine, data & 0x80);

	state->port_1_last = data;
}


WRITE8_HANDLER( astrof_audio_2_w )
{
	astrof_state *state = (astrof_state *)space->machine->driver_data;
	UINT8 rising_bits = data & ~state->port_2_last;

	/* D0-D2 - explosion select (triggered by D2 of the other port */
	if (state->astrof_start_explosion)
	{
		/* this is really a compound effect, made up of I believe 3 sound
           effects, but since our sample contains them all, disable playing
           the other effects while the explosion is playing */

		logerror("Explosion: %x\n", data);
		if (data & 0x04)
		{
			if (!state->astrof_bosskill_playing)
			{
				sample_start(state->samples, CHANNEL_EXPLOSION, SAMPLE_BOSSKILL, 0);
				state->astrof_bosskill_playing = 1;
			}
		}
		else if (data & 0x02)
			sample_start(state->samples, CHANNEL_EXPLOSION, SAMPLE_BOSSHIT, 0);
		else if (data & 0x01)
			sample_start(state->samples, CHANNEL_EXPLOSION, SAMPLE_EKILLED, 0);
		else
		{
			if (!state->astrof_death_playing)
			{
				sample_start(state->samples, CHANNEL_EXPLOSION, SAMPLE_DEATH, 0);
				state->astrof_death_playing = 1;
			}
		}

		state->astrof_start_explosion = 0;
	}

	/* D3 - low fuel warning */
	if (rising_bits & 0x08)
		sample_start(state->samples, CHANNEL_FUEL, SAMPLE_FUEL, 0);

	state->port_2_last = data;
}


static const char *const astrof_sample_names[] =
{
	"*astrof",
	"fire.wav",
	"ekilled.wav",
	"wave1.wav",
	"wave2.wav",
	"wave3.wav",
	"wave4.wav",
	"bossfire.wav",
	"fuel.wav",
	"death.wav",
	"bosshit.wav",
	"bosskill.wav",
	0
};


static const samples_interface astrof_samples_interface =
{
	4,	/* 4 channels */
	astrof_sample_names
};



MACHINE_DRIVER_START( astrof_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(astrof_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  Space Fighter
 *
 *************************************/

WRITE8_HANDLER( spfghmk2_audio_w )
{
	/* nothing yet */
}


MACHINE_DRIVER_START( spfghmk2_audio )
	/* nothing yet */
MACHINE_DRIVER_END



/*************************************
 *
 *  Tomahawk
 *
 *************************************/

WRITE8_HANDLER( tomahawk_audio_w )
{
	astrof_state *state = (astrof_state *)space->machine->driver_data;

	/* D0 - sonar */

	/* D1 - UFO explosion */

	/* D2 - morse */

	/* D3 - missile */

	/* D4 - UFO */

	/* D5 - UFO under water */
	sn76477_enable_w(state->sn, (~data >> 5) & 0x01);

	/* D6 - explosion */

	/* D7 - sound enable bit */
	sound_global_enable(space->machine, data & 0x80);
}


static const sn76477_interface tomahawk_sn76477_interface =
{
	0,				/*  4 noise_res (N/C)        */
	0,				/*  5 filter_res (N/C)       */
	0,				/*  6 filter_cap (N/C)       */
	0,				/*  7 decay_res (N/C)        */
	0,				/*  8 attack_decay_cap (N/C) */
	0,				/* 10 attack_res (N/C)       */
	RES_K(47),		/* 11 amplitude_res          */
	RES_K(47),		/* 12 feedback_res           */
	0,				/* 16 vco_voltage (N/C)      */
	CAP_U(0.033),	/* 17 vco_cap                */
	RES_K(33),		/* 18 vco_res                */
	5.0,			/* 19 pitch_voltage          */
	RES_K(47),		/* 20 slf_res                */
	CAP_U(2.2),		/* 21 slf_cap                */
	0,				/* 23 oneshot_cap (N/C)      */
	0,				/* 24 oneshot_res (N/C)      */
	1,				/* 22 vco                    */
	0,				/* 26 mixer A                */
	0,				/* 25 mixer B                */
	0,				/* 27 mixer C                */
	0,				/* 1  envelope 1             */
	0,				/* 28 envelope 2             */
	1				/* 9  enable (variable)      */
};


MACHINE_DRIVER_START( tomahawk_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("snsnd", SN76477, 0)
	MDRV_SOUND_CONFIG(tomahawk_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
