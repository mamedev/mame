// license:BSD-3-Clause
// copyright-holders:Alice Shelton, Peter Wilhelmsen
#include "emu.h"
#include "leapster_a.h"

#define LOG_SOUND    (1U << 1)

#define VERBOSE (LOG_SOUND)

#include "logmacro.h"


/*
    The legacy 16-bit voice interface may share a design with earlier LeapFrog
    products, but the speech engine's manufacturer and physical integration
    have not been established.

    The Leapster has 8 sound channels:
      0-4: Pitched audio: Take an ALAW waveform, with a register providing the pitch
      5-6: Raw audio: Simply take an 8 KHz ALAW waveform
        7: Speech: LFC, using quantised LPC reflection coefficients and excitation.
           Output is mono at 8 kHz, using a tenth-order synthesis filter.
           The first little-endian 16-bit word selects Level4/6/8 (0/1/2).
           Each record starts with a 16-bit control word; its upper four bits are zero.
           Bits 0-5 mark active subframes; bits 6-11 select pulses rather than noise.
           A nonzero active mask is followed by a 32-bit reflection-coefficient word.
           Its ten indices have widths 5,4,4,3,3,3,3,3,2,2 bits, least significant first.
           Indices select signed Q15 values from scalar tables at codebook + 0x880.
           The BIOS supplies the 0x4f00-byte codebook; no tables are embedded here.
           Each record represents six 32-sample subframes: 192 samples / 24 ms.
           Excitation payload belongs to the PREVIOUS record's control flags.
           Subframes are stored in descending flag-bit order, bit 5 through bit 0.
           Unvoiced subframes use one 16-bit word selecting a 32-sample noise vector,
           a five-bit gain index and a polarity bit; inactive subframes have no payload.
           Voiced subframes occupy 32/48/64 bits in Level4/6/8 respectively.
           They encode 4/6/8 signed pulses, a five-bit gain and a six-bit shape index.
           Level4 positions are even samples; Level6/8 can address all 32 positions.
           The first pulse sets the amplitude; the shape gives ratios for the others.
           Coincident pulses add; noise vectors, gains and shapes come from the codebook.
           Reflection coefficients are interpolated for each 32-sample subframe,
           then converted to prediction coefficients by the step-up recursion.
           Excitation drives the synthesis filter, retaining ten samples of history.
           Control 0x0fc0 terminates the stream after its pending excitation payload.
           A zero control word inside a stream is valid and does not terminate it.
           The current decoder discards 256 startup samples and drains a 192-sample tail.
           Interpolation and pipeline alignment were fitted empirically to source WAVs.
           Fixed-point arithmetic, saturation and exact hardware timing remain unverified.

    School House Rock! America Rock and Grammar Rock also use their own software
    "Screech2" decoder for separate CodecAudio assets, feeding the ARC PCM mixer.

    All 93,721 distinct valid indexed LFC streams in the 135 supplied cartridge dumps
    decoded successfully; one malformed Spider-Man L-MAX indexed entry is excluded.

    Two circular DMA channels let software mix decoded PCM with these voices.
    The first captures the legacy mix; the second feeds signed 16-bit PCM to
    the DAC. The observed 0x7f01 configuration runs at 32 kHz, with an interrupt
    after each half-buffer. Other control settings and analog filtering need
    further hardware verification.


*/

DEFINE_DEVICE_TYPE(LEAPSTER_SOUND, leapster_snd_device, "leapster_snd", "Leapster Sound")

leapster_snd_device::leapster_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LEAPSTER_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_pcm_irq_cb(*this)
{

}

