// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Silicon Systems SSI-263A Phoneme Speech Synthesizer

    Temporary implementation using the Votrax SC-01A

    NOTE: This is completely wrong, and exists only to have
    working audio in Thayer's Quest, which would not otherwise
    be playable due to relying on speech output for important
    gameplay cues.

****************************************************************************/

#include "emu.h"

#include "ssi263hle.h"


DEFINE_DEVICE_TYPE(SSI263HLE, ssi263hle_device, "ssi263hle", "SSI-263A Speech Synthesizer")

namespace
{

static const char PHONEME_NAMES[0x40][5] =
{
	"PA", "E", "E1", "Y", "YI", "AY", "IE", "I", "A", "AI", "EH", "EH1", "AE", "AE1", "AH", "AH1", "W", "O", "OU", "OO", "IU", "IU1", "U", "U1", "UH", "UH1", "UH2", "UH3", "ER", "R", "R1", "R2",
	"L", "L1", "LF", "W", "B", "D", "KV", "P", "T", "K", "HV", "HVC", "HF", "HFC", "HN", "Z", "S", "J", "SCH", "V", "F", "THV", "TH", "M", "N", "NG", ":A", ":OH", ":U", ":UH", "E2", "LB"
};

static const u8 PHONEMES_TO_SC01[0x40] =
{
	0x03, 0x2c, 0x3b, 0x3c, 0x22, 0x21, 0x29, 0x27, 0x20, 0x05, 0x01, 0x00, 0x2e, 0x2f, 0x15, 0x15,
	0x13, 0x26, 0x35, 0x17, 0x36, 0x16, 0x28, 0x37, 0x32, 0x32, 0x31, 0x23, 0x3a, 0x2b, 0x2b, 0x2b,
	0x18, 0x18, 0x18, 0x2d, 0x0e, 0x1e, 0x1c, 0x25, 0x2a, 0x19, 0x03, 0x03, 0x1b, 0x03, 0x03, 0x12,
	0x1f, 0x07, 0x11, 0x0f, 0x1d, 0x38, 0x39, 0x0c, 0x0d, 0x14, 0x08, 0x34, 0x28, 0x37, 0x02, 0x18
};

} // anonymous namespace

ssi263hle_device::ssi263hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SSI263HLE, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_votrax(*this, "votrax")
	, m_ar_cb(*this)
	, m_phoneme_timer(nullptr)
	, m_duration(0)
	, m_phoneme(0)
	, m_inflection(0)
	, m_rate(0)
	, m_articulation(0)
	, m_control(false)
	, m_amplitude(0)
	, m_filter(0)
	, m_mode(0)
	, m_data_request(1)
	, m_votrax_fifo_wr(0)
	, m_votrax_fifo_rd(0)
	, m_votrax_fifo_cnt(0)
{
}

void ssi263hle_device::device_start()
{
	m_phoneme_timer = timer_alloc(FUNC(ssi263hle_device::phoneme_tick), this);

	save_item(NAME(m_duration));
	save_item(NAME(m_phoneme));
	save_item(NAME(m_inflection));
	save_item(NAME(m_rate));
	save_item(NAME(m_articulation));
	save_item(NAME(m_control));
	save_item(NAME(m_amplitude));
	save_item(NAME(m_filter));
	save_item(NAME(m_mode));

	save_item(NAME(m_votrax_fifo));
	save_item(NAME(m_votrax_fifo_wr));
	save_item(NAME(m_votrax_fifo_rd));
	save_item(NAME(m_votrax_fifo_cnt));
}

void ssi263hle_device::device_reset()
{
	m_phoneme_timer->adjust(attotime::never);

	m_duration = 0;
	m_phoneme = 0;
	m_inflection = 0;
	m_rate = 0;
	m_articulation = 0;
	m_control = false;
	m_amplitude = 0;
	m_filter = 0;
	m_mode = 0;

	m_data_request = 1;

	std::fill(std::begin(m_votrax_fifo), std::end(m_votrax_fifo), 0);
	m_votrax_fifo_wr = 0;
	m_votrax_fifo_rd = 0;
	m_votrax_fifo_cnt = 0;
}

