/**************************************************************************************
* Game Boy sound emulation (c) Anthony Kruize (trandor@labyrinth.net.au)
*
* Anyways, sound on the Game Boy consists of 4 separate 'channels'
*   Sound1 = Quadrangular waves with SWEEP and ENVELOPE functions  (NR10,11,12,13,14)
*   Sound2 = Quadrangular waves with ENVELOPE functions (NR21,22,23,24)
*   Sound3 = Wave patterns from WaveRAM (NR30,31,32,33,34)
*   Sound4 = White noise with an envelope (NR41,42,43,44)
*
* Each sound channel has 2 modes, namely ON and OFF...  whoa
*
* These tend to be the two most important equations in
* converting between Hertz and GB frequency registers:
* (Sounds will have a 2.4% higher frequency on Super GB.)
*       gb = 2048 - (131072 / Hz)
*       Hz = 131072 / (2048 - gb)
*
* Changes:
*
*   10/2/2002       AK - Preliminary sound code.
*   13/2/2002       AK - Added a hack for mode 4, other fixes.
*   23/2/2002       AK - Use lookup tables, added sweep to mode 1. Re-wrote the square
*                        wave generation.
*   13/3/2002       AK - Added mode 3, better lookup tables, other adjustments.
*   15/3/2002       AK - Mode 4 can now change frequencies.
*   31/3/2002       AK - Accidently forgot to handle counter/consecutive for mode 1.
*    3/4/2002       AK - Mode 1 sweep can still occur if shift is 0.  Don't let frequency
*                        go past the maximum allowed value. Fixed Mode 3 length table.
*                        Slight adjustment to Mode 4's period table generation.
*    5/4/2002       AK - Mode 4 is done correctly, using a polynomial counter instead
*                        of being a total hack.
*    6/4/2002       AK - Slight tweak to mode 3's frequency calculation.
*   13/4/2002       AK - Reset envelope value when sound is initialized.
*   21/4/2002       AK - Backed out the mode 3 frequency calculation change.
*                        Merged init functions into gameboy_sound_w().
*   14/5/2002       AK - Removed magic numbers in the fixed point math.
*   12/6/2002       AK - Merged SOUNDx structs into one SOUND struct.
*  26/10/2002       AK - Finally fixed channel 3!
*
***************************************************************************************/

#include "emu.h"
#include "gb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NR10 0x00
#define NR11 0x01
#define NR12 0x02
#define NR13 0x03
#define NR14 0x04
#define NR21 0x06
#define NR22 0x07
#define NR23 0x08
#define NR24 0x09
#define NR30 0x0A
#define NR31 0x0B
#define NR32 0x0C
#define NR33 0x0D
#define NR34 0x0E
#define NR41 0x10
#define NR42 0x11
#define NR43 0x12
#define NR44 0x13
#define NR50 0x14
#define NR51 0x15
#define NR52 0x16
#define AUD3W0 0x20
#define AUD3W1 0x21
#define AUD3W2 0x22
#define AUD3W3 0x23
#define AUD3W4 0x24
#define AUD3W5 0x25
#define AUD3W6 0x26
#define AUD3W7 0x27
#define AUD3W8 0x28
#define AUD3W9 0x29
#define AUD3WA 0x2A
#define AUD3WB 0x2B
#define AUD3WC 0x2C
#define AUD3WD 0x2D
#define AUD3WE 0x2E
#define AUD3WF 0x2F

#define LEFT 1
#define RIGHT 2
#define MAX_FREQUENCIES 2048
#define FIXED_POINT 16

/* Represents wave duties of 12.5%, 25%, 50% and 75% */
static const float wave_duty_table[4] = { 8.0f, 4.0f, 2.0f, 1.33f };


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct SOUND
{
	/* Common */
	UINT8  on;
	UINT8  channel;
	INT32  length;
	INT32  pos;
	UINT32 period;
	INT32  count;
	INT8   mode;
	/* Mode 1, 2, 3 */
	INT8   duty;
	/* Mode 1, 2, 4 */
	INT32  env_value;
	INT8   env_direction;
	INT32  env_length;
	INT32  env_count;
	INT8   signal;
	/* Mode 1 */
	UINT32 frequency;
	INT32  swp_shift;
	INT32  swp_direction;
	INT32  swp_time;
	INT32  swp_count;
	/* Mode 3 */
	INT8   level;
	UINT8  offset;
	UINT32 dutycount;
	/* Mode 4 */
	INT32  ply_step;
	INT16  ply_value;
};

