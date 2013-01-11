/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/
#include "emu.h"
#include "includes/cps3.h"

#define CPS3_VOICES     16

struct cps3_voice
{
	UINT32 regs[8];
	UINT32 pos;
	UINT16 frac;
};

struct cps3_sound_state
{
	sound_stream *m_stream;
	cps3_voice m_voice[CPS3_VOICES];
	UINT16     m_key;
	INT8*      m_base;
};

INLINE cps3_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CPS3);

	return (cps3_sound_state *)downcast<cps3_sound_device *>(device)->token();
}

static STREAM_UPDATE( cps3_stream_update )
{
	cps3_sound_state *state = get_safe_token(device);
	int i;

	// the actual 'user5' region only exists on the nocd sets, on the others it's allocated in the initialization.
	// it's a shared gfx/sound region, so can't be allocated as part of the sound device.
	state->m_base = (INT8*)device->machine().driver_data<cps3_state>()->m_user5region;

	/* Clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	for (i = 0; i < CPS3_VOICES; i ++)
	{
		if (state->m_key & (1 << i))
		{
			int j;

			/* TODO */
			#define SWAP(a) ((a >> 16) | ((a & 0xffff) << 16))

			cps3_voice *vptr = &state->m_voice[i];

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
						state->m_key &= ~(1 << i);
						break;
					}
				}

				sample = state->m_base[BYTE4_XOR_LE(start + pos)];
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
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 2, device->clock() / 384, NULL, cps3_stream_update);
}

WRITE32_DEVICE_HANDLER( cps3_sound_w )
{
	cps3_sound_state *state = get_safe_token(device);

	state->m_stream->update();

	if (offset < 0x80)
	{
		COMBINE_DATA(&state->m_voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		int i;
		UINT16 key = data >> 16;

		for (i = 0; i < CPS3_VOICES; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(state->m_key & (1 << i)))
			{
				state->m_voice[i].frac = 0;
				state->m_voice[i].pos = 0;
			}
		}
		state->m_key = key;
	}
	else
	{
		// during boot: Sound [84] 230000
		logerror("Sound [%x] %x\n", offset, data);
	}
}

READ32_DEVICE_HANDLER( cps3_sound_r )
{
	cps3_sound_state *state = get_safe_token(device);
	state->m_stream->update();

	if (offset < 0x80)
	{
		return state->m_voice[offset / 8].regs[offset & 7] & mem_mask;
	}
	else if (offset == 0x80)
	{
		return state->m_key << 16;
	}
	else
	{
		logerror("Unk sound read : %x\n", offset);
		return 0;
	}
}


const device_type CPS3 = &device_creator<cps3_sound_device>;

cps3_sound_device::cps3_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CPS3, "CPS3 Custom", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(cps3_sound_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cps3_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cps3_sound_device::device_start()
{
	DEVICE_START_NAME( cps3_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cps3_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
