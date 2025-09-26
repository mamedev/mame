// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/

#include "emu.h"
#include "cps3_a.h"


// device type definition
DEFINE_DEVICE_TYPE(CPS3, cps3_sound_device, "cps3_custom", "CPS3 Custom Sound")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cps3_sound_device - constructor
//-------------------------------------------------

cps3_sound_device::cps3_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CPS3, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_key(0)
	, m_base(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cps3_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, clock() / 384);

	save_item(NAME(m_key));
	for (int i = 0; i < 16; i++)
	{
		save_item(NAME(m_voice[i].regs), i);
		save_item(NAME(m_voice[i].pos), i);
		save_item(NAME(m_voice[i].frac), i);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cps3_sound_device::sound_stream_update(sound_stream &stream)
{
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

			uint32_t start = (vptr->regs[1] >> 16 & 0x0000ffff) | (vptr->regs[1] << 16 & 0xffff0000);
			uint32_t end   = (vptr->regs[5] >> 16 & 0x0000ffff) | (vptr->regs[5] << 16 & 0xffff0000);
			uint32_t loop  = (vptr->regs[3] & 0x0000ffff) | (vptr->regs[4] << 16 & 0xffff0000);
			bool loop_enable = (vptr->regs[2] & 1) ? true : false;
			uint32_t step  = vptr->regs[3] >> 16 & 0xffff;

			int16_t vol_l = (vptr->regs[7] & 0xffff);
			int16_t vol_r = (vptr->regs[7] >> 16 & 0xffff);

			uint32_t pos = vptr->pos;
			uint32_t frac = vptr->frac;

			/* TODO */
			start -= 0x400000;
			end -= 0x400000;
			loop -= 0x400000;

			/* Go through the buffer and add voice contributions */
			for (int j = 0; j < stream.samples(); j++)
			{
				int32_t sample;

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

				stream.add_int(0, j, sample * vol_l, 32768 << 8);
				stream.add_int(1, j, sample * vol_r, 32768 << 8);
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}


void cps3_sound_device::sound_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_stream->update();

	if (offset < 0x80)
	{
		COMBINE_DATA(&m_voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		assert((mem_mask & 0xffff0000) == 0xffff0000); // doesn't happen

		uint16_t key = data >> 16;

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


uint32_t cps3_sound_device::sound_r(offs_t offset, uint32_t mem_mask)
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
