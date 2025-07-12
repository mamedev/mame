// license: BSD-3-Clause
// copyright-holders: Devin Acker

/***************************************************************************
    Texas Instruments CF61909 "DEVO"

    This is the sound and mapper ASIC used in the Jaminator.
    It generates 8 channels of PCM at ~44.5 kHz, and also handles all
    ROM access and clock generation for the 8039 MCU.

***************************************************************************/

#include "emu.h"
#include "cf61909.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(CF61909, cf61909_device, "cf61909", "Texas Instruments CF61909 (DEVO)")

cf61909_device::cf61909_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CF61909, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_sample_clock(*this, "sample_clock")
{
}

/**************************************************************************/
void cf61909_device::device_add_mconfig(machine_config &config)
{
	// DEVO outputs a brief low pulse every 4 samples (~11.127 kHz), which the Jaminator MCU uses
	// for syncing to the sample rate when updating sound registers
	CLOCK(config, m_sample_clock, DERIVED_CLOCK(1, CLOCKS_PER_SAMPLE * 4));
	m_sample_clock->set_duty_cycle(1.0 - (16.0 / CLOCKS_PER_SAMPLE));
}

/**************************************************************************/
void cf61909_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / CLOCKS_PER_SAMPLE);

	save_item(NAME(m_data_offset));

	save_item(STRUCT_MEMBER(m_voice, m_regs));
	save_item(STRUCT_MEMBER(m_voice, m_start));
	save_item(STRUCT_MEMBER(m_voice, m_loop));
	save_item(STRUCT_MEMBER(m_voice, m_pos));
	save_item(STRUCT_MEMBER(m_voice, m_pitch));
	save_item(STRUCT_MEMBER(m_voice, m_pitch_counter));
	save_item(STRUCT_MEMBER(m_voice, m_volume));
}

/**************************************************************************/
void cf61909_device::device_reset()
{
	std::fill(m_voice.begin(), m_voice.end(), voice_t());

	m_data_offset = 0;
}

/**************************************************************************/
void cf61909_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
}

/**************************************************************************/
void cf61909_device::rom_bank_pre_change()
{
	m_stream->update();
}

/**************************************************************************/
u8 cf61909_device::read(offs_t offset)
{
	return read_byte(m_data_offset | (offset & 0xff));
}

/**************************************************************************/
void cf61909_device::write(offs_t offset, u8 data)
{
	voice_t &voice = m_voice[BIT(offset, 4, 3)];
	const u8 reg = offset & 0xf;

	m_stream->update();
	voice.m_regs[reg] = data;

	switch (reg)
	{
	case 0x1: // position lsb
		voice.m_pos = (voice.m_regs[0x2] << 8) | data;
		break;

	case 0x2: // pitch / position msb
		break;

	case 0x3: // pitch lsb
		voice.m_pitch = (voice.m_regs[0x2] << 8) | data;
		break;

	case 0x4: // volume low nibble
		voice.m_volume = (voice.m_regs[0xc] << 4) | (data & 0xf);
		break;

	case 0x5: // program bank (TODO)
		break;

	case 0x6: // data bank
		m_data_offset = (data & 0x7f) << 8;
		if (BIT(data, 7))
			m_data_offset |= 0x20000; // cartridge memory
		break;

	case 0x9: // sample start lsb
		voice.m_start = (voice.m_regs[0xa] << 10) | (data << 2);
		break;

	case 0xa: // sample start / loop msb
		break;

	case 0xb: // sample loop lsb
		voice.m_loop = (voice.m_regs[0xa] << 8) | data;
		break;

	case 0xc: // volume high nibble
		break;

	default:
		logerror("%s: unknown register write %02x = %02x\n", machine().describe_context(), offset & 0xff, data);
		break;
	}
}

/**************************************************************************/
void cf61909_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 sample = 0;

		for (voice_t &voice : m_voice)
		{
			if (!voice.m_pitch) continue;

			s16 data = read_byte(voice.m_start + voice.m_pos);
			if (!data)
			{
				voice.m_pos += voice.m_loop;
				data = read_byte(voice.m_start + voice.m_pos);
			}
			sample += (data - 0x80) * voice.m_volume;

			voice.m_pitch_counter += voice.m_pitch;
			voice.m_pos += (voice.m_pitch_counter >> 14);
			voice.m_pitch_counter &= 0x3fff;
		}

		// Jaminator patent shows 10-bit sampling, assume that's actually true
		stream.put_int_clamp(0, i, sample >> 9, 1 << 9);
	}
}
