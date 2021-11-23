// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)
***************************************************************************/

#ifndef MAME_AUDIO_GT913_H
#define MAME_AUDIO_GT913_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_sound_device

class gt913_sound_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	gt913_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void data_w(offs_t offset, u16 data);
	u16 data_r(offs_t offset);
	void command_w(u16 data);
	u16 status_r();

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	memory_access<21, 1, 0, ENDIANNESS_BIG>::cache m_cache;
	required_region_ptr<u16> m_rom;

	sound_stream *m_stream;

	u8 m_gain;
	u16 m_data[3];

	static const u8 exp_2_to_3[4];
	static const s8 sample_7_to_8[128];

	struct voice_t
	{
		bool m_enable;

		u32 m_addr_start;
		u32 m_addr_end;
		u32 m_addr_loop;

		u32 m_addr_current;
		u32 m_addr_frac, m_pitch;

		s16 m_sample, m_sample_at_loop;
		u8 m_exp, m_exp_at_loop;

		u32 m_volume_current, m_volume_target;
		u32 m_volume_rate;
		bool m_volume_end;

		u8 m_balance[2];
		u8 m_gain;
	};

	void mix_sample(voice_t& voice, s32& left, s32& right);
	void update_sample(voice_t& voice);

	voice_t m_voices[24];
};

// device type definition
DECLARE_DEVICE_TYPE(GT913_SOUND, gt913_sound_device)

#endif // MAME_AUDIO_GT913_H
