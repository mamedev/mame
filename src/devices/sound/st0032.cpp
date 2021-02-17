// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/************************************
      Seta custom ST-0032 chip (sound emulation)
      based on ST-0026 Nile emulation by Tomasz Slanina
      similar as Nile sound, but it's has 16 voices

      Register Format (32 byte per voices)

      00-1f: Voice 0
      Offset Bit               Description
             fedcba98 76543210 
      04     xxxxxxxx xxxxxxxx Start position LSB
      06     xxxxxxxx xxxxxxxx Start position MSB
      0a     -------- -----xx- Used but unknown
             -------- -------x Loop enable
      0c     xxxxxxxx xxxxxxxx Frequency
      0e     xxxxxxxx xxxxxxxx Loop Start position LSB
      12     xxxxxxxx xxxxxxxx Loop Start position MSB
      14     xxxxxxxx xxxxxxxx Loop End position LSB
      16     xxxxxxxx xxxxxxxx Loop End position MSB
      18     xxxxxxxx xxxxxxxx End position LSB
      1a     xxxxxxxx xxxxxxxx End position MSB
      1c     xxxxxxxx xxxxxxxx Right Volume
      1e     xxxxxxxx xxxxxxxx Left Volume

      20-3f: Voice 1
      ...
      1e0-1ff: Voice 15

      200: Keyon/off, Bit 0-15 means Voice 0-15
      210: Used but unknown

      Other registers are unknown/unused

      TODO:
      - Verify loop behavior from real hardware

************************************/

#include "emu.h"
#include "st0032.h"

enum
{
	ST0032_REG_UNK0=0,
	ST0032_REG_SPTR_TOP,
	ST0032_REG_SPTR_LO,
	ST0032_REG_SPTR_HI,
	ST0032_REG_UNK_4,
	ST0032_REG_FLAGS,
	ST0032_REG_FREQ,
	ST0032_REG_LSPTR_LO,
	ST0032_REG_UNK_8,
	ST0032_REG_LSPTR_HI,
	ST0032_REG_LEPTR_LO,
	ST0032_REG_LEPTR_HI,
	ST0032_REG_EPTR_LO,
	ST0032_REG_EPTR_HI,
	ST0032_REG_VOL_R,
	ST0032_REG_VOL_L
};


DEFINE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device, "st0032_sound", "Seta ST-0032 Sound Hardware")

st0032_sound_device::st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ST0032_SOUND, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_sound_ram(*this, DEVICE_SELF),
		m_ctrl(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0032_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 384);
	save_item(NAME(m_sound_regs));
	save_item(NAME(m_vpos));
	save_item(NAME(m_frac));
	save_item(NAME(m_lponce));
	save_item(NAME(m_ctrl));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void st0032_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	uint8_t *sound_ram = &m_sound_ram[0];
	int v, i, snum;
	uint16_t *slot;
	int32_t mix[4800*2];
	int32_t *mixp;
	int16_t sample;
	int sptr, eptr, freq, lsptr, leptr;

	lsptr=leptr=0;

	sound_assert(outputs[0].samples() * 2 < std::size(mix));
	std::fill_n(&mix[0], outputs[0].samples()*2, 0);

	for (v = 0; v < ST0032_VOICES; v++)
	{
		slot = &m_sound_regs[v * 16];

		if (m_ctrl&(1<<v))
		{
			mixp = &mix[0];

			sptr = slot[ST0032_REG_SPTR_HI]<<16 | slot[ST0032_REG_SPTR_LO];
			eptr = slot[ST0032_REG_EPTR_HI]<<16 | slot[ST0032_REG_EPTR_LO];

			freq  = slot[ST0032_REG_FREQ];
			lsptr = slot[ST0032_REG_LSPTR_HI]<<16 | slot[ST0032_REG_LSPTR_LO];
			leptr = slot[ST0032_REG_LEPTR_HI]<<16 | slot[ST0032_REG_LEPTR_LO];

			for (snum = 0; snum < outputs[0].samples(); snum++)
			{
				sample = sound_ram[sptr + m_vpos[v]]<<8;

				*mixp++ += (sample * (int32_t)slot[ST0032_REG_VOL_R]) >> 16;
				*mixp++ += (sample * (int32_t)slot[ST0032_REG_VOL_L]) >> 16;

				m_frac[v] += freq;
				m_vpos[v] += m_frac[v]>>12;
				m_frac[v] &= 0xfff;

				// stop if we're at the end
				if (m_lponce[v])
				{
					// we've looped once, check loop end rather than sample end
					if ((m_vpos[v] + sptr) >= leptr)
					{
						m_vpos[v] = (lsptr - sptr);
					}
				}
				else
				{
					// not looped yet, check sample end
					if ((m_vpos[v] + sptr) >= eptr)
					{
						// check loop flag
						if (slot[ST0032_REG_FLAGS] & 0x1)
						{
							m_vpos[v] = (lsptr - sptr);
							m_lponce[v] = 1;
						}
						else
						{
							m_ctrl &= ~(1<<v);
							m_vpos[v] = (eptr - sptr);
							m_frac[v] = 0;
						}
					}
				}
			}
		}
	}
	mixp = &mix[0];
	for (i = 0; i < outputs[0].samples(); i++)
	{
		outputs[0].put_int(i, *mixp++, 32768 * 16);
		outputs[1].put_int(i, *mixp++, 32768 * 16);
	}
}


void st0032_sound_device::st0032_sndctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t ctrl=m_ctrl;

	m_stream->update();

	COMBINE_DATA(&m_ctrl);

//  logerror("CTRL: %04x -> %04x %s\n", ctrl, m_ctrl, machine().describe_context());

	ctrl^=m_ctrl;
}


uint16_t st0032_sound_device::st0032_sndctrl_r()
{
	m_stream->update();
	return m_ctrl;
}


uint16_t st0032_sound_device::st0032_snd_r(offs_t offset)
{
	int reg=offset&0xf;

	m_stream->update();

	if(reg==2 || reg==3)
	{
		int slot=offset/16;
		int sptr = ((m_sound_regs[slot*16+3]<<16)|m_sound_regs[slot*16+2])+m_vpos[slot];

		if(reg==2)
		{
			return sptr&0xffff;
		}
		else
		{
			return sptr>>16;
		}
	}
	return m_sound_regs[offset];
}


void st0032_sound_device::st0032_snd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int v, r;

	m_stream->update();

	COMBINE_DATA(&m_sound_regs[offset]);

	v = offset / 16;
	r = offset % 16;

	if ((r == 2) || (r == 3))
	{
		m_vpos[v] = m_frac[v] = m_lponce[v] = 0;
	}

	//logerror("v%02d: %04x to reg %02d (PC=%x)\n", v, m_sound_regs[offset], r, machine().describe_context());
}
