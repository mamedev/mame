// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)
***************************************************************************/

#ifndef MAME_MACHINE_GT913_SND_H
#define MAME_MACHINE_GT913_SND_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_sound_device

class gt913_sound_device : public device_t,
	public device_sound_interface,
	public device_rom_interface<21, 1, 0, ENDIANNESS_BIG>
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	sound_stream *m_stream;

	u8 m_gain;
	u16 m_data[3];

	static const u8 exp_2_to_3[4];
	static const s8 sample_7_to_8[128];
	static const u16 volume_ramp[17];

	struct voice_t
	{
		bool m_enable;

		u32 m_addr_start;
		u32 m_addr_end;
		u32 m_addr_loop;

		u32 m_addr_current;
		u32 m_addr_frac, m_pitch;

		s16 m_sample, m_sample_next;
		u8 m_exp;

		u16 m_volume_data;
		u32 m_volume_current, m_volume_target;
		u32 m_volume_rate;

		u8 m_balance[2];
		u8 m_gain;
	};

	void mix_sample(voice_t& voice, s64& left, s64& right);
	void update_envelope(voice_t& voice);
	void update_sample(voice_t& voice);

	voice_t m_voices[24];
};

// device type definition
DECLARE_DEVICE_TYPE(GT913_SOUND, gt913_sound_device)

#endif // MAME_MACHINE_GT913_SND_H
