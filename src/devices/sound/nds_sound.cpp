// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    nds_sound.cpp
    Nintendo DS sound: 16 channels of PCM, ADPCM, PSG, or noise
    Emulation by R. Belmont

    TODO:
    - bugs, lots of bugs
    - sound DMA FIFOs and bus contention

***************************************************************************/

#include "emu.h"
#include "nds_sound.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(NDS_SOUND, nds_sound_device, "nds_sound", "Nintendo DS sound")

namespace
{

// IMA-ADPCM magic numbers
constexpr int16_t s_ima_step[89] =
{
	7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
	19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
	130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

constexpr int s_ima_index[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

} // anonymous namespace


nds_sound_device::nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NDS_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_space(nullptr)
	, m_sync_timer(nullptr)
	, m_capture_timer{}
	, m_capture_remaining{}
	, m_last_update(0)
	, m_muted(false)
	, m_sleeping(false)
	, m_regs{}
	, m_chan{}
	, m_capture{}
{
}

void nds_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, OUTPUT_RATE);
	m_sync_timer = timer_alloc(FUNC(nds_sound_device::sync_tick), this);
	for (auto &timer : m_capture_timer)
	{
		timer = timer_alloc(FUNC(nds_sound_device::capture_tick), this);
	}

	save_item(NAME(m_last_update));
	save_item(NAME(m_muted));
	save_item(NAME(m_sleeping));
	save_item(NAME(m_capture_remaining));
	save_item(NAME(m_regs));
	save_item(STRUCT_MEMBER(m_chan, active));
	save_item(STRUCT_MEMBER(m_chan, format));
	save_item(STRUCT_MEMBER(m_chan, repeat));
	save_item(STRUCT_MEMBER(m_chan, volume));
	save_item(STRUCT_MEMBER(m_chan, vol_shift));
	save_item(STRUCT_MEMBER(m_chan, pan));
	save_item(STRUCT_MEMBER(m_chan, duty));
	save_item(STRUCT_MEMBER(m_chan, hold));
	save_item(STRUCT_MEMBER(m_chan, sad));
	save_item(STRUCT_MEMBER(m_chan, loopstart));
	save_item(STRUCT_MEMBER(m_chan, length));
	save_item(STRUCT_MEMBER(m_chan, addr));
	save_item(STRUCT_MEMBER(m_chan, frac));
	save_item(STRUCT_MEMBER(m_chan, sample));
	save_item(STRUCT_MEMBER(m_chan, startup_delay));
	save_item(STRUCT_MEMBER(m_chan, adpcm_val));
	save_item(STRUCT_MEMBER(m_chan, adpcm_index));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_val));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_index));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_saved));
	save_item(STRUCT_MEMBER(m_chan, adpcm_low_nibble));
	save_item(STRUCT_MEMBER(m_chan, psg_phase));
	save_item(STRUCT_MEMBER(m_chan, noise_lfsr));
	save_item(STRUCT_MEMBER(m_capture, dad));
	save_item(STRUCT_MEMBER(m_capture, length));
	save_item(STRUCT_MEMBER(m_capture, offset));
}

void nds_sound_device::device_reset()
{
	m_last_update = machine().time().as_ticks(timer_clock());
	m_muted = false;
	m_sleeping = false;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (auto &c : m_chan)
	{
		c = channel_t{};
	}
	m_sync_timer->adjust(attotime::never);
	for (int cap = 0; cap < NUM_CAPTURES; cap++)
	{
		m_capture[cap] = capture_t{};
		m_capture_remaining[cap] = attotime::never;
		m_capture_timer[cap]->adjust(attotime::never);
	}
}

// the channel timers count up at half the 33.513982 MHz bus clock
uint32_t nds_sound_device::timer_clock() const
{
	return clock() / 2;
}

uint32_t nds_sound_device::chan_period(int ch) const
{
	const uint32_t tmr = m_regs[(ch * 4) + 2] & 0xffff;
	return 0x10000 - tmr;
}

void nds_sound_device::update()
{
	m_stream->update();
	advance_channels(machine().time().as_ticks(timer_clock()));
}

