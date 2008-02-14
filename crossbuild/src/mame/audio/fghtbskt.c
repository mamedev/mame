/*

Fighting Basketball PCM unsigned 8 bit mono samples

*/

#include "driver.h"
#include "sound/samples.h"

static INT16 *samplebuf;

WRITE8_HANDLER( fghtbskt_samples_w )
{
	if( data & 1 )
		sample_start_raw(0, samplebuf + ((data & 0xf0) << 8), 0x2000, 8000, 0);
}

void fghtbskt_sh_start(void)
{
	int i;
	UINT8 *ROM = memory_region(REGION_SOUND1);

	samplebuf = auto_malloc(memory_region_length(REGION_SOUND1) * 2);

	for(i=0;i<memory_region_length(REGION_SOUND1);i++)
		samplebuf[i] = ((INT8)(ROM[i] ^ 0x80)) * 256;
}
