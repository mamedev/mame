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
   05 - flags? 00000000 0000?L0?   - bit 0 loops, other bits appear to be not used by the chip
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

#include "emu.h"
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



typedef struct _nile_state nile_state;
struct _nile_state
{
	sound_stream * stream;
	UINT8 *sound_ram;
	UINT16 sound_regs[0x80];
	int vpos[NILE_VOICES], frac[NILE_VOICES], lponce[NILE_VOICES];
	UINT16 ctrl;
};

INLINE nile_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == NILE);
	return (nile_state *)downcast<nile_device *>(device)->token();
}


WRITE16_DEVICE_HANDLER( nile_sndctrl_w )
{
	nile_state *info = get_safe_token(device);
	UINT16 ctrl=info->ctrl;

	info->stream->update();

	COMBINE_DATA(&info->ctrl);

//  printf("CTRL: %04x -> %04x (PC=%x)\n", ctrl, info->ctrl, space->device().safe_pc());

	ctrl^=info->ctrl;
}

READ16_DEVICE_HANDLER( nile_sndctrl_r )
{
	nile_state *info = get_safe_token(device);

	info->stream->update();

	return info->ctrl;
}

READ16_DEVICE_HANDLER( nile_snd_r )
{
	nile_state *info = get_safe_token(device);
	int reg=offset&0xf;

	info->stream->update();

	if(reg==2 || reg==3)
	{
		int slot=offset/16;
		int sptr = ((info->sound_regs[slot*16+3]<<16)|info->sound_regs[slot*16+2])+info->vpos[slot];

		if(reg==2)
		{
			return sptr&0xffff;
		}
		else
		{
			return sptr>>16;
		}
	}
	return info->sound_regs[offset];
}

WRITE16_DEVICE_HANDLER( nile_snd_w )
{
	nile_state *info = get_safe_token(device);
	int v, r;

	info->stream->update();

	COMBINE_DATA(&info->sound_regs[offset]);

	v = offset / 16;
	r = offset % 16;

	if ((r == 2) || (r == 3))
	{
		info->vpos[v] = info->frac[v] = info->lponce[v] = 0;
	}

	//printf("v%02d: %04x to reg %02d (PC=%x)\n", v, info->sound_regs[offset], r, space->device().safe_pc());
}

static STREAM_UPDATE( nile_update )
{
	nile_state *info = (nile_state *)param;
	UINT8 *sound_ram = info->sound_ram;
	int v, i, snum;
	UINT16 *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;

	lsptr=leptr=0;

	memset(mix, 0, sizeof(mix[0])*samples*2);

	for (v = 0; v < NILE_VOICES; v++)
	{
		slot = &info->sound_regs[v * 16];

		if (info->ctrl&(1<<v))
		{
			mixp = &mix[0];

			sptr = slot[NILE_REG_SPTR_HI]<<16 | slot[NILE_REG_SPTR_LO];
			eptr = slot[NILE_REG_EPTR_HI]<<16 | slot[NILE_REG_EPTR_LO];

			freq=slot[NILE_REG_FREQ]*14;
			lsptr = slot[NILE_REG_LSPTR_HI]<<16 | slot[NILE_REG_LSPTR_LO];
			leptr = slot[NILE_REG_LEPTR_HI]<<16 | slot[NILE_REG_LEPTR_LO];

			for (snum = 0; snum < samples; snum++)
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
							info->ctrl &= ~(1<<v);
							info->vpos[v] = (eptr - sptr);
							info->frac[v] = 0;
						}

					}
				}
			}
		}
	}
	mixp = &mix[0];
	for (i = 0; i < samples; i++)
	{
		outputs[0][i] = (*mixp++)>>4;
		outputs[1][i] = (*mixp++)>>4;
	}
}

static DEVICE_START( nile )
{
	nile_state *info = get_safe_token(device);

	info->sound_ram = *device->region();

	info->stream = device->machine().sound().stream_alloc(*device, 0, 2, 44100, info, nile_update);
}

const device_type NILE = &device_creator<nile_device>;

nile_device::nile_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NILE, "NiLe", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(nile_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void nile_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nile_device::device_start()
{
	DEVICE_START_NAME( nile )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void nile_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


