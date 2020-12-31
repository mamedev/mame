// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Philips UDA1344 Stereo Audio Codec skeleton

****************************************************************************/

#include "emu.h"
#include "sound/uda1344.h"

#define LOG_ADDR			(1 << 1)
#define LOG_STATUS_REG		(1 << 2)
#define LOG_DATA_REG		(1 << 3)
#define LOG_ALL				(LOG_ADDR | LOG_STATUS_REG | LOG_DATA_REG)

#define VERBOSE				(LOG_ALL)
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(UDA1344, uda1344_device, "ud1344", "Philips UDA1344 Codec")


uda1344_device::uda1344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UDA1344, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_data_transfer_mode(0)
	, m_status_reg(0)
	, m_volume_reg(0)
	, m_equalizer_reg(0)
	, m_filter_reg(0)
	, m_power_reg(0)
	, m_l3_ack_out(*this)
{
}

void uda1344_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);

	save_item(NAME(m_data_transfer_mode));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_volume_reg));
	save_item(NAME(m_equalizer_reg));
	save_item(NAME(m_filter_reg));
	save_item(NAME(m_power_reg));

	m_l3_ack_out.resolve_safe();
}

void uda1344_device::device_reset()
{
	m_data_transfer_mode = 0;
	m_status_reg = 0;
	m_volume_reg = 0;
	m_equalizer_reg = 0;
	m_filter_reg = 0;
	m_power_reg = 0;
}

void uda1344_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &buffer = outputs[0];

	/* fill in the samples */
	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		// TODO: Generate audio
		buffer.put(sampindex, 0);
	}
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
	}
	else
	{
		// Data transfer type
		switch ((data & REG_TYPE_MASK) >> REG_TYPE_BIT)
		{
		case VOLUME_REG:
		{
			const uint8_t reg_bits = data & VOLUME_REG_MASK;
			if (reg_bits < 2)
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, no attenuation\n", machine().describe_context(), reg_bits);
			else if (reg_bits >= 62)
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, full attenuation\n", machine().describe_context(), reg_bits);
			else
				LOGMASKED(LOG_DATA_REG, "%s: Volume register data: %02x, -%ddB attenuation\n", machine().describe_context(), reg_bits, reg_bits - 1);

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
			break;
		}
		}
	}

	// Pulse acknowledge line
	m_l3_ack_out(1);
	m_l3_ack_out(0);
}
