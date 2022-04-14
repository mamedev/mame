// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina, David Haywood
#ifndef MAME_SOUND_ST0016_H
#define MAME_SOUND_ST0016_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> st0016_device

class st0016_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	st0016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 snd_r(offs_t offset);
	void snd_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	struct voice_t
	{
		voice_t(memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache &host) : m_host(host) { }

		bool update();

		u8 reg_r(offs_t offset);
		void reg_w(offs_t offset, u8 data, int voice);

		memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache &m_host; // host device
		u8 m_regs[0x20] = {0};   // 32 registers per voices
		u32 m_start     = 0;     // Start position
		u32 m_end       = 0;     // End position
		u32 m_lpstart   = 0;     // Loop start position
		u32 m_lpend     = 0;     // Loop end position
		u16 m_freq      = 0;     // Frequency (.16 fixed point)
		char m_vol_l    = 0;     // Left volume
		char m_vol_r    = 0;     // Right volume
		u8 m_flags      = 0;     // Flags
		u32 m_pos       = 0;     // Current position
		u32 m_frac      = 0;     // Position fraction
		bool m_lponce   = false; // Is looped once?
		s16 m_out       = 0;     // output value
	};

	memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	sound_stream *m_stream;
	voice_t m_voice[8];           // 8 Voice engines
};

DECLARE_DEVICE_TYPE(ST0016, st0016_device)

#endif // MAME_SOUND_ST0016_H
