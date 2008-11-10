/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)

                    rewrite by Manbow-J(manbowj@hamal.freemail.ne.jp)

                    X1-010 Seta Custom Sound Chip (80 Pin PQFP)

 Custom programmed Mitsubishi M60016 Gate Array, 3608 gates, 148 Max I/O ports

    The X1-010 is 16 Voices sound generator, each channel gets it's
    waveform from RAM (128 bytes per waveform, 8 bit unsigned data)
    or sampling PCM(8bit unsigned data).

Registers:
    8 registers per channel (mapped to the lower bytes of 16 words on the 68K)

    Reg:    Bits:       Meaning:

    0       7654 3---
            ---- -2--   PCM/Waveform repeat flag (0:Ones 1:Repeat) (*1)
            ---- --1-   Sound out select (0:PCM 1:Waveform)
            ---- ---0   Key on / off

    1       7654 ----   PCM Volume 1 (L?)
            ---- 3210   PCM Volume 2 (R?)
                        Waveform No.

    2                   PCM Frequency
                        Waveform Pitch Lo

    3                   Waveform Pitch Hi

    4                   PCM Sample Start / 0x1000           [Start/End in bytes]
                        Waveform Envelope Time

    5                   PCM Sample End 0x100 - (Sample End / 0x1000)    [PCM ROM is Max 1MB?]
                        Waveform Envelope No.
    6                   Reserved
    7                   Reserved

    offset 0x0000 - 0x0fff  Wave form data
    offset 0x1000 - 0x1fff  Envelope data

    *1 : when 0 is specified, hardware interrupt is caused(allways return soon)

***************************************************************************/

#include "sndintrf.h"
#include "deprecat.h"
#include "cpuintrf.h"
#include "streams.h"
#include "x1_010.h"


#define VERBOSE_SOUND 0
#define VERBOSE_REGISTER_WRITE 0
#define VERBOSE_REGISTER_READ 0

#define LOG_SOUND(x) do { if (VERBOSE_SOUND) logerror x; } while (0)
#define LOG_REGISTER_WRITE(x) do { if (VERBOSE_REGISTER_WRITE) logerror x; } while (0)
#define LOG_REGISTER_READ(x) do { if (VERBOSE_REGISTER_READ) logerror x; } while (0)

#define SETA_NUM_CHANNELS 16

#define FREQ_BASE_BITS		  8					// Frequency fixed decimal shift bits
#define ENV_BASE_BITS		 16					// wave form envelope fixed decimal shift bits
#define	VOL_BASE	(2*32*256/30)					// Volume base

/* this structure defines the parameters for a channel */
typedef struct {
	unsigned char	status;
	unsigned char	volume;						//        volume / wave form no.
	unsigned char	frequency;					//     frequency / pitch lo
	unsigned char	pitch_hi;					//      reserved / pitch hi
	unsigned char	start;						// start address / envelope time
	unsigned char	end;						//   end address / envelope no.
	unsigned char	reserve[2];
} X1_010_CHANNEL;

struct x1_010_info
{
	/* Variables only used here */
	int	rate;								// Output sampling rate (Hz)
	sound_stream *	stream;					// Stream handle
	int	address;							// address eor data
	const char *region;						// region name
	int	sound_enable;						// sound output enable/disable
	UINT8	reg[0x2000];				// X1-010 Register & wave form area
	UINT8	HI_WORD_BUF[0x2000];			// X1-010 16bit access ram check avoidance work
	UINT32	smp_offset[SETA_NUM_CHANNELS];
	UINT32	env_offset[SETA_NUM_CHANNELS];

	UINT32 base_clock;
};

/* mixer tables and internal buffers */
//static short  *mixer_buffer = NULL;


