// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Tone generators of the KN2400, KN6000 and KN7000 families. The register
// interface is decoded. The KN7000's voices play their wave ROMs; the wave
// ROMs of the other models are undumped, so their streams are silent.

#include "emu.h"
#include "kn_tonegen.h"

#include <algorithm>
#include <cmath>
#include <iterator>

DEFINE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device, "kn6000_tonegen", "KN6000 tone generator")
DEFINE_DEVICE_TYPE(KN2400_TONEGEN, kn2400_tonegen_device, "kn2400_tonegen", "KN2400 tone generator")
DEFINE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device, "kn7000_tonegen", "KN7000 tone generator")

kn_tonegen_base_device::kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int inputs)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_inputs(inputs)
	, m_wave_config{
			{ "wave_y", ENDIANNESS_LITTLE, 16, 24, -1, address_map_constructor(FUNC(kn_tonegen_base_device::wave_bus_map), this) },
			{ "wave_x", ENDIANNESS_LITTLE, 16, 24, -1, address_map_constructor(FUNC(kn_tonegen_base_device::wave_bus_map), this) } }
	, m_wave_rom(*this, { "waveform_y", "waveform_x" })
{
}

device_memory_interface::space_config_vector kn_tonegen_base_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_wave_config[0]),
		std::make_pair(1, &m_wave_config[1]) };
}

// Nothing answers above the ROMs on a wave bus without an expansion board
void kn_tonegen_base_device::wave_bus_map(address_map &map)
{
	map.unmap_value_high();
}

void kn_tonegen_base_device::device_start()
{
	for (uint32_t i = 0; i < std::size(m_wave); i++)
	{
		space(i).install_rom(0, m_wave_rom[i]->bytes() / 2 - 1, m_wave_rom[i]->base());
		space(i).cache(m_wave[i]);
	}

	// One stereo sample every 384 clocks. A device configured without a clock
	// is silent and takes the output rate.
	m_stream = stream_alloc(m_inputs, 2, clock() ? (clock() / 384) : SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_regs = make_unique_clear<uint16_t []>(0x10000);
	save_pointer(NAME(m_regs), 0x10000);

	m_addr = 0;
	m_wave_bank = 0;
	m_wave_addr = 0;
	save_item(NAME(m_addr));
	save_item(NAME(m_wave_bank));
	save_item(NAME(m_wave_addr));
}

uint16_t kn_tonegen_base_device::wave_word(uint32_t word)
{
	return m_wave[word & 1].read_word(word >> 1);
}

void kn_tonegen_base_device::tg_write(uint16_t addr, uint16_t data)
{
	m_stream->update();
	m_regs[addr] = data;
}

uint16_t kn_tonegen_base_device::tg_read(uint16_t)
{
	return 0;
}

void kn_tonegen_base_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, 0.0);
	stream.fill(1, 0.0);
}

// The chip's window. The key bed FIFO read at +4 and the serial port at +c and
// +e are mapped by the driver.
void kn_tonegen_base_device::map(address_map &map)
{
	map(0x0, 0x1).w(FUNC(kn_tonegen_base_device::addr_w));
	map(0x2, 0x3).rw(FUNC(kn_tonegen_base_device::data_r), FUNC(kn_tonegen_base_device::data_w));
	map(0x4, 0x7).nopw();     // +4: initialisation writes, not decoded; +6 below
	map(0x6, 0x7).w(FUNC(kn_tonegen_base_device::wave_bank_w));
	map(0x8, 0x9).w(FUNC(kn_tonegen_base_device::wave_addr_w));
	map(0xa, 0xb).r(FUNC(kn_tonegen_base_device::wave_data_r));
	map(0x10, 0x13).nopw();
}

// The chip takes a register address, then its data
void kn_tonegen_base_device::addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_addr);
}

uint16_t kn_tonegen_base_device::data_r()
{
	return tg_read(m_addr);
}

void kn_tonegen_base_device::data_w(uint16_t data)
{
	tg_write(m_addr, data);
}

// The service mode's wave ROM test latches a bank at +6 and a word address at
// +8, then reads the sample word at +a; bit 0 of the address picks the X bus
void kn_tonegen_base_device::wave_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wave_bank);
}

void kn_tonegen_base_device::wave_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wave_addr);
}

uint16_t kn_tonegen_base_device::wave_data_r()
{
	const uint32_t bank = m_wave_bank & 0x7fff;     // bit 15 is an enable
	const uint32_t word = (m_wave_addr >> 1) & 0x3fff;
	return m_wave[BIT(m_wave_addr, 0)].read_word(bank * 0x4000 + word);
}

kn6000_tonegen_device::kn6000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN6000_TONEGEN, tag, owner, clock, 0)
{
}

