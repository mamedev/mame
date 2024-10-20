// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_SHARED_WSWANSOUND_H
#define MAME_SHARED_WSWANSOUND_H

#pragma once

#include "dirom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wswan_sound_device

class wswan_sound_device : public device_t,
	public device_sound_interface,
	public device_rom_interface<14,1>
{
public:
	wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void port_w(offs_t offset, u16 data, u16 mem_mask);
	u16 port_r(offs_t offset, u16 mem_mask);

protected:
	struct CHAN
	{
		CHAN() :
		freq(0),
		period(0),
		pos(0),
		vol_left(0),
		vol_right(0),
		on(0),
		offset(0),
		signal(0) { }

		u16  freq;
		u16  period;
		u32  pos;
		u8   vol_left;
		u8   vol_right;
		u8   on;
		u8   offset;
		u8   signal;
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	virtual void rom_bank_pre_change() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	int fetch_sample(int channel, int offset);

	sound_stream *m_channel;
	CHAN m_audio[4];
	s8   m_sweep_step;
	u32  m_sweep_time;
	u32  m_sweep_count;
	u8   m_noise_type;
	u8   m_noise_reset;
	u8   m_noise_enable;
	u8   m_noise_output;
	u16  m_sample_address;
	u8   m_audio2_voice;
	u8   m_audio3_sweep;
	u8   m_audio4_noise;
	u8   m_mono;
	u8   m_output_volume;
	u8   m_external_stereo;
	u8   m_external_speaker;
	u16  m_noise_shift;
	u8   m_master_volume;
	u8   m_system_volume = 0;
};

DECLARE_DEVICE_TYPE(WSWAN_SND, wswan_sound_device)

#endif // MAME_SHARED_WSWANSOUND_H
