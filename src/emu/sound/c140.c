/*
C140.c

Simulator based on AMUSE sources.
The C140 sound chip is used by Namco System 2 and System 21
The 219 ASIC (which incorporates a modified C140) is used by Namco NA-1 and NA-2
This chip controls 24 channels (C140) or 16 (219) of PCM.
16 bytes are associated with each channel.
Channels can be 8 bit signed PCM, or 12 bit signed PCM.

Timer behavior is not yet handled.

Unmapped registers:
    0x1f8:timer interval?   (Nx0.1 ms)
    0x1fa:irq ack? timer restart?
    0x1fe:timer switch?(0:off 1:on)

--------------

    ASIC "219" notes

    On the 219 ASIC used on NA-1 and NA-2, the high registers have the following
    meaning instead:
    0x1f7: bank for voices 0-3
    0x1f1: bank for voices 4-7
    0x1f3: bank for voices 8-11
    0x1f5: bank for voices 12-15

    Some games (bkrtmaq, xday2) write to 0x1fd for voices 12-15 instead.  Probably the bank registers
    mirror at 1f8, in which case 1ff is also 0-3, 1f9 is also 4-7, 1fb is also 8-11, and 1fd is also 12-15.

    Each bank is 0x20000 (128k), and the voice addresses on the 219 are all multiplied by 2.
    Additionally, the 219's base pitch is the same as the C352's (42667).  But these changes
    are IMO not sufficient to make this a separate file - all the other registers are
    fully compatible.

    Finally, the 219 only has 16 voices.
*/
/*
    2000.06.26  CAB     fixed compressed pcm playback
    2002.07.20  R.Belmont   added support for multiple banking types
    2006.01.08  R.Belmont   added support for NA-1/2 "219" derivative
*/


#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "c140.h"

#define MAX_VOICE 24

struct voice_registers
{
	UINT8 volume_right;
	UINT8 volume_left;
	UINT8 frequency_msb;
	UINT8 frequency_lsb;
	UINT8 bank;
	UINT8 mode;
	UINT8 start_msb;
	UINT8 start_lsb;
	UINT8 end_msb;
	UINT8 end_lsb;
	UINT8 loop_msb;
	UINT8 loop_lsb;
	UINT8 reserved[4];
};

typedef struct
{
	long	ptoffset;
	long	pos;
	long	key;
	//--work
	long	lastdt;
	long	prevdt;
	long	dltdt;
	//--reg
	long	rvol;
	long	lvol;
	long	frequency;
	long	bank;
	long	mode;

	long	sample_start;
	long	sample_end;
	long	sample_loop;
} VOICE;

struct c140_info
{
	int sample_rate;
	sound_stream *stream;
	int banking_type;
	/* internal buffers */
	INT16 *mixer_buffer_left;
	INT16 *mixer_buffer_right;

	int baserate;
	void *pRom;
	UINT8 REG[0x200];

	INT16 pcmtbl[8];		//2000.06.26 CAB

	VOICE voi[MAX_VOICE];
};

static void init_voice( VOICE *v )
{
	v->key=0;
	v->ptoffset=0;
	v->rvol=0;
	v->lvol=0;
	v->frequency=0;
	v->bank=0;
	v->mode=0;
	v->sample_start=0;
	v->sample_end=0;
	v->sample_loop=0;
}
READ8_HANDLER( C140_r )
{
	struct c140_info *info = sndti_token(SOUND_C140, 0);
	offset&=0x1ff;
	return info->REG[offset];
}

/*
   find_sample: compute the actual address of a sample given it's
   address and banking registers, as well as the board type.

   I suspect in "real life" this works like the Sega MultiPCM where the banking
   is done by a small PAL or GAL external to the sound chip, which can be switched
   per-game or at least per-PCB revision as addressing range needs grow.
 */