/*--------------------------------------------------------------
 generate sound to the mix buffer
--------------------------------------------------------------*/
static void seta_update( void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length )
{
	struct x1_010_info *info = param;
	X1_010_CHANNEL	*reg;
	int		ch, i, volL, volR, freq;
	register INT8	*start, *end, data;
	register UINT8	*env;
	register UINT32	smp_offs, smp_step, env_offs, env_step, delta;
	UINT8 *snd1 = memory_region(Machine, info->region);

	// mixer buffer zero clear
	memset( buffer[0], 0, length*sizeof(*buffer[0]) );
	memset( buffer[1], 0, length*sizeof(*buffer[1]) );

//  if( info->sound_enable == 0 ) return;

	for( ch = 0; ch < SETA_NUM_CHANNELS; ch++ ) {
		reg = (X1_010_CHANNEL *)&(info->reg[ch*sizeof(X1_010_CHANNEL)]);
		if( (reg->status&1) != 0 ) {							// Key On
			stream_sample_t *bufL = buffer[0];
			stream_sample_t *bufR = buffer[1];
			if( (reg->status&2) == 0 ) {						// PCM sampling
				start    = (INT8 *)(reg->start      *0x1000+snd1);
				end      = (INT8 *)((0x100-reg->end)*0x1000+snd1);
				volL     = ((reg->volume>>4)&0xf)*VOL_BASE;
				volR     = ((reg->volume>>0)&0xf)*VOL_BASE;
				smp_offs = info->smp_offset[ch];
				freq     = reg->frequency&0x1f;
				// Meta Fox does not write the frequency register. Ever
				if( freq == 0 ) freq = 4;
				smp_step = (UINT32)((float)info->base_clock/8192.0
							*freq*(1<<FREQ_BASE_BITS)/(float)info->rate);
				if( smp_offs == 0 ) {
					LOG_SOUND(( "Play sample %p - %p, channel %X volume %d:%d freq %X step %X offset %X\n",
						start, end, ch, volL, volR, freq, smp_step, smp_offs ));
				}
				for( i = 0; i < length; i++ ) {
					delta = smp_offs>>FREQ_BASE_BITS;
					// sample ended?
					if( start+delta >= end ) {
						reg->status &= 0xfe;					// Key off
						break;
					}
					data = *(start+delta);
					*bufL++ += (data*volL/256);
					*bufR++ += (data*volR/256);
					smp_offs += smp_step;
				}
				info->smp_offset[ch] = smp_offs;
			} else {											// Wave form
				start    = (INT8 *)&(info->reg[reg->volume*128+0x1000]);
				smp_offs = info->smp_offset[ch];
				freq     = (reg->pitch_hi<<8)+reg->frequency;
				smp_step = (UINT32)((float)info->base_clock/128.0/1024.0/4.0*freq*(1<<FREQ_BASE_BITS)/(float)info->rate);

				env      = (UINT8 *)&(info->reg[reg->end*128]);
				env_offs = info->env_offset[ch];
				env_step = (UINT32)((float)info->base_clock/128.0/1024.0/4.0*reg->start*(1<<ENV_BASE_BITS)/(float)info->rate);
				/* Print some more debug info */
				if( smp_offs == 0 ) {
					LOG_SOUND(( "Play waveform %X, channel %X volume %X freq %4X step %X offset %X\n",
						reg->volume, ch, reg->end, freq, smp_step, smp_offs ));
				}
				for( i = 0; i < length; i++ ) {
					int vol;
					delta = env_offs>>ENV_BASE_BITS;
	 				// Envelope one shot mode
					if( (reg->status&4) != 0 && delta >= 0x80 ) {
						reg->status &= 0xfe;					// Key off
						break;
					}
					vol = *(env+(delta&0x7f));
					volL = ((vol>>4)&0xf)*VOL_BASE;
					volR = ((vol>>0)&0xf)*VOL_BASE;
					data  = *(start+((smp_offs>>FREQ_BASE_BITS)&0x7f));
					*bufL++ += (data*volL/256);
					*bufR++ += (data*volR/256);
					smp_offs += smp_step;
					env_offs += env_step;
				}
				info->smp_offset[ch] = smp_offs;
				info->env_offset[ch] = env_offs;
			}
		}
	}
}



static void *x1_010_start(const char *tag, int sndindex, int clock, const void *config)
{
	int i;
	const x1_010_interface *intf = config;
	struct x1_010_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->region		= tag;
	info->base_clock	= clock;
	info->rate			= clock / 1024;
	info->address		= intf->adr;

	for( i = 0; i < SETA_NUM_CHANNELS; i++ ) {
		info->smp_offset[i] = 0;
		info->env_offset[i] = 0;
	}
	/* Print some more debug info */
	LOG_SOUND(("masterclock = %d rate = %d\n", clock, info->rate ));

	/* get stream channels */
	info->stream = stream_create(0,2,info->rate,info,seta_update);

	return info;
}


void seta_sound_enable_w(int data)
{
	struct x1_010_info *info = sndti_token(SOUND_X1_010, 0);
	info->sound_enable = data;
}



/* Use these for 8 bit CPUs */


READ8_HANDLER( seta_sound_r )
{
	struct x1_010_info *info = sndti_token(SOUND_X1_010, 0);
	offset ^= info->address;
	return info->reg[offset];
}




WRITE8_HANDLER( seta_sound_w )
{
	struct x1_010_info *info = sndti_token(SOUND_X1_010, 0);
	int channel, reg;
	offset ^= info->address;

	channel	= offset/sizeof(X1_010_CHANNEL);
	reg		= offset%sizeof(X1_010_CHANNEL);

	if( channel < SETA_NUM_CHANNELS && reg == 0
	 && (info->reg[offset]&1) == 0 && (data&1) != 0 ) {
	 	info->smp_offset[channel] = 0;
	 	info->env_offset[channel] = 0;
	}
	LOG_REGISTER_WRITE(("PC: %06X : offset %6X : data %2X\n", cpu_get_pc(machine->activecpu), offset, data ));
	info->reg[offset] = data;
}




/* Use these for 16 bit CPUs */

READ16_HANDLER( seta_sound_word_r )
{
	struct x1_010_info *info = sndti_token(SOUND_X1_010, 0);
	UINT16	ret;

	ret = info->HI_WORD_BUF[offset]<<8;
	ret += (seta_sound_r( machine, offset )&0xff);
	LOG_REGISTER_READ(( "Read X1-010 PC:%06X Offset:%04X Data:%04X\n", cpu_get_pc(machine->activecpu), offset, ret ));
	return ret;
}

WRITE16_HANDLER( seta_sound_word_w )
{
	struct x1_010_info *info = sndti_token(SOUND_X1_010, 0);
	info->HI_WORD_BUF[offset] = (data>>8)&0xff;
	seta_sound_w( machine, offset, data&0xff );
	LOG_REGISTER_WRITE(( "Write X1-010 PC:%06X Offset:%04X Data:%04X\n", cpu_get_pc(machine->activecpu), offset, data ));
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void x1_010_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void x1_010_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = x1_010_set_info;		break;
		case SNDINFO_PTR_START:							info->start = x1_010_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "X1-010";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Seta custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

