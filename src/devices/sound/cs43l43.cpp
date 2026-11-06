// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Cirrus Logic CS43L43 I2S Stereo DAC

****************************************************************************/

#include "emu.h"
#include "sound/cs43l43.h"

#define LOG_WRITES		(1U << 1)
#define LOG_READS		(1U << 2)
#define LOG_OVERRUNS	(1U << 3)
#define LOG_SAMPLES		(1U << 4)
#define VERBOSE			( 0 )
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CS43L43, cs43l43_device, "cs43l43", "Cirrus Logic CS43L43 I2S DAC")

cs43l43_device::cs43l43_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CS43L43, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	i2c_hle_interface(mconfig, *this, I2C_ADDRESS >> 1),
	m_power_ctrl(0),
	m_chan_a_analog_ctrl(0),
	m_chan_b_analog_ctrl(0),
	m_chan_a_digital_ctrl(0),
	m_chan_b_digital_ctrl(0),
	m_tone_ctrl(0),
	m_mode_ctrl(0),
	m_limiter_attack_rate(0),
	m_limiter_release_rate(0),
	m_volume_mix_ctrl(0),
	m_mode_ctrl_2(0),
	m_stream(nullptr),
	m_volume(1.0),
	m_frequency(BASE_FREQUENCY)
{
}

void cs43l43_device::device_start()
{
	m_stream = stream_alloc(0, 2, BASE_FREQUENCY);

	m_buffer[0].resize(BUFFER_SIZE);
	m_buffer[1].resize(BUFFER_SIZE);

	save_item(NAME(m_power_ctrl));
	save_item(NAME(m_chan_a_analog_ctrl));
	save_item(NAME(m_chan_b_analog_ctrl));
	save_item(NAME(m_chan_a_digital_ctrl));
	save_item(NAME(m_chan_b_digital_ctrl));
	save_item(NAME(m_tone_ctrl));
	save_item(NAME(m_mode_ctrl));
	save_item(NAME(m_limiter_attack_rate));
	save_item(NAME(m_limiter_release_rate));
	save_item(NAME(m_volume_mix_ctrl));
	save_item(NAME(m_mode_ctrl_2));

	save_item(NAME(m_buffer[0]));
	save_item(NAME(m_buffer[1]));
	save_item(NAME(m_bufin));
	save_item(NAME(m_bufout));
	save_item(NAME(m_volume));
	save_item(NAME(m_frequency));
}

void cs43l43_device::device_reset()
{
	m_power_ctrl = 0xd2;
	m_chan_a_analog_ctrl = 0;
	m_chan_b_analog_ctrl = 0;
	m_chan_a_digital_ctrl = 0;
	m_chan_b_digital_ctrl = 0;
	m_tone_ctrl = 0;
	m_mode_ctrl = 0;
	m_limiter_attack_rate = 0x10;
	m_limiter_release_rate = 0x20;
	m_volume_mix_ctrl = 0x09;
	m_mode_ctrl_2 = 0;

	m_volume = 1.0;
	m_frequency = BASE_FREQUENCY;

	memset(m_bufin, 0, sizeof(u32) * 2);
	memset(m_bufout, 0, sizeof(u32) * 2);
}

void cs43l43_device::sound_stream_update(sound_stream &stream)
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

