// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,cam900
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

// constants
template<unsigned MaxVoices, unsigned Divider>
constexpr unsigned setapcm_device<MaxVoices, Divider>::MAX_VOICES;
template<unsigned MaxVoices, unsigned Divider>
constexpr unsigned setapcm_device<MaxVoices, Divider>::CLOCK_DIVIDER;

// device type definition
DEFINE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device,   "nile_sound",   "Seta ST-0026 NiLe (Sound)")
DEFINE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device, "st0032_sound", "Seta ST-0032 (Sound)")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  setapcm_device - constructor
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
setapcm_device<MaxVoices, Divider>::setapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_keyctrl(0)
{
}

nile_sound_device::nile_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: setapcm_device<8, 160>(mconfig, NILE_SOUND, tag, owner, clock)
{
}

st0032_sound_device::st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: setapcm_device<16, 384>(mconfig, ST0032_SOUND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::device_start()
{
	// allocate stream
	m_stream = stream_alloc(0, 2, clock() / CLOCK_DIVIDER);

	// set host device to each voices
	for (auto & elem : m_voice)
		elem.m_host = this;

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
	save_item(STRUCT_MEMBER(m_voice, m_out));

	save_item(NAME(m_keyctrl));
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCK_DIVIDER);
}

//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::sound_stream_update(sound_stream &stream)
{
	for (int sampleind = 0; sampleind < stream.samples(); sampleind++)
	{
		for (int v = 0; v < MAX_VOICES; v++)
		{
			// check if voice is activated
			if (m_voice[v].update())
			{
				stream.add_int(0, sampleind, (m_voice[v].m_out * m_voice[v].m_vol_l) >> 16, 32768 * MAX_VOICES);
				stream.add_int(1, sampleind, (m_voice[v].m_out * m_voice[v].m_vol_r) >> 16, 32768 * MAX_VOICES);
			}
		}
	}
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::rom_bank_pre_change()
{
	m_stream->update();
}

//-------------------------------------------------
//  update - update single voice
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
bool setapcm_device<MaxVoices, Divider>::voice_t::update()
{
	if (m_keyon)
	{
		// fetch sample
		m_out = s16(s8(m_host->read_byte(m_pos))) << 8;

		// advance
		m_frac += m_freq;
		m_pos += m_frac >> 12;
		m_frac &= 0xfff;

		// stop if we're at the end
		if (m_lponce)
		{
			// we've looped once, check loop end rather than sample end
			if (m_pos >= m_lpend)
			{
				m_pos = m_lpstart;
			}
		}
		else
		{
			// not looped yet, check sample end
			if (m_pos >= m_end)
			{
				// code at 11d8c:
				// if bit 2 (0x4) is set, check if loop start = loop end.
				// if they are equal, clear bit 0 and don't set the loop start/end
				// registers in the NiLe.  if they aren't, set bit 0 and set
				// the loop start/end registers in the NiLe.
				// TODO: ST-0032 has same behavior?
				if (BIT(m_flags, 0))
				{
					m_pos = m_lpstart;
					m_lponce = true;
				}
				else
				{
					m_keyon = false;
				}
			}
		}
		return true;
	}
	// clear output
	m_out = 0;
	return false;
}

//-------------------------------------------------
//  keyon - set keyon flag for single voice
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::voice_t::keyon()
{
	m_keyon = true;
	m_pos = m_start;
	m_frac = 0;
	m_lponce = false;
}

//-------------------------------------------------
//  keyoff - set keyoff flag for single voice
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::voice_t::keyoff()
{
	m_keyon = false;
}

//-------------------------------------------------
//  reg_r - read single voice registers
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
u16 setapcm_device<MaxVoices, Divider>::voice_t::reg_r(offs_t reg)
{
	u16 ret = 0;
	switch (reg & 0xf)
	{
	case 0x02: // Current position LSB
		ret = m_pos & 0xffff;
		break;
	case 0x03: // Current position MSB
		ret = m_pos >> 16;
		break;
	case 0x05: // Flags
		ret = m_flags;
		break;
	case 0x06: // Frequency
		ret = m_freq;
		break;
	case 0x07: // Loop start position LSB
		ret = m_lpstart & 0xffff;
		break;
	case 0x09: // Loop start position MSB
		ret = m_lpstart >> 16;
		break;
	case 0x0a: // Loop end position LSB
		ret = m_lpend & 0xffff;
		break;
	case 0x0b: // Loop end position MSB
		ret = m_lpend >> 16;
		break;
	case 0x0c: // End position LSB
		ret = m_end & 0xffff;
		break;
	case 0x0d: // End position MSB
		ret = m_end >> 16;
		break;
	case 0x0e: // Right volume
		ret = m_vol_r;
		break;
	case 0x0f: // Left volume
		ret = m_vol_l;
		break;
	}
	return ret;
}

//-------------------------------------------------
//  reg_w - write single voice registers
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::voice_t::reg_w(offs_t reg, u16 data, u16 mem_mask)
{
	switch (reg & 0xf)
	{
	case 0x02: // Start position LSB, also affected current position?
		m_start = m_pos = (m_start & ~mem_mask) | (data & mem_mask);
		break;
	case 0x03: // Start position MSB
		m_start = m_pos = (m_start & ~(u32(mem_mask) << 16)) | (u32(data & mem_mask) << 16);
		break;
	case 0x05: // Flags
		COMBINE_DATA(&m_flags);
		break;
	case 0x06: // Frequency
		COMBINE_DATA(&m_freq);
		break;
	case 0x07: // Loop start position LSB
		m_lpstart = (m_lpstart & ~mem_mask) | (data & mem_mask);
		break;
	case 0x09: // Loop start position MSB
		m_lpstart = (m_lpstart & ~(u32(mem_mask) << 16)) | (u32(data & mem_mask) << 16);
		break;
	case 0x0a: // Loop end position LSB
		m_lpend = (m_lpend & ~mem_mask) | (data & mem_mask);
		break;
	case 0x0b: // Loop end position MSB
		m_lpend = (m_lpend & ~(u32(mem_mask) << 16)) | (u32(data & mem_mask) << 16);
		break;
	case 0x0c: // End position LSB
		m_end = (m_end & ~mem_mask) | (data & mem_mask);
		break;
	case 0x0d: // End position MSB
		m_end = (m_end & ~(u32(mem_mask) << 16)) | (u32(data & mem_mask) << 16);
		break;
	case 0x0e: // Right volume
		COMBINE_DATA(&m_vol_r);
		break;
	case 0x0f: // Left volume
		COMBINE_DATA(&m_vol_l);
		break;
	}
}

//-------------------------------------------------
//  snd_w - write each voice registers
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::snd_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();

	const int v = (offset >> 4);

	if (v >= MAX_VOICES)
		return;

	m_voice[v].reg_w(offset & 0xf, data, mem_mask);
	//logerror("v%02d: %04x & %04x to reg %02d (PC=%x)\n", v, data, mem_mask, r, machine().describe_context());
}


