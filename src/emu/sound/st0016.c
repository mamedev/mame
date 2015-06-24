// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina, David Haywood
/************************************
      Seta custom ST-0016 chip
      sound emulation by R. Belmont, Tomasz Slanina, and David Haywood
************************************/

#include "emu.h"
#include "st0016.h"

#define VERBOSE (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


// device type definition
const device_type ST0016 = &device_creator<st0016_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  st0016_device - constructor
//-------------------------------------------------

st0016_device::st0016_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ST0016, "ST0016 (Audio)", tag, owner, clock, "st0016_audio", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_ram_read_cb(*this)
{
	memset(m_vpos, 0, sizeof(int)*8);
	memset(m_frac, 0, sizeof(int)*8);
	memset(m_lponce, 0, sizeof(int)*8);
	memset(m_regs, 0, sizeof(UINT8)*0x100);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0016_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);
	m_ram_read_cb.resolve_safe(0);

	save_item(NAME(m_vpos));
	save_item(NAME(m_frac));
	save_item(NAME(m_lponce));
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void st0016_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int v, i, snum;
	unsigned char *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;

	memset(mix, 0, sizeof(mix[0])*samples*2);

	for (v = 0; v < 8; v++)
	{
		slot = (unsigned char *)&m_regs[v * 32];

		if (slot[0x16] & 0x06)
		{
			mixp = &mix[0];

			sptr = slot[0x02]<<16 | slot[0x01]<<8 | slot[0x00];
			eptr = slot[0x0e]<<16 | slot[0x0d]<<8 | slot[0x0c];
			freq = slot[0x11]<<8 | slot[0x10];
			lsptr = slot[0x06]<<16 | slot[0x05]<<8 | slot[0x04];
			leptr = slot[0x0a]<<16 | slot[0x09]<<8 | slot[0x08];

			for (snum = 0; snum < samples; snum++)
			{
				sample = m_ram_read_cb((sptr + m_vpos[v]) & 0x1fffff) << 8;

				*mixp++ += (sample * (char)slot[0x14]) >> 8;
				*mixp++ += (sample * (char)slot[0x15]) >> 8;

				m_frac[v] += freq;
				m_vpos[v] += (m_frac[v]>>16);
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
						if (slot[0x16] & 0x01)  // loop?
						{
							m_vpos[v] = (lsptr - sptr);
							m_lponce[v] = 1;
						}
						else
						{
							slot[0x16] = 0;
							m_vpos[v] = m_frac[v] = 0;
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


READ8_MEMBER( st0016_device::st0016_snd_r )
{
	return m_regs[offset];
}

WRITE8_MEMBER( st0016_device::st0016_snd_w )
{
	int voice = offset/32;
	int reg = offset & 0x1f;
	int oldreg = m_regs[offset];
	int vbase = offset & ~0x1f;

	m_regs[offset] = data;

	if ((voice < 8) && (data != oldreg))
	{
		if ((reg == 0x16) && (data != 0))
		{
			m_vpos[voice] = m_frac[voice] = m_lponce[voice] = 0;

			LOG(("Key on V%02d: st %06x-%06x lp %06x-%06x frq %x flg %x\n", voice,
				m_regs[vbase+2]<<16   | m_regs[vbase+1]<<8   | m_regs[vbase+2],
				m_regs[vbase+0xe]<<16 | m_regs[vbase+0xd]<<8 | m_regs[vbase+0xc],
				m_regs[vbase+6]<<16   | m_regs[vbase+5]<<8   | m_regs[vbase+4],
				m_regs[vbase+0xa]<<16 | m_regs[vbase+0x9]<<8 | m_regs[vbase+0x8],
				m_regs[vbase+0x11]<<8 | m_regs[vbase+0x10],
				m_regs[vbase+0x16]));
		}
	}
}
