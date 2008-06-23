/************************************
      Seta custom Nile ST-0026 chip
      sound emulation by Tomasz Slanina
      based on ST-0016 emulation

8 voices, 16 words of config data for each:

   00
   01 - sptr  ?? (always 0)
   02 - sptr  LO 
   03 - sptr  HI
   04
   05 - flags? 00000000 0000?L0?   - bit 2 could be looping flag
   06 - freq
   07 - lsptr LO
   08
   09 - lsptr HI
   0a - leptr LO
   0b - leptr HI
   0c - eptr  LO
   0d - eptr  HI
   0e - vol R
   0f - vol L

************************************/

#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "cpuintrf.h"
#include "nile.h"

#define NILE_VOICES 8

enum
{
	NILE_REG_UNK0=0,
	NILE_REG_SPTR_TOP,
	NILE_REG_SPTR_LO,
	NILE_REG_SPTR_HI,
	NILE_REG_UNK_4,
	NILE_REG_FLAGS,
	NILE_REG_FREQ,
	NILE_REG_LSPTR_LO,
	MILE_REG_UNK_8,
	NILE_REG_LSPTR_HI,
	NILE_REG_LEPTR_LO,
	NILE_REG_LEPTR_HI,
	NILE_REG_EPTR_LO,
	NILE_REG_EPTR_HI,
	NILE_REG_VOL_R,
	NILE_REG_VOL_L
};



UINT16 *nile_sound_regs;

struct nile_info
{
	sound_stream * stream;
	UINT8 *sound_ram;
	int vpos[NILE_VOICES], frac[NILE_VOICES], lponce[NILE_VOICES];
	UINT16 ctrl;
};

WRITE16_HANDLER(nile_sndctrl_w)
{
	struct nile_info *info = sndti_token(SOUND_NILE, 0);
	UINT16 ctrl=info->ctrl;
	int voice;
	COMBINE_DATA(&info->ctrl);
	
	ctrl^=info->ctrl;
	//ctrl&=info->ctrl; //0->1
	
	for(voice=0;voice<NILE_VOICES;++voice) 
	{ 
		if(ctrl&(1<<voice)) 
		{ 
			info->vpos[voice] = info->frac[voice] = info->lponce[voice] = 0;
		}
	}
}

READ16_HANDLER(nile_sndctrl_r)
{
	struct nile_info *info = sndti_token(SOUND_NILE, 0);
	return info->ctrl;
}

READ16_HANDLER(nile_snd_r)
{
	int reg=offset&0xf;
	if(reg==2 || reg==3)
	{
		int slot=offset/16;
		struct nile_info *info = sndti_token(SOUND_NILE, 0);
		int sptr = ((nile_sound_regs[slot*16+3]<<16)|nile_sound_regs[slot*16+2])+info->vpos[slot];
		
		
		if(reg==2)
		{
			return sptr&0xffff;
		}
		else
		{
			return sptr>>16;
		}
	}
	return nile_sound_regs[offset];
}

WRITE16_HANDLER(nile_snd_w)
{
	COMBINE_DATA(&nile_sound_regs[offset]);
}

static void nile_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	struct nile_info *info = param;
	UINT8 *sound_ram = info->sound_ram;
	int v, i, snum;
	UINT16 *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;
	
	lsptr=leptr=0;

	memset(mix, 0, sizeof(mix[0])*length*2);

	for (v = 0; v < NILE_VOICES; v++)
	{
		slot = &nile_sound_regs[v * 16];

		if (info->ctrl&(1<<v))
		{
			mixp = &mix[0];

			sptr = slot[NILE_REG_SPTR_HI]<<16 | slot[NILE_REG_SPTR_LO];
			eptr = slot[NILE_REG_EPTR_HI]<<16 | slot[NILE_REG_EPTR_LO];
			
			freq=slot[NILE_REG_FREQ]<<4;
			lsptr = slot[NILE_REG_LSPTR_HI]<<16 | slot[NILE_REG_LSPTR_LO];
			leptr = slot[NILE_REG_LEPTR_HI]<<16 | slot[NILE_REG_LEPTR_LO];

			for (snum = 0; snum < length; snum++)
			{
				sample = sound_ram[sptr + info->vpos[v]]<<8;

				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_R]) >> 16;
				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_L]) >> 16;
				
				info->frac[v] += freq;
				info->vpos[v] += info->frac[v]>>16;
				info->frac[v] &= 0xffff;

				// stop if we're at the end
				if (info->lponce[v])
				{
					// we've looped once, check loop end rather than sample end
					if ((info->vpos[v] + sptr) >= leptr)
					{
						info->vpos[v] = (lsptr - sptr);
					}
				}
				else
				{
					// not looped yet, check sample end
					if ((info->vpos[v] + sptr) >= eptr)
					{
						// code at 11d8c: 
						// if bit 2 (0x4) is set, check if loop start = loop end.
						// if they are equal, clear bit 0 and don't set the loop start/end
						// registers in the NiLe.  if they aren't, set bit 0 and set
						// the loop start/end registers in the NiLe.
						if ((slot[NILE_REG_FLAGS] & 0x5) == 0x5)
						{
							info->vpos[v] = (lsptr - sptr);
							info->lponce[v] = 1;
						}
						else
						{
						
						
							info->ctrl&=~(1<<v);
							info->vpos[v] = info->frac[v] = 0;
						}
						
					}
				}
			}
		}
	}
	mixp = &mix[0];
	for (i = 0; i < length; i++)
	{
		outputs[0][i] = (*mixp++)>>4;
		outputs[1][i] = (*mixp++)>>4;
	}
}

static void *nile_start(int sndindex, int clock, const void *config)
{
	const struct NiLe_interface *intf = config;
	struct nile_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->sound_ram = (UINT8 *)memory_region(intf->region);


	info->stream = stream_create(0, 2, 44100, info, nile_update);

	return info;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void nile_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void nile_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = nile_set_info;		break;
		case SNDINFO_PTR_START:							info->start = nile_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "NiLe";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Seta custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

