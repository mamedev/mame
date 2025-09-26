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

	void set_headphone_connected(bool headphone_connected) { m_headphone_connected = headphone_connected; }

	void hypervoice_dma_w(u8 data);

	void hypervoice_w(offs_t offset, u16 data, u16 mem_mask);
	u16 hypervoice_r(offs_t offset, u16 mem_mask);

	void port_w(offs_t offset, u16 data, u16 mem_mask);
	u16 port_r(offs_t offset, u16 mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	virtual void rom_bank_pre_change() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct channel_t
	{
		channel_t() :
		freq(0),
		period(2048),
		pos(0),
		vol_left(0),
		vol_right(0),
		on(false),
		offset(0),
		signal(0) { }

		u16  freq;
		u16  period;
		u32  pos;
		u8   vol_left;
		u8   vol_right;
		bool on;
		u8   offset;
		u8   signal;
	};

	struct hypervoice_t
	{
		hypervoice_t() :
		loutput(0),
		routput(0),
		linput(0),
		rinput(0),
		input_channel(false),
		volume(0),
		scale_mode(0),
		div(0),
		counter(0),
		enable(false),
		channel_mode(0) {}

		void stereo_input(u8 input);
		s32 scale(u8 input);

		s32  loutput;
		s32  routput;
		u8   linput;
		u8   rinput;
		bool input_channel;
		u8   volume;
		u8   scale_mode;
		u8   div;
		u8   counter;
		bool enable;
		u8   channel_mode;
	} m_hypervoice;

	u8 fetch_sample(int channel, int offset);

	sound_stream *m_channel;
	channel_t m_audio[4];
	s8        m_sweep_step;
	u32       m_sweep_time;
	u32       m_sweep_count;
	u8        m_noise_type;
	u8        m_noise_enable;
	u8        m_noise_output;
	u16       m_sample_address;
	u8        m_audio2_voice;
	u8        m_audio3_sweep;
	u8        m_audio4_noise;
	u8        m_speaker_enable;
	u8        m_speaker_volume;
	u8        m_headphone_enable;
	u8        m_headphone_connected;
	u16       m_noise_shift;
	u8        m_sample_volume;
	u8        m_system_volume = 0;
	u16       m_loutput;
	u16       m_routput;
};

DECLARE_DEVICE_TYPE(WSWAN_SND, wswan_sound_device)

#endif // MAME_SHARED_WSWANSOUND_H