static long find_sample(struct c140_info *info, long adrs, long bank, int voice)
{
	long newadr = 0;

	static INT16 asic219banks[4] = { 0x1f7, 0x1f1, 0x1f3, 0x1f5 };

	adrs=(bank<<16)+adrs;

	switch (info->banking_type)
	{
		case C140_TYPE_SYSTEM2:
			// System 2 banking
			newadr = ((adrs&0x200000)>>2)|(adrs&0x7ffff);
			break;

		case C140_TYPE_SYSTEM21_A:
			// System 21 type A (simple) banking.
			// similar to System 2's.
			newadr = ((adrs&0x300000)>>1)+(adrs&0x7ffff);
			break;

		case C140_TYPE_SYSTEM21_B:
			// System 21 type B (chip select) banking

			// get base address of sample inside the bank
			newadr = ((adrs&0x100000)>>2) + (adrs&0x3ffff);

			// now add the starting bank offsets based on the 2
			// chip select bits.
			// 0x40000 picks individual 512k ROMs
			if (adrs & 0x40000)
			{
				newadr += 0x80000;
			}

			// and 0x200000 which group of chips...
			if (adrs & 0x200000)
			{
				newadr += 0x100000;
			}
			break;

		case C140_TYPE_ASIC219:
			// ASIC219's banking is fairly simple
			newadr = ((info->REG[asic219banks[voice/4]]&0x3) * 0x20000) + adrs;
			break;
	}

	return (newadr);
}
WRITE8_HANDLER( C140_w )
{
	struct c140_info *info = sndti_token(SOUND_C140, 0);
	stream_update(info->stream);

	offset&=0x1ff;

	// mirror the bank registers on the 219, fixes bkrtmaq (and probably xday2 based on notes in the HLE)
	if ((offset >= 0x1f8) && (info->banking_type == C140_TYPE_ASIC219))
	{
		offset -= 8;
	}

	info->REG[offset]=data;
	if( offset<0x180 )
	{
		VOICE *v = &info->voi[offset>>4];

		if( (offset&0xf)==0x5 )
		{
			if( data&0x80 )
			{
				const struct voice_registers *vreg = (struct voice_registers *) &info->REG[offset&0x1f0];
				v->key=1;
				v->ptoffset=0;
				v->pos=0;
				v->lastdt=0;
				v->prevdt=0;
				v->dltdt=0;
				v->bank = vreg->bank;
				v->mode = data;

				// on the 219 asic, addresses are in words
				if (info->banking_type == C140_TYPE_ASIC219)
				{
					v->sample_loop = (vreg->loop_msb*256 + vreg->loop_lsb)*2;
					v->sample_start = (vreg->start_msb*256 + vreg->start_lsb)*2;
					v->sample_end = (vreg->end_msb*256 + vreg->end_lsb)*2;

					#if 0
					logerror("219: play v %d mode %02x start %x loop %x end %x\n",
						offset>>4, v->mode,
						find_sample(info, v->sample_start, v->bank, offset>>4),
						find_sample(info, v->sample_loop, v->bank, offset>>4),
						find_sample(info, v->sample_end, v->bank, offset>>4));
					#endif
				}
				else
				{
				v->sample_loop = vreg->loop_msb*256 + vreg->loop_lsb;
				v->sample_start = vreg->start_msb*256 + vreg->start_lsb;
				v->sample_end = vreg->end_msb*256 + vreg->end_lsb;
			}
			}
			else
			{
				v->key=0;
			}
		}
	}
}

void C140_set_base(int which, void *base)
{
	struct c140_info *info = sndti_token(SOUND_C140, 0);
	info->pRom = base;
}

INLINE int limit(INT32 in)
{
	if(in>0x7fff)		return 0x7fff;
	else if(in<-0x8000)	return -0x8000;
	return in;
}