// Streamed audio is refilled just behind the play position, so keep the stream
// within one output sample of the CPUs while a channel reads sample memory.
void nds_sound_device::update_sync()
{
	bool fetching = false;
	if (!m_muted && !m_sleeping && BIT(m_regs[0x100 / 4], 15))
	{
		for (const channel_t &c : m_chan)
		{
			if (c.active && (c.format != FORMAT_PSG))
			{
				fetching = true;
				break;
			}
		}
	}

	if (fetching && m_sync_timer->expire().is_never())
	{
		const attotime period = attotime::from_hz(OUTPUT_RATE);
		m_sync_timer->adjust(period, 0, period);
	}
	else if (!fetching && !m_sync_timer->expire().is_never())
	{
		m_sync_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(nds_sound_device::sync_tick)
{
	update();
	update_sync();
}

void nds_sound_device::advance_channels(uint64_t ticks)
{
	if (ticks <= m_last_update)
	{
		return;
	}

	const uint64_t elapsed = ticks - m_last_update;
	m_last_update = ticks;
	if (m_sleeping || !BIT(m_regs[0x100 / 4], 15))
	{
		return;
	}

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		channel_t &c = m_chan[ch];
		if (c.active)
		{
			uint64_t clocks = c.frac + elapsed;
			const uint32_t period = chan_period(ch);
			while (c.active && (clocks >= period))
			{
				clocks -= period;
				advance_sample(c, ch);
			}
			c.frac = c.active ? clocks : 0;
		}
	}
}

void nds_sound_device::update_controls(int ch)
{
	channel_t &c = m_chan[ch];
	const uint32_t cnt = m_regs[ch * 4];

	c.volume = cnt & 0x7f;
	static constexpr uint8_t div_shift[4] = { 0, 1, 2, 4 };
	c.vol_shift = div_shift[(cnt >> 8) & 3];
	c.hold = BIT(cnt, 15);
	c.pan = (cnt >> 16) & 0x7f;
	c.duty = (cnt >> 24) & 7;

	if (!c.active && !c.hold)
	{
		c.sample = 0;
	}
}

void nds_sound_device::key_on(int ch)
{
	channel_t &c = m_chan[ch];
	const uint32_t cnt = m_regs[ch * 4];

	update_controls(ch);
	c.repeat = (cnt >> 27) & 3;
	c.format = (cnt >> 29) & 3;

	c.sad = m_regs[(ch * 4) + 1] & 0x07fffffc;
	const uint32_t pnt = (m_regs[(ch * 4) + 2] >> 16) & 0xffff;
	const uint32_t len = m_regs[(ch * 4) + 3] & 0x3fffff;
	c.loopstart = pnt * 4;
	c.length = (pnt + len) * 4;

	c.frac = 0;
	c.startup_delay = 0;
	c.adpcm_loop_saved = false;

	// Sample data follows the startup delay; ADPCM initializes its decoder here.
	c.addr = c.sad;
	switch (c.format)
	{
		case FORMAT_PCM8:
		case FORMAT_PCM16:
			if (!c.hold)
			{
				c.sample = 0;
			}
			c.startup_delay = 3;
			break;

		case FORMAT_ADPCM:
		{
			const uint32_t header = m_space ? m_space->read_dword(c.addr) : 0;
			c.adpcm_val = int16_t(header & 0xffff);
			c.adpcm_index = std::clamp<int32_t>((header >> 16) & 0x7f, 0, 88);
			c.adpcm_low_nibble = true;
			if (!c.hold)
			{
				c.sample = 0;
			}
			// Three startup periods plus eight periods for the header.
			c.startup_delay = 11;
			c.addr += 4;
			break;
		}

		case FORMAT_PSG:
			c.psg_phase = 0;
			c.noise_lfsr = 0x7fff;
			c.sample = 0;
			break;
	}

	c.active = true;
}

void nds_sound_device::advance_sample(channel_t &c, int ch)
{
	if (c.startup_delay != 0)
	{
		// A held sample survives only the first startup period.
		c.sample = 0;
		c.startup_delay--;
		if (c.startup_delay != 0)
		{
			return;
		}
	}

	if (c.format == FORMAT_PSG)
	{
		if (ch >= 14)
		{
			// LFSR noise
			if (c.noise_lfsr & 1)
			{
				c.noise_lfsr = (c.noise_lfsr >> 1) ^ 0x6000;
				c.sample = -0x7fff;
			}
			else
			{
				c.noise_lfsr >>= 1;
				c.sample = 0x7fff;
			}
		}
		else if (ch >= 8)
		{
			// Square waves begin at the start of the low segment.
			const int low = (c.duty == 7) ? 8 : (7 - c.duty);
			c.sample = (int(c.psg_phase) < low) ? -0x7fff : 0x7fff;
			c.psg_phase = (c.psg_phase + 1) & 7;
		}
		return;
	}

	// PCM/ADPCM blocks below four total words stay silent with busy set.
	if (c.length < 16)
	{
		c.sample = 0;
		return;
	}

	// end of the sample?
	if (c.addr >= (c.sad + c.length))
	{
		if (c.repeat == 1)
		{
			c.addr = c.sad + c.loopstart;
			if ((c.format == FORMAT_ADPCM) && c.adpcm_loop_saved)
			{
				c.adpcm_val = c.adpcm_loop_val;
				c.adpcm_index = c.adpcm_loop_index;
				c.adpcm_low_nibble = true;
			}
		}
		else
		{
			c.active = false;
			if (!c.hold)
			{
				c.sample = 0;
			}
			return;
		}
	}

	// save the ADPCM decoder state at loop start
	if ((c.format == FORMAT_ADPCM) && !c.adpcm_loop_saved && (c.addr >= (c.sad + c.loopstart)) && (c.loopstart >= 4))
	{
		c.adpcm_loop_val = c.adpcm_val;
		c.adpcm_loop_index = c.adpcm_index;
		c.adpcm_loop_saved = true;
	}

	if (!m_space)
	{
		return;
	}

	switch (c.format)
	{
		case FORMAT_PCM8:
			c.sample = int16_t(int8_t(m_space->read_byte(c.addr))) << 8;
			c.addr += 1;
			break;

		case FORMAT_PCM16:
			c.sample = int16_t(m_space->read_word(c.addr));
			c.addr += 2;
			break;

		case FORMAT_ADPCM:
		{
			const uint8_t byte = m_space->read_byte(c.addr);
			const int nibble = c.adpcm_low_nibble ? (byte & 0xf) : (byte >> 4);

			const int step = s_ima_step[c.adpcm_index];
			int diff = step >> 3;
			if (nibble & 1)
			{
				diff += step >> 2;
			}
			if (nibble & 2)
			{
				diff += step >> 1;
			}
			if (nibble & 4)
			{
				diff += step;
			}
			if (nibble & 8)
			{
				c.adpcm_val = std::max<int32_t>(c.adpcm_val - diff, -0x7fff);
			}
			else
			{
				c.adpcm_val = std::min<int32_t>(c.adpcm_val + diff, 0x7fff);
			}
			c.adpcm_index = std::clamp<int32_t>(c.adpcm_index + s_ima_index[nibble & 7], 0, 88);
			c.sample = c.adpcm_val;

			if (c.adpcm_low_nibble)
			{
				c.adpcm_low_nibble = false;
			}
			else
			{
				c.adpcm_low_nibble = true;
				c.addr += 1;
			}
			break;
		}
	}

	// Busy clears when the last sample begins; its output lasts one more period.
	if ((c.repeat != 1) && (c.addr >= (c.sad + c.length)))
	{
		m_regs[ch * 4] &= ~0x80000000;
	}
}

void nds_sound_device::mute_all()
{
	update();
	m_muted = true;
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		m_chan[ch].active = false;
		m_chan[ch].sample = 0;
		m_regs[ch * 4] &= ~0x80000000;
	}
	m_regs[0x108 / 4] &= ~0x8080;
	for (auto &timer : m_capture_timer)
	{
		timer->adjust(attotime::never);
	}
	update_sync();
}

