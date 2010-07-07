/***************************************************************************
    meadows.c
    Sound handler
    Dead Eye, Gypsy Juggler

    J. Buchmueller, June '98
****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "includes/meadows.h"
#include "sound/samples.h"
#include "sound/dac.h"


UINT8 meadows_0c00 = 0;
UINT8 meadows_0c01 = 0;
UINT8 meadows_0c02 = 0;
UINT8 meadows_0c03 = 0;
static UINT8 meadows_dac  = 0;
static int dac_enable;

#define BASE_CLOCK		5000000
#define BASE_CTR1       (BASE_CLOCK / 256)
#define BASE_CTR2		(BASE_CLOCK / 32)

#define DIV2OR4_CTR2	0x01
#define ENABLE_CTR2     0x02
#define ENABLE_DAC      0x04
#define ENABLE_CTR1     0x08

static	int channel;
static	int freq1;
static	int freq2;
static const INT16 waveform[2] = { -120*256, 120*256 };
static  UINT8 latched_0c01 = 0;
static	UINT8 latched_0c02 = 0;
static	UINT8 latched_0c03 = 0;

/************************************/
/* Sound handler start              */
/************************************/
SAMPLES_START( meadows_sh_start )
{
	meadows_0c00 = meadows_0c01 = meadows_0c02 = meadows_0c03 = 0;
	meadows_dac = 0;
	dac_enable = 0;
	channel = 0;
	freq1 = freq2 = 1000;
	latched_0c01 = latched_0c02 = latched_0c03 = 0;

	sample_set_volume(device,0,0);
	sample_start_raw(device,0,waveform,ARRAY_LENGTH(waveform),freq1,1);
	sample_set_volume(device,1,0);
	sample_start_raw(device,1,waveform,ARRAY_LENGTH(waveform),freq2,1);
}

/************************************/
/* Sound handler update             */
/************************************/
void meadows_sh_update(running_machine *machine)
{
	running_device *samples = machine->device("samples");
	int preset, amp;

	if (latched_0c01 != meadows_0c01 || latched_0c03 != meadows_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01 */
		/* and bit 4 merged from S2650's flag output */
		amp = ((meadows_0c03 & ENABLE_CTR1) == 0) ? 0 : (meadows_0c01 & 0xf0) >> 1;
		if( cpu_get_reg(machine->device("maincpu"), S2650_FO) )
			amp += 0x80;
		/* calculate frequency for counter #1 */
		/* bit 0..3 of 0c01 are ctr preset */
		preset = (meadows_0c01 & 15) ^ 15;
		if (preset)
			freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		logerror("meadows ctr1 channel #%d preset:%3d freq:%5d amp:%d\n", channel, preset, freq1, amp);
		sample_set_freq(samples, 0, freq1 * sizeof(waveform)/2);
		sample_set_volume(samples, 0,amp/255.0);
	}

	if (latched_0c02 != meadows_0c02 || latched_0c03 != meadows_0c03)
	{
		/* calculate frequency for counter #2 */
		/* 0c02 is ctr preset, 0c03 bit 0 enables division by 2 */
		amp = ((meadows_0c03 & ENABLE_CTR2) != 0) ? 0xa0 : 0;
		preset = meadows_0c02 ^ 0xff;
		if (preset)
		{
			freq2 = BASE_CTR2 / (preset + 1) / 2;
			if ((meadows_0c03 & DIV2OR4_CTR2) == 0)
				freq2 >>= 1;
		}
		else amp = 0;
		logerror("meadows ctr2 channel #%d preset:%3d freq:%5d amp:%d\n", channel+1, preset, freq2, amp);
		sample_set_freq(samples, 1, freq2 * sizeof(waveform));
		sample_set_volume(samples, 1,amp/255.0);
	}

	if (latched_0c03 != meadows_0c03)
	{
		dac_enable = meadows_0c03 & ENABLE_DAC;

		if (dac_enable)
			dac_data_w(machine->device("dac"), meadows_dac);
		else
			dac_data_w(machine->device("dac"), 0);
	}

	latched_0c01 = meadows_0c01;
	latched_0c02 = meadows_0c02;
	latched_0c03 = meadows_0c03;
}

/************************************/
/* Write DAC value                  */
/************************************/
void meadows_sh_dac_w(running_machine *machine, int data)
{
	meadows_dac = data;
	if (dac_enable)
		dac_data_w(machine->device("dac"), meadows_dac);
	else
		dac_data_w(machine->device("dac"), 0);
}


