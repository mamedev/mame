// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_SOUND_SETAPCM_H
#define MAME_SOUND_SETAPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device)
DECLARE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device)

// ======================> setapcm_device

// TODO: unknown address bus width
template<unsigned MaxVoices, unsigned Divider>
class setapcm_device : public device_t,
					public device_sound_interface,
					public device_rom_interface<32>
{
public:

	//-------------------------------------------------
	//  snd_w - write each voice registers
	//-------------------------------------------------

	void snd_w(offs_t offset, u16 data, u16 mem_mask = ~0)
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

	u16 snd_r(offs_t offset)
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

	void key_w(offs_t offset, u16 data, u16 mem_mask = ~0)
	{
		u16 ctrl = m_ctrl;

		m_stream->update();

		COMBINE_DATA(&m_ctrl);

		//  logerror("CTRL: %04x -> %04x %s\n", ctrl, m_ctrl, machine().describe_context());

		u16 delta = ctrl ^ m_ctrl;
		for (int v = 0; v < MAX_VOICES; v++)
		{
			if (BIT(delta, v))
			{
				if (BIT(m_ctrl, v) && (!(BIT(ctrl, v)))) // keyon
				{
					m_voice[v].keyon();
				}
				else if ((!(BIT(m_ctrl, v))) && BIT(ctrl, v)) // keyoff
				{
					m_voice[v].keyoff();
				}
			}
		}
	}


	//-------------------------------------------------
	//  key_r - get keyon/off status from each voices
	//-------------------------------------------------

	u16 key_r()
	{
		m_stream->update();
		return m_ctrl;
	}

protected:
	setapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_sound_interface(mconfig, *this)
		, device_rom_interface(mconfig, *this)
		, m_stream(nullptr)
		, m_ctrl(0)
	{
	}

	// device-level overrides

	//-------------------------------------------------
	//  device_start - device-specific startup
	//-------------------------------------------------

	virtual void device_start() override
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

		save_item(NAME(m_ctrl));
	}

	//-------------------------------------------------
	//  device_clock_changed - called if the clock
	//  changes
	//-------------------------------------------------

	virtual void device_clock_changed() override
	{
		m_stream->set_sample_rate(clock() / CLOCK_DIVIDER);
	}

	// sound stream update overrides

	//-------------------------------------------------
	//  sound_stream_update - handle update requests
	//  for our sound stream
	//-------------------------------------------------

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		outputs[0].fill(0);
		outputs[1].fill(0);

		for (int sampleind = 0; sampleind < outputs[0].samples(); sampleind++)
		{
			for (int v = 0; v < MAX_VOICES; v++)
			{
				// check if voice is activated
				if (m_voice[v].update())
				{
					outputs[0].add_int(sampleind, (m_voice[v].m_out * m_voice[v].m_vol_l) >> 16, 32768 * MAX_VOICES);
					outputs[1].add_int(sampleind, (m_voice[v].m_out * m_voice[v].m_vol_r) >> 16, 32768 * MAX_VOICES);
				}
			}
		}
	}

	// device_rom_interface implementation

	//-------------------------------------------------
	//  rom_bank_updated - the rom bank has changed
	//-------------------------------------------------

	virtual void rom_bank_updated() override
	{
		m_stream->update();
	}

	static constexpr unsigned MAX_VOICES = MaxVoices;  // max voices
	static constexpr unsigned CLOCK_DIVIDER = Divider; // clock divider for generate output rate
private:
	struct voice_t
	{
		//-------------------------------------------------
		//  update - update single voice
		//-------------------------------------------------

		bool update()
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

		void keyon()
		{
			m_keyon = true;
			m_pos = m_start;
			m_frac = 0;
			m_lponce = false;
		}

		//-------------------------------------------------
		//  keyoff - set keyoff flag for single voice
		//-------------------------------------------------

		void keyoff()
		{
			m_keyon = false;
		}

		//-------------------------------------------------
		//  reg_r - read single voice registers
		//-------------------------------------------------

		u16 reg_r(offs_t reg)
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

		void reg_w(offs_t reg, u16 data, u16 mem_mask = ~0)
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

		device_rom_interface<32> *m_host; // host device
		u32 m_start = 0;                  // Start position
		u16 m_flags = 0;                  // Flags (Bit 0 = loop)
		u16 m_freq = 0;                   // Frequency (4.12 fixed point)
		u32 m_lpstart = 0;                // Loop start position
		u32 m_lpend = 0;                  // Loop end position
		u32 m_end = 0;                    // End position
		s32 m_vol_r = 0;                  // Right volume
		s32 m_vol_l = 0;                  // Left volume
		u32 m_pos = 0;                    // Current position
		u32 m_frac = 0;                   // Position fraction
		bool m_lponce = false;            // Is looped once?
		bool m_keyon = false;             // Keyon status
		s32 m_out = 0;                    // output value
	};

	sound_stream *m_stream;

	voice_t m_voice[MaxVoices];
	u16 m_ctrl;
};

// ======================> nile_sound_device

class nile_sound_device : public setapcm_device<8, 371>
{
public:
	nile_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: setapcm_device<8, 371>(mconfig, NILE_SOUND, tag, owner, clock)
	{
	}
};

// ======================> st0032_sound_device

class st0032_sound_device : public setapcm_device<16, 384>
{
public:
	st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: setapcm_device<16, 384>(mconfig, ST0032_SOUND, tag, owner, clock)
	{
	}
};

//**************************************************************************
//  EXTERNAL TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class setapcm_device<8, 371>;
extern template class setapcm_device<16, 384>;

#endif // MAME_SOUND_SETAPCM_H