void nds_sound_device::set_sleep(bool sleeping)
{
	if (m_sleeping == sleeping)
	{
		return;
	}

	update();
	m_sleeping = sleeping;
	for (int cap = 0; cap < NUM_CAPTURES; cap++)
	{
		if (sleeping)
		{
			m_capture_remaining[cap] = m_capture_timer[cap]->remaining();
			m_capture_timer[cap]->adjust(attotime::never);
		}
		else if (BIT(m_regs[0x108 / 4], cap * 8 + 7))
		{
			m_capture_timer[cap]->adjust(m_capture_remaining[cap], cap);
		}
	}
	update_sync();
}

uint32_t nds_sound_device::read(offs_t offset, uint32_t mem_mask)
{
	if (offset >= std::size(m_regs))
	{
		return 0;
	}
	if (!machine().side_effects_disabled())
	{
		update();
	}
	// TODO: Verify the bus value for write-only register reads; use zero for now.
	if (offset < (NUM_CHANNELS * 4))
	{
		return ((offset & 3) == 0) ? m_regs[offset] : 0;
	}
	switch (offset)
	{
		case 0x100 / 4: // SOUNDCNT
		case 0x104 / 4: // SOUNDBIAS
		case 0x108 / 4: // SNDCAP0CNT/SNDCAP1CNT
		case 0x110 / 4: // SNDCAP0DAD
		case 0x118 / 4: // SNDCAP1DAD
			return m_regs[offset];

		default:
			return 0;
	}
}

