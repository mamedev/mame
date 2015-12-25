// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/

#include "emu.h"
#include "cps3.h"


// device type definition
const device_type CPS3 = &device_creator<cps3_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cps3_sound_device - constructor
//-------------------------------------------------

cps3_sound_device::cps3_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CPS3, "CPS3 Audio Custom", tag, owner, clock, "cps3_custom", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_key(0),
		m_base(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cps3_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, clock() / 384);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cps3_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	/* Clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	for (int i = 0; i < 16; i ++)
	{
		if (m_key & (1 << i))
		{
			/* note: unmarked bits are presumed to have no function (always 0)
			0  -
			1  xxxxxxxx xxxxxxxx  -------- --------  start address low
			   -------- --------  xxxxxxxx xxxxxxxx  start address high
			2  -------- --------  -------- -------x  loop enable
			3  xxxxxxxx xxxxxxxx  -------- --------  frequency
			   -------- --------  xxxxxxxx xxxxxxxx  loop address low
			4  -------- --------  xxxxxxxx xxxxxxxx  loop address high
			5  xxxxxxxx xxxxxxxx  -------- --------  end address low (*)
			   -------- --------  xxxxxxxx xxxxxxxx  end address high
			6  xxxxxxxx xxxxxxxx  -------- --------  end address low (*)
			   -------- --------  xxxxxxxx xxxxxxxx  end address high
			7  xxxxxxxx xxxxxxxx  -------- --------  volume right (signed?)
			   -------- --------  xxxxxxxx xxxxxxxx  volume left (signed?)

			(*) reg 5 and 6 are always the same. One of them probably means loop-end address,
			    but we won't know which until we do tests on real hw.
			*/
			cps3_voice *vptr = &m_voice[i];

			UINT32 start = (vptr->regs[1] >> 16 & 0x0000ffff) | (vptr->regs[1] << 16 & 0xffff0000);
			UINT32 end   = (vptr->regs[5] >> 16 & 0x0000ffff) | (vptr->regs[5] << 16 & 0xffff0000);
			UINT32 loop  = (vptr->regs[3] & 0x0000ffff) | (vptr->regs[4] << 16 & 0xffff0000);
			bool loop_enable = (vptr->regs[2] & 1) ? true : false;
			UINT32 step  = vptr->regs[3] >> 16 & 0xffff;

			INT16 vol_l = (vptr->regs[7] & 0xffff);
			INT16 vol_r = (vptr->regs[7] >> 16 & 0xffff);

			UINT32 pos = vptr->pos;
			UINT32 frac = vptr->frac;

			/* TODO */
			start -= 0x400000;
			end -= 0x400000;
			loop -= 0x400000;

			/* Go through the buffer and add voice contributions */
			for (int j = 0; j < samples; j++)
			{
				INT32 sample;

				pos += (frac >> 12);
				frac &= 0xfff;

				if ((start + pos) >= end)
				{
					if (loop_enable)
					{
						// loop
						pos = (pos + loop) - end;
					}
					else
					{
						// sample end (don't force key off)
						break;
					}
				}

				sample = m_base[BYTE4_XOR_LE(start + pos)];
				frac += step;

				outputs[0][j] += ((sample * vol_l) >> 8);
				outputs[1][j] += ((sample * vol_r) >> 8);
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}


WRITE32_MEMBER( cps3_sound_device::cps3_sound_w )
{
	m_stream->update();

	if (offset < 0x80)
	{
		COMBINE_DATA(&m_voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		assert((mem_mask & 0xffff0000) == 0xffff0000); // doesn't happen

		UINT16 key = data >> 16;

		for (int i = 0; i < 16; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(m_key & (1 << i)))
			{
				m_voice[i].frac = 0;
				m_voice[i].pos = 0;
			}
		}
		m_key = key;
	}
	else
	{
		// during boot: cps3_sound_w [84] = 230000
		logerror("cps3_sound_w [%x] = %x & %x\n", offset, data, mem_mask);
	}
}


READ32_MEMBER( cps3_sound_device::cps3_sound_r )
{
	m_stream->update();

	if (offset < 0x80)
	{
		return m_voice[offset / 8].regs[offset & 7] & mem_mask;
	}
	else if (offset == 0x80)
	{
		return m_key << 16;
	}
	else
	{
		logerror("cps3_sound_r unknown %x & %x\n", offset, mem_mask);
		return 0;
	}
}
