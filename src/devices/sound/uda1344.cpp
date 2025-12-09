// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Philips UDA1344 Stereo Audio Codec skeleton

****************************************************************************/

#include "emu.h"
#include "uda1344.h"

#define LOG_ADDR            (1U << 1)
#define LOG_STATUS_REG      (1U << 2)
#define LOG_DATA_REG        (1U << 3)
#define LOG_INPUT           (1U << 4)
#define LOG_OVERRUNS        (1U << 5)
#define LOG_ALL             (LOG_ADDR | LOG_STATUS_REG | LOG_DATA_REG | LOG_INPUT | LOG_OVERRUNS)

#define VERBOSE             (0)
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(UDA1344, uda1344_device, "ud1344", "Philips UDA1344 Codec")


uda1344_device::uda1344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UDA1344, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_volume(1.0)
	, m_frequency(BASE_FREQUENCY)
	, m_data_transfer_mode(0)
	, m_status_reg(0)
	, m_clock_divider(512)
	, m_volume_reg(0)
	, m_equalizer_reg(0)
	, m_filter_reg(0)
	, m_power_reg(0)
	, m_dac_enable(false)
	, m_adc_enable(false)
	, m_l3_ack_out(*this)
{
}

void uda1344_device::device_start()
{
	m_stream = stream_alloc(0, 2, BASE_FREQUENCY);

	m_buffer[0].resize(BUFFER_SIZE);
	m_buffer[1].resize(BUFFER_SIZE);

	save_item(NAME(m_buffer[0]));
	save_item(NAME(m_buffer[1]));
	save_item(NAME(m_bufin));
	save_item(NAME(m_bufout));
	save_item(NAME(m_volume));
	save_item(NAME(m_frequency));

	save_item(NAME(m_data_transfer_mode));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_volume_reg));
	save_item(NAME(m_equalizer_reg));
	save_item(NAME(m_filter_reg));
	save_item(NAME(m_power_reg));
	save_item(NAME(m_dac_enable));
	save_item(NAME(m_adc_enable));
}

void uda1344_device::device_reset()
{
	m_data_transfer_mode = 0;
	m_status_reg = 0;
	m_clock_divider = 512;
	m_volume_reg = 0;
	m_equalizer_reg = 0;
	m_filter_reg = 0;
	m_power_reg = 0;

	m_dac_enable = false;
	m_adc_enable = false;

	m_volume = 1.0;
	m_frequency = BASE_FREQUENCY;

	memset(m_bufin, 0, sizeof(uint32_t) * 2);
	memset(m_bufout, 0, sizeof(uint32_t) * 2);
}

void uda1344_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 2 && channel < stream.output_count(); channel++)
	{
		uint32_t curout = m_bufout[channel];
		uint32_t curin = m_bufin[channel];

		// feed as much as we can
		int sampindex;
		for (sampindex = 0; curout != curin && sampindex < stream.samples(); sampindex++)
		{
			stream.put(channel, sampindex, sound_stream::sample_t(m_buffer[channel][curout]) * m_volume);
			curout = (curout + 1) % BUFFER_SIZE;
		}

		// save the new output pointer
		m_bufout[channel] = curout;
	}
}

void uda1344_device::ingest_samples(int16_t left, int16_t right)
{
	const int16_t samples[2] = { left, right };

	const sound_stream::sample_t sample_scale = 1.0 / 32768.0;
	const sound_stream::sample_t enable_scale = m_dac_enable ? 1.0 : 0.0;

	m_stream->update();

	for (int channel = 0; channel < 2; channel++)
	{
		int maxin = (m_bufout[channel] + BUFFER_SIZE - 1) % BUFFER_SIZE;
		if (m_bufin[channel] != maxin)
		{
			m_buffer[channel][m_bufin[channel]] = sound_stream::sample_t(samples[channel]) * sample_scale * enable_scale;
			m_bufin[channel] = (m_bufin[channel] + 1) % BUFFER_SIZE;
		}
		else
		{
			LOGMASKED(LOG_OVERRUNS, "ingest_samples: buffer overrun (short 1 frame on channel %d)\n", channel);
		}
	}
}

void uda1344_device::device_clock_changed()
{
	if (clock() == 0)
		return;

	m_stream->update();
	m_stream->set_sample_rate(clock() / m_clock_divider);
}

void uda1344_device::set_clock_divider(const uint32_t divider)
{
	m_clock_divider = divider;
	device_clock_changed();
}

void uda1344_device::i2s_input_w(uint32_t data)
{
	const int16_t left = (int16_t)(data >> 16);
	const int16_t right = (int16_t)data;
	ingest_samples(left, right);
}

void uda1344_device::l3_addr_w(offs_t offset, uint8_t data)
{
	// Check for L3 address match, ignore if not addressed to us
	if ((data & CHIP_ADDR_MASK) != CHIP_ADDR)
	{
		LOGMASKED(LOG_ADDR, "%s: L3 address %02x received, ignoring due to address mismatch\n", machine().describe_context(), data);
		return;
	}

	m_data_transfer_mode = data & ~CHIP_ADDR_MASK;
	LOGMASKED(LOG_ADDR, "%s: L3 address %02x received, preparing to receive data\n", machine().describe_context(), data);
}

