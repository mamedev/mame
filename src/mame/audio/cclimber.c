#include "emu.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "audio/cclimber.h"


/* macro to convert 4-bit unsigned samples to 16-bit signed samples */
#define SAMPLE_CONV4(a) (0x1111*((a&0x0f))-0x8000)

#define SND_CLOCK 3072000   /* 3.072 MHz */


static INT16 *samplebuf;    /* buffer to decode samples at run time */


static SAMPLES_START( cclimber_sh_start )
{
	running_machine &machine = device.machine();
	samplebuf = 0;
	if (machine.root_device().memregion("samples")->base())
		samplebuf = auto_alloc_array(machine, INT16, 2 * machine.root_device().memregion("samples")->bytes());
}


static void cclimber_play_sample(running_machine &machine, int start,int freq,int volume)
{
	int len;
	int romlen = machine.root_device().memregion("samples")->bytes();
	const UINT8 *rom = machine.root_device().memregion("samples")->base();
	samples_device *samples = machine.device<samples_device>("samples");


	if (!rom) return;

	/* decode the rom samples */
	len = 0;
	while (start + len < romlen && rom[start+len] != 0x70)
	{
		int sample;

		sample = (rom[start + len] & 0xf0) >> 4;
		samplebuf[2*len] = SAMPLE_CONV4(sample) * volume / 31;

		sample = rom[start + len] & 0x0f;
		samplebuf[2*len + 1] = SAMPLE_CONV4(sample) * volume / 31;

		len++;
	}

	samples->start_raw(0,samplebuf,2 * len,freq);
}


static int sample_num,sample_freq,sample_volume;

static WRITE8_DEVICE_HANDLER( cclimber_sample_select_w )
{
	sample_num = data;
}

WRITE8_HANDLER( cclimber_sample_rate_w )
{
	/* calculate the sampling frequency */
	sample_freq = SND_CLOCK / 4 / (256 - data);
}

WRITE8_HANDLER( cclimber_sample_volume_w )
{
	sample_volume = data & 0x1f;    /* range 0-31 */
}

WRITE8_HANDLER( cclimber_sample_trigger_w )
{
	if (data == 0)
		return;

	cclimber_play_sample(space.machine(), 32 * sample_num,sample_freq,sample_volume);
}


const ay8910_interface cclimber_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(cclimber_sample_select_w),
	DEVCB_NULL
};

const samples_interface cclimber_samples_interface =
{
	1,
	NULL,
	cclimber_sh_start
};
