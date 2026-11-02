// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SOUND_AD1859_H
#define MAME_SOUND_AD1859_H

#pragma once

// outputs 0 and 1 are left and right
class ad1859_device : public device_t, public device_sound_interface
{
public:
	enum input_format : u8 { RIGHT_JUSTIFIED, I2S, LEFT_JUSTIFIED, DSP };

	ad1859_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_input_format(input_format format) { m_format = format; }
	void set_word_length(u8 bits) { m_word_length = bits; }

	void sdata_w(int state) { m_sdata = bool(state); }
	void bclk_w(int state);
	void lrclk_w(int state);
	void cdata_w(int state) { m_cdata = bool(state); }
	void cclk_w(int state);
	void clatch_w(int state);
	void mute_w(int state);
	void pd_rst_w(int state);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr u32 FILTER_TAPS = 80;
	static constexpr u32 FILTER_PHASES = 512;
	static constexpr u32 OUTPUT_RATE = 192'000;

	static const float *filter();

	void reset_registers();
	void sample_w(u32 channel, u32 data);
	void update_gain(u32 channel);
	float target_gain(u32 channel) const;

	sound_stream *m_stream;
	u8 m_format;
	u8 m_word_length;
	bool m_sdata, m_bclk, m_lrclk;
	bool m_cdata, m_cclk, m_clatch;
	bool m_mute, m_pd_rst;
	u32 m_shift;
	u8 m_bits;
	u8 m_channel;
	bool m_sync;
	bool m_skip;
	bool m_receiving;
	u8 m_control_shift;
	u8 m_control_latch;
	bool m_control_pending;
	u8 m_control[2];
	bool m_gain_pending[2];
	u16 m_gain_samples[2];
	float m_gain[2];
	float m_previous[2];
	float m_history[2][2 * FILTER_TAPS];
	u8 m_history_pos[2];
	attotime m_sample_time[2];
	double m_sample_period[2];
};

DECLARE_DEVICE_TYPE(AD1859, ad1859_device)

#endif // MAME_SOUND_AD1859_H