void uda1344_device::l3_data_w(offs_t offset, uint8_t data)
{
	// Registers with bit 0 of the address set are unused
	if (BIT(m_data_transfer_mode, 0))
	{
		LOGMASKED(LOG_DATA_REG, "%s: Data transfer mode has bit 0 set, ignoring L3 data write\n", machine().describe_context());
		return;
	}

	if (BIT(m_data_transfer_mode, 1))
	{
		// Status transfer type
		static const char *const s_clock_names[4] = { "512*freq", "384*freq", "256*freq", "unused" };
		static const char *const s_format_names[8] =
		{
			"I2S-bus",
			"LSB-justified 16-bits",
			"LSB-justified 18-bits",
			"LSB-justified 20-bits",
			"MSB-justified",
			"Input LSB-justified 16-bits / Output MSB-justified",
			"Input LSB-justified 18-bits / Output MSB-justified",
			"Input LSB-justified 20-bits / Output MSB-justified"
		};
		const uint8_t reg_bits = data & STATUS_REG_MASK;
		LOGMASKED(LOG_STATUS_REG, "%s: Status register data: %02x (system clock: %s, format: %s, DC filtering: %s)\n", machine().describe_context(), reg_bits,
			s_clock_names[(reg_bits & STATUS_SC_MASK) >> STATUS_SC_BIT],
			s_format_names[(reg_bits & STATUS_IF_MASK) >> STATUS_IF_BIT],
			BIT(reg_bits, STATUS_DC_BIT) ? "on" : "off");
		m_status_reg = reg_bits;

		switch ((reg_bits & STATUS_SC_MASK) >> STATUS_SC_BIT)
		{
		case 1:
			set_clock_divider(384);
			break;
		case 2:
			set_clock_divider(256);
			break;
		default:
			set_clock_divider(512);
			break;
		}
	}
	else
	{
		// Data transfer type
		switch ((data & REG_TYPE_MASK) >> REG_TYPE_BIT)
		{
		case VOLUME_REG:
		{
			m_stream->update();

			const uint8_t reg_bits = data & VOLUME_REG_MASK;
			if (reg_bits < 2)
			{
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, no attenuation\n", machine().describe_context(), reg_bits);
				m_volume = 1.0;
			}
			else if (reg_bits >= 62)
			{
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, full attenuation\n", machine().describe_context(), reg_bits);
				m_volume = 0.0;
			}
			else
			{
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, -%ddB attenuation\n", machine().describe_context(), reg_bits, reg_bits - 1);
				m_volume = 1.0 - ((reg_bits - 1) / 62.0);
			}

			m_volume_reg = reg_bits;
			break;
		}

		case EQUALIZER_REG:
		{
			const uint8_t reg_bits = data & EQUALIZER_REG_MASK;
			LOGMASKED(LOG_DATA_REG, "%s: Equalizer register data: %02x (bass boost %02x, treble %d)\n", machine().describe_context(), reg_bits,
				(reg_bits & EQUALIZER_BB_MASK) >> EQUALIZER_BB_BIT, (reg_bits & EQUALIZER_TR_MASK) >> EQUALIZER_TR_BIT);
			m_equalizer_reg = reg_bits;
			break;
		}

		case FILTER_REG:
		{
			static const char *const s_de_names[4] = { "none", "32kHz", "44.1kHz", "48kHz" };
			static const char *const s_mode_names[4] = { "flat", "min(1)", "min(2)", "max" };
			const uint8_t reg_bits = data & FILTER_REG_MASK;
			LOGMASKED(LOG_DATA_REG, "%s: Filter register data: %02x (de-emphasis %s, mute %d, mode %s)\n", machine().describe_context(), reg_bits,
				s_de_names[(reg_bits & FILTER_DE_MASK) >> FILTER_DE_BIT], BIT(reg_bits, FILTER_MT_BIT),
				s_mode_names[(reg_bits & FILTER_MODE_MASK) >> FILTER_MODE_BIT]);
			m_filter_reg = reg_bits;
			break;
		}

		case POWER_REG:
		{
			const uint8_t reg_bits = data & POWER_REG_MASK;
			LOGMASKED(LOG_DATA_REG, "%s: Power register data: %02x (ADC %s, DAC %s)\n", machine().describe_context(), reg_bits,
				BIT(reg_bits, POWER_ADC_BIT) ? "on" : "off",
				BIT(reg_bits, POWER_DAC_BIT) ? "on" : "off");
			m_power_reg = reg_bits;

			m_stream->update();
			m_dac_enable = BIT(reg_bits, POWER_DAC_BIT);
			m_adc_enable = BIT(reg_bits, POWER_ADC_BIT);
			break;
		}
		}
	}

	// Pulse acknowledge line
	m_l3_ack_out(1);
	m_l3_ack_out(0);
}
