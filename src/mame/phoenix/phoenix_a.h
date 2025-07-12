// license:BSD-3-Clause
// copyright-holders:Richard Davies
#ifndef MAME_PHOENIX_PHOENIX_A_H
#define MAME_PHOENIX_PHOENIX_A_H

#pragma once

#include "sound/discrete.h"
#include "sound/tms36xx.h"


class phoenix_sound_device : public device_t, public device_sound_interface
{
public:
	phoenix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void control_a_w(uint8_t data);
	void control_b_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct c_state
	{
		int32_t counter = 0;
		int32_t level = 0;
	};

	struct n_state
	{
		int32_t counter = 0;
		int32_t polyoffs = 0;
		int32_t polybit = 0;
		int32_t lowpass_counter = 0;
		int32_t lowpass_polybit = 0;
	};

	// internal state
	c_state             m_c24_state;
	c_state             m_c25_state;
	n_state             m_noise_state;
	uint8_t             m_sound_latch_a = 0;
	sound_stream *      m_channel = nullptr;
	std::unique_ptr<uint32_t[]> m_poly18;
	required_device<discrete_device> m_discrete;
	required_device<tms36xx_device> m_tms;

	int update_c24(int samplerate);
	int update_c25(int samplerate);
	int noise(int samplerate);
};

DECLARE_DEVICE_TYPE(PHOENIX_SOUND, phoenix_sound_device)

DISCRETE_SOUND_EXTERN(phoenix_discrete);

#endif // MAME_PHOENIX_PHOENIX_A_H
