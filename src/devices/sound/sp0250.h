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
	void set_pwm_mode() { m_pwm_mode = true; }

	void write(uint8_t data);
	int drq_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	TIMER_CALLBACK_MEMBER(delayed_stream_update) { m_stream->update(); }

private:
	// internal helpers
	int8_t next();
	void load_values();

	// state for each filter
	class filter
	{
	public:
		filter() : z1(0), z2(0) { }
		void reset() { z1 = z2 = 0; }
		int16_t apply(int16_t in)
		{
			int16_t z0 = in + ((z1 * F) >> 8) + ((z2 * B) >> 9);
			z2 = z1;
			z1 = z0;
			return z0;
		}
		int16_t F, B;
		int16_t z1, z2;
	};

	// timer for stream updates
	emu_timer *m_timer;

	// PWM state
	bool m_pwm_mode;
	uint8_t m_pwm_index;
	uint8_t m_pwm_count;
	uint32_t m_pwm_counts;

	// LPC state
	bool m_voiced;
	int16_t m_amp;
	uint16_t m_lfsr;
	uint8_t m_pitch;
	uint8_t m_pcount;
	uint8_t m_repeat;
	uint8_t m_rcount;
	filter m_filter[6];

	// FIFO state
	uint8_t m_fifo[15];
	uint8_t m_fifo_pos;

	// external interfacing
	sound_stream *m_stream;
	devcb_write_line m_drq;
};

DECLARE_DEVICE_TYPE(SP0250, sp0250_device)

#endif // MAME_SOUND_SP0250_H