void ssi263hle_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(ssi263hle_device::status_r), FUNC(ssi263hle_device::duration_phoneme_w));
	map(0x01, 0x01).rw(FUNC(ssi263hle_device::status_r), FUNC(ssi263hle_device::inflection_w));
	map(0x02, 0x02).rw(FUNC(ssi263hle_device::status_r), FUNC(ssi263hle_device::rate_inflection_w));
	map(0x03, 0x03).rw(FUNC(ssi263hle_device::status_r), FUNC(ssi263hle_device::control_articulation_amplitude_w));
	map(0x04, 0x07).rw(FUNC(ssi263hle_device::status_r), FUNC(ssi263hle_device::filter_frequency_w));
}

void ssi263hle_device::device_add_mconfig(machine_config &config)
{
	VOTRAX_SC01(config, m_votrax, DERIVED_CLOCK(1, 1));
	m_votrax->ar_callback().set(FUNC(ssi263hle_device::votrax_request));
	m_votrax->add_route(ALL_OUTPUTS, *this, 1.0, 0);
}

TIMER_CALLBACK_MEMBER(ssi263hle_device::phoneme_tick)
{
	m_data_request = 0;
	m_ar_cb(m_data_request);
}

void ssi263hle_device::duration_phoneme_w(u8 data)
{
	const int frame_time = ((4096 * (16 - m_rate)) / 2); // microseconds, should actually be derived from our clock, but this way we get microseconds directly
	const int phoneme_time = frame_time * (4 - m_duration); // microseconds

	m_duration = (data >> 5) & 0x03;
	m_phoneme = data & 0x3f;

	m_data_request = 1;
	m_ar_cb(m_data_request);

	switch (m_mode)
	{
		case 0:
		case 1:
			// phoneme timing response
			m_phoneme_timer->adjust(attotime::from_usec(phoneme_time));
			break;
		case 2:
			// frame timing response
			m_phoneme_timer->adjust(attotime::from_usec(frame_time));
			break;
		case 3:
			// disable A/_R output
			break;
	}

	if (m_phoneme)
	{
		if (m_votrax_fifo_cnt < std::size(m_votrax_fifo))
		{
			m_votrax_fifo[m_votrax_fifo_wr] = PHONEMES_TO_SC01[m_phoneme];
			if (m_votrax_fifo_cnt == 0)
			{
				m_votrax->write(PHONEMES_TO_SC01[m_phoneme]);
			}
			m_votrax_fifo_wr = (m_votrax_fifo_wr + 1) % std::size(m_votrax_fifo);
			m_votrax_fifo_cnt++;
		}
	}
}

void ssi263hle_device::inflection_w(u8 data)
{
	m_inflection &= 0x807;
	m_inflection |= data << 3;
}

void ssi263hle_device::rate_inflection_w(u8 data)
{
	m_inflection &= 0x7f8;
	m_inflection |= (BIT(data, 3) << 11) | (data & 0x07);
	m_rate = data >> 4;
	m_votrax->inflection_w(1);
}

void ssi263hle_device::control_articulation_amplitude_w(u8 data)
{
	if (m_control && !BIT(data, 7))
	{
		m_mode = m_duration;
	}

	m_control = BIT(data, 7);
	m_articulation = (data >> 4) & 0x07;
	m_amplitude = data & 0x0f;
}

void ssi263hle_device::filter_frequency_w(u8 data)
{
	m_filter = data;
}

u8 ssi263hle_device::status_r()
{
	// D7 is an output for the inverted state of A/_R. Register address bits are ignored.
	return BIT(~m_data_request, 0) << 7;
}

void ssi263hle_device::votrax_request(int state)
{
	if (m_votrax_fifo_cnt == 0 || state != ASSERT_LINE)
	{
		return;
	}

	m_votrax_fifo_cnt--;
	const u8 previous_phoneme = m_votrax_fifo[m_votrax_fifo_rd];
	m_votrax_fifo_rd = (m_votrax_fifo_rd + 1) % std::size(m_votrax_fifo);
	if (m_votrax_fifo_cnt == 0)
	{
		if (previous_phoneme != 0x3f)
		{
			m_votrax_fifo[m_votrax_fifo_wr] = 0x3f;
			m_votrax_fifo_wr = (m_votrax_fifo_wr + 1) % std::size(m_votrax_fifo);
			m_votrax_fifo_cnt++;
			m_votrax->write(0x3f);
		}
		return;
	}

	m_votrax->write(m_votrax_fifo[m_votrax_fifo_rd]);
}
