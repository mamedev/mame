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
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	struct voice_t
	{
		voice_t(memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache &host) : m_host(host) { }

		s16 fetch(u32 pos) const { return m_table[m_host.read_byte(pos & 0x1fffff)]; }
		u32 next_pos(u32 pos) const; // position of the following sample, honouring the loop

		bool update();

		u8 reg_r(offs_t offset);
		void reg_w(offs_t offset, u8 data, int voice);

		memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache &m_host; // host device
		const s16 *m_table = nullptr; // sample byte -> 16-bit value (device's linear or non-linear table)
		u8 m_regs[0x20] = {0};   // 32 registers per voices
		u32 m_start     = 0;     // Start position
		u32 m_end       = 0;     // End position
		u32 m_lpstart   = 0;     // Loop start position
		u32 m_lpend     = 0;     // Loop end position
		u16 m_freq      = 0;     // Frequency (.16 fixed point)
		s32 m_vol_l     = 0;     // Left volume (gain x256)
		s32 m_vol_r     = 0;     // Right volume (gain x256)
		const s32 *m_voltab = nullptr;
		u8 m_flags      = 0;     // Flags
		u32 m_pos       = 0;     // Current position
		u32 m_frac      = 0;     // Position fraction
		bool m_lponce   = false; // Is looped once?
		s16 m_out       = 0;     // output value
	};

	memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	sound_stream *m_stream;
	voice_t m_voice[8];           // 8 Voice engines
	s16 m_linear[256];            // sample decode tables, selected by voice 7 reg $1f bit 1
	s16 m_nonlinear[256];
	void select_table(bool nonlinear);
	s32 m_voltab[256];            // volume register -> gain (x256)
};

DECLARE_DEVICE_TYPE(ST0016, st0016_device)

#endif // MAME_SOUND_ST0016_H
