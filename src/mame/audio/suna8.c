/*

    SunA 8 Bit Games samples

    Format: PCM unsigned 8 bit mono 4Khz

*/

#include "driver.h"
#include "sound/samples.h"

static INT16 *samplebuf;
static int sample;

WRITE8_HANDLER( suna8_play_samples_w )
{
	if( data )
	{
		if( ~data & 0x10 )
		{
			sample_start_raw(0, &samplebuf[0x800*sample], 0x0800, 4000, 0);
		}
		else if( ~data & 0x08 )
		{
			sample &= 3;
			sample_start_raw(0, &samplebuf[0x800*(sample+7)], 0x0800, 4000, 0);
		}
	}
}

WRITE8_HANDLER( suna8_samples_number_w )
{
	sample = data & 0xf;
}

void suna8_sh_start(void)
{
	int i;
	UINT8 *ROM = memory_region(REGION_SOUND1);

	samplebuf = auto_malloc(memory_region_length(REGION_SOUND1) * sizeof(samplebuf[0]));

	for(i=0;i<memory_region_length(REGION_SOUND1);i++)
		samplebuf[i] = (INT8)(ROM[i] ^ 0x80) * 256;
}