struct SOUNDC
{
	UINT8 on;
	UINT8 vol_left;
	UINT8 vol_right;
	UINT8 mode1_left;
	UINT8 mode1_right;
	UINT8 mode2_left;
	UINT8 mode2_right;
	UINT8 mode3_left;
	UINT8 mode3_right;
	UINT8 mode4_left;
	UINT8 mode4_right;
};


typedef struct _gb_sound_t gb_sound_t;
struct _gb_sound_t
{
	sound_stream *channel;
	int rate;

	INT32 env_length_table[8];
	INT32 swp_time_table[8];
	UINT32 period_table[MAX_FREQUENCIES];
	UINT32 period_mode3_table[MAX_FREQUENCIES];
	UINT32 period_mode4_table[8][16];
	UINT32 length_table[64];
	UINT32 length_mode3_table[256];

	struct SOUND  snd_1;
	struct SOUND  snd_2;
	struct SOUND  snd_3;
	struct SOUND  snd_4;
	struct SOUNDC snd_control;

	UINT8 snd_regs[0x30];
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE gb_sound_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GAMEBOY);
	return (gb_sound_t *) downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    PROTOTYPES
***************************************************************************/

static STREAM_UPDATE( gameboy_update );


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

READ8_DEVICE_HANDLER( gb_wave_r )
{
	gb_sound_t *gb = get_token(device);

	/* TODO: properly emulate scrambling of wave ram area when playback is active */
	return ( gb->snd_regs[ AUD3W0 + offset ] | gb->snd_3.on );
}

WRITE8_DEVICE_HANDLER( gb_wave_w )
{
	gb_sound_t *gb = get_token(device);
	gb->snd_regs[ AUD3W0 + offset ] = data;
}

READ8_DEVICE_HANDLER( gb_sound_r )
{
	gb_sound_t *gb = get_token(device);

	switch( offset ) {
	case 0x05:
	case 0x0F:
		return 0xFF;
	case NR52:
		return 0x70 | gb->snd_regs[offset];
	default:
		return gb->snd_regs[offset];
	}
}