kn2400_tonegen_device::kn2400_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN2400_TONEGEN, tag, owner, clock, 0)
{
}


//**************************************************************************
//  KN7000
//**************************************************************************

// Register address: plane in bits 15-10, channel (the voice) in bits 9-4,
// index in bits 3-0.
//
// Plane 0x00, indexes 0-2: the amplitude envelope, three [rate | level] pairs
// (attack to the peak, first decay to a level, second decay to a level). A
// higher rate byte is faster. An index 0 write with bit 7 set releases the
// voice at its rate, as does 0xc000 written to index 1.
// FIXME: release by 0xc000 in index 1 is inferred from the firmware's writes
// Plane 0x07: the sample select. The index is bits 19-16, the data bits 15-0;
// bits 19-12 are the wave ROM page, bits 11-0 the entry in its directory.
// Plane 0x09, indexes 0-1: pitch. Bit 16 is the index, bits 15-0 the data;
// 0x400 per semitone, 0x1e200 is C7 for a sample of exponent 9. A pitch write
// keys a voice that is not keyed.
// FIXME: the octave of 0x1e200 is inferred; the pitch class is checked against
// the firmware's notes
// FIXME: key-on by a pitch write is inferred from the firmware's writes, and
// would restart a releasing voice on a pitch bend
// Plane 0x3f: status. 0xfc08 + g reads which of voices 16g-16g+15 still sound;
// 0xfc02 | ch << 4 reads the envelope level of channel ch, 13 bits.
//
// The chip reads its pair of wave ROMs as a stream of 16-bit words: word 2n is
// word n of the Y ROM, word 2n+1 word n of the X ROM. The stream holds 16 pages
// of 0x100000 words, each starting with a directory of four words per entry: A
// (the entry's segment record, in words from the page start), P (0x1000 |
// reverse flag << 9 | exponent << 4 | 8-bit flag << 3 | K), B (the first sample
// word, in units of 16 words from the page start) and a copy of the record's
// second word. Entry 0's record follows the directory, so its A is four times
// the entry count. Words 1 and up of the record are [flags:2][code:6][value:8],
// each a run of granules of 2^(K+4) samples: 256 - value of them for a value of
// 0x80 or more, one for any other value. Flag bit 0 puts the loop start at the
// end of the run, bit 1 the loop end; both flags end the record. A loop end
// ends the record too. A loop end with no loop start loops from the first
// sample. Without a loop end the loop is the record's last granule. 8-bit
// samples are scaled by 2^(-b/2) for b in bits 2-0 of their run's code.
// FIXME: what the chip does after a record's last granule, and the gain law,
// are inferred
//
// FIXME: the reverse flag is inferred from the one entry that sets it, the
// Reverse Cymbal: its B is minus the last unit of the ride cymbal's recording,
// so the chip is taken to negate its address counter, and unit -(B + n) holds
// samples 16n to 16n + 15 (32n to 32n + 31 when 8-bit), read last first.

kn7000_tonegen_device::kn7000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN7000_TONEGEN, tag, owner, clock, 2)
{
}

void kn7000_tonegen_device::device_start()
{
	kn_tonegen_base_device::device_start();

	parse_samples();

	// FIXME: the rate byte's law is fitted to the firmware's envelopes, not measured
	const double rate = m_stream->sample_rate();
	for (uint32_t r = 0; r < std::size(m_attack_time); r++)
	{
		const double tau = 13.0 * std::exp2(-double(r) / 20.0);
		m_attack_time[r] = tau * rate;
		m_decay_coef[r] = std::exp(-1.0 / (tau * rate));
	}

	std::fill(std::begin(m_keyed), std::end(m_keyed), false);
	std::fill(std::begin(m_stage), std::end(m_stage), EG_RELEASE);
	std::fill(std::begin(m_release_rate), std::end(m_release_rate), 0);
	std::fill(std::begin(m_select), std::end(m_select), 0);
	std::fill(std::begin(m_pitch), std::end(m_pitch), 0);
	std::fill(std::begin(m_sample), std::end(m_sample), -1);
	std::fill(std::begin(m_env), std::end(m_env), 0.0);
	std::fill(std::begin(m_pos), std::end(m_pos), 0.0);
	std::fill(std::begin(m_step), std::end(m_step), 0.0);
	std::fill(std::begin(m_seg), std::end(m_seg), 0);

	save_item(NAME(m_keyed));
	save_item(NAME(m_stage));
	save_item(NAME(m_release_rate));
	save_item(NAME(m_select));
	save_item(NAME(m_pitch));
	save_item(NAME(m_sample));
	save_item(NAME(m_env));
	save_item(NAME(m_pos));
	save_item(NAME(m_step));
	save_item(NAME(m_seg));
}

