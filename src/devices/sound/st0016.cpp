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

	// Sample decode tables.  Linear: plain signed 8-bit.  Non-linear: two's complement code whose
	// 7-bit magnitude is a 3-bit exponent / 4-bit mantissa float with hidden bit (exponent 0 =
	// denormal), i.e. a mu-law-like expansion, selected by voice 7 reg $1f bit 1 (renju, srmp5,
	// gostop, koikois, nratechu and dcrown set it; mayjinsn, mayjisn2, speglsht, macs and jclub2
	// keep it clear).  Derived from PCB recordings of renju and mayjinsn/mayjisn2 - the exact
	// curve is still an approximation.
	for (int i = 0; i < 256; i++)
	{
		const s8 code = s8(i);
		const int mag = std::min(std::abs(int(code)), 127);
		const int e = mag >> 4, m = mag & 15;
		const int v = (e == 0) ? (m << 1) : ((m | 16) << e);   // 0..3968
		m_linear[i] = s16(code) << 8;
		m_nonlinear[i] = s16((code < 0 ? -v : v) << 3);        // +-31744
	}
	select_table(false);

	// Volume: 7-bit value with a roughly exponential law, ~6 dB per 8 steps (0 = mute), realised
	// here as a 4-bit exponent / 3-bit mantissa float: gain = (8 | (v & 7)) << (v >> 3), full scale
	// at 0x7f.  Fitted to PCB recordings of mayjisn2 and renju (frame-level spread vs.
	// the recording is minimal between 6 and 8 steps per 6 dB; a linear law leaves ~3 dB spread).
	for (int v = 0; v < 256; v++)
	{
		const int v7 = v & 0x7f;
		m_voltab[v] = (v7 == 0) ? 0 : (((8 | (v7 & 7)) << (v7 >> 3)) * 256) / (15 << 15);
	}
	for (auto &vc : m_voice)
		vc.m_voltab = m_voltab;

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


void st0016_device::select_table(bool nonlinear)
{
	for (auto &v : m_voice)
		v.m_table = nonlinear ? m_nonlinear : m_linear;
}

void st0016_device::device_post_load()
{
	select_table(BIT(m_voice[7].m_regs[0x1f], 1));
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void st0016_device::sound_stream_update(sound_stream &stream)
{
	for (int sampleind = 0; sampleind < stream.samples(); sampleind++)
	{
		for (int v = 0; v < 8; v++)
		{
			// check if voice is activated
			if (m_voice[v].update())
			{
				stream.add_int(0, sampleind, (m_voice[v].m_out * m_voice[v].m_vol_l) >> 8, 32768); // full scale per voice: the exponential volume law keeps typical levels ~13 dB down
				stream.add_int(1, sampleind, (m_voice[v].m_out * m_voice[v].m_vol_r) >> 8, 32768);
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
		// linear interpolation between the current and the next sample: real hardware
		// shows none of the images a nearest-sample fetch produces (verified against
		// PCB recordings of mayjinsn/mayjisn2, whose samples play at 5.6-16 kHz)
		const s32 a = fetch(m_pos), b = fetch(next_pos(m_pos));
		m_out = s16(a + (((b - a) * s32(m_frac & 0xffff)) >> 16));
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

u32 st0016_device::voice_t::next_pos(u32 pos) const
{
	const u32 next = pos + 1;
	if (m_lponce)
		return (next >= m_lpend) ? m_lpstart : next;
	if (next >= m_end)
		return BIT(m_flags, 0) ? m_lpstart : pos;
	return next;
}

//-------------------------------------------------
//  snd_r - read sound registers
//-------------------------------------------------

u8 st0016_device::snd_r(offs_t offset)
{
	if (offset < 0x100)
	{
		if (!machine().side_effects_disabled())
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
		if (offset == 0xff) // global control (voice 7 reg $1f): bit 1 = non-linear sample format
			select_table(BIT(data, 1));
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
		m_vol_l = m_voltab[data];
		break;
	case 0x15: // Right volume
		m_vol_r = m_voltab[data];
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
