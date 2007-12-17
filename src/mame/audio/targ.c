/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

/* Sound channel usage
   0 = CPU music,  Shoot
   1 = Crash
   2 = Spectar sound
   3 = Tone generator
*/

#include "driver.h"
#include "exidy.h"
#include "sound/samples.h"
#include "sound/dac.h"

UINT8 targ_spec_flag;
static UINT8 targ_sh_ctrl0=0;
static UINT8 targ_sh_ctrl1=0;
static UINT8 tone_active;

#define MAXFREQ_A_TARG 125000
#define MAXFREQ_A_SPECTAR 525000

static int sound_a_freq;
static UINT8 tone_pointer;
static UINT8 tone_offset;

static UINT8 tone_prom[32] =
{
    0xE5,0xE5,0xED,0xED,0xE5,0xE5,0xED,0xED,0xE7,0xE7,0xEF,0xEF,0xE7,0xE7,0xEF,0xEF,
    0xC1,0xE1,0xC9,0xE9,0xC5,0xE5,0xCD,0xED,0xC3,0xE3,0xCB,0xEB,0xC7,0xE7,0xCF,0xEF
};

/* waveforms for the audio hardware */
static INT16 waveform1[32] =
{
	/* sine-wave */
	0x0F0F, 0x0F0F, 0x0F0F, 0x0606, 0x0606, 0x0909, 0x0909, 0x0606, 0x0606, 0x0909, 0x0606, 0x0D0D, 0x0F0F, 0x0F0F, 0x0D0D, 0x0000,
	-0x191A, -0x2122, -0x1E1F, -0x191A, -0x1314, -0x191A, -0x1819, -0x1819, -0x1819, -0x1314, -0x1314, -0x1314, -0x1819, -0x1E1F, -0x1E1F, -0x1819
};

/* some macros to make detecting bit changes easier */
#define RISING_EDGE(bit)  ((data & bit) && !(targ_sh_ctrl0 & bit))
#define FALLING_EDGE(bit) (!(data & bit) && (targ_sh_ctrl0 & bit))


static void targ_tone_generator(int data)
{
	int maxfreq;


	if (targ_spec_flag) maxfreq = MAXFREQ_A_TARG;
	else maxfreq = MAXFREQ_A_SPECTAR;

    sound_a_freq = data;
    if (sound_a_freq == 0xFF || sound_a_freq == 0x00)
	{
		sample_set_volume(3,0);
    }
    else
	{
		sample_set_freq(3,maxfreq/(0xFF-sound_a_freq));
		sample_set_volume(3,tone_active*1.0);
	}
}

void targ_sh_start(void)
{
	tone_pointer=0;
	tone_offset=0;
	tone_active=0;
	sound_a_freq = 0x00;
	sample_set_volume(3,0);
	sample_start_raw(3,waveform1,32,1000,1);
}

WRITE8_HANDLER( targ_sh_w )
{
	int maxfreq;


	if (targ_spec_flag) maxfreq = MAXFREQ_A_TARG;
	else maxfreq = MAXFREQ_A_SPECTAR;

    if (offset) {
        if (targ_spec_flag) {
            if (data & 0x02)
                tone_offset=16;
            else
                tone_offset=0;

            if ((data & 0x01) && !(targ_sh_ctrl1 & 0x01)) {
                tone_pointer++;
                if (tone_pointer > 15) tone_pointer = 0;
                targ_tone_generator(tone_prom[tone_pointer+tone_offset]);
            }
       }
       else {
            targ_tone_generator(data);
       }
       targ_sh_ctrl1=data;
    }
    else
    {
        /* cpu music */
        if ((data & 0x01) != (targ_sh_ctrl0 & 0x01)) {
            DAC_data_w(0,(data & 0x01) * 0xFF);
        }
        /* Shoot */
        if FALLING_EDGE(0x02) {
            if (!sample_playing(0))  sample_start(0,1,0);
        }
        if RISING_EDGE(0x02) {
            sample_stop(0);
        }

        /* Crash */
        if RISING_EDGE(0x20) {
            if (data & 0x40) {
                sample_start(1,2,0); }
            else {
                sample_start(1,0,0); }
        }

        /* Sspec */
        if (data & 0x10) {
            sample_stop(2);
        }
        else {
            if ((data & 0x08) != (targ_sh_ctrl0 & 0x08)) {
            if (data & 0x08) {
                sample_start(2,3,1); }
            else {
                sample_start(2,4,1); }
            }
        }

        /* Game (tone generator enable) */
        if FALLING_EDGE(0x80) {
           tone_pointer=0;
           tone_active=0;
           if (sound_a_freq == 0xFF || sound_a_freq == 0x00)
		   {
				sample_set_volume(3,0);
           }
           else
		   {
             sample_set_freq(3,maxfreq/(0xFF-sound_a_freq));
             sample_set_volume(3,0);
		   }
        }
        if RISING_EDGE(0x80) {
            tone_active=1;
        }
        targ_sh_ctrl0 = data;
    }
}

