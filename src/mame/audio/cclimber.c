#include "driver.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "includes/cclimber.h"


/* macro to convert 4-bit unsigned samples to 16-bit signed samples */
#define SAMPLE_CONV4(a) (0x1111*((a&0x0f))-0x8000)

#define SND_CLOCK 3072000	/* 3.072 MHz */


static INT16 *samplebuf;	/* buffer to decode samples at run time */


static void cclimber_sh_start(void)
{
	samplebuf = 0;
	if (memory_region(REGION_SOUND1))
		samplebuf = auto_malloc(sizeof(*samplebuf)*2*memory_region_length(REGION_SOUND1));
}


static void cclimber_play_sample(int start,int freq,int volume)
{
	int len;
	int romlen = memory_region_length(REGION_SOUND1);
	const UINT8 *rom = memory_region(REGION_SOUND1);


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

	sample_start_raw(0,samplebuf,2 * len,freq,0);
}


static int sample_num,sample_freq,sample_volume;

static WRITE8_HANDLER( cclimber_sample_select_w )
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
	sample_volume = data & 0x1f;	/* range 0-31 */
}

WRITE8_HANDLER( cclimber_sample_trigger_w )
{
	if (data == 0)
		return;

	cclimber_play_sample(32 * sample_num,sample_freq,sample_volume);
}


const struct AY8910interface cclimber_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	NULL,
	NULL,
	cclimber_sample_select_w,
	NULL
};

const struct Samplesinterface cclimber_samples_interface =
{
	1,
	NULL,
	cclimber_sh_start
};