void nds_sound_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= std::size(m_regs))
	{
		return;
	}

	update();

	if (offset < (NUM_CHANNELS * 4))
	{
		static constexpr uint32_t CHANNEL_MASKS[4] =
		{
			0xff7f837f, // SOUNDxCNT
			0x07fffffc, // SOUNDxSAD
			0xffffffff, // SOUNDxTMR/SOUNDxPNT
			0x003fffff  // SOUNDxLEN
		};
		mem_mask &= CHANNEL_MASKS[offset & 3];
	}
	else
	{
		switch (offset)
		{
			case 0x100 / 4: // SOUNDCNT
				mem_mask &= 0xbf7f;
				break;

			case 0x104 / 4: // SOUNDBIAS
				mem_mask &= 0x3ff;
				break;

			case 0x108 / 4: // SNDCAP0CNT/SNDCAP1CNT
				mem_mask &= 0x8f8f;
				break;

			case 0x110 / 4: // SNDCAP0DAD
			case 0x118 / 4: // SNDCAP1DAD
				mem_mask &= 0x07fffffc;
				break;

			case 0x114 / 4: // SNDCAP0LEN
			case 0x11c / 4: // SNDCAP1LEN
				mem_mask &= 0xffff;
				break;

			default:
				return;
		}
	}

	const uint32_t old = m_regs[offset];
	COMBINE_DATA(&m_regs[offset]);
	if ((offset < (NUM_CHANNELS * 4)) && ((offset & 3) == 0))
	{
		const int ch = offset / 4;
		if (BIT(m_regs[offset], 31) && !BIT(old, 31))
		{
			key_on(ch);
		}
		else
		{
			if (BIT(mem_mask, 31) && !BIT(m_regs[offset], 31))
			{
				m_chan[ch].active = false;
			}
			update_controls(ch);
		}
		update_sync();
	}
	else if (offset == (0x100 / 4))
	{
		update_sync();
	}
	else if (offset == (0x108 / 4))
	{
		for (int cap = 0; cap < NUM_CAPTURES; cap++)
		{
			const int bit = cap * 8 + 7;
			if (BIT(m_regs[offset], bit) && !BIT(old, bit))
			{
				capture_start(cap);
			}
			else if (!BIT(m_regs[offset], bit))
			{
				m_capture_timer[cap]->adjust(attotime::never);
			}
		}
	}
}

void nds_sound_device::capture_start(int cap)
{
	capture_t &c = m_capture[cap];
	c.dad = m_regs[0x110 / 4 + cap * 2];
	c.length = std::max<uint32_t>(m_regs[0x114 / 4 + cap * 2], 1) * 4;
	c.offset = 0;
	const attotime period = attotime::from_ticks(chan_period(cap * 2 + 1), timer_clock());
	m_capture_remaining[cap] = period;
	m_capture_timer[cap]->adjust(m_sleeping ? attotime::never : period, cap);
}

TIMER_CALLBACK_MEMBER(nds_sound_device::capture_tick)
{
	update();
	const bool master_enable = BIT(m_regs[0x100 / 4], 15);
	int32_t capture[NUM_CAPTURES] = { 0, 0 };
	if (master_enable)
	{
		int32_t output[2];
		mix(output, capture);
	}

	// Captures due together share one mix, taken before a one-shot stop changes it.
	for (int cap = 0; cap < NUM_CAPTURES; cap++)
	{
		if ((cap == param) || (m_capture_timer[cap]->enabled() && m_capture_timer[cap]->remaining().is_zero()))
		{
			if (master_enable)
			{
				capture_sample(cap, capture[cap]);
			}
			if (BIT(m_regs[0x108 / 4], cap * 8 + 7))
			{
				m_capture_timer[cap]->adjust(attotime::from_ticks(chan_period(cap * 2 + 1), timer_clock()), cap);
			}
			else
			{
				m_capture_timer[cap]->adjust(attotime::never);
			}
		}
	}
}

void nds_sound_device::capture_sample(int cap, int32_t sample)
{
	const uint8_t cnt = m_regs[0x108 / 4] >> (cap * 8);
	const int shift = BIT(cnt, 3) ? 16 : 8;
	// Negative samples with the top fraction bit set round towards zero.
	if ((sample < 0) && BIT(sample, shift - 1))
	{
		sample += 1 << shift;
	}
	sample >>= shift;

	capture_t &c = m_capture[cap];
	if (m_space)
	{
		const uint32_t address = (c.dad + c.offset) & 0x07ffffff;
		if (BIT(cnt, 3))
		{
			m_space->write_byte(address, uint8_t(sample));
		}
		else
		{
			m_space->write_word(address, uint16_t(sample));
		}
	}
	c.offset += BIT(cnt, 3) ? 1 : 2;
	if (c.offset >= c.length)
	{
		c.offset = 0;
		if (BIT(cnt, 2))
		{
			m_regs[0x108 / 4] &= ~(0x80 << (cap * 8));
		}
	}
}

