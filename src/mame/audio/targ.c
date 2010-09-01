/*************************************************************************

    Targ hardware

*************************************************************************/

/* Sound channel usage
   0 = CPU music,  Shoot
   1 = Crash
   2 = Spectar sound
   3 = Tone generator
*/

#include "emu.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "includes/targ.h"



#define SPECTAR_MAXFREQ		525000
#define TARG_MAXFREQ		125000


static int max_freq;

static UINT8 port_1_last;
static UINT8 port_2_last;

static UINT8 tone_freq;
static UINT8 tone_active;
static UINT8 tone_pointer;


static const INT16 sine_wave[32] =
{
	 0x0f0f,  0x0f0f,  0x0f0f,  0x0606,  0x0606,  0x0909,  0x0909,  0x0606,  0x0606,  0x0909,  0x0606,  0x0d0d,  0x0f0f,  0x0f0f,  0x0d0d,  0x0000,
	-0x191a, -0x2122, -0x1e1f, -0x191a, -0x1314, -0x191a, -0x1819, -0x1819, -0x1819, -0x1314, -0x1314, -0x1314, -0x1819, -0x1e1f, -0x1e1f, -0x1819
};


/* some macros to make detecting bit changes easier */
#define RISING_EDGE(bit)  ( (data & bit) && !(port_1_last & bit))
#define FALLING_EDGE(bit) (!(data & bit) &&  (port_1_last & bit))



static void adjust_sample(running_device *samples, UINT8 freq)
{
	tone_freq = freq;

	if ((tone_freq == 0xff) || (tone_freq == 0x00))
		sample_set_volume(samples, 3, 0);
	else
	{
		sample_set_freq(samples, 3, 1.0 * max_freq / (0xff - tone_freq));
		sample_set_volume(samples, 3, tone_active);
	}
}


WRITE8_HANDLER( targ_audio_1_w )
{
	running_device *samples = space->machine->device("samples");

	/* CPU music */
	if ((data & 0x01) != (port_1_last & 0x01))
		dac_data_w(space->machine->device("dac"),(data & 0x01) * 0xff);

	/* shot */
	if (FALLING_EDGE(0x02) && !sample_playing(samples, 0))  sample_start(samples, 0,1,0);
	if (RISING_EDGE(0x02)) sample_stop(samples, 0);

	/* crash */
	if (RISING_EDGE(0x20))
	{
		if (data & 0x40)
			sample_start(samples, 1,2,0);
		else
			sample_start(samples, 1,0,0);
	}

	/* Sspec */
	if (data & 0x10)
		sample_stop(samples, 2);
	else
	{
		if ((data & 0x08) != (port_1_last & 0x08))
		{
			if (data & 0x08)
				sample_start(samples, 2,3,1);
			else
				sample_start(samples, 2,4,1);
		}
	}

	/* Game (tone generator enable) */
	if (FALLING_EDGE(0x80))
	{
		tone_pointer = 0;
		tone_active = 0;

		adjust_sample(samples, tone_freq);
	}

	if (RISING_EDGE(0x80))
		tone_active=1;

	port_1_last = data;
}


WRITE8_HANDLER( targ_audio_2_w )
{
	if ((data & 0x01) && !(port_2_last & 0x01))
	{
		running_device *samples = space->machine->device("samples");
		UINT8 *prom = memory_region(space->machine, "targ");

		tone_pointer = (tone_pointer + 1) & 0x0f;

		adjust_sample(samples, prom[((data & 0x02) << 3) | tone_pointer]);
	}

	port_2_last = data;
}


WRITE8_HANDLER( spectar_audio_2_w )
{
	running_device *samples = space->machine->device("samples");
	adjust_sample(samples, data);
}


static const char *const sample_names[] =
{
	"*targ",
	"expl.wav",
	"shot.wav",
	"sexpl.wav",
	"spslow.wav",
	"spfast.wav",
	0
};


static void common_audio_start(running_machine *machine, int freq)
{
	running_device *samples = machine->device("samples");
	max_freq = freq;

	tone_freq = 0;
	tone_active = 0;

	sample_set_volume(samples, 3, 0);
	sample_start_raw(samples, 3, sine_wave, 32, 1000, 1);

	state_save_register_global(machine, port_1_last);
	state_save_register_global(machine, port_2_last);
	state_save_register_global(machine, tone_freq);
	state_save_register_global(machine, tone_active);
}


static SAMPLES_START( spectar_audio_start )
{
	running_machine *machine = device->machine;
	common_audio_start(machine, SPECTAR_MAXFREQ);
}


static SAMPLES_START( targ_audio_start )
{
	running_machine *machine = device->machine;
	common_audio_start(machine, TARG_MAXFREQ);

	tone_pointer = 0;

	state_save_register_global(machine, tone_pointer);
}


static const samples_interface spectar_samples_interface =
{
	4,	/* number of channel */
	sample_names,
	spectar_audio_start
};


static const samples_interface targ_samples_interface =
{
	4,	/* number of channel */
	sample_names,
	targ_audio_start
};


MACHINE_CONFIG_FRAGMENT( spectar_audio )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(spectar_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( targ_audio )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(targ_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