static void update_stereo(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct c140_info *info = param;
	int		i,j;

	INT32	rvol,lvol;
	INT32	dt;
	INT32	sdt;
	INT32	st,ed,sz;

	INT8	*pSampleData;
	INT32	frequency,delta,offset,pos;
	INT32	cnt, voicecnt;
	INT32	lastdt,prevdt,dltdt;
	float	pbase=(float)info->baserate*2.0 / (float)info->sample_rate;

	INT16	*lmix, *rmix;

	if(length>info->sample_rate) length=info->sample_rate;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer_left, 0, length * sizeof(INT16));
	memset(info->mixer_buffer_right, 0, length * sizeof(INT16));

	/* get the number of voices to update */
	voicecnt = (info->banking_type == C140_TYPE_ASIC219) ? 16 : 24;

	//--- audio update
	for( i=0;i<voicecnt;i++ )
	{
		VOICE *v = &info->voi[i];
		const struct voice_registers *vreg = (struct voice_registers *)&info->REG[i*16];

		if( v->key )
		{
			frequency= vreg->frequency_msb*256 + vreg->frequency_lsb;

			/* Abort voice if no frequency value set */
			if(frequency==0) continue;

			/* Delta =  frequency * ((8MHz/374)*2 / sample rate) */
			delta=(long)((float)frequency * pbase);

			/* Calculate left/right channel volumes */
			lvol=(vreg->volume_left*32)/MAX_VOICE; //32ch -> 24ch
			rvol=(vreg->volume_right*32)/MAX_VOICE;

			/* Set mixer buffer base pointers */
			lmix = info->mixer_buffer_left;
			rmix = info->mixer_buffer_right;

			/* Retrieve sample start/end and calculate size */
			st=v->sample_start;
			ed=v->sample_end;
			sz=ed-st;

			/* Retrieve base pointer to the sample data */
			pSampleData=(signed char*)((FPTR)info->pRom + find_sample(info, st, v->bank, i));

			/* Fetch back previous data pointers */
			offset=v->ptoffset;
			pos=v->pos;
			lastdt=v->lastdt;
			prevdt=v->prevdt;
			dltdt=v->dltdt;

			/* Switch on data type - compressed PCM is only for C140 */
			if ((v->mode&8) && (info->banking_type != C140_TYPE_ASIC219))
			{
				//compressed PCM (maybe correct...)
				/* Loop for enough to fill sample buffer as requested */
				for(j=0;j<length;j++)
				{
					offset += delta;
					cnt = (offset>>16)&0x7fff;
					offset &= 0xffff;
					pos+=cnt;
					//for(;cnt>0;cnt--)
					{
						/* Check for the end of the sample */
						if(pos >= sz)
						{
							/* Check if its a looping sample, either stop or loop */
							if(v->mode&0x10)
							{
								pos = (v->sample_loop - st);
							}
							else
							{
								v->key=0;
								break;
							}
						}

						/* Read the chosen sample byte */
						dt=pSampleData[pos];

						/* decompress to 13bit range */		//2000.06.26 CAB
						sdt=dt>>3;				//signed
						if(sdt<0)	sdt = (sdt<<(dt&7)) - info->pcmtbl[dt&7];
						else		sdt = (sdt<<(dt&7)) + info->pcmtbl[dt&7];

						prevdt=lastdt;
						lastdt=sdt;
						dltdt=(lastdt - prevdt);
					}

					/* Caclulate the sample value */
					dt=((dltdt*offset)>>16)+prevdt;

					/* Write the data to the sample buffers */
					*lmix++ +=(dt*lvol)>>(5+5);
					*rmix++ +=(dt*rvol)>>(5+5);
				}
			}
			else
			{
				/* linear 8bit signed PCM */
				for(j=0;j<length;j++)
				{
					offset += delta;
					cnt = (offset>>16)&0x7fff;
					offset &= 0xffff;
					pos += cnt;
					/* Check for the end of the sample */
					if(pos >= sz)
					{
						/* Check if its a looping sample, either stop or loop */
						if( v->mode&0x10 )
						{
							pos = (v->sample_loop - st);
						}
						else
						{
							v->key=0;
							break;
						}
					}

					if( cnt )
					{
						prevdt=lastdt;
						lastdt=pSampleData[pos];
						if ((v->mode & 0x40) && (info->banking_type == C140_TYPE_ASIC219))
						{
							lastdt ^= 0x80;	// flip signedness
						}
						dltdt=(lastdt - prevdt);
					}

					/* Caclulate the sample value */
					dt=((dltdt*offset)>>16)+prevdt;

					/* Write the data to the sample buffers */
					*lmix++ +=(dt*lvol)>>5;
					*rmix++ +=(dt*rvol)>>5;
				}
			}

			/* Save positional data for next callback */
			v->ptoffset=offset;
			v->pos=pos;
			v->lastdt=lastdt;
			v->prevdt=prevdt;
			v->dltdt=dltdt;
		}
	}

	/* render to MAME's stream buffer */
	lmix = info->mixer_buffer_left;
	rmix = info->mixer_buffer_right;
	{
		stream_sample_t *dest1 = buffer[0];
		stream_sample_t *dest2 = buffer[1];
		for (i = 0; i < length; i++)
		{
			*dest1++ = limit(8*(*lmix++));
			*dest2++ = limit(8*(*rmix++));
		}
	}
}

static void *c140_start(int sndindex, int clock, const void *config)
{
	const struct C140interface *intf = config;
	struct c140_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->sample_rate=info->baserate=clock;

	info->banking_type = intf->banking_type;

	info->stream = stream_create(0,2,info->sample_rate,info,update_stereo);

	if (intf->region)
		info->pRom=memory_region(intf->region);

	/* make decompress pcm table */		//2000.06.26 CAB
	{
		int i;
		INT32 segbase=0;
		for(i=0;i<8;i++)
		{
			info->pcmtbl[i]=segbase;	//segment base value
			segbase += 16<<i;
		}
	}

	memset(info->REG,0,0x200 );
	{
		int i;
		for(i=0;i<MAX_VOICE;i++) init_voice( &info->voi[i] );
	}

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	info->mixer_buffer_left = auto_malloc(2 * sizeof(INT16)*info->sample_rate );
	info->mixer_buffer_right = info->mixer_buffer_left + info->sample_rate;
	return info;
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void c140_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void c140_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = c140_set_info;			break;
		case SNDINFO_PTR_START:							info->start = c140_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "C140";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco PCM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

