// license:BSD-3-Clause
// copyright-holders:Devin Acker
#ifndef MAME_SOUND_L6009_H
#define MAME_SOUND_L6009_H

#pragma once

#include "machine/clock.h"

#include "dimemory.h"

class l6009_device : public device_t,
	public device_sound_interface,
	public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	enum
	{
		AS_ROM,
		AS_RAM
	};

	enum // output channels
	{
		OUT_MONO,
		OUT_STEREO_L = 8,
		OUT_STEREO_R,
		OUT_EFFECT,
		OUT_COUNT
	};

	l6009_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto sample_clock_cb() { return m_sample_clock.lookup()->signal_handler(); }

	u16 itp_r(offs_t offset);
	u16 flr_r(offs_t offset);

	void itp_w(offs_t offset, u16 data);
	void flr_w(offs_t offset, u16 data);

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned CLOCKS_PER_SAMPLE = 768;
	static constexpr unsigned ENV_SHIFT = 7; // TODO verify this for both vol and filter

	struct voice_t
	{
		u32 m_addr = 0;
		u32 m_addr_frac = 0;
		u32 m_loop = 0;
		u32 m_loop_frac = 0;
		u32 m_loop_end = 0;

		u16 m_pitch = 0;
		u8 m_pitch_unk = 0;

		u16 m_output_sel = 0;
		u8 m_stereo[2] = {0};

		u16 m_volume_env = 0;
		u32 m_volume_set = 0;
		u32 m_volume = 0;

		u16 m_filter_env = 0;
		u32 m_filter_set = 0;
		u32 m_filter = 0;

		s16 m_filter_out[3] = {0};
	};

	void update_env(u32 &val, u32 dest, u16 rate);

	required_device<clock_device> m_sample_clock;

	sound_stream *m_stream;

	address_space_config m_ram_config;
	address_space_config m_rom_config;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::specific m_ram;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_rom;

	voice_t m_voices[16];

	u32 m_ram_addr;
	u32 m_temp;
};

DECLARE_DEVICE_TYPE(L6009, l6009_device)

#endif // MAME_SOUND_L6009_H
