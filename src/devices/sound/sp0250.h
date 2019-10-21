// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SOUND_SP0250_H
#define MAME_SOUND_SP0250_H

#pragma once

class sp0250_device : public device_t, public device_sound_interface
{
public:
	sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto drq() { return m_drq.bind(); }

	DECLARE_WRITE8_MEMBER( write );
	uint8_t drq_r();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	int16_t m_amp;
	uint8_t m_pitch;
	uint8_t m_repeat;
	int m_pcount, m_rcount;
	int m_playing;
	uint32_t m_RNG;
	sound_stream * m_stream;
	int m_voiced;
	uint8_t m_fifo[15];
	int m_fifo_pos;
	devcb_write_line m_drq;

	struct
	{
		int16_t F, B;
		int16_t z1, z2;
	} m_filter[6];

	void load_values();
	TIMER_CALLBACK_MEMBER( timer_tick );
	emu_timer * m_tick_timer;
};

DECLARE_DEVICE_TYPE(SP0250, sp0250_device)

#endif // MAME_SOUND_SP0250_H
