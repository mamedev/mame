// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_ADC_H
#define MAME_MACHINE_XAVIX_ADC_H

#pragma once

#include <array>

class xavix_adc_device : public device_t, public device_sound_interface
{
public:
	xavix_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void set_audio_channel(u8 channel) { set_audio_input(0, channel); }
	void set_legacy_control(bool enabled) { m_legacy_control = enabled; }
	void set_audio_input(unsigned input, u8 channel)
	{
		assert(input < m_audio_channel.size() && channel < m_audio_channel.size());
		m_audio_channel[input] = channel;
		if (m_audio_input_count <= input)
			m_audio_input_count = input + 1;
	}
	auto irq_callback() { return m_irq_cb.bind(); }

	template <unsigned Channel> auto input_callback() { return m_input_cb[Channel].bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr std::array<u16, 8> CONVERSION_PERIODS = { 255, 512, 1023, 11, 15, 31, 63, 127 };

	u8 data_r();
	u8 ctrl_r();
	void ctrl_w(u8 data);
	u8 rate_select_r();
	void rate_select_w(u8 data);

	u8 sample_input();
	u64 current_ticks() const;
	void advance(u64 clocks);
	void synchronize();
	void schedule_conversion();
	void update_irq();
	u8 status() const;
	TIMER_CALLBACK_MEMBER(conversion_tick);

	devcb_write_line m_irq_cb;
	devcb_read8::array<8> m_input_cb;

	u8 m_result;
	u8 m_converter_result;
	u8 m_converter_sample;
	u8 m_converter_state;
	u8 m_ctrl;
	u8 m_rate_select;
	u8 m_clock_phase;
	u16 m_countdown;
	bool m_request_pending;
	bool m_conversion_pulse;
	bool m_result_pending;
	bool m_capture_edge;
	bool m_legacy_control;
	u64 m_last_update_ticks;
	emu_timer *m_conversion_timer;
	sound_stream *m_audio_stream;
	std::array<u8, 8> m_audio_channel;
	std::array<u8, 8> m_audio_sample;
	unsigned m_audio_input_count;
};

DECLARE_DEVICE_TYPE(XAVIX_ADC, xavix_adc_device)

#endif // MAME_MACHINE_XAVIX_ADC_H