void nds_sound_device::mix(int32_t (&output)[2], int32_t (&capture)[NUM_CAPTURES]) const
{
	const uint32_t soundcnt = m_regs[0x100 / 4];
	int32_t volume[NUM_CHANNELS];
	int32_t panned[NUM_CHANNELS][2];
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		const channel_t &c = m_chan[ch];
		const int vol = (c.volume == 127) ? 128 : c.volume;
		const int pan = (c.pan == 127) ? 128 : c.pan;
		// Divider and volume retain 11 fractional bits; pan retains eight.
		volume[ch] = ((int32_t(c.sample) << 4) >> c.vol_shift) * vol;
		panned[ch][0] = (int64_t(volume[ch]) * (128 - pan)) >> 10;
		panned[ch][1] = (int64_t(volume[ch]) * pan) >> 10;
	}

	int32_t mixer[2] = { 0, 0 };
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		if ((ch == 1) || (ch == 3))
		{
			const int cap = ch / 2;
			const uint8_t cnt = m_regs[0x108 / 4] >> (cap * 8);
			if (BIT(soundcnt, 12 + cap))
			{
				continue;
			}
			if ((cnt & 0x81) == 0x81)
			{
				const channel_t &a = m_chan[ch - 1];
				if (a.active || a.hold)
				{
					const int pan = (a.pan == 127) ? 128 : a.pan;
					mixer[0] += (int64_t(volume[ch]) * (128 - pan)) >> 10;
					mixer[1] += (int64_t(volume[ch]) * pan) >> 10;
				}
				continue;
			}
		}
		mixer[0] += panned[ch][0];
		mixer[1] += panned[ch][1];
	}

	for (int cap = 0; cap < NUM_CAPTURES; cap++)
	{
		const uint8_t cnt = m_regs[0x108 / 4] >> (cap * 8);
		if (BIT(cnt, 1))
		{
			const int32_t a = volume[cap * 2];
			const int32_t b = volume[cap * 2 + 1];
			if ((cnt & 0x81) == 0x81)
			{
				// The capture adder wraps at 16 integer bits, unlike the mixer path.
				capture[cap] = util::sext((a + b) >> 3, 24);
			}
			else
			{
				// Hardware fault even with addition off: both negative clips to minimum.
				capture[cap] = ((a < 0) && (b < 0)) ? -0x800000 : (a >> 3);
			}
		}
		else
		{
			capture[cap] = std::clamp(mixer[cap], -0x800000, 0x7fff00);
		}

		const int select = (soundcnt >> (8 + cap * 2)) & 3;
		switch (select)
		{
			case 0:
				output[cap] = mixer[cap];
				break;

			case 1:
				output[cap] = panned[1][cap];
				break;

			case 2:
				output[cap] = panned[3][cap];
				break;

			case 3:
				output[cap] = panned[1][cap] + panned[3][cap];
				break;
		}
	}
}

void nds_sound_device::sound_stream_update(sound_stream &stream)
{
	if (m_muted || m_sleeping)
	{
		stream.fill(0, 0);
		stream.fill(1, 0);
		return;
	}

	const uint32_t soundcnt = m_regs[0x100 / 4];
	const bool master_enable = BIT(soundcnt, 15);
	const int master_vol = ((soundcnt & 0x7f) == 127) ? 128 : (soundcnt & 0x7f);
	const int bias = m_regs[0x104 / 4] & 0x3ff;
	attotime time = stream.start_time();
	const attotime period = stream.sample_period();

	for (int i = 0; i < stream.samples(); i++, time += period)
	{
		advance_channels(time.as_ticks(timer_clock()));
		int32_t output[2] = { 0, 0 };
		if (master_enable)
		{
			int32_t capture[NUM_CAPTURES];
			mix(output, capture);
			// Strip the eight mixer fraction bits, seven volume bits and six PCM bits.
			output[0] = (int64_t(output[0]) * master_vol) >> 21;
			output[1] = (int64_t(output[1]) * master_vol) >> 21;
		}

		// The bias reaches the 10-bit PWM even with master enable off.
		output[0] = std::clamp(output[0] + bias, 0, 0x3ff) - 0x200;
		output[1] = std::clamp(output[1] + bias, 0, 0x3ff) - 0x200;
		stream.put_int(0, i, output[0], 512);
		stream.put_int(1, i, output[1], 512);
	}
}