static void gb_sound_w_internal(device_t *device, int offset, UINT8 data )
{
	gb_sound_t *gb = get_token(device);

	/* Store the value */
	gb->snd_regs[offset] = data;

	switch( offset )
	{
	/*MODE 1 */
	case NR10: /* Sweep (R/W) */
		gb->snd_1.swp_shift = data & 0x7;
		gb->snd_1.swp_direction = (data & 0x8) >> 3;
		gb->snd_1.swp_direction |= gb->snd_1.swp_direction - 1;
		gb->snd_1.swp_time = gb->swp_time_table[ (data & 0x70) >> 4 ];
		break;
	case NR11: /* Sound length/Wave pattern duty (R/W) */
		gb->snd_1.duty = (data & 0xC0) >> 6;
		gb->snd_1.length = gb->length_table[data & 0x3F];
		break;
	case NR12: /* Envelope (R/W) */
		gb->snd_1.env_value = data >> 4;
		gb->snd_1.env_direction = (data & 0x8) >> 3;
		gb->snd_1.env_direction |= gb->snd_1.env_direction - 1;
		gb->snd_1.env_length = gb->env_length_table[data & 0x7];
		break;
	case NR13: /* Frequency lo (R/W) */
		gb->snd_1.frequency = ((gb->snd_regs[NR14]&0x7)<<8) | gb->snd_regs[NR13];
		gb->snd_1.period = gb->period_table[gb->snd_1.frequency];
		break;
	case NR14: /* Frequency hi / Initialize (R/W) */
		gb->snd_1.mode = (data & 0x40) >> 6;
		gb->snd_1.frequency = ((gb->snd_regs[NR14]&0x7)<<8) | gb->snd_regs[NR13];
		gb->snd_1.period = gb->period_table[gb->snd_1.frequency];
		if( data & 0x80 )
		{
			if( !gb->snd_1.on )
				gb->snd_1.pos = 0;
			gb->snd_1.on = 1;
			gb->snd_1.count = 0;
			gb->snd_1.env_value = gb->snd_regs[NR12] >> 4;
			gb->snd_1.env_count = 0;
			gb->snd_1.swp_count = 0;
			gb->snd_1.signal = 0x1;
			gb->snd_regs[NR52] |= 0x1;
		}
		break;

	/*MODE 2 */
	case NR21: /* Sound length/Wave pattern duty (R/W) */
		gb->snd_2.duty = (data & 0xC0) >> 6;
		gb->snd_2.length = gb->length_table[data & 0x3F];
		break;
	case NR22: /* Envelope (R/W) */
		gb->snd_2.env_value = data >> 4;
		gb->snd_2.env_direction = (data & 0x8 ) >> 3;
		gb->snd_2.env_direction |= gb->snd_2.env_direction - 1;
		gb->snd_2.env_length = gb->env_length_table[data & 0x7];
		break;
	case NR23: /* Frequency lo (R/W) */
		gb->snd_2.period = gb->period_table[((gb->snd_regs[NR24]&0x7)<<8) | gb->snd_regs[NR23]];
		break;
	case NR24: /* Frequency hi / Initialize (R/W) */
		gb->snd_2.mode = (data & 0x40) >> 6;
		gb->snd_2.period = gb->period_table[((gb->snd_regs[NR24]&0x7)<<8) | gb->snd_regs[NR23]];
		if( data & 0x80 )
		{
			if( !gb->snd_2.on )
				gb->snd_2.pos = 0;
			gb->snd_2.on = 1;
			gb->snd_2.count = 0;
			gb->snd_2.env_value = gb->snd_regs[NR22] >> 4;
			gb->snd_2.env_count = 0;
			gb->snd_2.signal = 0x1;
			gb->snd_regs[NR52] |= 0x2;
		}
		break;

	/*MODE 3 */
	case NR30: /* Sound On/Off (R/W) */
		gb->snd_3.on = (data & 0x80) >> 7;
		break;
	case NR31: /* Sound Length (R/W) */
		gb->snd_3.length = gb->length_mode3_table[data];
		break;
	case NR32: /* Select Output Level */
		gb->snd_3.level = (data & 0x60) >> 5;
		break;
	case NR33: /* Frequency lo (W) */
		gb->snd_3.period = gb->period_mode3_table[((gb->snd_regs[NR34]&0x7)<<8) + gb->snd_regs[NR33]];
		break;
	case NR34: /* Frequency hi / Initialize (W) */
		gb->snd_3.mode = (data & 0x40) >> 6;
		gb->snd_3.period = gb->period_mode3_table[((gb->snd_regs[NR34]&0x7)<<8) + gb->snd_regs[NR33]];
		if( data & 0x80 )
		{
			if( !gb->snd_3.on )
			{
				gb->snd_3.pos = 0;
				gb->snd_3.offset = 0;
				gb->snd_3.duty = 0;
			}
			gb->snd_3.on = 1;
			gb->snd_3.count = 0;
			gb->snd_3.duty = 1;
			gb->snd_3.dutycount = 0;
			gb->snd_regs[NR52] |= 0x4;
		}
		break;

	/*MODE 4 */
	case NR41: /* Sound Length (R/W) */
		gb->snd_4.length = gb->length_table[data & 0x3F];
		break;
	case NR42: /* Envelope (R/W) */
		gb->snd_4.env_value = data >> 4;
		gb->snd_4.env_direction = (data & 0x8 ) >> 3;
		gb->snd_4.env_direction |= gb->snd_4.env_direction - 1;
		gb->snd_4.env_length = gb->env_length_table[data & 0x7];
		break;
	case NR43: /* Polynomial Counter/Frequency */
		gb->snd_4.period = gb->period_mode4_table[data & 0x7][(data & 0xF0) >> 4];
		gb->snd_4.ply_step = (data & 0x8) >> 3;
		break;
	case NR44: /* Counter/Consecutive / Initialize (R/W)  */
		gb->snd_4.mode = (data & 0x40) >> 6;
		if( data & 0x80 )
		{
			if( !gb->snd_4.on )
				gb->snd_4.pos = 0;
			gb->snd_4.on = 1;
			gb->snd_4.count = 0;
			gb->snd_4.env_value = gb->snd_regs[NR42] >> 4;
			gb->snd_4.env_count = 0;
			gb->snd_4.signal = device->machine().rand();
			gb->snd_4.ply_value = 0x7fff;
			gb->snd_regs[NR52] |= 0x8;
		}
		break;

	/* CONTROL */
	case NR50: /* Channel Control / On/Off / Volume (R/W)  */
		gb->snd_control.vol_left = data & 0x7;
		gb->snd_control.vol_right = (data & 0x70) >> 4;
		break;
	case NR51: /* Selection of Sound Output Terminal */
		gb->snd_control.mode1_right = data & 0x1;
		gb->snd_control.mode1_left = (data & 0x10) >> 4;
		gb->snd_control.mode2_right = (data & 0x2) >> 1;
		gb->snd_control.mode2_left = (data & 0x20) >> 5;
		gb->snd_control.mode3_right = (data & 0x4) >> 2;
		gb->snd_control.mode3_left = (data & 0x40) >> 6;
		gb->snd_control.mode4_right = (data & 0x8) >> 3;
		gb->snd_control.mode4_left = (data & 0x80) >> 7;
		break;
	case NR52: /* Sound On/Off (R/W) */
		/* Only bit 7 is writable, writing to bits 0-3 does NOT enable or
           disable sound.  They are read-only */
		gb->snd_control.on = (data & 0x80) >> 7;
		if( !gb->snd_control.on )
		{
			gb_sound_w_internal( device, NR10, 0x80 );
			gb_sound_w_internal( device, NR11, 0x3F );
			gb_sound_w_internal( device, NR12, 0x00 );
			gb_sound_w_internal( device, NR13, 0xFE );
			gb_sound_w_internal( device, NR14, 0xBF );
//          gb_sound_w_internal( device, NR20, 0xFF );
			gb_sound_w_internal( device, NR21, 0x3F );
			gb_sound_w_internal( device, NR22, 0x00 );
			gb_sound_w_internal( device, NR23, 0xFF );
			gb_sound_w_internal( device, NR24, 0xBF );
			gb_sound_w_internal( device, NR30, 0x7F );
			gb_sound_w_internal( device, NR31, 0xFF );
			gb_sound_w_internal( device, NR32, 0x9F );
			gb_sound_w_internal( device, NR33, 0xFF );
			gb_sound_w_internal( device, NR34, 0xBF );
//          gb_sound_w_internal( device, NR40, 0xFF );
			gb_sound_w_internal( device, NR41, 0xFF );
			gb_sound_w_internal( device, NR42, 0x00 );
			gb_sound_w_internal( device, NR43, 0x00 );
			gb_sound_w_internal( device, NR44, 0xBF );
			gb_sound_w_internal( device, NR50, 0x00 );
			gb_sound_w_internal( device, NR51, 0x00 );
			gb->snd_1.on = 0;
			gb->snd_2.on = 0;
			gb->snd_3.on = 0;
			gb->snd_4.on = 0;
			gb->snd_regs[offset] = 0;
		}
		break;
	}
}