void leapster_snd_device::do_voice_command(uint8_t command, uint8_t voice)
{
	LOGMASKED(LOG_SOUND, "Received command %i.%i\n", command, voice);

	m_stream->update();

	if(command == 0x10)
	{
		if(voice < 5)
		{
			m_channel_dx[voice] =  ((float) m_pitch[voice]) / 0x8000;
			m_channel_index[voice] = (float) BIT(m_data_source_start[voice], 0, 16);
			m_channel_triggered[voice] = true;
			LOGMASKED(LOG_SOUND, "Triggering channel %i, Diff: %i, Pitch register: %04hX (%f)\n",
				   voice, (BIT(m_data_source_end[voice], 16, 16) - BIT(m_data_source_end[voice], 0, 16)),
				   m_pitch[voice], m_channel_dx[voice]);
		}
		else if(voice < 7)
		{
			m_channel_dx[voice] = 1.0f;
			m_channel_index[voice] = 0.0f;
			m_channel_triggered[voice] = true;
			LOGMASKED(LOG_SOUND, "Triggering channel %i\n", voice);
		}
		else
		{
			lfc_start();
		}
	}
	else
	{
		m_channel_triggered[voice] = false;
	}
}

// The BIOS writes the low and high halves of (codebook_address >> 15).
// The 0x4f00-byte table must reside at offset 0x900 within that page.
void leapster_snd_device::lfc_codebook_w(offs_t offset, uint16_t data)
{
	m_stream->update();
	if (offset == 0)
		m_lfc_codebook_page = (m_lfc_codebook_page & 0xffff0000) | data;
	else if (offset == 2)
		m_lfc_codebook_page = (m_lfc_codebook_page & 0x0000ffff) | (uint32_t(data) << 16);
}

uint16_t leapster_snd_device::lfc_read_word()
{
	uint16_t const value = m_space->read_word(m_lfc_pointer);
	m_lfc_pointer += 2;
	return value;
}

double leapster_snd_device::lfc_table(uint32_t offset)
{
	return int16_t(m_space->read_word(((m_lfc_codebook_page << 15) | 0x900) + offset)) / 32768.0;
}

void leapster_snd_device::lfc_start()
{
	m_lfc_pointer = m_data_source_start[7];
	m_lfc_mode = lfc_read_word();
	m_lfc_flags = 0;
	m_lfc_position = 192;
	m_lfc_skip = 256;
	m_lfc_end = 0;
	for (auto &coeff : m_lfc_coeff)
		std::fill_n(coeff, 10, 0.0);
	std::fill_n(m_lfc_history, 10, 0.0);
	m_channel_triggered[7] = (m_lfc_mode <= 2) && m_lfc_codebook_page;
}

void leapster_snd_device::lfc_excitation(double *samples, bool voiced)
{
	uint64_t code = lfc_read_word();
	if (!voiced)
	{
		double const gain = 32768.0 * lfc_table(0x840 + 2 * BIT(code, 8, 5));
		double const sign = BIT(code, 13) ? -1.0 : 1.0;
		uint32_t const base = 0xf00 + 64 * BIT(code, 0, 8);
		for (int i = 0; i < 32; ++i)
			samples[i] = gain * sign * lfc_table(base + 2 * i);
		return;
	}

	for (int i = 1; i < 2 + m_lfc_mode; ++i)
		code |= uint64_t(lfc_read_word()) << (16 * i);

	static constexpr int positions[3][8] = {
		{ 12, 20, 24, 28 },
		{ 11, 22, 27, 32, 37, 42 },
		{ 11, 22, 27, 32, 37, 42, 48, 53 }
	};
	static constexpr int signs[3][8] = {
		{ 16, 17, 18, 19 },
		{ 16, 17, 18, 19, 20, 21 },
		{ 16, 17, 18, 19, 20, 21, 47, 63 }
	};
	static constexpr uint32_t shapes[] = { 0x000, 0x180, 0x400 };
	int const count = 4 + 2 * m_lfc_mode;
	double const gain = 32768.0 * lfc_table(0x780 + 0x40 * m_lfc_mode + 2 * BIT(code, 0, 5));
	uint32_t const shape = shapes[m_lfc_mode] + 2 * (count - 1) * BIT(code, 5, 6);
	for (int i = 0; i < count; ++i)
	{
		int const position = BIT(code, positions[m_lfc_mode][i], m_lfc_mode ? 5 : 4) * (m_lfc_mode ? 1 : 2);
		double const amplitude = gain * (i ? lfc_table(shape + 2 * (i - 1)) : 1.0);
		samples[position] += BIT(code, signs[m_lfc_mode][i]) ? -amplitude : amplitude;
	}
}

