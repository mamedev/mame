// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Philips UCB1200 Advanced modem/audio analog front-end skeleton

***************************************************************************/

#include "emu.h"
#include "ucb1200.h"

#define LOG_UNKNOWN     (1 << 1)
#define LOG_READS       (1 << 2)
#define LOG_WRITES      (1 << 3)
#define LOG_ALL         (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(UCB1200, ucb1200_device, "ucb1200", "Philips UCB1200 modem/audio codec")

ucb1200_device::ucb1200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UCB1200, tag, owner, clock)
	, m_gpio_out(*this)
	, m_audio_out(*this)
	, m_telecom_out(*this)
	, m_irq_out(*this)
	, m_adc_in(*this)
{
}

void ucb1200_device::device_start()
{
	m_gpio_out.resolve_all_safe();
	m_audio_out.resolve_safe();
	m_telecom_out.resolve_safe();
	m_irq_out.resolve_safe();
	m_adc_in.resolve_all_safe(0xffff);

	save_item(NAME(m_gpio_out_latch));
	save_item(NAME(m_gpio_in_latch));
	save_item(NAME(m_gpio_dir));
	save_item(NAME(m_rising_int_en));
	save_item(NAME(m_falling_int_en));
	save_item(NAME(m_int_status));
	save_item(NAME(m_telecom_ctrl_a));
	save_item(NAME(m_telecom_ctrl_b));
	save_item(NAME(m_audio_ctrl_a));
	save_item(NAME(m_audio_ctrl_b));
	save_item(NAME(m_touch_ctrl));
	save_item(NAME(m_adc_ctrl));
	save_item(NAME(m_adc_data));
	save_item(NAME(m_id));
	save_item(NAME(m_mode));
}

void ucb1200_device::device_reset()
{
	m_gpio_out_latch = 0;
	m_gpio_in_latch = 0;
	m_gpio_dir = 0;
	m_rising_int_en = 0;
	m_falling_int_en = 0;
	m_int_status = 0;
	m_telecom_ctrl_a = 0x10 << TEL_DIV_BIT;
	m_telecom_ctrl_b = 0;
	m_audio_ctrl_a = 0x06 << AUD_DIV_BIT;
	m_audio_ctrl_b = 0;
	m_touch_ctrl = 0;
	m_adc_ctrl = 0;
	m_adc_data = 0;
	m_id = 0x1004;
	m_mode = 0;
}

void ucb1200_device::gpio_in(const uint16_t line, const int state)
{
	const uint16_t mask = (1 << line);
	const uint16_t old_latch = m_gpio_in_latch;
	m_gpio_in_latch &= ~mask;
	m_gpio_in_latch |= (state << line);

	const bool old_irq = (m_int_status != 0);
	if (old_latch != m_gpio_in_latch && !BIT(m_gpio_dir, line))
	{
		if (state && BIT(m_rising_int_en, line))
			m_int_status |= mask;
		else if (!state && BIT(m_falling_int_en, line))
			m_int_status |= mask;
	}

	if (!old_irq && m_int_status != 0)
		m_irq_out(1);
}

void ucb1200_device::update_gpio_direction(const uint16_t old_dir)
{
	const uint16_t new_outputs = ~old_dir & m_gpio_dir;
	if (new_outputs)
	{
		for (uint32_t line = 0; line < 10; line++)
		{
			if (BIT(new_outputs, line))
			{
				m_gpio_out[line](BIT(m_gpio_out_latch, line));
			}
		}
	}

	// TODO: Do we need to check rising/falling edges based on the transition from output to input?
}

void ucb1200_device::update_gpio_outputs(const uint16_t old_latch, const uint16_t changed)
{
	uint16_t remaining_changed = changed;

	for (uint32_t line = 0; line < 10 && remaining_changed != 0; line++)
	{
		if (BIT(remaining_changed, line))
		{
			m_gpio_out[line](BIT(m_gpio_out_latch, line));
			remaining_changed &= ~(1 << line);
		}
	}
}

void ucb1200_device::audio_sample_in(const uint16_t sample)
{
	// TODO: Handle incoming audio samples
}

void ucb1200_device::telecom_sample_in(const uint16_t sample)
{
	// TODO: Handle incoming telecom samples
}