void kn7000_tonegen_device::device_reset()
{
	kn_tonegen_base_device::device_reset();

	// IORST goes to both chips; what it clears beyond the voices is unknown
	std::fill(std::begin(m_keyed), std::end(m_keyed), false);
	std::fill(std::begin(m_stage), std::end(m_stage), EG_RELEASE);
	std::fill(std::begin(m_env), std::end(m_env), 0.0);
}

void kn7000_tonegen_device::parse_samples()
{
	m_samples.clear();
	for (int page = 0; page < PAGES; page++)
	{
		const uint32_t base = page << 20;
		const uint32_t count = wave_word(base) / 4;
		m_page_first[page] = m_samples.size();
		m_page_count[page] = count;
		for (uint32_t j = 0; j < count; j++)
		{
			const uint32_t a = wave_word(base + 4 * j);
			const uint16_t p = wave_word(base + 4 * j + 1);
			const uint32_t b = wave_word(base + 4 * j + 2);
			const uint32_t granule = 1 << (BIT(p, 0, 3) + 4);

			sample_info s;
			s.reverse = BIT(p, 9);
			// reversed: one past the last word of unit -B
			s.start = base + (s.reverse ? (((0x10000 - b) & 0xffff) + 1) : b) * 16;
			s.is8 = BIT(p, 3);
			s.exponent = BIT(p, 4, 5);

			uint32_t pos = 0;
			bool have_start = false, have_end = false;
			float gain = 1.0f;
			for (int k = 1; k < 1024; k++)
			{
				const uint16_t w = wave_word(base + a + k);
				const uint8_t value = BIT(w, 0, 8);
				pos += (value >= 0x80 ? (0x100 - value) : 1) * granule;
				gain = std::exp2(-0.5f * BIT(w, 8, 3));
				if (s.is8)
					s.gain.emplace_back(pos, gain);
				if (BIT(w, 14, 2) == 3)
					break;
				if (BIT(w, 14) && !have_start)
				{
					s.loop_start = pos;
					have_start = true;
				}
				if (BIT(w, 15))
				{
					s.loop_end = pos;
					have_end = true;
					break;
				}
			}
			if (!have_end)
			{
				s.loop_end = std::max(pos, granule);
				s.loop_start = s.loop_end - granule;
			}
			else if (!have_start)
				s.loop_start = 0;
			if (s.is8)
			{
				s.gain.emplace_back(~uint32_t(0), gain);
				s.loop_seg = std::ranges::upper_bound(s.gain, s.loop_start, {}, &std::pair<uint32_t, float>::first) - s.gain.begin();
			}

			const uint32_t needed = s.is8 ? ((s.loop_end + 1) / 2) : s.loop_end;
			s.valid = s.reverse ? (s.start >= base + needed + 1) : (uint64_t(s.start) + needed + 1 < STREAM_WORDS);
			m_samples.emplace_back(std::move(s));
		}
	}
}

// seg is the gain segment that holds sample i of an 8-bit sample
double kn7000_tonegen_device::fetch(const sample_info &s, uint32_t i, uint32_t seg)
{
	if (!s.is8)
		return int16_t(wave_word(s.reverse ? (s.start - 1 - i) : (s.start + i)));
	const uint32_t byte = s.reverse ? (2 * s.start - 1 - i) : (2 * s.start + i);
	const uint16_t w = wave_word(byte >> 1);
	const int8_t b = int8_t(BIT(byte, 0) ? (w >> 8) : w);
	return b * 256.0 * s.gain[seg].second;
}

// Source samples per output sample
double kn7000_tonegen_device::step(int v) const
{
	if (m_sample[v] < 0)
		return 0.0;
	const double c7 = 440.0 * std::exp2(27.0 / 12.0);
	const sample_info &s = m_samples[m_sample[v]];
	return std::exp2((double(m_pitch[v]) - 0x1e200) / 0x3000 + s.exponent - 9) * c7 / m_stream->sample_rate();
}

void kn7000_tonegen_device::key_on(int v)
{
	const uint32_t page = BIT(m_select[v], 12, 8);
	const uint32_t entry = BIT(m_select[v], 0, 12);
	m_sample[v] = -1;
	if (page < PAGES && entry < m_page_count[page])
	{
		const uint32_t index = m_page_first[page] + entry;
		if (m_samples[index].valid)
			m_sample[v] = index;
	}
	m_pos[v] = 0.0;
	m_seg[v] = 0;
	m_step[v] = step(v);
	m_env[v] = 0.0;
	m_stage[v] = EG_ATTACK;
	m_keyed[v] = true;
}

