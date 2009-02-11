/*

    SunA 8 Bit Games samples

    Format: PCM unsigned 8 bit mono 4Khz

*/

#include "driver.h"
#include "sound/samples.h"
#include "includes/suna8.h"

static INT16 *samplebuf;
static int sample;

WRITE8_DEVICE_HANDLER( suna8_play_samples_w )
{
	if( data )
	{
		if( ~data & 0x10 )
		{
			sample_start_raw(device, 0, &samplebuf[0x800*sample], 0x0800, 4000, 0);
		}
		else if( ~data & 0x08 )
		{
			sample &= 3;
			sample_start_raw(device, 0, &samplebuf[0x800*(sample+7)], 0x0800, 4000, 0);
		}
	}
}

WRITE8_DEVICE_HANDLER( suna8_samples_number_w )
{
	sample = data & 0xf;
}

SAMPLES_START( suna8_sh_start )
{
	running_machine *machine = device->machine;
	int i, len = memory_region_length(machine, "samples");
	UINT8 *ROM = memory_region(machine, "samples");

	samplebuf = auto_malloc(len * sizeof(samplebuf[0]));

	for(i=0;i<len;i++)
		samplebuf[i] = (INT8)(ROM[i] ^ 0x80) * 256;
}
