// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/************************************
      Seta PCM emulation
      sound emulation by Tomasz Slanina
      based on ST-0016 emulation

      used by
	  - ST-0026 NiLe (srmp6, 8 voices)
	  - ST-0032 (jclub2, 16 voices)
	  - Capcom CPS3 sound hardware has similarity?

      Register Format (32 byte per voices)

      00-1f: Voice 0
      Offset Bit               Description
             fedcba98 76543210 
      04     xxxxxxxx xxxxxxxx Start position LSB
      06     xxxxxxxx xxxxxxxx Start position MSB
      0a     -------- ----xxx- Used but unknown
             -------- -----x-- See below for NiLe specific? notes
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
	  e0-ff: Voice 7

      100: Keyon/off, Bit 0-7 means Voice 0-7
      110: Used but unknown

      below for 16 voice configurations:
      100-11f: Voice 8
      120-13f: Voice 9
      ...
      1e0-1ff: Voice 15

      200: Keyon/off, Bit 0-15 means Voice 0-15
      210: Used but unknown

      Other registers are unknown/unused

      TODO:
      - Verify loop and flag bit behavior from real hardware

************************************/

#include "emu.h"
#include "setapcm.h"

DEFINE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device,   "nile_sound",   "Seta ST-0026 NiLe (Sound)")
DEFINE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device, "st0032_sound", "Seta ST-0032 (Sound)")

setapcm_device::setapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_ctrl(0)
{
}

nile_sound_device::nile_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: setapcm_device(mconfig, NILE_SOUND, tag, owner, clock)
{
	m_max_voices = 8;
}

st0032_sound_device::st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: setapcm_device(mconfig, ST0032_SOUND, tag, owner, clock)
{
	m_max_voices = 16;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void setapcm_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock()); // TODO: verify from real hardware

	save_item(STRUCT_MEMBER(m_voice, m_start));
	save_item(STRUCT_MEMBER(m_voice, m_flags));
	save_item(STRUCT_MEMBER(m_voice, m_freq));
	save_item(STRUCT_MEMBER(m_voice, m_lpstart));
	save_item(STRUCT_MEMBER(m_voice, m_lpend));
	save_item(STRUCT_MEMBER(m_voice, m_end));
	save_item(STRUCT_MEMBER(m_voice, m_vol_r));
	save_item(STRUCT_MEMBER(m_voice, m_vol_l));
	save_item(STRUCT_MEMBER(m_voice, m_pos));
	save_item(STRUCT_MEMBER(m_voice, m_frac));
	save_item(STRUCT_MEMBER(m_voice, m_lponce));
	save_item(STRUCT_MEMBER(m_voice, m_keyon));
	save_item(NAME(m_sound_regs));
	save_item(NAME(m_ctrl));
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void setapcm_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
}

//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void setapcm_device::rom_bank_updated()
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void setapcm_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
	outputs[1].fill(0);

	for (int sampleind = 0; sampleind < outputs[0].samples(); sampleind++)
	{
		for (int v = 0; v < m_max_voices; v++)
		{
			voice_t *voice = &m_voice[v];

			if (voice->m_keyon)
			{
				s16 sample = s16(s8(read_byte(voice->m_pos))) << 8;

				voice->m_frac += voice->m_freq;
				voice->m_pos += voice->m_frac >> 12;
				voice->m_frac &= 0xfff;

				// stop if we're at the end
				if (voice->m_lponce)
				{
					// we've looped once, check loop end rather than sample end
					if (voice->m_pos >= voice->m_lpend)
					{
						voice->m_pos = voice->m_lpstart;
					}
				}
				else
				{
					// not looped yet, check sample end
					if (voice->m_pos >= voice->m_end)
					{
						// code at 11d8c:
						// if bit 2 (0x4) is set, check if loop start = loop end.
						// if they are equal, clear bit 0 and don't set the loop start/end
						// registers in the NiLe.  if they aren't, set bit 0 and set
						// the loop start/end registers in the NiLe.
						// TODO: ST-0032 has same behavior?
						if (BIT(voice->m_flags, 0))
						{
							voice->m_pos = voice->m_lpstart;
							voice->m_lponce = true;
						}
						else
						{
							voice->m_keyon = false;
						}
					}
				}

				outputs[0].add_int(sampleind, (sample * voice->m_vol_l) >> 16, 32768 * m_max_voices);
				outputs[1].add_int(sampleind, (sample * voice->m_vol_r) >> 16, 32768 * m_max_voices);
			}
		}
	}
}