void leapster_snd_device::lfc_synth(double *samples)
{
	// Approximation fitted to the source recordings. Hardware interpolation,
	// fixed-point rounding, saturation and exact pipeline/tail timing are TODO.
	for (int block = 0; block < 6; ++block)
	{
		double const phase = (144.0 + 32.0 * block) / 192.0;
		int const first = phase < 1.0 ? 0 : 1;
		double const fraction = phase - first;
		double a[10]{};
		for (int order = 0; order < 10; ++order)
		{
			double const k = m_lfc_coeff[first][order] * (1.0 - fraction) + m_lfc_coeff[first + 1][order] * fraction;
			double old[10];
			std::copy_n(a, 10, old);
			for (int i = 0; i < order; ++i)
				a[i] = old[i] - k * old[order - 1 - i];
			a[order] = k;
		}
		for (int i = 0; i < 32; ++i)
		{
			double value = samples[32 * block + i];
			for (int j = 0; j < 10; ++j)
				value += a[j] * m_lfc_history[j];
			for (int j = 9; j > 0; --j)
				m_lfc_history[j] = m_lfc_history[j - 1];
			m_lfc_history[0] = value;
			samples[32 * block + i] = std::clamp(value, -32768.0, 32767.0);
		}
	}
}

bool leapster_snd_device::lfc_frame()
{
	if (m_lfc_end == 2)
		return false;
	std::fill_n(m_lfc_samples, 192, 0.0);
	std::copy_n(m_lfc_coeff[1], 10, m_lfc_coeff[0]);
	std::copy_n(m_lfc_coeff[2], 10, m_lfc_coeff[1]);
	if (m_lfc_end)
	{
		// Drain the synthesis filter after the terminal record's pending blocks.
		std::copy_n(m_lfc_coeff[2], 10, m_lfc_coeff[0]);
		m_lfc_end = 2;
	}
	else
	{
		uint16_t const flags = lfc_read_word();
		if (flags & 0xf000)
			return false;
		if (flags & 0x3f)
		{
			uint32_t code = lfc_read_word();
			code |= uint32_t(lfc_read_word()) << 16;
			static constexpr int bits[] = { 5, 4, 4, 3, 3, 3, 3, 3, 2, 2 };
			uint32_t table = 0x880;
			for (int i = 0; i < 10; ++i)
			{
				m_lfc_coeff[2][i] = lfc_table(table + 2 * BIT(code, 0, bits[i]));
				code >>= bits[i];
				table += 2 << bits[i];
			}
		}
		// Flags describe excitation carried by the following record. The six
		// subframes are stored in descending flag-bit order, each 32 samples.
		for (int i = 0; i < 6; ++i)
			if (BIT(m_lfc_flags, 5 - i))
				lfc_excitation(m_lfc_samples + 32 * i, BIT(m_lfc_flags, 11 - i));
		m_lfc_flags = flags;
		if (flags == 0x0fc0)
			m_lfc_end = 1;
	}
	lfc_synth(m_lfc_samples);
	m_lfc_position = 0;
	return true;
}

void leapster_snd_device::voice_generic_w(uint32_t off, uint16_t value)
{
	LOGMASKED(LOG_SOUND, "Wrote %04X to %03X\n", value, off << 1);
}