void kn7000_tonegen_device::release(int v, uint8_t rate)
{
	m_keyed[v] = false;
	m_stage[v] = EG_RELEASE;
	m_release_rate[v] = rate;
}

uint16_t kn7000_tonegen_device::level(int v) const
{
	return std::clamp<long>(std::lround(m_env[v] * 0x1fff), 0, 0x1fff);
}

void kn7000_tonegen_device::tg_write(uint16_t addr, uint16_t data)
{
	kn_tonegen_base_device::tg_write(addr, data);

	const int v = BIT(addr, 4, 6);
	const uint32_t index = BIT(addr, 0, 4);
	switch (BIT(addr, 10, 6))
	{
	case 0x00:
		if ((index == 0 && BIT(data, 7)) || (index == 1 && data == 0xc000))
			release(v, data >> 8);
		break;

	case 0x07:
		m_select[v] = (index << 16) | data;
		break;

	case 0x09:
		if (index < 2)
		{
			m_pitch[v] = (index << 16) | data;
			if (!m_keyed[v])
				key_on(v);
			else
				m_step[v] = step(v);
		}
		break;
	}
}

uint16_t kn7000_tonegen_device::tg_read(uint16_t addr)
{
	if (BIT(addr, 10, 6) != 0x3f)
		return 0;

	m_stream->update();
	const int ch = BIT(addr, 4, 6);
	const uint32_t index = BIT(addr, 0, 4);
	if (ch == 0 && index >= 0x8 && index <= 0xb)
	{
		uint16_t bits = 0;
		for (int i = 0; i < 16; i++)
		{
			const int v = ((index - 0x8) << 4) | i;
			if (m_stage[v] == EG_ATTACK || level(v))
				bits |= 1 << i;
		}
		return bits;
	}
	if (index == 0x2)
		return level(ch);
	return 0;
}

void kn7000_tonegen_device::sound_stream_update(sound_stream &stream)
{
	for (int n = 0; n < stream.samples(); n++)
	{
		double acc = 0.0;
		for (int v = 0; v < VOICES; v++)
		{
			const uint16_t eg = v << 4;
			double &env = m_env[v];
			switch (m_stage[v])
			{
			case EG_ATTACK:
			{
				const uint16_t r0 = reg(eg);
				const double peak = BIT(r0, 0, 7) / 127.0;
				env += peak / m_attack_time[r0 >> 8];
				if (env >= peak)
				{
					env = peak;
					m_stage[v] = EG_DECAY1;
				}
				break;
			}

			case EG_DECAY1:
			{
				const uint16_t r1 = reg(eg + 1);
				const double target = BIT(r1, 0, 7) / 127.0;
				env = target + (env - target) * m_decay_coef[r1 >> 8];
				if (std::abs(env - target) < 1.0 / 1024)
				{
					env = target;
					m_stage[v] = EG_DECAY2;
				}
				break;
			}

			case EG_DECAY2:
			{
				const uint16_t r2 = reg(eg + 2);
				const double target = BIT(r2, 0, 7) / 127.0;
				env = target + (env - target) * m_decay_coef[r2 >> 8];
				break;
			}

			case EG_RELEASE:
				env *= m_decay_coef[m_release_rate[v]];
				if (env < 0.0005)
					env = 0.0;
				break;
			}

			if (env <= 0.0 || m_sample[v] < 0)
				continue;

			// linear interpolation through the loop
			const sample_info &s = m_samples[m_sample[v]];
			const uint32_t i0 = uint32_t(m_pos[v]);
			const uint32_t i1 = (i0 + 1 < s.loop_end) ? (i0 + 1) : s.loop_start;
			uint32_t seg0 = 0, seg1 = 0;
			if (s.is8)
			{
				// the position only moves forward between loop wraps
				while (i0 >= s.gain[m_seg[v]].first)
					m_seg[v]++;
				seg0 = m_seg[v];
				seg1 = (i1 == i0 + 1) ? seg0 : s.loop_seg;
				while (i1 >= s.gain[seg1].first)
					seg1++;
			}
			const double frac = m_pos[v] - i0;
			const double smp = fetch(s, i0, seg0) * (1.0 - frac) + fetch(s, i1, seg1) * frac;
			acc += smp / 32768.0 * env;

			m_pos[v] += m_step[v];
			if (m_pos[v] >= s.loop_end)
			{
				m_pos[v] = s.loop_start + std::fmod(m_pos[v] - s.loop_start, double(s.loop_end - s.loop_start));
				m_seg[v] = s.loop_seg;
			}
		}
		stream.put(0, n, acc / 8 + stream.get(0, n));
		stream.put(1, n, acc / 8 + stream.get(1, n));
	}
}