u8 cs43l43_device::read_data(u16 offset)
{
	switch (offset)
	{
	case 0x01:
		LOGMASKED(LOG_READS, "%s: read_data: Power and Muting Control: %02x\n", machine().describe_context(), m_power_ctrl);
		return m_power_ctrl;
	case 0x02:
		LOGMASKED(LOG_READS, "%s: read_data: Channel A Analog Attenuation Control: %02x\n", machine().describe_context(), m_chan_a_analog_ctrl);
		return m_chan_a_analog_ctrl;
	case 0x03:
		LOGMASKED(LOG_READS, "%s: read_data: Channel B Analog Attenuation Control: %02x\n", machine().describe_context(), m_chan_b_analog_ctrl);
		return m_chan_b_analog_ctrl;
	case 0x04:
		LOGMASKED(LOG_READS, "%s: read_data: Channel A Digital Volume Control: %02x\n", machine().describe_context(), m_chan_a_digital_ctrl);
		return m_chan_a_digital_ctrl;
	case 0x05:
		LOGMASKED(LOG_READS, "%s: read_data: Channel B Digital Volume Control: %02x\n", machine().describe_context(), m_chan_b_digital_ctrl);
		return m_chan_b_digital_ctrl;
	case 0x06:
		LOGMASKED(LOG_READS, "%s: read_data: Tone Control: %02x\n", machine().describe_context(), m_tone_ctrl);
		return m_tone_ctrl;
	case 0x07:
		LOGMASKED(LOG_READS, "%s: read_data: Mode Control: %02x\n", machine().describe_context(), m_mode_ctrl);
		return m_mode_ctrl;
	case 0x08:
		LOGMASKED(LOG_READS, "%s: read_data: Limiter Attack Rate: %02x\n", machine().describe_context(), m_limiter_attack_rate);
		return m_limiter_attack_rate;
	case 0x09:
		LOGMASKED(LOG_READS, "%s: read_data: Limiter Release Rate: %02x\n", machine().describe_context(), m_limiter_release_rate);
		return m_limiter_release_rate;
	case 0x0a:
		LOGMASKED(LOG_READS, "%s: read_data: Volume and Mixing Control: %02x\n", machine().describe_context(), m_volume_mix_ctrl);
		return m_volume_mix_ctrl;
	case 0x0b:
		LOGMASKED(LOG_READS, "%s: read_data: Mode Control 2: %02x\n", machine().describe_context(), m_mode_ctrl_2);
		return m_mode_ctrl_2;
	default:
		LOGMASKED(LOG_READS, "%s: read_data: Unknown Register %04x\n", machine().describe_context(), offset);
		return 0;
	}
}

