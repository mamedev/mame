// license:BSD-3-Clause
// copyright-holders: Devin Acker

/***************************************************************************
    Casio GT155 (HG51B155FD)
***************************************************************************/

#ifndef MAME_SOUND_GT155_H
#define MAME_SOUND_GT155_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class gt155_device : public device_t,
	public device_sound_interface,
	public device_rom_interface<23>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	gt155_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	static constexpr unsigned CLOCKS_PER_SAMPLE = 512;
	static constexpr unsigned ENV_SHIFT = 8;

	struct voice_t
	{
		u8 m_enable = 0;
		u8 m_format = 0;

		u32 m_addr = 0;
		u32 m_addr_frac = 0;
		u32 m_addr_end = 0;
		u32 m_addr_loop = 0;
		u32 m_addr_loop_frac = 0;

		u32 m_pitch = 0;

		u32 m_filter_gain = 0;
		u16 m_filter = 0;
		s32 m_filter_out = 0;
		u16 m_filter_unk = 0;

		s16 m_sample_last = 0;
		s16 m_sample = 0;

		u32 m_env_current = 0;
		u32 m_env_target = 0;
		u16 m_env_level = 0;
		u8  m_env_scale = 0;
		u16 m_env_rate = 0;

		u8 m_balance[2] = {0};
		u8 m_dsp_send[2] = {0};

		void update_envelope();
	};

	void mix_sample(voice_t &voice, s64 &left, s64 &right);
	void update_sample(voice_t &voice);

	u16 reg16(u8 num) const;
	u32 reg24(u8 num) const;
	u32 reg32(u8 num) const;

	void voice_command(u8 data);

	sound_stream *m_stream;

	u16 m_volume[0x800];

	u8 m_data[6];
	u32 m_dsp_data[128];
	u32 m_rom_addr;

	voice_t m_voices[32];
};

DECLARE_DEVICE_TYPE(GT155, gt155_device)

#endif // MAME_SOUND_GT155_H
