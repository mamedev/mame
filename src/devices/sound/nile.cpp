// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
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


const device_type NILE = &device_creator<nile_device>;

nile_device::nile_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NILE, "NiLe", tag, owner, clock, "nile", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_sound_ram(*this, DEVICE_SELF),
		m_ctrl(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nile_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void nile_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT8 *sound_ram = &m_sound_ram[0];
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
		slot = &m_sound_regs[v * 16];

		if (m_ctrl&(1<<v))
		{
			mixp = &mix[0];

			sptr = slot[NILE_REG_SPTR_HI]<<16 | slot[NILE_REG_SPTR_LO];
			eptr = slot[NILE_REG_EPTR_HI]<<16 | slot[NILE_REG_EPTR_LO];

			freq=slot[NILE_REG_FREQ]*14;
			lsptr = slot[NILE_REG_LSPTR_HI]<<16 | slot[NILE_REG_LSPTR_LO];
			leptr = slot[NILE_REG_LEPTR_HI]<<16 | slot[NILE_REG_LEPTR_LO];

			for (snum = 0; snum < samples; snum++)
			{
				sample = sound_ram[sptr + m_vpos[v]]<<8;

				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_R]) >> 16;
				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_L]) >> 16;

				m_frac[v] += freq;
				m_vpos[v] += m_frac[v]>>16;
				m_frac[v] &= 0xffff;

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
						// code at 11d8c:
						// if bit 2 (0x4) is set, check if loop start = loop end.
						// if they are equal, clear bit 0 and don't set the loop start/end
						// registers in the NiLe.  if they aren't, set bit 0 and set
						// the loop start/end registers in the NiLe.
						if ((slot[NILE_REG_FLAGS] & 0x5) == 0x5)
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
	for (i = 0; i < samples; i++)
	{
		outputs[0][i] = (*mixp++)>>4;
		outputs[1][i] = (*mixp++)>>4;
	}
}


WRITE16_MEMBER( nile_device::nile_sndctrl_w )
{
	UINT16 ctrl=m_ctrl;

	m_stream->update();

	COMBINE_DATA(&m_ctrl);

//  printf("CTRL: %04x -> %04x (PC=%x)\n", ctrl, m_ctrl, space.device().safe_pc());

	ctrl^=m_ctrl;
}


READ16_MEMBER( nile_device::nile_sndctrl_r )
{
	m_stream->update();
	return m_ctrl;
}


READ16_MEMBER( nile_device::nile_snd_r )
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


WRITE16_MEMBER( nile_device::nile_snd_w )
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

	//printf("v%02d: %04x to reg %02d (PC=%x)\n", v, m_sound_regs[offset], r, space.device().safe_pc());
}