void cs43l43_device::write_data(u16 offset, u8 data)
{
	static const char *const SZC_NAMES[4] = { "Immediate Change", "Zero-Cross D/A", "Ramped D/A", "Reserved" };
	static const char *const DEM_NAMES[4] = { "Disabled", "44.1 kHz", "48 kHz", "32 kHz" };
	static const char *const TBCF_NAMES[4] = { "2 kHz", "4 kHz", "7 kHz", "Reserved" };
	static const char *const BBCF_NAMES[4] = { "50 Hz", "100 Hz", "200 Hz", "Reserved" };
	static const char *const TC_NAMES[4] =
	{
		"All User Registers",
		"12 dB Bass at 100 Hz and 6 dB Treble at 7 kHz",
		"8 dB Bass at 100 Hz and 4 dB Treble at 7 kHz",
		"4 dB Bass at 100 Hz and 2 dB Treble at 7 kHz"
	};
	static const char *const DIF_NAMES[8] =
	{
		"I2S, 24-bit data, 64x Fs Internal SCLK",
		"I2S, 16-bit data, 32x Fs Internal SCLK",
		"Left-Justified, 24-bit data",
		"Right-Justified, 24-bit data",
		"Right-Justified, 20-bit data",
		"Right-Justified, 16-bit data",
		"Right-Justified, 18-bit data",
		"I2S, 16-bit data, 32x Fs Internal SCLK (Alias)"
	};

	switch (offset)
	{
	case 0x00:
		LOGMASKED(LOG_WRITES, "%s: write_data: Reserved (ignored): %02x\n", machine().describe_context(), data);
		break;
	case 0x01:
		LOGMASKED(LOG_WRITES, "%s: write_data: Power and Muting Control: %02x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:             Control Port Enable: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_WRITES, "%s:             Power Down: %d\n", machine().describe_context(), BIT(data, 1));
		LOGMASKED(LOG_WRITES, "%s:             Popguard Transient Enable: %d\n", machine().describe_context(), BIT(data, 4));
		LOGMASKED(LOG_WRITES, "%s:             Soft Ramp and Zero-Cross Control: %s\n", machine().describe_context(), SZC_NAMES[BIT(data, 5, 2)]);
		LOGMASKED(LOG_WRITES, "%s:             Auto-Mute Enable: %d\n", machine().describe_context(), BIT(data, 7));
		m_power_ctrl = data;
		break;
	case 0x02:
		LOGMASKED(LOG_WRITES, "%s: write_data: Channel A Analog Attenuation Control: %02x (%d dB)\n", machine().describe_context(), data, (s8)data);
		m_chan_a_analog_ctrl = data;
		break;
	case 0x03:
		LOGMASKED(LOG_WRITES, "%s: write_data: Channel B Analog Attenuation Control: %02x (%d dB)\n", machine().describe_context(), data, (s8)data);
		m_chan_b_analog_ctrl = data;
		break;
	case 0x04:
		LOGMASKED(LOG_WRITES, "%s: write_data: Channel A Digital Volume Control: %02x (%d dB)\n", machine().describe_context(), data, (s8)data);
		m_chan_a_digital_ctrl = data;
		break;
	case 0x05:
		LOGMASKED(LOG_WRITES, "%s: write_data: Channel B Digital Volume Control: %02x (%d dB)\n", machine().describe_context(), data, (s8)data);
		m_chan_b_digital_ctrl = data;
		break;
	case 0x06:
		LOGMASKED(LOG_WRITES, "%s: write_data: Tone Control: %02x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:             Treble Boost: %d dB\n", machine().describe_context(), BIT(data, 3) ? (s8)(data | 0xf0) : (data & 0x0f));
		LOGMASKED(LOG_WRITES, "%s:             Bass Boost: %d dB\n", machine().describe_context(), BIT(data, 7) ? (s8)((data >> 4) | 0xf0) : (data >> 4));
		m_tone_ctrl = data;
		break;
	case 0x07:
		LOGMASKED(LOG_WRITES, "%s: write_data: Mode Control: %02x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:             Digital Volume Control Bypass: %d\n", machine().describe_context(), BIT(data, 0));
		LOGMASKED(LOG_WRITES, "%s:             De-Emphasis Frequency: %s\n", machine().describe_context(), DEM_NAMES[BIT(data, 1, 2)]);
		LOGMASKED(LOG_WRITES, "%s:             Ignore Channel B Attentuation/Volume: %d\n", machine().describe_context(), BIT(data, 3));
		LOGMASKED(LOG_WRITES, "%s:             Treble Boost Corner Frequency: %s\n", machine().describe_context(), TBCF_NAMES[BIT(data, 4, 2)]);
		LOGMASKED(LOG_WRITES, "%s:             Bass Boost Corner Frequency: %s\n", machine().describe_context(), BBCF_NAMES[BIT(data, 6, 2)]);
		m_mode_ctrl = data;
		break;
	case 0x08:
		LOGMASKED(LOG_WRITES, "%s: write_data: Limiter Attack Rate: %02x\n", machine().describe_context(), data);
		m_limiter_attack_rate = data;
		break;
	case 0x09:
		LOGMASKED(LOG_WRITES, "%s: write_data: Limiter Release Rate: %02x\n", machine().describe_context(), data);
		m_limiter_release_rate = data;
		break;
	case 0x0a:
		LOGMASKED(LOG_WRITES, "%s: write_data: Volume and Mixing Control: %02x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:             ATAPI Mix/Mute Mode: %d\n", machine().describe_context(), BIT(data, 0, 4));
		LOGMASKED(LOG_WRITES, "%s:             Peak Limiter Enable: %d\n", machine().describe_context(), BIT(data, 4));
		LOGMASKED(LOG_WRITES, "%s:             Tone Control Enable: %d\n", machine().describe_context(), BIT(data, 5));
		LOGMASKED(LOG_WRITES, "%s:             Tone Control Mode: %s\n", machine().describe_context(), TC_NAMES[BIT(data, 6, 2)]);
		m_volume_mix_ctrl = data;
		break;
	case 0x0b:
		LOGMASKED(LOG_WRITES, "%s: write_data: Mode Control 2: %02x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "%s:             Digital Interface Format: %s\n", machine().describe_context(), DIF_NAMES[BIT(data, 0, 3)]);
		LOGMASKED(LOG_WRITES, "%s:             Master Clock Divide Enable: %d\n", machine().describe_context(), BIT(data, 7));
		m_mode_ctrl_2 = data;
		break;
	default:
		LOGMASKED(LOG_WRITES, "%s: write_data: Unknown Register %04x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

void cs43l43_device::write_i2s(offs_t offset, u16 data)
{
	m_stream->update();

	constexpr sound_stream::sample_t SAMPLE_SCALE = 1.0 / 32768.0;

	const s16 sample = (s16)data;

	LOGMASKED(LOG_SAMPLES, "write_i2s: %04x to channel %d\n", data, offset);

	int maxin = (m_bufout[offset] + BUFFER_SIZE - 1) % BUFFER_SIZE;
	if (m_bufin[offset] != maxin)
	{
		m_buffer[offset][m_bufin[offset]] = sound_stream::sample_t(sample) * SAMPLE_SCALE;
		m_bufin[offset] = (m_bufin[offset] + 1) % BUFFER_SIZE;
	}
	else
	{
		printf("write_i2s: buffer overrun (short 1 frame on buffer %d)\n", offset); fflush(stdout);
		LOGMASKED(LOG_OVERRUNS, "write_i2s: buffer overrun (short 1 frame on buffer %d)\n", offset);
	}
}

void cs43l43_device::set_frequency(u32 frequency)
{
	m_frequency = double(frequency);
	m_stream->set_sample_rate(m_frequency);
}
