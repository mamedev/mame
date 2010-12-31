/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/
#include "emu.h"
#include "streams.h"
#include "includes/cps3.h"

#define CPS3_VOICES		16

typedef struct _cps3_voice cps3_voice;
struct _cps3_voice
{
	UINT32 regs[8];
	UINT32 pos;
	UINT16 frac;
};

typedef struct _cps3_sound_state cps3_sound_state;
struct _cps3_sound_state
{
	sound_stream *stream;
	cps3_voice voice[CPS3_VOICES];
	UINT16     key;
	INT8*	   base;
};

INLINE cps3_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CPS3);

	return (cps3_sound_state *)downcast<legacy_device_base *>(device)->token();
}

static STREAM_UPDATE( cps3_stream_update )
{
	cps3_sound_state *state = get_safe_token(device);
	int i;

	// the actual 'user5' region only exists on the nocd sets, on the others it's allocated in the initialization.
	// it's a shared gfx/sound region, so can't be allocated as part of the sound device.
	state->base = (INT8*)cps3_user5region;

	/* Clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	for (i = 0; i < CPS3_VOICES; i ++)
	{
		if (state->key & (1 << i))
		{
			int j;

			/* TODO */
			#define SWAP(a) ((a >> 16) | ((a & 0xffff) << 16))

			cps3_voice *vptr = &state->voice[i];

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
						state->key &= ~(1 << i);
						break;
					}
				}

				sample = state->base[BYTE4_XOR_LE(start + pos)];
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
	cps3_sound_state *state = get_safe_token(device);

	/* Allocate the stream */
	state->stream = stream_create(device, 0, 2, device->clock() / 384, NULL, cps3_stream_update);
}

DEVICE_GET_INFO( cps3_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cps3_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cps3_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "CPS3 Custom");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


WRITE32_DEVICE_HANDLER( cps3_sound_w )
{
	cps3_sound_state *state = get_safe_token(device);

	stream_update(state->stream);

	if (offset < 0x80)
	{
		COMBINE_DATA(&state->voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		int i;
		UINT16 key = data >> 16;

		for (i = 0; i < CPS3_VOICES; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(state->key & (1 << i)))
			{
				state->voice[i].frac = 0;
				state->voice[i].pos = 0;
			}
		}
		state->key = key;
	}
	else
	{
		printf("Sound [%x] %x\n", offset, data);
	}
}

READ32_DEVICE_HANDLER( cps3_sound_r )
{
	cps3_sound_state *state = get_safe_token(device);
	stream_update(state->stream);

	if (offset < 0x80)
	{
		return state->voice[offset / 8].regs[offset & 7] & mem_mask;
	}
	else if (offset == 0x80)
	{
		return state->key << 16;
	}
	else
	{
		printf("Unk sound read : %x\n", offset);
		return 0;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(CPS3, cps3_sound);