WRITE8_DEVICE_HANDLER( gb_sound_w )
{
	gb_sound_t *gb = get_token(device);

	/* change in registers so update first */
	gb->channel->update();

	/* Only register NR52 is accessible if the sound controller is disabled */
	if( !gb->snd_control.on && offset != NR52 )
	{
		return;
	}

	gb_sound_w_internal( device, offset, data );
}



static STREAM_UPDATE( gameboy_update )
{
	gb_sound_t *gb = get_token(device);
	stream_sample_t sample, left, right, mode4_mask;

	while( samples-- > 0 )
	{
		left = right = 0;

		/* Mode 1 - Wave with Envelope and Sweep */
		if( gb->snd_1.on )
		{
			sample = gb->snd_1.signal * gb->snd_1.env_value;
			gb->snd_1.pos++;
			if( gb->snd_1.pos == (UINT32)(gb->snd_1.period / wave_duty_table[gb->snd_1.duty]) >> FIXED_POINT)
			{
				gb->snd_1.signal = -gb->snd_1.signal;
			}
			else if( gb->snd_1.pos > (gb->snd_1.period >> FIXED_POINT) )
			{
				gb->snd_1.pos = 0;
				gb->snd_1.signal = -gb->snd_1.signal;
			}

			if( gb->snd_1.length && gb->snd_1.mode )
			{
				gb->snd_1.count++;
				if( gb->snd_1.count >= gb->snd_1.length )
				{
					gb->snd_1.on = 0;
					gb->snd_regs[NR52] &= 0xFE;
				}
			}

			if( gb->snd_1.env_length )
			{
				gb->snd_1.env_count++;
				if( gb->snd_1.env_count >= gb->snd_1.env_length )
				{
					gb->snd_1.env_count = 0;
					gb->snd_1.env_value += gb->snd_1.env_direction;
					if( gb->snd_1.env_value < 0 )
						gb->snd_1.env_value = 0;
					if( gb->snd_1.env_value > 15 )
						gb->snd_1.env_value = 15;
				}
			}

			if( gb->snd_1.swp_time )
			{
				gb->snd_1.swp_count++;
				if( gb->snd_1.swp_count >= gb->snd_1.swp_time )
				{
					gb->snd_1.swp_count = 0;
					if( gb->snd_1.swp_direction > 0 )
					{
						gb->snd_1.frequency -= gb->snd_1.frequency / (1 << gb->snd_1.swp_shift );
						if( gb->snd_1.frequency <= 0 )
						{
							gb->snd_1.on = 0;
							gb->snd_regs[NR52] &= 0xFE;
						}
					}
					else
					{
						gb->snd_1.frequency += gb->snd_1.frequency / (1 << gb->snd_1.swp_shift );
						if( gb->snd_1.frequency >= MAX_FREQUENCIES )
						{
							gb->snd_1.frequency = MAX_FREQUENCIES - 1;
						}
					}

					gb->snd_1.period = gb->period_table[gb->snd_1.frequency];
				}
			}

			if( gb->snd_control.mode1_left )
				left += sample;
			if( gb->snd_control.mode1_right )
				right += sample;
		}

		/* Mode 2 - Wave with Envelope */
		if( gb->snd_2.on )
		{
			sample = gb->snd_2.signal * gb->snd_2.env_value;
			gb->snd_2.pos++;
			if( gb->snd_2.pos == (UINT32)(gb->snd_2.period / wave_duty_table[gb->snd_2.duty]) >> FIXED_POINT)
			{
				gb->snd_2.signal = -gb->snd_2.signal;
			}
			else if( gb->snd_2.pos > (gb->snd_2.period >> FIXED_POINT) )
			{
				gb->snd_2.pos = 0;
				gb->snd_2.signal = -gb->snd_2.signal;
			}

			if( gb->snd_2.length && gb->snd_2.mode )
			{
				gb->snd_2.count++;
				if( gb->snd_2.count >= gb->snd_2.length )
				{
					gb->snd_2.on = 0;
					gb->snd_regs[NR52] &= 0xFD;
				}
			}

			if( gb->snd_2.env_length )
			{
				gb->snd_2.env_count++;
				if( gb->snd_2.env_count >= gb->snd_2.env_length )
				{
					gb->snd_2.env_count = 0;
					gb->snd_2.env_value += gb->snd_2.env_direction;
					if( gb->snd_2.env_value < 0 )
						gb->snd_2.env_value = 0;
					if( gb->snd_2.env_value > 15 )
						gb->snd_2.env_value = 15;
				}
			}

			if( gb->snd_control.mode2_left )
				left += sample;
			if( gb->snd_control.mode2_right )
				right += sample;
		}

		/* Mode 3 - Wave patterns from WaveRAM */
		if( gb->snd_3.on )
		{
			/* NOTE: This is extremely close, but not quite right.
               The problem is for GB frequencies above 2000 the frequency gets
               clipped. This is caused because gb->snd_3.pos is never 0 at the test.*/
			sample = gb->snd_regs[AUD3W0 + (gb->snd_3.offset/2)];
			if( !(gb->snd_3.offset % 2) )
			{
				sample >>= 4;
			}
			sample = (sample & 0xF) - 8;

			if( gb->snd_3.level )
				sample >>= (gb->snd_3.level - 1);
			else
				sample = 0;

			gb->snd_3.pos++;
			if( gb->snd_3.pos >= ((UINT32)(((gb->snd_3.period ) >> 21)) + gb->snd_3.duty) )
			{
				gb->snd_3.pos = 0;
				if( gb->snd_3.dutycount == ((UINT32)(((gb->snd_3.period ) >> FIXED_POINT)) % 32) )
				{
					gb->snd_3.duty--;
				}
				gb->snd_3.dutycount++;
				gb->snd_3.offset++;
				if( gb->snd_3.offset > 31 )
				{
					gb->snd_3.offset = 0;
					gb->snd_3.duty = 1;
					gb->snd_3.dutycount = 0;
				}
			}

			if( gb->snd_3.length && gb->snd_3.mode )
			{
				gb->snd_3.count++;
				if( gb->snd_3.count >= gb->snd_3.length )
				{
					gb->snd_3.on = 0;
					gb->snd_regs[NR52] &= 0xFB;
				}
			}

			if( gb->snd_control.mode3_left )
				left += sample;
			if( gb->snd_control.mode3_right )
				right += sample;
		}

		/* Mode 4 - Noise with Envelope */
		if( gb->snd_4.on )
		{
			/* Similar problem to Mode 3, we seem to miss some notes */
			sample = gb->snd_4.signal & gb->snd_4.env_value;
			gb->snd_4.pos++;
			if( gb->snd_4.pos == (gb->snd_4.period >> (FIXED_POINT + 1)) )
			{
				/* Using a Polynomial Counter (aka Linear Feedback Shift Register)
                   Mode 4 has a 7 bit and 15 bit counter so we need to shift the
                   bits around accordingly */
				mode4_mask = (((gb->snd_4.ply_value & 0x2) >> 1) ^ (gb->snd_4.ply_value & 0x1)) << (gb->snd_4.ply_step ? 6 : 14);
				gb->snd_4.ply_value >>= 1;
				gb->snd_4.ply_value |= mode4_mask;
				gb->snd_4.ply_value &= (gb->snd_4.ply_step ? 0x7f : 0x7fff);
				gb->snd_4.signal = (INT8)gb->snd_4.ply_value;
			}
			else if( gb->snd_4.pos > (gb->snd_4.period >> FIXED_POINT) )
			{
				gb->snd_4.pos = 0;
				mode4_mask = (((gb->snd_4.ply_value & 0x2) >> 1) ^ (gb->snd_4.ply_value & 0x1)) << (gb->snd_4.ply_step ? 6 : 14);
				gb->snd_4.ply_value >>= 1;
				gb->snd_4.ply_value |= mode4_mask;
				gb->snd_4.ply_value &= (gb->snd_4.ply_step ? 0x7f : 0x7fff);
				gb->snd_4.signal = (INT8)gb->snd_4.ply_value;
			}

			if( gb->snd_4.length && gb->snd_4.mode )
			{
				gb->snd_4.count++;
				if( gb->snd_4.count >= gb->snd_4.length )
				{
					gb->snd_4.on = 0;
					gb->snd_regs[NR52] &= 0xF7;
				}
			}

			if( gb->snd_4.env_length )
			{
				gb->snd_4.env_count++;
				if( gb->snd_4.env_count >= gb->snd_4.env_length )
				{
					gb->snd_4.env_count = 0;
					gb->snd_4.env_value += gb->snd_4.env_direction;
					if( gb->snd_4.env_value < 0 )
						gb->snd_4.env_value = 0;
					if( gb->snd_4.env_value > 15 )
						gb->snd_4.env_value = 15;
				}
			}

			if( gb->snd_control.mode4_left )
				left += sample;
			if( gb->snd_control.mode4_right )
				right += sample;
		}

		/* Adjust for master volume */
		left *= gb->snd_control.vol_left;
		right *= gb->snd_control.vol_right;

		/* pump up the volume */
		left <<= 6;
		right <<= 6;

		/* Update the buffers */
		*(outputs[0]++) = left;
		*(outputs[1]++) = right;
	}

	gb->snd_regs[NR52] = (gb->snd_regs[NR52]&0xf0) | gb->snd_1.on | (gb->snd_2.on << 1) | (gb->snd_3.on << 2) | (gb->snd_4.on << 3);
}