void ucb1200_device::adc_begin_conversion()
{
	const uint16_t adc_input = (m_adc_ctrl & ADC_INPUT_MASK) >> ADC_INPUT_BIT;
	switch (adc_input)
	{
	case ADC_INPUT_TSPX:
	case ADC_INPUT_TSMX:
	case ADC_INPUT_TSPY:
	case ADC_INPUT_TSMY:
	default:
		m_adc_data = 0;
		break;
	case ADC_INPUT_AD0:
	case ADC_INPUT_AD1:
	case ADC_INPUT_AD2:
	case ADC_INPUT_AD3:
		m_adc_data = m_adc_in[adc_input - ADC_INPUT_AD0]();
		m_adc_data <<= ADC_DATA_BIT;
		m_adc_data &= ADC_DATA_MASK;
		break;
	}

	m_adc_data |= (1 << ADC_DAT_VAL_BIT);
}

uint16_t ucb1200_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0:
	{
		const uint16_t data = (m_gpio_out_latch & m_gpio_dir) | (m_gpio_in_latch & ~m_gpio_dir);
		LOGMASKED(LOG_READS, "%s: read: GPIO Data Register: %04x\n", machine().describe_context(), data);
		return data;
	}
	case 1:
		LOGMASKED(LOG_READS, "%s: read: GPIO Direction Register: %04x\n", machine().describe_context(), m_gpio_dir);
		return m_gpio_dir;
	case 2:
		LOGMASKED(LOG_READS, "%s: read: GPIO Rising-Edge Interrupt Enable Register: %04x\n", machine().describe_context(), m_rising_int_en);
		return m_rising_int_en;
	case 3:
		LOGMASKED(LOG_READS, "%s: read: GPIO Falling-Edge Interrupt Enable Register: %04x\n", machine().describe_context(), m_falling_int_en);
		return m_falling_int_en;
	case 4:
		LOGMASKED(LOG_READS, "%s: read: Interrupt Clear/Status Register: %04x\n", machine().describe_context(), m_int_status);
		return m_int_status;
	case 5:
		LOGMASKED(LOG_READS, "%s: read: Telecom Control Register A: %04x\n", machine().describe_context(), m_telecom_ctrl_a);
		return m_telecom_ctrl_a;
	case 6:
		LOGMASKED(LOG_READS, "%s: read: Telecom Control Register B: %04x\n", machine().describe_context(), m_telecom_ctrl_b);
		return m_telecom_ctrl_b;
	case 7:
		LOGMASKED(LOG_READS, "%s: read: Audio Control Register A: %04x\n", machine().describe_context(), m_audio_ctrl_a);
		return m_audio_ctrl_a;
	case 8:
		LOGMASKED(LOG_READS, "%s: read: Audio Control Register B: %04x\n", machine().describe_context(), m_audio_ctrl_b);
		return m_audio_ctrl_b;
	case 9:
		LOGMASKED(LOG_READS, "%s: read: Touchscreen Control Register: %04x\n", machine().describe_context(), m_touch_ctrl);
		return m_touch_ctrl;
	case 10:
		LOGMASKED(LOG_READS, "%s: read: ADC Control Register: %04x\n", machine().describe_context(), m_adc_ctrl);
		return m_adc_ctrl;
	case 11:
		LOGMASKED(LOG_READS, "%s: read: ADC Data Register: %04x\n", machine().describe_context(), m_adc_data);
		return m_adc_data;
	case 12:
		LOGMASKED(LOG_READS, "%s: read: ID Register: %04x\n", machine().describe_context(), m_id);
		return m_id;
	case 13:
		LOGMASKED(LOG_READS, "%s: read: Mode Register: %04x\n", machine().describe_context(), m_mode);
		return m_mode;
	case 14:
		LOGMASKED(LOG_READS, "%s: read: Reserved Register: %04x\n", machine().describe_context(), 0);
		return 0;
	case 15:
		LOGMASKED(LOG_READS, "%s: read: NULL Register: %04x\n", machine().describe_context(), 0xffff);
		return 0xffff;
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: read: Unknown Register: %d\n", machine().describe_context(), offset);
		return 0;
	}
}