void leapster_snd_device::voice_start_w(uint32_t off, uint16_t value)
{
	auto channel = off >> 2;

	switch(BIT(off, 0, 2))
	{
		case 0:
			m_data_source_start[channel] = (((uint32_t) value) << 16) | BIT(m_data_source_start[channel], 0, 16);
			break;
		case 2:
			m_data_source_start[channel] = (BIT(m_data_source_start[channel], 16, 16) << 16) | value;
			break;
		case 3:
		default:
			LOGMASKED(LOG_SOUND, "%s: voice_start_w write to %i\n", machine().describe_context(), BIT(off, 0, 2));
	}
}

void leapster_snd_device::voice_end_w(uint32_t off, uint16_t value)
{
	auto channel = off >> 2;

	switch(BIT(off, 0, 2))
	{
		case 0:
			m_data_source_end[channel] = (((uint32_t) value) << 16) | BIT(m_data_source_end[channel], 0, 16);
			break;
		case 2:
			m_data_source_end[channel] = (BIT(m_data_source_end[channel], 16, 16) << 16) | value;
			break;
		default:
			LOGMASKED(LOG_SOUND, "%s: voice_start_w write to %i\n", machine().describe_context(), BIT(off, 0, 2));
	}
}

void leapster_snd_device::voice_volume_w(uint32_t off, uint16_t value)
{
	if(off % 1)
	{
		LOGMASKED(LOG_SOUND, "%s: voice_volume_w write odd halfword\n", machine().describe_context());
		return;
	}

	LOGMASKED(LOG_SOUND, "Set volume %i\n", value);

	m_volume[off >> 1] = value;
}

// 1.15 fixed point number giving "dx" value in bytes for the wave source each sampling tick
void leapster_snd_device::voice_pitch_w(uint32_t off, uint16_t value)
{
	if(off & 1)
	{
		LOGMASKED(LOG_SOUND, "%s: voice_pitch_w write odd halfword\n", machine().describe_context());
		return;
	}

	m_pitch[off >> 1] = value;
}

void leapster_snd_device::map(address_map &map)
{
	map(0x000, 0xfff).w(FUNC(leapster_snd_device::voice_generic_w));
	map(0x0c4, 0x103).w(FUNC(leapster_snd_device::voice_start_w));
	map(0x104, 0x13b).w(FUNC(leapster_snd_device::voice_end_w));
	map(0x13c, 0x15b).w(FUNC(leapster_snd_device::voice_volume_w));
	map(0x15c, 0x16f).w(FUNC(leapster_snd_device::voice_pitch_w));
}

void leapster_snd_device::device_start()
{
	m_stream = stream_alloc(0, 1, 32000);
	for (int i = 0; i < 2; ++i)
		m_pcm_timer[i] = timer_alloc(FUNC(leapster_snd_device::pcm_tick), this);
	save_item(NAME(m_pcm_control));
	save_item(NAME(m_pcm_base));
	save_item(NAME(m_pcm_position));
	save_item(NAME(m_legacy_sample));
	save_item(NAME(m_legacy_phase));

	save_item(NAME(m_lfc_codebook_page));
	save_item(NAME(m_lfc_pointer));
	save_item(NAME(m_lfc_mode));
	save_item(NAME(m_lfc_flags));
	save_item(NAME(m_lfc_position));
	save_item(NAME(m_lfc_skip));
	save_item(NAME(m_lfc_end));
	save_item(NAME(m_lfc_coeff));
	save_item(NAME(m_lfc_history));
	save_item(NAME(m_lfc_samples));
	save_item(NAME(m_data_source_start));
	save_item(NAME(m_data_source_end));
	save_item(NAME(m_volume));
	save_item(NAME(m_pitch));
	save_item(NAME(m_channel_triggered));
	save_item(NAME(m_channel_dx));
	save_item(NAME(m_channel_index));
}

void leapster_snd_device::device_reset()
{
	std::fill_n(m_channel_triggered, 8, false);
	std::fill_n(m_pcm_control, 2, 0);
	std::fill_n(m_pcm_base, 2, 0);
	std::fill_n(m_pcm_position, 2, 0);
	for (auto *timer : m_pcm_timer)
		timer->adjust(attotime::never);
	m_legacy_sample = 0;
	m_legacy_phase = 0;
	m_lfc_codebook_page = 0;
}


