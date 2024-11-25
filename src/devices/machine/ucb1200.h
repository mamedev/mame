// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Philips UCB1200 Advanced modem/audio analog front-end skeleton

***************************************************************************/

#ifndef MAME_MACHINE_UCB1200
#define MAME_MACHINE_UCB1200

#pragma once

class ucb1200_device : public device_t
{
public:
	ucb1200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <unsigned Line> auto gpio_out() { return m_gpio_out[Line].bind(); }
	template <unsigned Line> void gpio_in(int state) { gpio_in((uint16_t)Line, state); }
	template <unsigned N> auto adc_in() { return m_adc_in[N].bind(); }
	auto audio_sample_out() { return m_audio_out.bind(); }
	auto telecom_sample_out() { return m_telecom_out.bind(); }
	auto irq_out() { return m_irq_out.bind(); }

	void audio_sample_in(const uint16_t sample);
	void telecom_sample_in(const uint16_t sample);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void gpio_in(const uint16_t line, const int state);
	void update_gpio_direction(const uint16_t old_dir);
	void update_gpio_outputs(const uint16_t old_latch, const uint16_t changed);

	void adc_begin_conversion();

	// register contents
	enum : uint16_t
	{
		SIB_ZERO_BIT        = 15,

		IO_INT0_BIT         = 0,
		ADC_INT_BIT         = 11,
		TSPX_INT_BIT        = 12,
		TSMX_INT_BIT        = 13,
		TCLIP_INT_BIT       = 14,
		ACLIP_INT_BIT       = 15,

		TEL_DIV_BIT         = 0,
		TEL_DIV_MASK        = 0x007f,
		TEL_LOOP_BIT        = 7,

		TEL_VOICE_ENA_BIT   = 3,
		TEL_CLIP_BIT        = 4,
		TEL_ATT_BIT         = 6,
		TEL_SIDE_ENA_BIT    = 11,
		TEL_MUTE_BIT        = 13,
		TEL_IN_ENA_BIT      = 14,
		TEL_OUT_ENA_BIT     = 15,

		AUD_DIV_BIT         = 0,
		AUD_DIV_MASK        = 0x007f,
		AUD_GAIN_BIT        = 7,
		AUD_GAIN_MASK       = 0x0f80,

		AUD_ATT_BIT         = 0,
		AUD_ATT_MASK        = 0x001f,
		AUD_CLIP_BIT        = 6,
		AUD_LOOP_BIT        = 8,
		AUD_MUTE_BIT        = 13,
		AUD_IN_ENA_BIT      = 14,
		AUD_OUT_ENA_BIT     = 15,

		TSMX_POW_BIT        = 0,
		TSPX_POW_BIT        = 1,
		TSMY_POW_BIT        = 2,
		TSPY_POW_BIT        = 3,
		TSMX_GND_BIT        = 4,
		TSPX_GND_BIT        = 5,
		TSMY_GND_BIT        = 6,
		TSPY_GND_BIT        = 7,
		TSC_MODE_BIT        = 8,
		TSC_MODE_MASK       = 0x0300,
		TSC_BIAS_ENA_BIT    = 11,
		TSPX_LOW_BIT        = 12,
		TSMX_LOW_BIT        = 13,
		TOUCH_WRITE_MASK    = 0x0fff,

		ADC_SYNC_ENA_BIT    = 0,
		VREFBYP_CON_BIT     = 1,
		ADC_INPUT_BIT       = 2,
		ADC_INPUT_MASK      = 0x001c,
		ADC_INPUT_TSPX      = 0,
		ADC_INPUT_TSMX      = 1,
		ADC_INPUT_TSPY      = 2,
		ADC_INPUT_TSMY      = 3,
		ADC_INPUT_AD0       = 4,
		ADC_INPUT_AD1       = 5,
		ADC_INPUT_AD2       = 6,
		ADC_INPUT_AD3       = 7,
		EXT_REF_ENA_BIT     = 5,
		ADC_START_BIT       = 7,
		ADC_ENA_BIT         = 15,

		ADC_DATA_BIT        = 5,
		ADC_DATA_MASK       = 0x7fe0,
		ADC_DAT_VAL_BIT     = 15,

		AUD_TEST_BIT        = 0,
		TEL_TEST_BIT        = 1,
		PROD_TEST_MODE_BIT  = 2,
		PROD_TEST_MODE_MASK = 0x003c,
		DYN_VFLAG_ENA_BIT   = 12,
		AUD_OFF_CAN_BIT     = 13
	};

	uint16_t m_gpio_out_latch;
	uint16_t m_gpio_in_latch;
	uint16_t m_gpio_dir;
	uint16_t m_rising_int_en;
	uint16_t m_falling_int_en;
	uint16_t m_int_status;
	uint16_t m_telecom_ctrl_a;
	uint16_t m_telecom_ctrl_b;
	uint16_t m_audio_ctrl_a;
	uint16_t m_audio_ctrl_b;
	uint16_t m_touch_ctrl;
	uint16_t m_adc_ctrl;
	uint16_t m_adc_data;
	uint16_t m_id;
	uint16_t m_mode;

	devcb_write_line::array<10> m_gpio_out;
	devcb_write16 m_audio_out;
	devcb_write16 m_telecom_out;
	devcb_write_line m_irq_out;
	devcb_read16::array<4> m_adc_in;
};

DECLARE_DEVICE_TYPE(UCB1200, ucb1200_device)

#endif // MAME_MACHINE_UCB1200