void setapcm_device::sndctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 ctrl = m_ctrl;

	m_stream->update();

	COMBINE_DATA(&m_ctrl);

//  logerror("CTRL: %04x -> %04x %s\n", ctrl, m_ctrl, machine().describe_context());

	u16 delta = ctrl ^ m_ctrl;
	for (int v = 0; v < m_max_voices; v++)
	{
		voice_t *voice = &m_voice[v];
		if (BIT(delta, v))
		{
			if (BIT(m_ctrl, v) && (!(BIT(ctrl, v)))) // keyon
			{
				voice->m_keyon = true;
				voice->m_pos = voice->m_start;
				voice->m_frac = 0;
				voice->m_lponce = false;
			}
			else if ((!(BIT(m_ctrl, v))) && BIT(ctrl, v)) // keyoff
			{
				voice->m_keyon = false;
			}
		}
	}
}


u16 setapcm_device::sndctrl_r()
{
	m_stream->update();
	return m_ctrl;
}


u16 setapcm_device::snd_r(offs_t offset)
{
	m_stream->update();

	int v = (offset >> 4);

	if (v >= m_max_voices)
		return m_sound_regs[offset];

	voice_t *voice = &m_voice[v];

	switch (offset & 0xf)
	{
	case 0x02: // Current position LSB
		return voice->m_pos & 0xffff;
		break;
	case 0x03: // Current position MSB
		return voice->m_pos >> 16;
		break;
	case 0x05: // Flags
		return voice->m_flags;
		break;
	case 0x06: // Frequency
		return voice->m_freq;
		break;
	case 0x07: // Loop start position LSB
		return voice->m_lpstart & 0xffff;
		break;
	case 0x09: // Loop start position MSB
		return voice->m_lpstart >> 16;
		break;
	case 0x0a: // Loop end position LSB
		return voice->m_lpend & 0xffff;
		break;
	case 0x0b: // Loop end position MSB
		return voice->m_lpend >> 16;
		break;
	case 0x0c: // End position LSB
		return voice->m_end & 0xffff;
		break;
	case 0x0d: // End position MSB
		return voice->m_end >> 16;
		break;
	case 0x0e: // Right volume
		return voice->m_vol_r;
		break;
	case 0x0f: // Left volume
		return voice->m_vol_l;
		break;
	}

	return m_sound_regs[offset];
}


void setapcm_device::snd_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();

	COMBINE_DATA(&m_sound_regs[offset]);

	int v = (offset >> 4);

	if (v >= m_max_voices)
		return;

	voice_t *voice = &m_voice[v];

	switch (offset & 0xf)
	{
	case 0x02: // Start position LSB, also affected current position?
		voice->m_start = voice->m_pos = (voice->m_start & 0xffff0000) | m_sound_regs[offset];
		break;
	case 0x03: // Start position MSB
		voice->m_start = voice->m_pos = (voice->m_start & 0x0000ffff) | (u32(m_sound_regs[offset]) << 16);
		break;
	case 0x05: // Flags
		voice->m_flags = m_sound_regs[offset];
		break;
	case 0x06: // Frequency
		voice->m_freq = m_sound_regs[offset];
		break;
	case 0x07: // Loop start position LSB
		voice->m_lpstart = (voice->m_lpstart & 0xffff0000) | m_sound_regs[offset];
		break;
	case 0x09: // Loop start position MSB
		voice->m_lpstart = (voice->m_lpstart & 0x0000ffff) | (u32(m_sound_regs[offset]) << 16);
		break;
	case 0x0a: // Loop end position LSB
		voice->m_lpend = (voice->m_lpend & 0xffff0000) | m_sound_regs[offset];
		break;
	case 0x0b: // Loop end position MSB
		voice->m_lpend = (voice->m_lpend & 0x0000ffff) | (u32(m_sound_regs[offset]) << 16);
		break;
	case 0x0c: // End position LSB
		voice->m_end = (voice->m_end & 0xffff0000) | m_sound_regs[offset];
		break;
	case 0x0d: // End position MSB
		voice->m_end = (voice->m_end & 0x0000ffff) | (u32(m_sound_regs[offset]) << 16);
		break;
	case 0x0e: // Right volume
		voice->m_vol_r = m_sound_regs[offset];
		break;
	case 0x0f: // Left volume
		voice->m_vol_l = m_sound_regs[offset];
		break;
	}
	//logerror("v%02d: %04x to reg %02d (PC=%x)\n", v, m_sound_regs[offset], r, machine().describe_context());
}
