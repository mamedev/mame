// license:BSD-3-Clause
// copyright-holders:cam900
#ifndef MAME_SOUND_HUC6230_H
#define MAME_SOUND_HUC6230_H

#pragma once

#include "sound/c6280.h"

class huc6230_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // anti-click, analog output stage not emulated

	huc6230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// sound inputs 0-1: unused (PSG), 2-3: line (CD-DA) left/right
	template <unsigned N> auto adpcm_update_cb() { return m_adpcm_update_cb[N].bind(); }

	// write only
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct adpcm_channel {
		int32_t         m_predictor;
		int32_t         m_step_index;
		int32_t         m_delta;
		uint8_t         m_delta_count;
		uint8_t         m_lvol;
		uint8_t         m_rvol;
	};

	static int32_t vca_step_time(int32_t level);
	void vca_set(int ch, u8 data);
	void vca_run(int ch, int32_t clocks);

	TIMER_CALLBACK_MEMBER(adpcm_timer);

	// internal state
	sound_stream *m_stream;
	emu_timer *m_adpcm_timer;
	required_device<c6280_device> m_psg;
	adpcm_channel m_adpcm_channel[2];
	uint8_t m_adpcm_control;
	float m_volume[0x40];
	uint32_t m_pcm_lvol;
	uint32_t m_pcm_rvol;

	// line input VCA: level 0-1023, the level it ramps towards, master clocks to the next step
	int32_t m_vca_level[2];
	int32_t m_vca_target[2];
	int32_t m_vca_wait[2];

	devcb_read8::array<2> m_adpcm_update_cb;
};

DECLARE_DEVICE_TYPE(HuC6230, huc6230_device)

#endif // MAME_SOUND_HUC6230_H
