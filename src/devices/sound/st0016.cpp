// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina, David Haywood
/************************************
      Seta custom ST-0016 chip
      sound emulation by R. Belmont, Tomasz Slanina, and David Haywood

      TODO:
      - Verify keyon/off flag behavior
************************************/

#include "emu.h"
#include "st0016.h"

//#define VERBOSE 1
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(ST0016, st0016_device, "st0016", "Seta ST0016 (Audio)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  st0016_device - constructor
//-------------------------------------------------

st0016_device::st0016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ST0016, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 21) // shares character RAM area?
	, m_stream(nullptr)
	, m_voice{ m_cache, m_cache, m_cache, m_cache, m_cache, m_cache, m_cache, m_cache }
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0016_device::device_start()
{
	// Find our direct access
	space(0).cache(m_cache);

	// allocate stream
	m_stream = stream_alloc(0, 2, clock() / 128);

	save_item(STRUCT_MEMBER(m_voice, m_regs));
	save_item(STRUCT_MEMBER(m_voice, m_start));
	save_item(STRUCT_MEMBER(m_voice, m_end));
	save_item(STRUCT_MEMBER(m_voice, m_lpstart));
	save_item(STRUCT_MEMBER(m_voice, m_lpend));
	save_item(STRUCT_MEMBER(m_voice, m_freq));
	save_item(STRUCT_MEMBER(m_voice, m_vol_l));
	save_item(STRUCT_MEMBER(m_voice, m_vol_r));
	save_item(STRUCT_MEMBER(m_voice, m_flags));
	save_item(STRUCT_MEMBER(m_voice, m_pos));
	save_item(STRUCT_MEMBER(m_voice, m_frac));
	save_item(STRUCT_MEMBER(m_voice, m_lponce));
	save_item(STRUCT_MEMBER(m_voice, m_out));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void st0016_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
	outputs[1].fill(0);

	for (int sampleind = 0; sampleind < outputs[0].samples(); sampleind++)
	{
		for (int v = 0; v < 8; v++)
		{
			// check if voice is activated
			if (m_voice[v].update())
			{
				outputs[0].add_int(sampleind, (m_voice[v].m_out * m_voice[v].m_vol_l) >> 8, 32768 << 4);
				outputs[1].add_int(sampleind, (m_voice[v].m_out * m_voice[v].m_vol_r) >> 8, 32768 << 4);
			}
		}
	}
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector st0016_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

//-------------------------------------------------
//  update - update single voice
//-------------------------------------------------

bool st0016_device::voice_t::update()
{
	if (m_flags & 0x06) // TODO: keyon flag?
	{
		m_out = s16(s8(m_host.read_byte(m_pos & 0x1fffff))) << 8;
		m_frac += m_freq;
		m_pos += (m_frac >> 16);
		m_frac &= 0xffff;

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
				if (BIT(m_flags, 0))  // loop?
				{
					m_pos = m_lpstart;
					m_lponce = true;
				}
				else
				{
					m_flags = 0;
					m_pos = m_frac = 0;
				}
			}
		}
		return true;
	}
	else
	{
		m_out = 0;
		return false;
	}
}

//-------------------------------------------------
//  snd_r - read sound registers
//-------------------------------------------------

u8 st0016_device::snd_r(offs_t offset)
{
	if (offset < 0x100)
	{
		m_stream->update();
		return m_voice[offset >> 5].reg_r(offset & 0x1f);
	}
	return 0;
}

//-------------------------------------------------
//  snd_w - write sound registers
//-------------------------------------------------

void st0016_device::snd_w(offs_t offset, u8 data)
{
	if (offset < 0x100)
	{
		m_stream->update();
		m_voice[offset >> 5].reg_w(offset & 0x1f, data, offset >> 5);
	}
}

//-------------------------------------------------
//  reg_r - read single voice registers
//-------------------------------------------------

u8 st0016_device::voice_t::reg_r(offs_t offset)
{
	return m_regs[offset & 0x1f];
}

//-------------------------------------------------
//  reg_w - write single voice registers
//-------------------------------------------------

void st0016_device::voice_t::reg_w(offs_t offset, u8 data, int voice)
{
	offset &= 0x1f;

	m_regs[offset] = data;
	switch (offset)
	{
	case 0x00: // Start position bit 0-7
	case 0x01: // Start position bit 8-15
	case 0x02: // Start position bit 16-23
		m_start = (m_regs[0x02] << 16) | (m_regs[0x01] << 8) | m_regs[0x00];
		break;
	case 0x04: // Loop start position bit 0-7
	case 0x05: // Loop start position bit 8-15
	case 0x06: // Loop start position bit 16-23
		m_lpstart = (m_regs[0x06] << 16) | (m_regs[0x05] << 8) | m_regs[0x04];
		break;
	case 0x08: // Loop end position bit 0-7
	case 0x09: // Loop end position bit 8-15
	case 0x0a: // Loop end position bit 16-23
		m_lpend = (m_regs[0x0a] << 16) | (m_regs[0x09] << 8) | m_regs[0x08];
		break;
	case 0x0c: // End position bit 0-7
	case 0x0d: // End position bit 8-15
	case 0x0e: // End position bit 16-23
		m_end = (m_regs[0x0e] << 16) | (m_regs[0x0d] << 8) | m_regs[0x0c];
		break;
	case 0x10: // Frequency bit 0-7
	case 0x11: // Frequency bit 8-15
		m_freq = (m_regs[0x11] << 8) | m_regs[0x10];
		break;
	case 0x14: // Left volume
		m_vol_l = (char)data;
		break;
	case 0x15: // Right volume
		m_vol_r = (char)data;
		break;
	case 0x16:
		if (data != m_flags)
		{
			if (data != 0)
			{
				m_pos = m_start;
				m_frac = 0;
				m_lponce = false;

				/*
				LOG("Key on V%02d: st %06x-%06x lp %06x-%06x frq %x flg %x\n", voice,
				    m_start,
				    m_end,
				    m_lpstart,
				    m_lpend,
				    m_freq,
				    m_regs[0x16]);
				*/
			}
		}
		m_flags = m_regs[0x16];
		break;
	}
}