static DEVICE_START( gameboy_sound )
{
	gb_sound_t *gb = get_token(device);
	int I, J;

	memset(&gb->snd_1, 0, sizeof(gb->snd_1));
	memset(&gb->snd_2, 0, sizeof(gb->snd_2));
	memset(&gb->snd_3, 0, sizeof(gb->snd_3));
	memset(&gb->snd_4, 0, sizeof(gb->snd_4));

	gb->channel = device->machine().sound().stream_alloc(*device, 0, 2, device->machine().sample_rate(), 0, gameboy_update);
	gb->rate = device->machine().sample_rate();

	/* Calculate the envelope and sweep tables */
	for( I = 0; I < 8; I++ )
	{
		gb->env_length_table[I] = (I * ((1 << FIXED_POINT) / 64) * gb->rate) >> FIXED_POINT;
		gb->swp_time_table[I] = (((I << FIXED_POINT) / 128) * gb->rate) >> (FIXED_POINT - 1);
	}

	/* Calculate the period tables */
	for( I = 0; I < MAX_FREQUENCIES; I++ )
	{
		gb->period_table[I] = ((1 << FIXED_POINT) / (131072 / (2048 - I))) * gb->rate;
		gb->period_mode3_table[I] = ((1 << FIXED_POINT) / (65536 / (2048 - I))) * gb->rate;
	}
	/* Calculate the period table for mode 4 */
	for( I = 0; I < 8; I++ )
	{
		for( J = 0; J < 16; J++ )
		{
			/* I is the dividing ratio of frequencies
               J is the shift clock frequency */
			gb->period_mode4_table[I][J] = ((1 << FIXED_POINT) / (524288 / ((I == 0)?0.5:I) / (1 << (J + 1)))) * gb->rate;
		}
	}

	/* Calculate the length table */
	for( I = 0; I < 64; I++ )
	{
		gb->length_table[I] = ((64 - I) * ((1 << FIXED_POINT)/256) * gb->rate) >> FIXED_POINT;
	}
	/* Calculate the length table for mode 3 */
	for( I = 0; I < 256; I++ )
	{
		gb->length_mode3_table[I] = ((256 - I) * ((1 << FIXED_POINT)/256) * gb->rate) >> FIXED_POINT;
	}

	gb_sound_w_internal( device, NR52, 0x00 );
	gb->snd_regs[AUD3W0] = 0xac;
	gb->snd_regs[AUD3W1] = 0xdd;
	gb->snd_regs[AUD3W2] = 0xda;
	gb->snd_regs[AUD3W3] = 0x48;
	gb->snd_regs[AUD3W4] = 0x36;
	gb->snd_regs[AUD3W5] = 0x02;
	gb->snd_regs[AUD3W6] = 0xcf;
	gb->snd_regs[AUD3W7] = 0x16;
	gb->snd_regs[AUD3W8] = 0x2c;
	gb->snd_regs[AUD3W9] = 0x04;
	gb->snd_regs[AUD3WA] = 0xe5;
	gb->snd_regs[AUD3WB] = 0x2c;
	gb->snd_regs[AUD3WC] = 0xac;
	gb->snd_regs[AUD3WD] = 0xdd;
	gb->snd_regs[AUD3WE] = 0xda;
	gb->snd_regs[AUD3WF] = 0x48;
}


DEVICE_GET_INFO( gameboy_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(gb_sound_t);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(gameboy_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "LR35902");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(GAMEBOY, gameboy_sound);
