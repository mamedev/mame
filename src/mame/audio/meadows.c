/***************************************************************************
    meadows.c
    Sound handler
    Dead Eye, Gypsy Juggler

    J. Buchmueller, June '98
****************************************************************************/

#include "driver.h"
#include "cpu/s2650/s2650.h"
#include "meadows.h"
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
static	int freq1 = 1000;
static	int freq2 = 1000;
static	INT16 waveform[2] = { -120*256, 120*256 };

/************************************/
/* Sound handler start              */
/************************************/
void meadows_sh_start(void)
{
	int vol[2];

	vol[0]=vol[1]=255;

	sample_set_volume(0,0);
	sample_start_raw(0,waveform,sizeof(waveform)/2,freq1,1);
	sample_set_volume(1,0);
	sample_start_raw(1,waveform,sizeof(waveform)/2,freq2,1);
}

/************************************/
/* Sound handler update             */
/************************************/
void meadows_sh_update(void)
{
static  UINT8 latched_0c01 = 0;
static	UINT8 latched_0c02 = 0;
static	UINT8 latched_0c03 = 0;
int preset, amp;

    if (latched_0c01 != meadows_0c01 || latched_0c03 != meadows_0c03)
	{
		/* amplitude is a combination of the upper 4 bits of 0c01 */
		/* and bit 4 merged from S2650's flag output */
		amp = ((meadows_0c03 & ENABLE_CTR1) == 0) ? 0 : (meadows_0c01 & 0xf0) >> 1;
		if( activecpu_get_reg(S2650_FO) )
			amp += 0x80;
		/* calculate frequency for counter #1 */
		/* bit 0..3 of 0c01 are ctr preset */
		preset = (meadows_0c01 & 15) ^ 15;
		if (preset)
			freq1 = BASE_CTR1 / (preset + 1);
		else amp = 0;
		logerror("meadows ctr1 channel #%d preset:%3d freq:%5d amp:%d\n", channel, preset, freq1, amp);
		sample_set_freq(0, freq1 * sizeof(waveform)/2);
		sample_set_volume(0,amp/255.0);
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
		sample_set_freq(1, freq2 * sizeof(waveform));
		sample_set_volume(1,amp/255.0);
    }

	if (latched_0c03 != meadows_0c03)
	{
		dac_enable = meadows_0c03 & ENABLE_DAC;

		if (dac_enable)
			DAC_data_w(0, meadows_dac);
		else
			DAC_data_w(0, 0);
    }

    latched_0c01 = meadows_0c01;
	latched_0c02 = meadows_0c02;
	latched_0c03 = meadows_0c03;
}

/************************************/
/* Write DAC value                  */
/************************************/
void meadows_sh_dac_w(int data)
{
	meadows_dac = data;
	if (dac_enable)
		DAC_data_w(0, meadows_dac);
	else
		DAC_data_w(0, 0);
}


