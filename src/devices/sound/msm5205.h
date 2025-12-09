// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_MSM5205_H
#define MAME_SOUND_MSM5205_H

#pragma once

/* an interface for the MSM5205 and similar chips */

class msm5205_device : public device_t, public device_sound_interface
{
public:
	// MSM5205 default master clock is 384KHz
	static constexpr int S96_3B = 0;     // prescaler 1/96(4KHz) , data 3bit
	static constexpr int S48_3B = 1;     // prescaler 1/48(8KHz) , data 3bit
	static constexpr int S64_3B = 2;     // prescaler 1/64(6KHz) , data 3bit
	static constexpr int SEX_3B = 3;     // VCK slave mode       , data 3bit
	static constexpr int S96_4B = 4;     // prescaler 1/96(4KHz) , data 4bit
	static constexpr int S48_4B = 5;     // prescaler 1/48(8KHz) , data 4bit
	static constexpr int S64_4B = 6;     // prescaler 1/64(6KHz) , data 4bit
	static constexpr int SEX_4B = 7;     // VCK slave mode       , data 4bit

	msm5205_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_prescaler_selector(int select)
	{
		m_s1 = BIT(select, 1);
		m_s2 = BIT(select, 0);
		m_bitwidth = (select & 4) ? 4 : 3;
	}
	auto vck_callback() { return m_vck_cb.bind(); }
	auto vck_legacy_callback() { return m_vck_legacy_cb.bind(); }

	// reset signal should keep for 2cycle of VCLK
	void reset_w(int state);

	// adpcmata is latched after vclk_interrupt callback
	void data_w(uint8_t data);

	// VCLK slave mode option
	// if VCLK and reset or data is changed at the same time,
	// call vclk_w after data_w and reset_w.
	void vclk_w(int state);

	// option , selected pin selector
	void playmode_w(int select);
	void s1_w(int state);
	void s2_w(int state);

protected:
	msm5205_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 dac_bits);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(toggle_vck);
	TIMER_CALLBACK_MEMBER(update_adpcm);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	void compute_tables();
	virtual int get_prescaler() const;
	virtual double adpcm_capture_divisor() const { return 6.0; }

	// internal state
	sound_stream *m_stream;     // number of stream system
	emu_timer *m_vck_timer;     // VCK callback timer
	emu_timer *m_capture_timer; // delay after VCK active edge for ADPCM input capture
	u8 m_data;                  // next adpcm data
	bool m_vck;                 // VCK signal
	bool m_reset;               // reset pin signal
	bool m_s1;                  // prescaler selector S1
	bool m_s2;                  // prescaler selector S2
	u8 m_bitwidth;              // bit width selector -3B/4B
	s32 m_signal;               // current ADPCM signal
	s32 m_step;                 // current ADPCM step
	u8 m_dac_bits;              // DAC output bits (10 for MSM5205, 12 for MSM6585)
	int m_diff_lookup[49*16];

	devcb_write_line m_vck_cb;
	devcb_write_line m_vck_legacy_cb;
};


class msm6585_device : public msm5205_device
{
public:
	/* MSM6585 default master clock is 640KHz */
	static constexpr int S160  = 4 + 8;  /* prescaler 1/160(4KHz), data 4bit */
	static constexpr int S40   = 5 + 8;  /* prescaler 1/40(16KHz), data 4bit */
	static constexpr int S80   = 6 + 8;  /* prescaler 1/80 (8KHz), data 4bit */
	static constexpr int S20   = 7 + 8;  /* prescaler 1/20(32KHz), data 4bit */

	msm6585_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual int get_prescaler() const override;
	virtual double adpcm_capture_divisor() const override { return 2.0; }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;
};


DECLARE_DEVICE_TYPE(MSM5205, msm5205_device)
DECLARE_DEVICE_TYPE(MSM6585, msm6585_device)

#endif // MAME_SOUND_MSM5205_H
