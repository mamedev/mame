// license:BSD-3-Clause
// copyright-holders:cam900
#ifndef MAME_SOUND_HUC6230_H
#define MAME_SOUND_HUC6230_H

#pragma once

#include "sound/okiadpcm.h"
#include "sound/c6280.h"

class huc6230_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // Incorrect ADPCM

	huc6230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto adpcm_update_cb() { return m_adpcm_update_cb[N].bind(); }
	auto vca_callback() { return m_vca_cb.bind(); }

	// write only
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct adpcm_channel {
		oki_adpcm_state m_adpcm;
		uint8_t         m_lvol;
		uint8_t         m_rvol;
		uint8_t         m_interpolate;
		uint8_t         m_playing;
		int32_t         m_prev_sample;
		int32_t         m_curr_sample;
		int32_t         m_output;
		uint32_t        m_pos;
		uint8_t         m_input;
	};

	// internal state
	sound_stream *m_stream;
	emu_timer *m_adpcm_timer;
	required_device<c6280_device> m_psg;
	adpcm_channel m_adpcm_channel[2];
	uint32_t m_adpcm_freq;
	uint32_t m_pcm_lvol;
	uint32_t m_pcm_rvol;

	TIMER_CALLBACK_MEMBER(adpcm_timer);

	devcb_read8::array<2> m_adpcm_update_cb;
	devcb_write8 m_vca_cb;
};

DECLARE_DEVICE_TYPE(HuC6230, huc6230_device)

#endif // MAME_SOUND_HUC6230_H