void ucb1200_device::write(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
	{
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Data Register: %04x\n", machine().describe_context(), data);
		const uint16_t old = m_gpio_out_latch;
		m_gpio_out_latch = data;
		const uint16_t changed = (old ^ m_gpio_out_latch) & m_gpio_dir;
		if (changed)
			update_gpio_outputs(old, changed);
		break;
	}
	case 1:
	{
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Direction Register: %04x\n", machine().describe_context(), data);
		const uint16_t old = m_gpio_dir;
		m_gpio_dir = data;
		if (old != m_gpio_dir)
			update_gpio_direction(old);
		break;
	}
	case 2:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Rising-Edge Interrupt Enable Register: %04x\n", machine().describe_context(), data);
		m_rising_int_en = data;
		break;
	case 3:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Falling-Edge Interrupt Enable Register: %04x\n", machine().describe_context(), data);
		m_falling_int_en = data;
		break;
	case 4:
	{
		LOGMASKED(LOG_WRITES, "%s: write: Interrupt Clear/Status Register: %04x\n", machine().describe_context(), data);
		const uint16_t old = m_int_status;
		m_int_status &= ~data;
		if (old != 0 && m_int_status == 0)
			m_irq_out(0);
		break;
	}
	case 5:
		LOGMASKED(LOG_WRITES, "%s: write: Telecom Control Register A: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        Telecom Codec Sample Rate Divisor: %02x\n", machine().describe_context(), (data & TEL_DIV_MASK) >> TEL_DIV_BIT);
		LOGMASKED(LOG_WRITES, "%s:        Telecom Codec Loopback: %d\n", machine().describe_context(), BIT(data, TEL_LOOP_BIT));
		m_telecom_ctrl_a = data;
		break;
	case 6:
	{
		LOGMASKED(LOG_WRITES, "%s: write: Telecom Control Register B: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        Telecom Voice Band Filter: %d\n", machine().describe_context(), BIT(data, TEL_VOICE_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Clip Detect Clear: %d\n", machine().describe_context(), BIT(data, TEL_CLIP_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Input Attenuation: %d\n", machine().describe_context(), BIT(data, TEL_ATT_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Sidetone Suppression: %d\n", machine().describe_context(), BIT(data, TEL_SIDE_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Output Mute: %d\n", machine().describe_context(), BIT(data, TEL_MUTE_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Input Enable: %d\n", machine().describe_context(), BIT(data, TEL_IN_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Telecom Output Enable: %d\n", machine().describe_context(), BIT(data, TEL_OUT_ENA_BIT));

		const uint16_t old_clip = m_telecom_ctrl_b & (1 << TEL_CLIP_BIT);
		const uint16_t new_clip = data & (1 << TEL_CLIP_BIT);

		m_telecom_ctrl_b = data;

		if (new_clip)
			m_telecom_ctrl_b &= ~(1 << TEL_CLIP_BIT);
		else if (old_clip)
			m_telecom_ctrl_b |= (1 << TEL_CLIP_BIT);
		break;
	}
	case 7:
		LOGMASKED(LOG_WRITES, "%s: write: Audio Control Register A: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        Audio Codec Sample Rate Divisor: %02x\n", machine().describe_context(), (data & AUD_DIV_MASK) >> AUD_DIV_BIT);
		LOGMASKED(LOG_WRITES, "%s:        Audio Input Gain: %02x\n", machine().describe_context(), (data & AUD_GAIN_MASK) >> AUD_GAIN_BIT);
		m_audio_ctrl_a = data;
		break;
	case 8:
	{
		LOGMASKED(LOG_WRITES, "%s: write: Audio Control Register B: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        Audio Output Attenuation: %02x\n", machine().describe_context(), (data & AUD_ATT_MASK) >> AUD_ATT_BIT);
		LOGMASKED(LOG_WRITES, "%s:        Audio Clip Detect Clear: %d\n", machine().describe_context(), BIT(data, AUD_CLIP_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Audio Codec Loopback: %d\n", machine().describe_context(), BIT(data, AUD_LOOP_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Audio Output Mute: %d\n", machine().describe_context(), BIT(data, AUD_MUTE_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Audio Input Enable: %d\n", machine().describe_context(), BIT(data, AUD_IN_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Audio Output Enable: %d\n", machine().describe_context(), BIT(data, AUD_OUT_ENA_BIT));

		const uint16_t old_clip = m_audio_ctrl_b & (1 << AUD_CLIP_BIT);
		const uint16_t new_clip = data & (1 << AUD_CLIP_BIT);

		m_audio_ctrl_b = data;

		if (new_clip)
			m_audio_ctrl_b &= ~(1 << AUD_CLIP_BIT);
		else if (old_clip)
			m_audio_ctrl_b |= (1 << AUD_CLIP_BIT);
		break;
	}
	case 9:
	{
		static const char *const s_tsc_modes[4] = { "Interrupt", "Pressure", "Position [2]", "Position [3]" };
		LOGMASKED(LOG_WRITES, "%s: write: Touchscreen Control Register: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        TSMX Pin Powered: %d\n", machine().describe_context(), BIT(data, TSMX_POW_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSPX Pin Powered: %d\n", machine().describe_context(), BIT(data, TSPX_POW_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSMY Pin Powered: %d\n", machine().describe_context(), BIT(data, TSMY_POW_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSPY Pin Powered: %d\n", machine().describe_context(), BIT(data, TSPY_POW_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSMX Pin Grounded: %d\n", machine().describe_context(), BIT(data, TSMX_GND_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSPX Pin Grounded: %d\n", machine().describe_context(), BIT(data, TSPX_GND_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSMY Pin Grounded: %d\n", machine().describe_context(), BIT(data, TSMY_GND_BIT));
		LOGMASKED(LOG_WRITES, "%s:        TSPY Pin Grounded: %d\n", machine().describe_context(), BIT(data, TSPY_GND_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Touch Screen Mode: %s\n", machine().describe_context(), s_tsc_modes[(data & TSC_MODE_MASK) >> TSC_MODE_BIT]);
		LOGMASKED(LOG_WRITES, "%s:        Touch Screen Bias Circuit Active: %d\n", machine().describe_context(), BIT(data, TSC_BIAS_ENA_BIT));
		m_touch_ctrl &= ~TOUCH_WRITE_MASK;
		m_touch_ctrl |= data & TOUCH_WRITE_MASK;
		break;
	}
	case 10:
	{
		static const char *const s_adc_inputs[8] = { "TSPX", "TSMX", "TSPY", "TSMY", "AD0", "AD1", "AD2", "AD3" };
		LOGMASKED(LOG_WRITES, "%s: write: ADC Control Register: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        ADC Sync Mode: %d\n", machine().describe_context(), BIT(data, ADC_SYNC_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Connect Internal Vref to VREFBYP Pin: %d\n", machine().describe_context(), BIT(data, VREFBYP_CON_BIT));
		LOGMASKED(LOG_WRITES, "%s:        ADC Input Select: %s\n", machine().describe_context(), s_adc_inputs[(data & ADC_INPUT_MASK) >> ADC_INPUT_BIT]);
		LOGMASKED(LOG_WRITES, "%s:        Apply External Voltage to VREFBYP Pin: %d\n", machine().describe_context(), BIT(data, EXT_REF_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        ADC Conversion Start: %d\n", machine().describe_context(), BIT(data, ADC_START_BIT));
		LOGMASKED(LOG_WRITES, "%s:        ADC Enabled: %d\n", machine().describe_context(), BIT(data, ADC_ENA_BIT));
		const uint16_t old = m_adc_ctrl;
		m_adc_ctrl = data;
		if (!BIT(old, ADC_START_BIT) && BIT(data, ADC_START_BIT) && BIT(data, ADC_ENA_BIT))
			adc_begin_conversion();
		break;
	}
	case 11:
		LOGMASKED(LOG_WRITES, "%s: write: ADC Data Register (ignored): %04x\n", machine().describe_context(), data);
		break;
	case 12:
		LOGMASKED(LOG_WRITES, "%s: write: ID Register (ignored): %04x\n", machine().describe_context(), data);
		break;
	case 13:
		LOGMASKED(LOG_WRITES, "%s: write: Mode Register: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:        Analog Audio Test Mode: %d\n", machine().describe_context(), BIT(data, AUD_TEST_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Analog Telecom Test Mode: %d\n", machine().describe_context(), BIT(data, TEL_TEST_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Production Test Mode: %02x\n", machine().describe_context(), (data & PROD_TEST_MODE_MASK) >> PROD_TEST_MODE_BIT);
		LOGMASKED(LOG_WRITES, "%s:        Dynamic Data Valid Flag Mode: %d\n", machine().describe_context(), BIT(data, DYN_VFLAG_ENA_BIT));
		LOGMASKED(LOG_WRITES, "%s:        Audio Offset-Cancelling: %d\n", machine().describe_context(), BIT(data, AUD_OFF_CAN_BIT));
		break;
	case 14:
		LOGMASKED(LOG_WRITES, "%s: write: Reserved Register (ignored): %04x\n", machine().describe_context(), data);
		break;
	case 15:
		LOGMASKED(LOG_WRITES, "%s: write: NULL Register (ignored): %04x\n", machine().describe_context(), data);
		break;
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: write: Unknown Register: %d = %04x\n", machine().describe_context(), offset, data);
		break;
	}
}
