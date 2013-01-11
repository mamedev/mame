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



#define SPECTAR_MAXFREQ     525000
#define TARG_MAXFREQ        125000


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



static void adjust_sample(samples_device *samples, UINT8 freq)
{
	tone_freq = freq;

	if ((tone_freq == 0xff) || (tone_freq == 0x00))
		samples->set_volume(3, 0);
	else
	{
		samples->set_frequency(3, 1.0 * max_freq / (0xff - tone_freq));
		samples->set_volume(3, tone_active);
	}
}


WRITE8_HANDLER( targ_audio_1_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");

	/* CPU music */
	if ((data & 0x01) != (port_1_last & 0x01))
		space.machine().device<dac_device>("dac")->write_unsigned8((data & 0x01) * 0xff);

	/* shot */
	if (FALLING_EDGE(0x02) && !samples->playing(0))  samples->start(0,1);
	if (RISING_EDGE(0x02)) samples->stop(0);

	/* crash */
	if (RISING_EDGE(0x20))
	{
		if (data & 0x40)
			samples->start(1,2);
		else
			samples->start(1,0);
	}

	/* Sspec */
	if (data & 0x10)
		samples->stop(2);
	else
	{
		if ((data & 0x08) != (port_1_last & 0x08))
		{
			if (data & 0x08)
				samples->start(2,3,true);
			else
				samples->start(2,4,true);
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
		samples_device *samples = space.machine().device<samples_device>("samples");
		UINT8 *prom = space.machine().root_device().memregion("targ")->base();

		tone_pointer = (tone_pointer + 1) & 0x0f;

		adjust_sample(samples, prom[((data & 0x02) << 3) | tone_pointer]);
	}

	port_2_last = data;
}


WRITE8_HANDLER( spectar_audio_2_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	adjust_sample(samples, data);
}


static const char *const sample_names[] =
{
	"*targ",
	"expl",
	"shot",
	"sexpl",
	"spslow",
	"spfast",
	0
};


static void common_audio_start(running_machine &machine, int freq)
{
	samples_device *samples = machine.device<samples_device>("samples");
	max_freq = freq;

	tone_freq = 0;
	tone_active = 0;

	samples->set_volume(3, 0);
	samples->start_raw(3, sine_wave, 32, 1000, true);

	state_save_register_global(machine, port_1_last);
	state_save_register_global(machine, port_2_last);
	state_save_register_global(machine, tone_freq);
	state_save_register_global(machine, tone_active);
}


static SAMPLES_START( spectar_audio_start )
{
	common_audio_start(device.machine(), SPECTAR_MAXFREQ);
}


static SAMPLES_START( targ_audio_start )
{
	running_machine &machine = device.machine();
	common_audio_start(machine, TARG_MAXFREQ);

	tone_pointer = 0;

	state_save_register_global(machine, tone_pointer);
}


static const samples_interface spectar_samples_interface =
{
	4,  /* number of channel */
	sample_names,
	spectar_audio_start
};


static const samples_interface targ_samples_interface =
{
	4,  /* number of channel */
	sample_names,
	targ_audio_start
};


MACHINE_CONFIG_FRAGMENT( spectar_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", spectar_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( targ_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", targ_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
