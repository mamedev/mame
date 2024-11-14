// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_ADC_H
#define MAME_TVGAMES_XAVIX_ADC_H

class xavix_adc_device : public device_t
{
public:
	xavix_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_in0_cb.bind(); }
	auto read_1_callback() { return m_in1_cb.bind(); }
	auto read_2_callback() { return m_in2_cb.bind(); }
	auto read_3_callback() { return m_in3_cb.bind(); }
	auto read_4_callback() { return m_in4_cb.bind(); }
	auto read_5_callback() { return m_in5_cb.bind(); }
	auto read_6_callback() { return m_in6_cb.bind(); }
	auto read_7_callback() { return m_in7_cb.bind(); }

	uint8_t adc_7b80_r();
	void adc_7b80_w(uint8_t data);
	uint8_t adc_7b81_r();
	void adc_7b81_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8 m_in0_cb;
	devcb_read8 m_in1_cb;
	devcb_read8 m_in2_cb;
	devcb_read8 m_in3_cb;
	devcb_read8 m_in4_cb;
	devcb_read8 m_in5_cb;
	devcb_read8 m_in6_cb;
	devcb_read8 m_in7_cb;

	uint8_t m_adc_inlatch;
	uint8_t m_adc_control;
	emu_timer *m_adc_timer;

	TIMER_CALLBACK_MEMBER(adc_timer_done);
};

DECLARE_DEVICE_TYPE(XAVIX_ADC, xavix_adc_device)

#endif // MAME_TVGAMES_XAVIX_ADC_H