// The two circular DMA channels connect the legacy 8 kHz synthesizer to
// the ARC mixer, and the mixer's signed 16-bit PCM back to the DAC. Firmware
// refills alternating halves on status bits 20/21 of the shared audio IRQ.
uint32_t leapster_snd_device::pcm_r(offs_t offset)
{
	m_stream->update();
	int const channel = offset / 2;
	return (offset & 1) ? m_pcm_base[channel] + m_pcm_position[channel] : m_pcm_control[channel];
}

void leapster_snd_device::pcm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_stream->update();
	int const channel = offset / 2;
	if (offset & 1)
	{
		COMBINE_DATA(&m_pcm_base[channel]);
		m_pcm_position[channel] = 0;
	}
	else
	{
		COMBINE_DATA(&m_pcm_control[channel]);
		m_pcm_position[channel] = 0;
		// 0x7f01 selects two halves of 128 16-bit samples. The DAC is
		// clocked at 32 kHz; the ARC resamples 8/11.025 kHz codec audio.
		u32 const samples = ((m_pcm_control[channel] >> 8) & 0x7f) + 1;
		attotime const period = attotime::from_ticks(samples, 32000);
		m_pcm_timer[channel]->adjust(BIT(m_pcm_control[channel], 0) ? period : attotime::never, channel, period);
	}
}

TIMER_CALLBACK_MEMBER(leapster_snd_device::pcm_tick)
{
	m_stream->update();
	m_pcm_irq_cb(1U << param);
}

int32_t leapster_snd_device::legacy_sample()
{
	double mix = 0;
	for (int voice = 0; voice < 7; ++voice)
	{
		if (!m_channel_triggered[voice])
			continue;
		u32 const start = m_data_source_start[voice];
		float &position = m_channel_index[voice];
		u32 const address = voice < 5 ? (start & 0xffff0000) | u16(position) : start + u32(position);
		mix += conv_alaw_sample(m_space->read_byte(address)) * (0.125 * m_volume[voice] / 0x4000);
		position += m_channel_dx[voice];
		if (voice < 5)
		{
			u16 const end = m_data_source_end[voice] >> 16;
			u16 const loop = m_data_source_end[voice];
			if (position >= end)
			{
				if (int(end) - int(loop) < 2)
					m_channel_triggered[voice] = false;
				else
					position = position - end + loop;
			}
		}
		else if (start + u32(position) >= m_data_source_end[voice])
			m_channel_triggered[voice] = false;
	}
	while (m_channel_triggered[7])
	{
		if (m_lfc_position == 192 && !lfc_frame())
		{
			m_channel_triggered[7] = false;
			break;
		}
		double const sample = m_lfc_samples[m_lfc_position++];
		if (m_lfc_skip)
			--m_lfc_skip;
		else
		{
			mix += sample * (0.125 * m_volume[7] / 0x4000);
			break;
		}
	}
	return std::clamp<int32_t>(mix, -32768, 32767);
}

void leapster_snd_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); ++i)
	{
		if (!m_legacy_phase)
			m_legacy_sample = legacy_sample();
		m_legacy_phase = (m_legacy_phase + 1) & 3;
		int32_t sample = m_legacy_sample;
		if (BIT(m_pcm_control[0], 0))
			m_space->write_word(m_pcm_base[0] + m_pcm_position[0], u16(sample));
		if (BIT(m_pcm_control[1], 0))
			sample = int16_t(m_space->read_word(m_pcm_base[1] + m_pcm_position[1]));
		for (int channel = 0; channel < 2; ++channel)
			if (BIT(m_pcm_control[channel], 0))
				m_pcm_position[channel] = (m_pcm_position[channel] + 2) % ((((m_pcm_control[channel] >> 8) & 0x7f) + 1) * 4);
		stream.put_int(0, i, sample, 32768);
	}
}