//-------------------------------------------------
//  snd_r - read each voice registers
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
u16 setapcm_device<MaxVoices, Divider>::snd_r(offs_t offset)
{
	const int v = (offset >> 4);

	if (v >= MAX_VOICES)
		return 0;

	m_stream->update();

	return m_voice[v].reg_r(offset & 0xf);
}


//-------------------------------------------------
//  key_w - set keyon/off flags for each voices
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
void setapcm_device<MaxVoices, Divider>::key_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 prev = m_keyctrl;

	m_stream->update();

	COMBINE_DATA(&m_keyctrl);

	//logerror("KEYCTRL: %04x -> %04x %s\n", prev, m_keyctrl, machine().describe_context());

	for (int v = 0; v < MAX_VOICES; v++)
	{
		if (BIT(m_keyctrl, v) && (!(BIT(prev, v)))) // keyon
		{
			m_voice[v].keyon();
		}
		else if ((!(BIT(m_keyctrl, v))) && BIT(prev, v)) // keyoff
		{
			m_voice[v].keyoff();
		}
	}
}


//-------------------------------------------------
//  key_r - get keyon/off status from each voices
//-------------------------------------------------

template<unsigned MaxVoices, unsigned Divider>
u16 setapcm_device<MaxVoices, Divider>::key_r()
{
	m_stream->update();
	return m_keyctrl;
}

// template class definition
template class setapcm_device<8, 160>;
template class setapcm_device<16, 384>;
