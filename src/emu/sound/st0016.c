/************************************
      Seta custom ST-0016 chip
      sound emulation by R. Belmont, Tomasz Slanina, and David Haywood
************************************/

#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "cpuintrf.h"
#include "st0016.h"

#define VERBOSE (0)

UINT8 *st0016_sound_regs;

struct st0016_info
{
	sound_stream * stream;
	UINT8 **sound_ram;
	int vpos[8], frac[8], lponce[8];
};

WRITE8_HANDLER(st0016_snd_w)
{
	struct st0016_info *info = sndti_token(SOUND_ST0016, 0);
	int voice = offset/32;
	int reg = offset & 0x1f;
	int oldreg = st0016_sound_regs[offset];
#if VERBOSE
	int vbase = offset & ~0x1f;
#endif

	st0016_sound_regs[offset] = data;

	if ((voice < 8) && (data != oldreg))
	{
		if ((reg == 0x16) && (data != 0))
		{
			info->vpos[voice] = info->frac[voice] = info->lponce[voice] = 0;

#if VERBOSE
			logerror("Key on V%02d: st %06x-%06x lp %06x-%06x frq %x flg %x\n", voice,
				st0016_sound_regs[vbase+2]<<16 | st0016_sound_regs[vbase+1]<<8 | st0016_sound_regs[vbase+2],
				st0016_sound_regs[vbase+0xe]<<16 | st0016_sound_regs[vbase+0xd]<<8 | st0016_sound_regs[vbase+0xc],
				st0016_sound_regs[vbase+6]<<16 | st0016_sound_regs[vbase+5]<<8 | st0016_sound_regs[vbase+4],
				st0016_sound_regs[vbase+0xa]<<16 | st0016_sound_regs[vbase+0x9]<<8 | st0016_sound_regs[vbase+0x8],
				st0016_sound_regs[vbase+0x11]<<8 | st0016_sound_regs[vbase+0x10],
				st0016_sound_regs[vbase+0x16]);
#endif
		}
	}
}

static void st0016_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	struct st0016_info *info = param;
	UINT8 *sound_ram = *info->sound_ram;
	int v, i, snum;
	unsigned char *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;

	memset(mix, 0, sizeof(mix[0])*length*2);

	for (v = 0; v < 8; v++)
	{
		slot = (unsigned char *)&st0016_sound_regs[v * 32];

		if (slot[0x16] & 0x06)
		{
			mixp = &mix[0];

			sptr = slot[0x02]<<16 | slot[0x01]<<8 | slot[0x00];
			eptr = slot[0x0e]<<16 | slot[0x0d]<<8 | slot[0x0c];
			freq = slot[0x11]<<8 | slot[0x10];
			lsptr = slot[0x06]<<16 | slot[0x05]<<8 | slot[0x04];
			leptr = slot[0x0a]<<16 | slot[0x09]<<8 | slot[0x08];

			for (snum = 0; snum < length; snum++)
			{
				sample = sound_ram[(sptr + info->vpos[v])&0x1fffff]<<8;

				*mixp++ += (sample * (char)slot[0x14]) >> 8;
				*mixp++ += (sample * (char)slot[0x15]) >> 8;

				info->frac[v] += freq;
				info->vpos[v] += (info->frac[v]>>16);
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
						if (slot[0x16] & 0x01)	// loop?
						{
							info->vpos[v] = (lsptr - sptr);
							info->lponce[v] = 1;
						}
						else
						{
							slot[0x16] = 0;
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

static void *st0016_start(int sndindex, int clock, const void *config)
{
	const struct ST0016interface *intf = config;
	struct st0016_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->sound_ram = intf->p_soundram;

	info->stream = stream_create(0, 2, 44100, info, st0016_update);

	return info;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void st0016_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void st0016_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = st0016_set_info;		break;
		case SNDINFO_PTR_START:							info->start = st0016_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "ST0016";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Seta custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

