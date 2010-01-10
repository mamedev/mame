/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/
#include "emu.h"
#include "streams.h"
#include "includes/cps3.h"

#define CPS3_VOICES		16

static sound_stream *cps3_stream;
extern UINT8* cps3_user5region;

typedef struct _cps3_voice_
{
	UINT32 regs[8];
	UINT32 pos;
	UINT16 frac;
} cps3_voice;

static struct
{
	cps3_voice voice[CPS3_VOICES];
	UINT16     key;
	INT8*	   base;
} chip;

static STREAM_UPDATE( cps3_stream_update )
{
	int i;

	// the actual 'user5' region only exists on the nocd sets, on the others it's allocated in the initialization.
	// it's a shared gfx/sound region, so can't be allocated as part of the sound device.
	chip.base = (INT8*)cps3_user5region;

	/* Clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	for (i = 0; i < CPS3_VOICES; i ++)
	{
		if (chip.key & (1 << i))
		{
			int j;

			/* TODO */
			#define SWAP(a) ((a >> 16) | ((a & 0xffff) << 16))

			cps3_voice *vptr = &chip.voice[i];

			UINT32 start = vptr->regs[1];
			UINT32 end   = vptr->regs[5];
			UINT32 loop  = (vptr->regs[3] & 0xffff) + ((vptr->regs[4] & 0xffff) << 16);
			UINT32 step  = (vptr->regs[3] >> 16);

			INT16 vol_l = (vptr->regs[7] & 0xffff);
			INT16 vol_r = ((vptr->regs[7] >> 16) & 0xffff);

			UINT32 pos = vptr->pos;
			UINT16 frac = vptr->frac;

			/* TODO */
			start = SWAP(start) - 0x400000;
			end = SWAP(end) - 0x400000;
			loop -= 0x400000;

			/* Go through the buffer and add voice contributions */
			for (j = 0; j < samples; j ++)
			{
				INT32 sample;

				pos += (frac >> 12);
				frac &= 0xfff;


				if (start + pos >= end)
				{
					if (vptr->regs[2])
					{
						pos = loop - start;
					}
					else
					{
						chip.key &= ~(1 << i);
						break;
					}
				}

				sample = chip.base[BYTE4_XOR_LE(start + pos)];
				frac += step;

				outputs[0][j] += (sample * (vol_l >> 8));
				outputs[1][j] += (sample * (vol_r >> 8));
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}

}

static DEVICE_START( cps3_sound )
{
	/* Allocate the stream */
	cps3_stream = stream_create(device, 0, 2, device->clock / 384, NULL, cps3_stream_update);

	memset(&chip, 0, sizeof(chip));
}

DEVICE_GET_INFO( cps3_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cps3_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "CPS3 Custom");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


WRITE32_HANDLER( cps3_sound_w )
{
	stream_update(cps3_stream);

	if (offset < 0x80)
	{
		COMBINE_DATA(&chip.voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		int i;
		UINT16 key = data >> 16;

		for (i = 0; i < CPS3_VOICES; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(chip.key & (1 << i)))
			{
				chip.voice[i].frac = 0;
				chip.voice[i].pos = 0;
			}
		}
		chip.key = key;
	}
	else
	{
		printf("Sound [%x] %x\n", offset, data);
	}
}

READ32_HANDLER( cps3_sound_r )
{
	stream_update(cps3_stream);

	if (offset < 0x80)
	{
		return chip.voice[offset / 8].regs[offset & 7] & mem_mask;
	}
	else if (offset == 0x80)
	{
		return chip.key << 16;
	}
	else
	{
		printf("Unk sound read : %x\n", offset);
		return 0;
	}
}
